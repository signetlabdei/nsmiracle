/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This software is based on the NIST NS2 WiMAX module, which can be found at
 * http://www.antd.nist.gov/seamlessandsecure/doc.html
 *
 * $Id: mac802_16BS.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "mac802_16BS.h"
#include "ns2wimax/scheduling/bsscheduler.h"
#include "ns2wimax/destclassifier.h"

/**
 * TCL Hooks for the simulator for wimax mac
 */
static class Mac802_16BSClass : public TclClass {
public:
  Mac802_16BSClass() : TclClass("Module/802_16/MAC/BS") {}
  TclObject* create(int, const char*const*) {
    return (new Mac802_16BS());
    
  }
} class_mac802_16BS;

/**
 * Creates a Mac 802.16
 */
Mac802_16BS::Mac802_16BS() : Mac802_16 (), cl_head_(0), cl_tail_(0), ctrlagent_(0)
{
  type_ = STA_BS; //type of MAC. In this case it is for BS
  
  //Default configuration
  addClassifier (new DestClassifier ());
  
  Tcl& tcl = Tcl::instance();
  tcl.evalf ("new WimaxScheduler/BS");
  scheduler_ = (WimaxScheduler*) TclObject::lookup(tcl.result());
  scheduler_->setMac (this); //register the mac

  init_default_connections ();

  LIST_INIT (&t17_head_);
  LIST_INIT (&scan_stations_);
  LIST_INIT (&fast_ranging_head_);
  bw_peer_ = NULL;
  bw_node_index_ = 0;
  sendDCD = false;
  dlccc_ = 0;
  sendUCD = false;
  ulccc_ = 0;

  map_ = new FrameMap (this);

  //create timers for DL/UL boundaries
  dl_timer_ = new DlTimer (this);
  ul_timer_ = new UlTimer (this);

}

/*
 * Interface with the TCL script
 * @param argc The number of parameter
 * @param argv The list of parameters
 */
int Mac802_16BS::command(int argc, const char*const* argv)
{
  return Mac802_16::command(argc, argv);
}

/**
 * Initialize default connections
 */
void Mac802_16BS::init_default_connections ()
{
  Connection * con;

  //create initial ranging and padding connection
  con = new Connection (CONN_INIT_RANGING);
  connectionManager_->add_connection (con, IN_CONNECTION); //uplink
  con = new Connection (CONN_INIT_RANGING);
  connectionManager_->add_connection (con, OUT_CONNECTION); //downlink
  con = new Connection (CONN_PADDING);
  connectionManager_->add_connection (con, IN_CONNECTION);
  con = new Connection (CONN_PADDING);
  connectionManager_->add_connection (con, OUT_CONNECTION);

  //we need to create a Broadcast connection and AAS init ranging CIDs
  con = new Connection (CONN_BROADCAST);
  connectionManager_->add_connection (con, OUT_CONNECTION);
  con = new Connection (CONN_AAS_INIT_RANGING);
  connectionManager_->add_connection (con, IN_CONNECTION);
}

/**
 * Initialize the MAC
 */
void Mac802_16BS::init ()
{
  //init the scheduler
  scheduler_->init();

  //schedule the first frame by using a random backoff to avoid
  //synchronization between BSs.
  
  double stime = getFrameDuration () + Random::uniform(0, getFrameDuration ());
  dl_timer_->sched (stime);

  //also start the DCD and UCD timer
  dcdtimer_ = new WimaxDCDTimer (this);
  ucdtimer_ = new WimaxUCDTimer (this);
  nbradvtimer_ = new WimaxMobNbrAdvTimer (this);
  dcdtimer_->start (macmib_.dcd_interval);
  ucdtimer_->start (macmib_.ucd_interval);
  nbradvtimer_->start (macmib_.nbr_adv_interval+stime);
}

/**
 * Set the control agent
 */
void Mac802_16BS::setCtrlAgent (WimaxCtrlAgent *agent)
{
  assert (agent);
  ctrlagent_ = agent;
}

/**** Packet processing methods ****/

/*
 * Process packets going out
 * @param p The packet to send out
 */
void Mac802_16BS::tx(Packet *p)
{
  //We first send it through the CS
  int cid = -1;

  if (!notify_upper_) {
    assert (!pktBuf_);
    pktBuf_ = p;
    return;
  } 

  cid = classify (p);

  if (cid == -1) {
    debug ("At %f in Mac %d drop packet because no classification were found\n", \
	    NOW, addr());
    drop(p, 1, "CID");
    //Packet::free (p);
  } else {
    //enqueue the packet 
    Connection *connection = connectionManager_->get_connection (cid, OUT_CONNECTION);
    if (connection == NULL) {
      debug ("Warning: At %f in Mac %d connection with cid = %d does not exist. Please check classifiers\n",\
	      NOW, addr(), cid);
      //Packet::free (p);
      update_watch (&loss_watch_, 1);
      drop(p, 1, "CID");
    }
    else {

      if (connection->queueLength ()==macmib_.queue_length) {
	//queue full 
	update_watch (&loss_watch_, 1);
	drop (p, 1, "QWI");
      } else {
	//update mac header information
	//set header information
	hdr_mac802_16 *wimaxHdr = HDR_MAC802_16(p);
	wimaxHdr->header.ht = 0;
	wimaxHdr->header.ec = 1;
        wimaxHdr->header.type_mesh = 0;
        wimaxHdr->header.type_arqfb = 0;
        wimaxHdr->header.type_ext = 0;
        wimaxHdr->header.type_frag = 0;
        wimaxHdr->header.type_pck = 0;
        wimaxHdr->header.type_fbgm = 0;
	wimaxHdr->header.ci = 0;
	wimaxHdr->header.eks = 0;
	wimaxHdr->header.cid = cid; //default
	wimaxHdr->header.hcs = 0;
	HDR_CMN(p)->size() += HDR_MAC802_16_SIZE;
	connection ->enqueue (p);
	//printf ("At %f in Mac %d Enqueue packet to cid=%d queue size=%d(max=%d)\n", NOW, index_, cid,connection->queueLength (), macmib_.queue_length);
      }
    }
  }
}

