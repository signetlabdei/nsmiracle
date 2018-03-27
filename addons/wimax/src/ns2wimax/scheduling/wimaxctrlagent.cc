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
 * $Id: wimaxctrlagent.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "wimaxctrlagent.h"
#include "mac802_16BS.h" 
#include "bsscheduler.h"
#include "../wimaxneighborentry.h"

#define MYNUM	Address::instance().print_nodeaddr(getNodeIp())

extern packet_t PT_WIMAXBS;

int hdr_wimaxbs::offset_;
/**
 * Tcl hook for Packet definitions
 */
static class WimaxBSHeaderClass : public PacketHeaderClass {
public:
	WimaxBSHeaderClass() : PacketHeaderClass("PacketHeader/WIMAXBS",
					     sizeof(hdr_wimaxbs)) {
	    bind_offset(&hdr_wimaxbs::offset_);
	    bind();
        }
} class_wimaxbshdr;

/**
 * Tcl hook for agent 
 */
static class WimaxCtrlAgentClass : public TclClass {
public:
	WimaxCtrlAgentClass() : TclClass("Module/WimaxCtrl") {}
	TclObject* create(int, const char*const*) {
		return (new WimaxCtrlAgent());
	}
} class_wimaxctrlagent;

/*
 * Handler for timer expiration
 */
void UpdateTimer::expire (Event*)
{
  a_->sendUpdate();
}

/*
 * Handler for response timer expiration
 */
void ScanRspTimer::expire (Event*)
{
  a_->agent()->send_scan_response(a_->cid());
}


/*
 * Creates a Wimax controler agent
 * Initializes the agent and bind variable to be accessible in TCL
 */
WimaxCtrlAgent::WimaxCtrlAgent() : mac_(0), nodeIp(0), updatetimer_ (this)
{
  nbmapentry_=0;
  LIST_INIT (&scan_req_head_);

  //bind attributes
  bind ("adv_interval_", &adv_interval_);
  bind ("default_association_level_", &defaultlevel_);
  bind ("synch_frame_delay_", &synch_frame_delay_);

  //schedule first update
  updatetimer_.sched (Random::uniform(0, UPDATE_JITTER));
}

/* 
 * Interface with TCL interpreter
 * @param argc The number of elements in argv
 * @param argv The list of arguments
 * @return TCL_OK if everything went well else TCL_ERROR
 */
int WimaxCtrlAgent::command(int argc, const char*const* argv)
{
  //Tcl& tcl= Tcl::instance();

  if (argc == 3) {
    // set the Minimum interval between two RAs
    if (strcmp(argv[1], "set-mac") == 0) {
      mac_ = (Mac802_16BS*) TclObject::lookup(argv[2]);
      mac_->setCtrlAgent (this);
      return TCL_OK;
    }
    // set node ip address
    if (strcmp(argv[1], "set-nodeip") == 0) {
	    nodeIp = (nsaddr_t) atoi(argv[2]);
	    return TCL_OK;
    }

  } else if (argc == 4) {
    if (strcmp(argv[1], "add-neighbor") == 0) {
      //the parameter is the mac, and we also extract the node
      Mac802_16 *tmp = (Mac802_16 *) TclObject::lookup(argv[2]);
      if (nbmapentry_ == MAX_MAP_ENTRY) {
	fprintf (stderr, "Table size exceeding. Increase MAX_MAP_ENTRY\n");
      }
      // The following is not supported outside of our mobility
      // package so we provide a workaround
      //tcl.evalf ("%s get-node", argv[2]);
      //Node *tmpNode = (Node *) TclObject::lookup(tcl.result());

      nsaddr_t tmpNode = (nsaddr_t) atoi(argv[3]);

      //add entry
      maptable_[nbmapentry_][0] = tmp->addr();
      maptable_[nbmapentry_][1] = tmpNode;
      debug ("Adding neighbor %s (mac %d) in %s\n", Address::instance().print_nodeaddr(tmpNode),
	      tmp->addr(), MYNUM);

      nbmapentry_++;
      return TCL_OK;
    }
  }

  return (Module::command(argc, argv));
}

/*
 * Send an update (DCD/UCD) to all neighboring BSs
 */