/*
 * Transmit a packet to the physical layer
 * @param p The packet to send out
 */
void Mac802_16BS::transmit(Packet *p)
{
  if (NOW < last_tx_time_+last_tx_duration_) {
    //still sending
    printf ("At %f in Mac %d is already transmitting (endTX=%.9f). Drop packet.\n", NOW, addr(),last_tx_time_+last_tx_duration_);
    Packet::free (p);
    return;
  }

  struct hdr_cmn *ch = HDR_CMN(p);
  
  /*
  debug ("At %f in Mac %d sending packet (type=%s, size=%d, txtime=%.9f) ", NOW, index_, packet_info.name(ch->ptype()), ch->size(), ch->txtime());
  if (ch->ptype()==PT_MAC) {
    if (HDR_MAC802_16(p)->header.ht == 0)
      debug ("mngt=%d\n", ((mac802_16_dl_map_frame*) p->accessdata())->type);
    else
      debug ("bwreq\n");
  } else {
    debug ("\n");
  }
  */

  //update stats for delay and jitter
  double delay = NOW-ch->timestamp();
  update_watch (&delay_watch_, delay);
  double jitter = fabs (delay - last_tx_delay_);
  update_watch (&jitter_watch_, jitter);
  last_tx_delay_ = delay;
  if (ch->ptype()!=PT_MAC) {
    update_throughput (&tx_data_watch_, 8*ch->size());
  } 
  update_throughput (&tx_traffic_watch_, 8*ch->size());
  
  last_tx_time_ = NOW;
  last_tx_duration_ = ch->txtime();
  //pass it down
  sendDown(p,0);

  // Tell queue we want more
  resume();
}

/*
 * Process incoming packets
 * @param p The incoming packet
 */
void Mac802_16BS::rx (Packet *p)
{
  struct hdr_cmn *ch = HDR_CMN(p);

#ifdef DEBUG_WIMAX
  debug ("At %f in Mac %d receive first bit..over at %f(txtime=%f) (type=%s) ", NOW, addr(), NOW+ch->txtime(),ch->txtime(), packet_info.name(ch->ptype()));
  if (ch->ptype()==PT_MAC) {
    if (HDR_MAC802_16(p)->header.ht == 0)
      debug ("mngt=%d\n", ((mac802_16_dl_map_frame*) p->accessdata())->type);
    else
      debug ("bwreq\n");
  } else {
    debug ("\n");
  }
#endif

  if (pktRx_ !=NULL) {
    /*
     *  If the power of the incoming packet is smaller than the
     *  power of the packet currently being received by at least
     *  the capture threshold, then we ignore the new packet.
     */
    if(pktRx_->txinfo_.RxPr / p->txinfo_.RxPr >= p->txinfo_.CPThresh) {
      Packet::free(p);
    } else {
      /*
       *  Since a collision has occurred, figure out
       *  which packet that caused the collision will
       *  "last" the longest.  Make this packet,
       *  pktRx_ and reset the Recv Timer if necessary.
       */
      if(txtime(p) > rxTimer_.expire()) {
	rxTimer_.stop();
	//printf ("\t drop pktRx..collision\n");
	drop(pktRx_, 1, "COL");
	update_watch (&loss_watch_, 1);
	pktRx_ = p;
	//mark the packet with error
	ch->error() = 1;
	collision_ = true;

	rxTimer_.start(ch->txtime()-0.000000001);
      }
      else {
	//printf ("\t drop new packet..collision\n");
	drop(p, 1, "COL");
	//mark the packet with error
	HDR_CMN(pktRx_)->error() = 1;
	collision_ = true;
      }
    }
    return;
  }
  assert (pktRx_==NULL);
  assert (rxTimer_.busy()==0);
  pktRx_ = p;
  //create a timer to wait for the end of reception
  //since the packets are received by burst, the beginning of the new packet 
  //is the same time as the end of this packet..we process this packet 1ns 
  //earlier to make room for the new packet.
  rxTimer_.start(ch->txtime()-0.000000001);
}

/**
 * Process the fully received packet
 */
void Mac802_16BS::receive ()
{
  assert (pktRx_);
  struct hdr_cmn *ch = HDR_CMN(pktRx_);

#ifdef DEBUG_WIMAX
  printf ("At %f in Mac %d packet received (type=%s) ", NOW, addr(), packet_info.name(ch->ptype()));
  if (ch->ptype()==PT_MAC) {
    if (HDR_MAC802_16(pktRx_)->header.ht == 0)
      printf ("mngt=%d\n", ((mac802_16_dl_map_frame*) pktRx_->accessdata())->type);
    else
      printf ("bwreq\n");
  } else {
    printf ("\n");
  }
#endif
    

  //drop the packet if corrupted
  if (ch->error()) {
    if (collision_) {
      //printf ("\t drop new pktRx..collision\n");
      drop (pktRx_, 1, "COL");
      collision_ = false;
    } else {
      //error in the packet, the Mac does not process
      Packet::free(pktRx_);
    }
    //update drop stat
    update_watch (&loss_watch_, 1);
    pktRx_ = NULL;
    return;
  }

  //process packet
  hdr_mac802_16 *wimaxHdr = HDR_MAC802_16(pktRx_);
  gen_mac_header_t header = wimaxHdr->header;
  int cid = header.cid;
  Connection *con = connectionManager_->get_connection (cid, IN_CONNECTION);

  if (con == NULL) {
    //This packet is not for us
    //printf ("At %f in Mac %d Connection null\n", NOW, index_);
    update_watch (&loss_watch_, 1);
    Packet::free(pktRx_);
    pktRx_=NULL;
    return;
  }
  //printf ("CID=%d\n", cid);

  //update rx time of last packet received
  PeerNode *peer;
  if (type_ == STA_MN)
    peer = getPeerNode_head(); //MN only has one peer
  else
    peer = con->getPeerNode(); //BS can have multiple peers

  if (peer) {
    peer->setRxTime (NOW);
    
    //collect receive signal strength stats
    peer->getStatWatch()->update(10*log10(pktRx_->txinfo_.RxPr*1e3));
    //debug ("At %f in Mac %d weighted RXThresh: %e rxp average %e\n", NOW, index_, macmib_.lgd_factor_*macmib_.RXThreshold_, pow(10,peer->getStatWatch()->average()/10)/1e3);
    double avg_w = pow(10,(peer->getStatWatch()->average()/10))/1e3;
    
    if ( avg_w < (macmib_.lgd_factor_*macmib_.RXThreshold_)) {
#ifdef USE_802_21
      double probability = ((macmib_.lgd_factor_*macmib_.RXThreshold_)-avg_w)/((macmib_.lgd_factor_*macmib_.RXThreshold_)-macmib_.RXThreshold_);
      Mac::send_link_going_down (peer->getAddr(), addr(), -1, (int)(100*probability), LGD_RC_LINK_PARAM_DEGRADING, eventId_++);
#endif
      if (peer->getPrimary(IN_CONNECTION)!=NULL) { //check if node registered
	peer->setGoingDown (true);      
      }
    }
    else {
      if (peer->isGoingDown()) {
#ifdef USE_802_21
	Mac::send_link_rollback (addr(), getPeerNode_head()->getAddr(), eventId_-1);
#endif
	peer->setGoingDown (false);
      }
    }
  }
  
  //process reassembly
  if (wimaxHdr->header.type_frag) {
    bool drop_pkt = true;
    bool frag_error = false;
    //printf ("Frag type = %d\n",wimaxHdr->fc & 0x3);
    switch (wimaxHdr->frag_subheader.fc & 0x3) {
    case FRAG_NOFRAG: 
      if (con->getFragmentationStatus()!=FRAG_NOFRAG)
	con->updateFragmentation (FRAG_NOFRAG, 0, 0); //reset
      drop_pkt = false;
      break; 
    case FRAG_FIRST: 
      //when it is the first fragment, it does not matter if we previously
      //received other fragments, since we reset the information
      assert (wimaxHdr->frag_subheader.fsn == 0);
      //printf ("\tReceived first fragment\n");
      con->updateFragmentation (FRAG_FIRST, 0, ch->size()-(HDR_MAC802_16_SIZE+HDR_MAC802_16_FRAGSUB_SIZE));
      break; 
    case FRAG_CONT: 
      if ( (con->getFragmentationStatus()!=FRAG_FIRST
	    && con->getFragmentationStatus()!=FRAG_CONT)
	   || ((wimaxHdr->frag_subheader.fsn&0x7) != (con->getFragmentNumber ()+1)%8) ) {
	frag_error = true;
	con->updateFragmentation (FRAG_NOFRAG, 0, 0); //reset
      } else {
	//printf ("\tReceived cont fragment\n");
	con->updateFragmentation (FRAG_CONT, wimaxHdr->frag_subheader.fsn&0x7, con->getFragmentBytes()+ch->size()-(HDR_MAC802_16_SIZE+HDR_MAC802_16_FRAGSUB_SIZE));	
      }
      break; 
    case FRAG_LAST: 
      if ( (con->getFragmentationStatus()==FRAG_FIRST
	    || con->getFragmentationStatus()==FRAG_CONT)
	   && ((wimaxHdr->frag_subheader.fsn&0x7) == (con->getFragmentNumber ()+1)%8) ) {
	//printf ("\tReceived last fragment\n");
	ch->size() += con->getFragmentBytes()-HDR_MAC802_16_FRAGSUB_SIZE;
	drop_pkt = false;
      } else {
	//printf ("Error with last frag seq=%d (expected=%d)\n", wimaxHdr->fsn&0x7, (con->getFragmentNumber ()+1)%8);
	frag_error = true;
      }     
      con->updateFragmentation (FRAG_NOFRAG, 0, 0); //reset
      break; 
    default:
      fprintf (stderr,"Error, unknown fragmentation type\n");
      exit (-1);
    }
    //if we got an error, or it is a fragment that is not the last, free the packet
    if (drop_pkt) {
      if (frag_error) {
	//update drop stat
	update_watch (&loss_watch_, 1);
	drop (pktRx_, 1, "FRG"); //fragmentation error
      } else {
	//silently discard this fragment.
	Packet::free(pktRx_);
      }
      pktRx_=NULL;
      return;
    } 
  }

  //check if this is a bandwidth request
  if (header.ht == 0 && con->getBw() >0) {
    debug2 ("At %f Connection %d, old=%d, rcv=%d, left=%d\n", NOW, con->get_cid(), con->getBw(), ch->size(), con->getBw()-ch->size());
    con->setBw(con->getBw()-ch->size());
  }

  //We check if it is a MAC packet or not
  if (HDR_CMN(pktRx_)->ptype()==PT_MAC) {
    process_mac_packet (con, pktRx_);
    update_throughput (&rx_traffic_watch_, 8*ch->size());
    Packet::free(pktRx_);
  }
  else { 
    update_throughput (&rx_data_watch_, 8*ch->size());    
    update_throughput (&rx_traffic_watch_, 8*ch->size());
    ch->size() -= HDR_MAC802_16_SIZE;
    sendUp(pktRx_, 0);
  }

  update_watch (&loss_watch_, 0);
  pktRx_=NULL;
}