void WimaxCtrlAgent::sendUpdate ()
{
  //get the DCD/UCD message to include in the update
  Packet *dcd = mac_->getMap()->getDCD();
  Packet *ucd = mac_->getMap()->getUCD();

  //allocate data to store information
  mac802_16_dcd_frame *dcdframe = (mac802_16_dcd_frame*) dcd->accessdata();
  mac802_16_ucd_frame *ucdframe = (mac802_16_ucd_frame*) ucd->accessdata();
  
  Packet *p = allocpkt();
  hdr_ip *iph = HDR_IP(p);
  hdr_wimaxbs *rh = HDR_WIMAXBS(p);
  hdr_cmn *hdrc = HDR_CMN(p);
  
  rh->getType() = WIMAX_BS_ADV; 
  hdrc->size() = HDR_CMN(dcd)->size()+HDR_CMN(ucd)->size(); //TBD: remove double header
  //set content
  rh->macAddr() = mac_->addr();
  p->allocdata (sizeof (mac802_16_dcd_frame)+sizeof (mac802_16_ucd_frame)); 
  unsigned char *data = p->accessdata();
  memcpy (data, dcdframe, sizeof (mac802_16_dcd_frame));
  memcpy (data+sizeof (mac802_16_dcd_frame), ucdframe, sizeof (mac802_16_ucd_frame));
  
  Packet *tmpPkt; 
  for (int i = 0; i < nbmapentry_ ; i++) {
    tmpPkt = p->copy();
    iph = HDR_IP(tmpPkt);
    //set packet destination
    iph->daddr() = maptable_[i][1];
    //iph->dport() = port(); need for a port?
    debug ("At %f in node %s, send update to node %s\n", NOW, MYNUM,Address::instance().print_nodeaddr(iph->daddr()));
    debug ("frame number=%d\n", dcdframe->frame_number);
    sendDown(tmpPkt);
  }

  //reschedule timer
  updatetimer_.resched (adv_interval_);
}

/* 
 * Process received packet
 * @param p The packet received
 * @param h The handler that sent the packet
 */
void WimaxCtrlAgent::recv(Packet* p)
{
  assert (p);

  if(HDR_CMN(p)->ptype() != PT_WIMAXBS)
  {
	  Packet::free (p);
	  return;
  }
  hdr_wimaxbs *rh = HDR_WIMAXBS(p);
  switch (rh->getType()) {
  case WIMAX_BS_ADV:
    processUpdate (p);
    break;
  case WIMAX_BS_SYNCH_REQ:
    process_synch_request (p);
    break;
  case WIMAX_BS_SYNCH_RSP:
    process_synch_response (p);
    break;
  default:
    fprintf (stderr, "Unknown message type in WimaxCtrlAgent\n");
  }
  Packet::free (p);
}

/* 
 * Process received packet
 * @param p The update received
 * @param h The handler that sent the packet
 */
void WimaxCtrlAgent::processUpdate(Packet* p)
{
  debug ("At %f in node %s, WimaxCtrlAgent received update message from %s\n", NOW, MYNUM,
	 Address::instance().print_nodeaddr(HDR_IP(p)->saddr()));

  hdr_wimaxbs *rh = HDR_WIMAXBS(p);
  WimaxNeighborEntry *entry = mac_->nbr_db_->getNeighbor(rh->macAddr());

  //check if we know about this neighbor
  bool found = false;
  for (int i = 0; i < nbmapentry_ ; i++) {
	  printf("Table entry: MAC: %d, IP: %d, SRCIP: %d\n", maptable_[i][0], maptable_[i][1], HDR_IP(p)->saddr());
    if (maptable_[i][1]==HDR_IP(p)->saddr())
      found = true;
  }
  assert (found);

  if (entry==NULL) {
    debug ("\tNew neighbor detected...add entry for mac %d\n", rh->macAddr());
    entry = new WimaxNeighborEntry (rh->macAddr());
    mac_->nbr_db_->addNeighbor(entry);
  }
  //update entry
  unsigned char *data = p->accessdata();
  mac802_16_dcd_frame *dcdframe = (mac802_16_dcd_frame *)malloc (sizeof (mac802_16_dcd_frame));
  mac802_16_ucd_frame *ucdframe = (mac802_16_ucd_frame *)malloc (sizeof (mac802_16_ucd_frame));

  memcpy (dcdframe, data, sizeof (mac802_16_dcd_frame));
  memcpy (ucdframe, data+sizeof (mac802_16_dcd_frame), sizeof (mac802_16_ucd_frame));
  debug ("\tframe number=%d ccc=%d\n", dcdframe->frame_number,ucdframe->config_change_count);
  mac802_16_dcd_frame *dcdtmp = entry->getDCD();
  if (dcdtmp)
    free(dcdtmp);
  mac802_16_ucd_frame *ucdtmp = entry->getUCD();
  if (ucdtmp)
    free(ucdtmp);
  entry->setDCD (dcdframe);
  entry->setUCD (ucdframe);
  //free (p);
}

/**
 * Process scanning request
 * @param p The request
 */