/**
 * Process a packet received by the Mac. Only scheduling related packets should be sent here (BW request, UL_MAP...)
 * @param con The connection by which it arrived
 * @param p The packet to process
 */
void Mac802_16BS::process_mac_packet (Connection *con, Packet * p)
{
  //debug2 ("Mac802_16BS received packet to process\n");

  assert (HDR_CMN(p)->ptype()==PT_MAC);
  debug2 ("Mac802_16BS received packet to process\n");
  
  hdr_mac802_16 *wimaxHdr = HDR_MAC802_16(p);
  gen_mac_header_t header = wimaxHdr->header;

  //check if this is a bandwidth request
  if (header.ht == 1) {
    process_bw_req (p);
    return;
  } 


  //we cast to this frame because all management frame start with
  //a type 
  mac802_16_dl_map_frame *frame = (mac802_16_dl_map_frame*) p->accessdata();
  
  switch (frame->type) {
  case MAC_RNG_REQ: 
    process_ranging_req (p);
    break;
  case MAC_REG_REQ:     
    process_reg_req (p);
    break;
  case MAC_MOB_SCN_REQ:
    if (ctrlagent_) 
      ctrlagent_->process_scan_request (p);
    else
      fprintf (stderr, "Warning: no controler to handle scan request in BS %d\n", addr());
    break;
  case MAC_MOB_MSHO_REQ:
    process_msho_req (p);
    break;
  case MAC_MOB_HO_IND:
    process_ho_ind (p);
    break;
  case MAC_DSA_REQ: 
  case MAC_DSA_RSP: 
  case MAC_DSA_ACK: 
    serviceFlowHandler_->process (pktRx_);
    break;
  default:
    debug ("unknown packet in BS\n");
  }

  //Packet::free (p);
}

/**
 * Process a RNG-REQ message
 * @param p The packet containing the ranging request information
 */
void Mac802_16BS::process_ranging_req (Packet *p)
{
  UlSubFrame *ulsubframe = getMap()->getUlSubframe();
  mac802_16_rng_req_frame *req = (mac802_16_rng_req_frame *) p->accessdata();

  if (HDR_MAC802_16(p)->header.cid != INITIAL_RANGING_CID) {
    //process request for DIUC
  } else {
    //here we can make decision to accept the SS or not.
    //for now, accept everybody
    //check if CID already assigned for the SS
    PeerNode *peer = getPeerNode (req->ss_mac_address);
    if (peer==NULL) {
      debug ("New peer node requesting ranging (%d)\n",req->ss_mac_address);
      //Assign Management CIDs
      Connection *basic = new Connection (CONN_BASIC);
      Connection *upbasic = new Connection (CONN_BASIC, basic->get_cid());
      Connection *primary = new Connection (CONN_PRIMARY);
      Connection *upprimary = new Connection (CONN_PRIMARY, primary->get_cid());
      
      //Create Peer information
      peer = new PeerNode (req->ss_mac_address);
      peer->setBasic (upbasic, basic);
      peer->setPrimary (upprimary, primary);
      addPeerNode (peer);
      getCManager()->add_connection (upbasic, IN_CONNECTION);
      getCManager()->add_connection (basic, OUT_CONNECTION);
      getCManager()->add_connection (upprimary, IN_CONNECTION);
      getCManager()->add_connection (primary, OUT_CONNECTION);
      peer->setDIUC (req->req_dl_burst_profile);
      //schedule timer in case the node never register
      addtimer17 (req->ss_mac_address);

      //create packet for answers
      Packet *rep = getPacket ();
      struct hdr_cmn *ch = HDR_CMN(rep);
      rep->allocdata (sizeof (struct mac802_16_rng_rsp_frame));
      mac802_16_rng_rsp_frame *frame = (mac802_16_rng_rsp_frame*) rep->accessdata();
      frame->type = MAC_RNG_RSP;
      frame->uc_id = ulsubframe->getChannelID();
      frame->rng_status = RNG_SUCCESS;
      frame->ss_mac_address = req->ss_mac_address;
      frame->basic_cid = basic->get_cid();
      frame->primary_cid = primary->get_cid();
      ch->size() = RNG_RSP_SIZE;

      //enqueue packet
      getCManager()->get_connection (BROADCAST_CID, OUT_CONNECTION)->enqueue (rep);

      if (cl_head_==NULL) {
	cl_head_ = (new_client_t*)malloc (sizeof (new_client_t));
	cl_tail_ = cl_head_;
      } else {
	cl_tail_->next = (new_client_t*)malloc (sizeof (new_client_t));
	cl_tail_=cl_tail_->next;
      }
      cl_tail_->cid = primary->get_cid();
      cl_tail_->next = NULL;

#ifdef USE_802_21
      send_link_detected (addr(), peer->getAddr(), 1);
#endif

    } else {
      debug ("Received ranging for known station (%d)\n", req->ss_mac_address);
      //reset invited ranging retries for SS
      //create packet for answers
      Connection *basic = peer->getBasic(IN_CONNECTION);
      Connection *primary = peer->getPrimary(IN_CONNECTION);
      Packet *rep = getPacket ();
      struct hdr_cmn *ch = HDR_CMN(rep);
      rep->allocdata (sizeof (struct mac802_16_rng_rsp_frame));
      mac802_16_rng_rsp_frame *frame = (mac802_16_rng_rsp_frame*) rep->accessdata();
      frame->type = MAC_RNG_RSP;
      frame->uc_id = ulsubframe->getChannelID();
      frame->rng_status = RNG_SUCCESS;
      frame->ss_mac_address = req->ss_mac_address;
      frame->basic_cid = basic->get_cid();
      frame->primary_cid = primary->get_cid();
      ch->size() = RNG_RSP_SIZE;
      //enqueue packet
      getCManager()->get_connection (BROADCAST_CID, OUT_CONNECTION)->enqueue (rep);
    }
  }
}

/**
 * Process bandwidth request
 * @param p The request
 */
void Mac802_16BS::process_bw_req (Packet *p)
{ 
  hdr_mac802_16 *wimaxHdr = HDR_MAC802_16(p);
  gen_mac_header_t header = wimaxHdr->header;

  bw_req_header_t *req;
  req = (bw_req_header_t *)&header;

  debug ("received bandwidth request of %d bytes from %d\n", req->br, req->cid); 
  
  //retrieve the CID and update bandwidth request information
  Connection *c =  getCManager()->get_connection (req->cid, IN_CONNECTION);
  assert (c);
  if ( (req->type & 0x7) == 0x001) {
    //aggregate
    c->setBw (req->br & 0x7FFFF); //masks 19 bits
    debug2 ("Aggregate request on connection %d of %d\n", c->get_cid(), c->getBw());
  } else if ( (req->type & 0x7) == 0x000) {
    //incremental
    c->setBw (c->getBw() + (req->br & 0x7FFFF));
  } 
}

/**
 * Process registration request
 * @param p The request
 */
void Mac802_16BS::process_reg_req (Packet *req)
{ 
  hdr_mac802_16 *wimaxHdr_req = HDR_MAC802_16(req);
  gen_mac_header_t header_req = wimaxHdr_req->header;
  
  debug ("received registration request from %d\n", header_req.cid);

  Packet *p;
  struct hdr_cmn *ch;
  hdr_mac802_16 *wimaxHdr;
  mac802_16_reg_rsp_frame *reg_frame;
  PeerNode *peer;

  //create packet for request
  p = getPacket ();
  ch = HDR_CMN(p);
  wimaxHdr = HDR_MAC802_16(p);
  p->allocdata (sizeof (struct mac802_16_reg_rsp_frame));
  reg_frame = (mac802_16_reg_rsp_frame*) p->accessdata();
  reg_frame->type = MAC_REG_RSP;
  reg_frame->response = 0; //OK
  peer = getCManager()->get_connection (header_req.cid, IN_CONNECTION)->getPeerNode();
  Connection *secondary = peer->getSecondary (OUT_CONNECTION);
  if (secondary==NULL) {
    //first time 
    secondary = new Connection (CONN_SECONDARY);
    Connection *upsecondary = new Connection (CONN_SECONDARY, secondary->get_cid());
    getCManager()->add_connection (upsecondary, IN_CONNECTION);
    getCManager()->add_connection (secondary, OUT_CONNECTION);
    peer->setSecondary (upsecondary, secondary);
  }
  reg_frame->sec_mngmt_cid = secondary->get_cid();
  wimaxHdr->header.cid = header_req.cid;
  ch->size() = REG_RSP_SIZE;
  
  //enqueue packet..must be replaced with second line later
  //getCManager()->get_connection (BROADCAST_CID, OUT_CONNECTION)->enqueue (p);
  peer->getPrimary(OUT_CONNECTION)->enqueue (p);

  //clear t17 timer for this node
  removetimer17 (peer->getAddr());

#ifdef USE_802_21
  debug ("At %f in Mac %d, send link up\n", NOW, addr());
  send_link_up (peer->getAddr(),addr(), -1);
#endif
}