void WimaxCtrlAgent::process_scan_request (Packet *req)
{
  hdr_mac802_16 *wimaxHdr_req = HDR_MAC802_16(req);
  gen_mac_header_t header_req = wimaxHdr_req->header;
  mac802_16_mob_scn_req_frame *req_frame;
  req_frame = (mac802_16_mob_scn_req_frame*) req->accessdata();

  mac_->debug ("At %f in Mac %d received scanning request from %d\n", NOW, mac_->addr(), header_req.cid);
  
  //for first implementation we disregard the information 
  //sent by MN. Just use default association mechanisms

  //should check if there is already pending request: TBD
  Scan_req *entry = NULL;
  
  for (entry = scan_req_head_.lh_first ; entry && (entry->cid() != header_req.cid); entry=entry->next_entry());

  if (entry) {
    mac_->debug ("\tDuplicate requests. Skip...\n");
    return;
  } else {
    entry = new Scan_req (this, synch_frame_delay_*mac_->getFrameDuration(), header_req.cid, req_frame);
    entry->insert_entry (&scan_req_head_);
    entry->start_frame()=mac_->getFrameNumber();
  }

  switch (defaultlevel_){
  case 0: //Scan without association
    entry->response()->scan_duration = req_frame->scan_duration;
    entry->response()->start_frame = 2;
    entry->response()->report_mode = 0; //no report for now
    entry->response()->interleaving_interval = req_frame->interleaving_interval;
    entry->response()->scan_iteration = req_frame->scan_iteration;
    entry->response()->n_recommended_bs_index = 0;
    entry->response()->n_recommended_bs_full = nbmapentry_;
    for (int i = 0; i < nbmapentry_ ; i++) {
      entry->response()->rec_bs_full[i].recommended_bs_id = maptable_[i][0];
      entry->response()->rec_bs_full[i].scanning_type = SCAN_WITHOUT_ASSOC;
    }
    //send response
    mac_->send_scan_response (entry->response(), header_req.cid);
    //clean data
    entry->remove_entry();
    delete entry;
    break;

  case 1: //Association without coordination
    entry->response()->scan_duration = req_frame->scan_duration;
    entry->response()->start_frame = 2;
    entry->response()->report_mode = 0; //no report for now
    entry->response()->interleaving_interval = req_frame->interleaving_interval;
    entry->response()->scan_iteration = req_frame->scan_iteration;
    entry->response()->n_recommended_bs_index = 0;
    entry->response()->n_recommended_bs_full = nbmapentry_;
    for (int i = 0; i < nbmapentry_ ; i++) {
      entry->response()->rec_bs_full[i].recommended_bs_id = maptable_[i][0];
      entry->response()->rec_bs_full[i].scanning_type = SCAN_ASSOC_LVL0;
    }
    //send response
    mac_->send_scan_response (entry->response(), header_req.cid);
    //clean data
    entry->remove_entry();
    delete entry;
    break;

  case 2: //Association with coordination
    //init data
    entry->response()->n_recommended_bs_index = 0;
    entry->response()->n_recommended_bs_full = 0;
    entry->pending_rsp () = 0;
 
    //send request to neighbors
    for (int i = 0; i < nbmapentry_ ; i++) {
      Packet *p = allocpkt();
      hdr_ip *iph = HDR_IP(p);
      hdr_wimaxbs *rh = HDR_WIMAXBS(p);
      hdr_cmn *hdrc = HDR_CMN(p);
      
      rh->getType() = WIMAX_BS_SYNCH_REQ; 
      hdrc->size() = 30; //We need to define proper size
      //set content
      iph = HDR_IP(p);
      iph->daddr() = maptable_[i][1];
//       iph->dport() = port(); // need for a port?
      
      rh->macAddr() = mac_->getCManager()->get_connection(header_req.cid, true)->getPeerNode()->getAddr();
      rh->cid = header_req.cid;
      rh->scanning_type = (wimax_scanning_type) defaultlevel_;
      //we suggest a rendez-vous time at the beginning of each 
      //scan iteration
      rh->current_frame = mac_->getFrameNumber();
      //if we want to start scanning 2 frames after, then add one (i.e 3) because 
      //the message will be sent on the next frame
      printf ("scan_duration=%d, scan_interval=%d\n", req_frame->scan_duration,
	      req_frame->interleaving_interval);
      rh->rdvt = (i+1)*(req_frame->scan_duration+req_frame->interleaving_interval)+synch_frame_delay_+START_FRAME_OFFSET+1;
      rh->rendezvous_time = NOW+rh->rdvt*mac_->getFrameDuration();     
      printf ("Request: current frame=%d, rdv frame=%d, rdv time=%f\n",
	      rh->current_frame, rh->rdvt, rh->rendezvous_time); 

      entry->pending_rsp ()++;
      sendDown(p);
      
      entry->response()->n_recommended_bs_full = i + 1;
      entry->response()->rec_bs_full[i].recommended_bs_id = maptable_[i][0];
      entry->response()->rec_bs_full[i].scanning_type = rh->scanning_type;
      entry->response()->rec_bs_full[i].rdv_time = rh->rdvt-synch_frame_delay_;  
    }

    //continue initializing response
    entry->response()->scan_duration = req_frame->scan_duration;
    entry->response()->start_frame = START_FRAME_OFFSET;
    entry->response()->report_mode = 0; //no report for now
    entry->response()->interleaving_interval = entry->request()->interleaving_interval;
    entry->response()->scan_iteration = entry->request()->scan_iteration;
    entry->response()->n_recommended_bs_index = 0;
    
    //printf ("Response: current frame=%d (now=%f), start in %d frame (t=%f)\n",
    //	    mac_->getFrameNumber (), NOW, entry->response()->start_frame, NOW+(entry->response()->start_frame*mac_->getFrameDuration()));
    //send response
    //((BSScheduler*)mac_->getScheduler())->send_scan_response (entry->response(), entry->cid());

    break;
  case 3: //Network Assisted Association reporting

    break;
  default:
    break;
  }
}