/**
 * Send a neighbor advertisement message
 */
void Mac802_16BS::send_nbr_adv ()
{
  debug ("At %f in BS %d send_nbr_adv (nb_neighbor=%d)\n", NOW, addr(), nbr_db_->getNbNeighbor());
  Packet *p;
  struct hdr_cmn *ch;
  hdr_mac802_16 *wimaxHdr;
  mac802_16_mob_nbr_adv_frame *frame;
  //PeerNode *peer;

  //create packet for request
  p = getPacket ();
  ch = HDR_CMN(p);
  wimaxHdr = HDR_MAC802_16(p);
  p->allocdata (sizeof (struct mac802_16_mob_nbr_adv_frame));
  frame = (mac802_16_mob_nbr_adv_frame*) p->accessdata();
  frame->type = MAC_MOB_NBR_ADV;
  frame->n_neighbors = nbr_db_->getNbNeighbor();
  frame->skip_opt_field = 0;
  for (int i = 0 ; i < frame->n_neighbors ; i++) {
    frame->nbr_info[i].phy_profile_id.FAindex = 0;
    frame->nbr_info[i].phy_profile_id.bs_eirp = 0;
    frame->nbr_info[i].nbr_bsid= nbr_db_->getNeighbors()[i]->getID();
    frame->nbr_info[i].dcd_included = true;
    memcpy (&(frame->nbr_info[i].dcd_settings), nbr_db_->getNeighbors ()[i]->getDCD(), sizeof(mac802_16_dcd_frame));
    frame->nbr_info[i].ucd_included = true;
    memcpy (&(frame->nbr_info[i].ucd_settings), nbr_db_->getNeighbors ()[i]->getUCD(), sizeof(mac802_16_ucd_frame));
    frame->nbr_info[i].phy_included = false;
  }
  ch->size() = Mac802_16pkt::getMOB_NBR_ADV_size(frame);
  getCManager()->get_connection (BROADCAST_CID, OUT_CONNECTION)->enqueue (p);
  
}

/**
 * Process handover request
 * @param p The request
 */
void Mac802_16BS::process_msho_req (Packet *req)
{
  hdr_mac802_16 *wimaxHdr_req = HDR_MAC802_16(req);
  gen_mac_header_t header_req = wimaxHdr_req->header;
  mac802_16_mob_msho_req_frame *req_frame = 
    (mac802_16_mob_msho_req_frame*) req->accessdata();
  
  debug ("At %f in Mac %d received handover request from %d\n", NOW, addr(), header_req.cid);

  //check the BS that has stronger power
  int maxIndex = 0;
  int maxRssi = 0; //max value
  for (int i = 0; i < req_frame->n_new_bs_full ; i++) {
    if (req_frame->bs_full[i].bs_rssi_mean >= maxRssi) {
      maxIndex = i;
      maxRssi = req_frame->bs_full[i].bs_rssi_mean;
    }
  }
  //reply with one recommended BS
  Packet *p;
  struct hdr_cmn *ch;
  hdr_mac802_16 *wimaxHdr;
  mac802_16_mob_bsho_rsp_frame *rsp_frame;

  send_nbr_adv (); //to force update with latest information

  //create packet for request
  p = getPacket ();
  ch = HDR_CMN(p);
  wimaxHdr = HDR_MAC802_16(p);
  p->allocdata (sizeof (struct mac802_16_mob_bsho_rsp_frame)+sizeof (mac802_16_mob_bsho_rsp_rec));
  rsp_frame = (mac802_16_mob_bsho_rsp_frame*) p->accessdata();
  rsp_frame->type = MAC_MOB_BSHO_RSP;
  
  rsp_frame->mode = 0; //HO request
  rsp_frame->ho_operation_mode = 1; //mandatory handover response
  rsp_frame->n_recommended = 1;
  rsp_frame->resource_retain_flag = 0; //release connection information
  rsp_frame->n_rec[0].neighbor_bsid = req_frame->bs_full[maxIndex].neighbor_bs_index;
  rsp_frame->n_rec[0].ho_process_optimization=0; //no optimization

  ch->size() += Mac802_16pkt::getMOB_BSHO_RSP_size(rsp_frame);
  wimaxHdr->header.cid = header_req.cid;
  getCManager()->get_connection (header_req.cid, OUT_CONNECTION)->enqueue (p);
}
 
/**
 * Process handover indication
 * @param p The indication
 */
void Mac802_16BS::process_ho_ind (Packet *p)
{
  hdr_mac802_16 *wimaxHdr_req = HDR_MAC802_16(p);
  gen_mac_header_t header_req = wimaxHdr_req->header;
  //mac802_16_mob_ho_ind_frame *req_frame = 
  //  (mac802_16_mob_ho_ind_frame*) p->accessdata();
  
  debug ("At %f in Mac %d received handover indication from %d\n", NOW, addr(), header_req.cid);
  

}
 
/**
 * Send a scan response to the MN
 * @param rsp The response from the control
 */
void Mac802_16BS::send_scan_response (mac802_16_mob_scn_rsp_frame *rsp, int cid)
{
  //create packet for request
  Packet *p = getPacket ();
  struct hdr_cmn *ch = HDR_CMN(p);
  hdr_mac802_16 *wimaxHdr = HDR_MAC802_16(p);
  p->allocdata (sizeof (struct mac802_16_mob_scn_rsp_frame));
  mac802_16_mob_scn_rsp_frame* rsp_frame = (mac802_16_mob_scn_rsp_frame*) p->accessdata();
  memcpy (rsp_frame, rsp, sizeof (mac802_16_mob_scn_rsp_frame));
  rsp_frame->type = MAC_MOB_SCN_RSP;
  
  wimaxHdr->header.cid = cid;
  ch->size() += Mac802_16pkt::getMOB_SCN_RSP_size(rsp_frame);
  
  //add scanning station to the list
  PeerNode *peer = getCManager()->get_connection (cid, false)->getPeerNode();

  /* The request is received in frame i, the reply is sent in frame i+1
   * so the frame at which the scanning start is start_frame+2
   */
  ScanningStation *sta = new ScanningStation (peer->getAddr(), rsp_frame->scan_duration & 0xFF, 
					     rsp_frame->start_frame+frame_number_+2, 
					     rsp_frame->interleaving_interval & 0xFF,
					     rsp_frame->scan_iteration & 0xFF);
  sta->insert_entry_head (&scan_stations_);

  //enqueue packet
  getCManager()->get_connection (cid, OUT_CONNECTION)->enqueue (p);
}

/**** Internal methods ****/

/**
 * Update the given timer and check if thresholds are crossed
 * @param watch the stat watch to update
 * @param value the stat value
 */
void Mac802_16BS::update_watch (StatWatch *watch, double value)
{
  char *name;

#ifdef USE_802_21 //Switch to activate when using 802.21 modules (external package)
  threshold_action_t action = watch->update (value);

  if (action != NO_ACTION_TH) {
    link_parameter_type_s param;
    union param_value old_value, new_value;

    if (watch == &loss_watch_) {
      param.link_type = LINK_GENERIC;
      param.parameter_type = LINK_GEN_FRAME_LOSS;
    } else if (watch == &delay_watch_) {
      param.link_type = LINK_GENERIC;
      param.parameter_type = LINK_GEN_PACKET_DELAY;
    } else if (watch == &jitter_watch_) {
      param.link_type = LINK_GENERIC;
      param.parameter_type = LINK_GEN_PACKET_JITTER;
    }
    old_value.data_d = watch->old_average();
    new_value.data_d = watch->average();

    send_link_parameters_report (addr(), addr(), param, old_value, new_value);      
  }
#else
  watch->update (value);
#endif

  if (watch == &loss_watch_) {
    name = "loss";
  } else if (watch == &delay_watch_) {
    name = "delay";
  } else if (watch == &jitter_watch_) {
    name = "jitter";
  } else {
    name = "other";
  }
  if (print_stats_)
    printf ("At %f in Mac %d, updating stats %s: %f\n", NOW, addr(), name, watch->average());
}

/**
 * Update the given timer and check if thresholds are crossed
 * @param watch the stat watch to update
 * @param value the stat value
 */
void Mac802_16BS::update_throughput (ThroughputWatch *watch, double size)
{
  char *name;

#ifdef USE_802_21 //Switch to activate when using 802.21 modules (external package)
  threshold_action_t action = watch->update (size, NOW);
  if (action != NO_ACTION_TH) {
    link_parameter_type_s param;
    union param_value old_value, new_value;
    if (watch == &rx_data_watch_) {
      param.link_type = LINK_GENERIC;
      param.parameter_type = LINK_GEN_RX_DATA_THROUGHPUT;
    } else if (watch == &rx_traffic_watch_) {
      param.link_type = LINK_GENERIC;
      param.parameter_type = LINK_GEN_RX_TRAFFIC_THROUGHPUT;
    } else if (watch == &tx_data_watch_) {
      param.link_type = LINK_GENERIC;
      param.parameter_type = LINK_GEN_TX_DATA_THROUGHPUT;
    } else if (watch == &tx_traffic_watch_) {
      param.link_type = LINK_GENERIC;
      param.parameter_type = LINK_GEN_TX_TRAFFIC_THROUGHPUT;
    }
    old_value.data_d = watch->old_average();
    new_value.data_d = watch->average();
    
    send_link_parameters_report (addr(), addr(), param, old_value, new_value);
  }
#else
  watch->update (size, NOW);
#endif 

  if (watch == &rx_data_watch_) {
    name = "rx_data";
    rx_data_timer_->resched (watch->get_timer_interval());
  } else if (watch == &rx_traffic_watch_) {
    rx_traffic_timer_->resched (watch->get_timer_interval());
    name = "rx_traffic";
  } else if (watch == &tx_data_watch_) {
    tx_data_timer_->resched (watch->get_timer_interval());
    name = "tx_data";
  } else if (watch == &tx_traffic_watch_) {
    tx_traffic_timer_->resched (watch->get_timer_interval());
    name = "tx_traffic";
  }

  if (print_stats_)
    printf ("At %f in Mac %d, updating stats %s: %f\n", NOW, addr(), name, watch->average());
}