/**
 * Process synchronization request
 * @param req The request
 */
void WimaxCtrlAgent::process_synch_request (Packet *req)
{
  debug ("At %f in node %s, WimaxCtrlAgent received synch request from %s\n", NOW, MYNUM,
	 Address::instance().print_nodeaddr(HDR_IP(req)->saddr()));

  //schedule rendez-vous time
  //schedule sending of Fast-ranging-IE
  mac_->addNewFastRanging (HDR_WIMAXBS(req)->rendezvous_time, HDR_WIMAXBS(req)->macAddr());
  

  //send response
  Packet *p = allocpkt();
  hdr_ip *iph = HDR_IP(p);
  hdr_wimaxbs *rh = HDR_WIMAXBS(p);
  hdr_cmn *hdrc = HDR_CMN(p);
  
  rh->getType() = WIMAX_BS_SYNCH_RSP; 
  hdrc->size() = 30; //We need to define proper size
  //set content
  iph = HDR_IP(p);
  iph->daddr() = HDR_IP(req)->saddr();
  //iph->dport() = port(); need for a port?
  
  //we accept what the serving BS sent
  rh->cid = HDR_WIMAXBS(req)->cid;
  rh->scanning_type = HDR_WIMAXBS(req)->scanning_type;
  rh->current_frame = HDR_WIMAXBS(req)->current_frame;
  rh->rdvt = HDR_WIMAXBS(req)->rdvt;
  rh->rendezvous_time = HDR_WIMAXBS(req)->rendezvous_time; 

  //send (p,0);
}

/**
 * Process synchronization response
 * @param p The response
 */
void WimaxCtrlAgent::process_synch_response (Packet *p)
{
  debug ("At %f in node %s, WimaxCtrlAgent received synch response from %s\n", NOW, MYNUM,
	 Address::instance().print_nodeaddr(HDR_IP(p)->saddr()));

  hdr_wimaxbs *rh = HDR_WIMAXBS(p);
  int i;

  //update information
  Scan_req *entry;
  for (entry = scan_req_head_.lh_first ; entry && (entry->cid() != rh->cid); entry=entry->next_entry());

  assert (entry);

  i = entry->response()->n_recommended_bs_full;
  entry->response()->n_recommended_bs_full = i + 1;
  entry->response()->rec_bs_full[i].recommended_bs_id = maptable_[i][0];
  entry->response()->rec_bs_full[i].scanning_type = rh->scanning_type;
  entry->response()->rec_bs_full[i].rdv_time = rh->rdvt;
}

/**
 * Send a scan response to the MN that has the given CID
 * @param cid The CID of the MN
 */
void WimaxCtrlAgent::send_scan_response (int cid)
{
  Scan_req *entry;
  for (entry = scan_req_head_.lh_first ; entry && (entry->cid() != cid); entry=entry->next_entry());
  
  assert (entry);
  
  entry->response()->scan_duration = entry->request()->scan_duration;
  printf ("Response: current frame=%d, start frame=%d diff=%d\n", 
	  mac_->getFrameNumber(), entry->start_frame(), 100-(mac_->getFrameNumber()-entry->start_frame()));
  printf ("Response: current frame=%d (now=%f), start in %d frame (t=%f)\n",
	  mac_->getFrameNumber (), NOW, entry->response()->start_frame, NOW+(entry->response()->start_frame*mac_->getFrameDuration()));
  //send response
  mac_->send_scan_response (entry->response(), entry->cid());
  //clean data
  entry->remove_entry();
  delete entry;
}

Packet* WimaxCtrlAgent::allocpkt()
{
	Packet* p = Packet::alloc();

	HDR_CMN(p)->ptype() = PT_WIMAXBS;
	HDR_CMN(p)->size() = 0;

	return (p);
}