/**
 * Called when a timer expires
 * @param The timer ID
 */
void Mac802_16BS::expire (timer_id id)
{
  switch (id) {
  case WimaxDCDTimerID:
    sendDCD = true;
    debug ("At %f in Mac %d DCDtimer expired\n", NOW, addr());
    dcdtimer_->start (macmib_.dcd_interval);
    break;
  case WimaxUCDTimerID:
    sendUCD = true;
    debug ("At %f in Mac %d UCDtimer expired\n", NOW, addr());
    ucdtimer_->start (macmib_.ucd_interval);
    break;
  case WimaxMobNbrAdvTimerID:
    send_nbr_adv();
    nbradvtimer_->start (macmib_.nbr_adv_interval);
    break;
  default:
    debug ("Warning: unknown timer expired in Mac802_16BS\n");
  }
}

/**
 * Start a new frame
 */
void Mac802_16BS::start_ulsubframe ()
{
  debug ("At %f in Mac %d BS scheduler ulsubframe expires\n", NOW, addr());

  //change PHY state
  getPhy()->setMode (OFDM_RECV);  

  //start handler of ulsubframe
  getMap()->getUlSubframe()->getTimer()->sched (0);

  //reschedule for next frame
  ul_timer_->resched (getFrameDuration());
}

/**
 * Start a new frame
 */
void Mac802_16BS::start_dlsubframe ()
{
  debug ("At %f in Mac %d BS scheduler dlsubframe expires (frame=%d)\n", 
	       NOW, addr(), frame_number_++);

  /* First lets clear the peers we haven't heard of for long time */
  for (PeerNode *pn = getPeerNode_head() ; pn ; ) {
    PeerNode *tmp = pn->next_entry(); //next elem
    if (isPeerScanning(pn->getAddr())) {
      //since a scanning node cannot send data we push the 
      //timeout while it is scanning
      pn->setRxTime(NOW); 
    } else if (NOW-pn->getRxTime()>macmib_.client_timeout) {
      debug ("Client timeout for node %d\n", pn->getAddr());
      removePeerNode(pn);      
    }
    pn = tmp;
  }

  assert (map_);
  map_->setStarttime(NOW);
  ((BSScheduler*)scheduler_)->schedule();

  //update some information  
  if (sendDCD || map_->getDlSubframe()->getCCC()!= dlccc_) {
    sendDCD = false;
    dlccc_ = map_->getDlSubframe()->getCCC();
    //reschedule timer
    dcdtimer_->stop();
    dcdtimer_->start (macmib_.dcd_interval);
  }

  if (sendUCD || map_->getUlSubframe()->getCCC()!= ulccc_) {
    sendUCD = false;
    ulccc_ = map_->getUlSubframe()->getCCC();
    //reschedule timer
    ucdtimer_->stop();
    ucdtimer_->start (macmib_.ucd_interval);
  }

  //change PHY state
  getPhy()->setMode (OFDM_SEND);

  //start handler of dlsubframe
  getMap()->getDlSubframe()->getTimer()->sched (0);

  //reschedule for next time (frame duration)
  dl_timer_->resched (getFrameDuration());  
  ul_timer_->resched (map_->getUlSubframe()->getStarttime()*getPhy()->getPS());
}

/** Add a new Fast Ranging allocation
 * @param time The time when to allocate data
 * @param macAddr The MN address
 */
void Mac802_16BS::addNewFastRanging (double time, int macAddr)
{
  //compute the frame where the allocation will be located
  int frame = int ((time-NOW)/getFrameDuration()) +2 ;
  frame += getFrameNumber();
  //printf ("Added fast RA for frame %d (current=%d) for time (%f)\n", 
  //	  frame, getFrameNumber(), time);
  FastRangingInfo *info= new FastRangingInfo (frame, macAddr);
  info->insert_entry(&fast_ranging_head_);
}

/**
 * Add a new timer17 in the list. It also performs cleaning of the list
 * @param index The client address
 */
void Mac802_16BS::addtimer17 (int index)
{
  //clean expired timers
  T17Element *entry;
  for (entry = t17_head_.lh_first; entry ; ) {
    if (entry->paused ()) {
      T17Element *tmp = entry;
      entry = entry->next_entry();
      tmp->remove_entry();
      free (tmp);
    }
    entry = entry->next_entry();
  }

  entry = new T17Element (this, index);
  entry->insert_entry (&t17_head_);
}
/**
 * Cancel and remove the timer17 associated with the node
 * @param index The client address
 */
void Mac802_16BS::removetimer17 (int index)
{
  //clean expired timers
  T17Element *entry;
  for (entry = t17_head_.lh_first; entry ; entry = entry->next_entry()) {
    if (entry->index ()==index) {
      entry->cancel();
      entry->remove_entry();
      delete (entry);
      break;
    }
  }
}

/** 
 * Finds out if the given station is currently scanning
 * @param nodeid The MS id
 */
bool Mac802_16BS::isPeerScanning (int nodeid)
{
  ScanningStation *sta;
  for (sta = scan_stations_.lh_first; sta ; sta = sta->next_entry()) {
    if (sta->getNodeId()==nodeid && sta->isScanning(frame_number_)) {
      //printf ("station %d scanning\n", nodeid);
      return true;
    }
  }
  return false;
}
