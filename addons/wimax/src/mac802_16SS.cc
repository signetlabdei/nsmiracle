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
 * $Id: mac802_16SS.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "mac802_16SS.h"
#include "ns2wimax/scheduling/wimaxscheduler.h"
#include "ns2wimax/scheduling/ssscheduler.h" 
#include "ns2wimax/destclassifier.h"

/**
 * TCL Hooks for the simulator for wimax mac
 */
static class Mac802_16SSClass : public TclClass {
	public:
		Mac802_16SSClass() : TclClass("Module/802_16/MAC/SS") {}
		TclObject* create(int, const char*const*) {
			return (new Mac802_16SS());
		}
} class_mac802_16SS;

/**
 * Creates a Mac 802.16
 */
Mac802_16SS::Mac802_16SS() : Mac802_16 ()
{
	type_ = STA_MN; //type of MAC. In this case it is for SS
	
	//Create default configuration
	addClassifier (new DestClassifier ());
	Tcl& tcl = Tcl::instance();
	tcl.evalf ("new WimaxScheduler/SS");
	scheduler_ = (WimaxScheduler*) TclObject::lookup(tcl.result());
	scheduler_->setMac (this); //register the mac
  
	init_default_connections ();

	//We can configure each SS with a different modulation
	//default transmission profile (64 QAM 3_4. Can be changed by TCL)
	//The DIUC to use will be determined via an algorithm considering 
	//the channel performance
	default_diuc_ = DIUC_PROFILE_7;

	//initialize state
	state_ = MAC802_16_DISCONNECTED; //At start up, we are not connected.
	//scanning status
	scan_info_ = (struct scanning_structure *) malloc (sizeof (struct scanning_structure));
	memset (scan_info_, 0, sizeof (struct scanning_structure));
	scan_info_->nbr = NULL;
	scan_info_->substate = NORMAL; //no scanning
	scan_flag_ = false;

	map_ = new FrameMap (this);

	//create timers for state machine
	t1timer_ = new WimaxT1Timer (this);
	t2timer_ = new WimaxT2Timer (this);
	t6timer_ = NULL;
	t12timer_ = new WimaxT12Timer (this);
	t21timer_ = new WimaxT21Timer (this);
	lostDLMAPtimer_ = new WimaxLostDLMAPTimer (this);
	lostULMAPtimer_ = new WimaxLostULMAPTimer (this);

	//create timers for DL/UL boundaries
	dl_timer_ = new DlTimer (this);
	ul_timer_ = new UlTimer (this);

	//initialize some other variables
	nb_reg_retry_ = 0;
	nb_scan_req_ = 0;
}

/*
 * Interface with the TCL script
 * @param argc The number of parameter
 * @param argv The list of parameters
 * @return command status
 */
int Mac802_16SS::command(int argc, const char*const* argv)
{
	if (argc == 3) {
		if (strcmp(argv[1], "set-diuc") == 0) {
			int diuc = atoi (argv[2]);
			if (diuc < DIUC_PROFILE_1 || diuc > DIUC_PROFILE_11)
				return TCL_ERROR;
			default_diuc_ = diuc;
			return TCL_OK;
		} 
	}

	return Mac802_16::command(argc, argv);
}

/**
 * Initialize default connections
 * These connections are not linked to a peer node
 */
void Mac802_16SS::init_default_connections ()
{
	Connection * con;

  //create initial ranging and padding connection
	con = new Connection (CONN_INIT_RANGING);
	connectionManager_->add_connection (con, OUT_CONNECTION); 
	con = new Connection (CONN_INIT_RANGING);
	connectionManager_->add_connection (con, IN_CONNECTION); 
	con = new Connection (CONN_PADDING);
	connectionManager_->add_connection (con, OUT_CONNECTION);
	con = new Connection (CONN_PADDING);
	connectionManager_->add_connection (con, IN_CONNECTION);

  //create connection to receive broadcast packets from BS
	con = new Connection (CONN_BROADCAST);
	connectionManager_->add_connection (con, IN_CONNECTION);
}

/**
 * Initialize the MAC
 */
void Mac802_16SS::init ()
{
  //init the scheduler
	scheduler_->init();

  //Set physical layer to receiving mode
	getPhy()->setMode (OFDM_RECV);

  //SS is looking for synchronization
	state_ = MAC802_16_WAIT_DL_SYNCH;  

  //start timer for expiration
	t21timer_->start (macmib_.t21_timeout);
}


/**
 * Set the mac state
 * @param state The new mac state
 */  
void Mac802_16SS::setMacState (Mac802_16State state)
{
	state_ = state;
}

/**
 * Return the mac state
 * @return The new mac state
 */  
Mac802_16State Mac802_16SS::getMacState ()
{
	return state_;
}

/**
 * Backup the state of the Mac
 * @return A structure containing a copy of current MAC state
 */
state_info* Mac802_16SS::backup_state ()
{
	state_info *backup_state = (state_info*) malloc (sizeof (state_info));
	backup_state->state = state_;
	backup_state->frameduration = getFrameDuration();
	backup_state->frame_number = frame_number_;
	backup_state->channel = getChannel();
	backup_state->connectionManager = connectionManager_;
	connectionManager_ = new ConnectionManager (this);
	init_default_connections ();
	backup_state->serviceFlowHandler = serviceFlowHandler_;
	serviceFlowHandler_ = new ServiceFlowHandler();
	backup_state->peer_list = peer_list_;
	backup_state->nb_peer = nb_peer_;
	peer_list_ = (struct peerNode *) malloc (sizeof(struct peerNode));
	LIST_INIT(peer_list_);
	return backup_state;
}

/**
 * Restore the state of the MAC
 * @param backup_state The MAC state to restore
 */
void Mac802_16SS::restore_state (state_info *backup_state)
{
	state_ = backup_state->state;
	setFrameDuration(backup_state->frameduration);
	frame_number_ = backup_state->frame_number;
	setChannel (backup_state->channel);
	delete (connectionManager_);
	connectionManager_ = backup_state->connectionManager;
	delete (serviceFlowHandler_);
	serviceFlowHandler_ = backup_state->serviceFlowHandler;
	while (getPeerNode_head()!=NULL) {
		removePeerNode (getPeerNode_head());
	}
	peer_list_ = backup_state->peer_list;
	nb_peer_ = backup_state->nb_peer;
}

/**
 * Called when a timer expires
 * @param The timer ID
 */
void Mac802_16SS::expire (timer_id id)
{
	switch (id) {
		case WimaxT21TimerID:
			debug ("At %f in Mac %d, synchronization failed\n", NOW, addr());
    //go to next channel
			nextChannel();
			t21timer_->start (macmib_.t21_timeout);
			break;
		case WimaxLostDLMAPTimerID:
			debug ("At %f in Mac %d, lost synchronization (DL_MAP)\n", NOW, addr());
			lost_synch ();
			break;
		case WimaxT1TimerID:
			debug ("At %f in Mac %d, lost synchronization (DCD)\n", NOW, addr());
			lost_synch ();
			break;
		case WimaxLostULMAPTimerID:
			debug ("At %f in Mac %d, lost synchronization (UL_MAP)\n", NOW, addr());
			lost_synch ();
			break;
		case WimaxT12TimerID:
			debug ("At %f in Mac %d, lost uplink param (UCD)\n", NOW, addr());
			lost_synch ();
			break;
		case WimaxT2TimerID:
			debug ("At %f in Mac %d, lost synchronization (RNG)\n", NOW, addr());
			getMap()->getUlSubframe()->getRanging()->removeRequest ();
			lost_synch ();
			break;
		case WimaxT3TimerID:
			debug ("At %f in Mac %d, no response from BS\n", NOW, addr());
    //we reach the maximum number of retries
    //mark DL channel usuable (i.e we go to next)
			getMap()->getUlSubframe()->getRanging()->removeRequest ();
			nextChannel();
			lost_synch ();
			break;
		case WimaxT6TimerID:
			debug ("At %f in Mac %d, registration timeout (nbretry=%d)\n", NOW, addr(),
			       nb_reg_retry_);
			if (nb_reg_retry_ == macmib_.reg_req_retry) {
				debug ("\tmax retry excedeed\n");
				lost_synch ();
			} else {
				send_registration();
			}
			break;
		case WimaxT44TimerID:
			debug ("At %f in Mac %d, did not receive MOB_SCN-RSP (nb_retry=%d/%d)\n", NOW, addr(), nb_scan_req_, macmib_.scan_req_retry);
			if (nb_scan_req_ <= macmib_.scan_req_retry) {
				send_scan_request ();
			} else { //reset for next time
				nb_scan_req_ = 0;
			}
			break;
		case WimaxScanIntervalTimerID:
			pause_scanning ();
			break;    
		case WimaxRdvTimerID:
    //we need to meet at another station. We cancel the current scanning
    //lost_synch ();
			debug ("At %f in Mac %d Rdv timer expired\n", NOW, addr());
			break;
		default:
			debug ("Trigger unkown\n");
	}
}


/**** Packet processing methods ****/

/*
 * Process packets going out
 * @param p The packet to send out
 */
void Mac802_16SS::tx(Packet *p)
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
void Mac802_16SS::transmit(Packet *p)
{
	//The following condition is valid for OFDM but in OFDMA
	//we can send multiple packets using different subcarriers
	//so we will have to update this.
	if (NOW < last_tx_time_+last_tx_duration_) {
		//still sending
		//printf ("MAC is already transmitting. Drop packet.\n");
		Packet::free (p);
		return;
	}

	struct hdr_cmn *ch = HDR_CMN(p);
  
	debug ("At %f in Mac %d sending packet (type=%s, size=%d, txtime=%f) ", NOW, addr(), packet_info.name(ch->ptype()), ch->size(), ch->txtime());
	if (ch->ptype()==PT_MAC) {
		if (HDR_MAC802_16(p)->header.ht == 0)
			debug ("mngt=%d\n", ((mac802_16_dl_map_frame*) p->accessdata())->type);
		else
			debug ("bwreq\n");
	} else {
		debug ("\n");
	}
	
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
	
	//pass it down, 0 delay???
	sendDown(p, 0);

	// Tell queue we want more
	resume();
}

/*
 * Process incoming packets
 * @param p The incoming packet
 */
void Mac802_16SS::rx (Packet *p)
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
void Mac802_16SS::receive ()
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
	PeerNode *peer_ = getPeerNode_head();
	if (peer_) {
		peer_->setRxTime (NOW);
    
    //collect receive signal strength stats
		peer_->getStatWatch()->update(10*log10(pktRx_->txinfo_.RxPr*1e3));
    //debug ("At %f in Mac %d weighted RXThresh: %e rxp average %e\n", NOW, index_, macmib_.lgd_factor_*macmib_.RXThreshold_, pow(10,peer->getStatWatch()->average()/10)/1e3);
		double avg_w = pow(10,(peer_->getStatWatch()->average()/10))/1e3;
    
		if ( avg_w < (macmib_.lgd_factor_*macmib_.RXThreshold_)) {
      //Removed the condition on going down to allow sending multiple events with different confidence level
      //if (!peer->isGoingDown () && type_ == STA_MN && state_==MAC802_16_CONNECTED) {
			if (state_==MAC802_16_CONNECTED) {
#ifdef USE_802_21
				if(mih_){
					double probability = ((macmib_.lgd_factor_*macmib_.RXThreshold_)-avg_w)/((macmib_.lgd_factor_*macmib_.RXThreshold_)-macmib_.RXThreshold_);
					Mac::send_link_going_down (addr(), peer_->getAddr(), -1, (int)(100*probability), LGD_RC_LINK_PARAM_DEGRADING, eventId_++);
				}else{
#endif
					if (!peer_->isGoingDown ()) //when we don't use 802.21, we only want to send the scan request once
						send_scan_request ();
#ifdef USE_802_21
				}
#endif
				peer_->setGoingDown (true);
			}
		}
		else {
			if (peer_->isGoingDown()) {
#ifdef USE_802_21
				Mac::send_link_rollback (addr(), peer_->getAddr(), eventId_-1);
#endif
				peer_->setGoingDown (false);
			}
		}
	}
  
  //process reassembly
	if (wimaxHdr->header.type_frag) {
		bool drop_pkt = true;
		bool frag_error = false;
    //printf ("Frag type = %d %d\n",wimaxHdr->frag_subheader.fc,wimaxHdr->frag_subheader.fc & 0x3);
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

  //We check if it is a MAC packet or not
	if (HDR_CMN(pktRx_)->ptype()==PT_MAC) {
		process_mac_packet (pktRx_);
		update_throughput (&rx_traffic_watch_, 8*ch->size());
		Packet::free(pktRx_);
	}
	else {    
    //only send to upper layer if connected
		if (state_ == MAC802_16_CONNECTED) {
			update_throughput (&rx_data_watch_, 8*ch->size());    
			update_throughput (&rx_traffic_watch_, 8*ch->size());
			ch->size() -= HDR_MAC802_16_SIZE;
			sendUp(pktRx_, 0);
		}
		else {
      //update drop stat, could be used to detect disconnect
			update_watch (&loss_watch_, 1);
			Packet::free(pktRx_);
			pktRx_=NULL;
			return;
		}
	}

	update_watch (&loss_watch_, 0);
	pktRx_=NULL;
}

/**
 * Process a MAC packet
 * @param p The MAC packet received
 */
void Mac802_16SS::process_mac_packet (Packet *p) {

	assert (HDR_CMN(p)->ptype()==PT_MAC);
	debug2 ("SS %d received MAC packet to process\n", addr());
  
	hdr_mac802_16 *wimaxHdr = HDR_MAC802_16(p);
	gen_mac_header_t header = wimaxHdr->header;

	if (header.ht == 1) {
		debug ("SS %d received bandwitdh request packet..don't process\n", addr());
		return;
	}

  //we cast to this frame because all management frame start with a type 
	mac802_16_dl_map_frame *frame = (mac802_16_dl_map_frame*) p->accessdata();

	switch (frame->type) {
		case MAC_DL_MAP: 
			getMap()->setStarttime (NOW-HDR_CMN(p)->txtime());
    //printf ("At %f frame start at %f\n", NOW, mac_->getMap()->getStarttime());
			process_dl_map (frame);
			break;
		case MAC_DCD: 
			process_dcd ((mac802_16_dcd_frame*)frame);
			break;
		case MAC_UL_MAP: 
			process_ul_map ((mac802_16_ul_map_frame*)frame);
			break;
		case MAC_UCD: 
			process_ucd ((mac802_16_ucd_frame*)frame);
			break;
		case MAC_RNG_RSP:
			process_ranging_rsp ((mac802_16_rng_rsp_frame*) frame);
			break;
		case MAC_REG_RSP:
			process_reg_rsp ((mac802_16_reg_rsp_frame*) frame);
			break;    
		case MAC_MOB_SCN_RSP:
			process_scan_rsp ((mac802_16_mob_scn_rsp_frame *) frame);
			break;
		case MAC_MOB_BSHO_RSP:
			process_bsho_rsp ((mac802_16_mob_bsho_rsp_frame *) frame);
			break;
		case MAC_MOB_NBR_ADV:
			process_nbr_adv ((mac802_16_mob_nbr_adv_frame *) frame);
			break;
		case MAC_DSA_REQ: 
		case MAC_DSA_RSP: 
		case MAC_DSA_ACK: 
			serviceFlowHandler_->process (pktRx_);
			break;
		default:
			debug ("unknown packet in SS %d\n", addr());
    //exit (0);
	}
}

/**
 * Process a DL_MAP message
 * @param frame The dl_map information
 */
void Mac802_16SS::process_dl_map (mac802_16_dl_map_frame *frame)
{
	assert (frame);
  
  //create an entry for the BS
	if (getPeerNode_head ()==NULL)
		addPeerNode (new PeerNode (frame->bsid));

	getMap()->parseDLMAPframe (frame);

	if (getMacState()==MAC802_16_WAIT_DL_SYNCH) {
		debug ("At %f in %d, received DL_MAP for synch from %d (substate=%d)\n", 
		       NOW, addr(), frame->bsid,scan_info_->substate);
		assert (t21timer_->busy()!=0);
    //synchronization is done
		t21timer_->stop();
    //start lost_dl_map
		lostDLMAPtimer_->start (macmib_.lost_dlmap_interval);
    //start T1: DCD
		t1timer_->start (macmib_.t1_timeout);
    //start T12: UCD
		t12timer_->start (macmib_.t12_timeout);

#ifdef USE_802_21
		if (scan_info_->substate != SCANNING) {
			debug ("At %f in Mac %d, send link detected\n", NOW, addr());
			send_link_detected (addr(), frame->bsid, 1);
		}
#endif
    
		setMacState(MAC802_16_WAIT_DL_SYNCH_DCD);

    //if I am doing handoff and we have dcd/ucd information 
    //from scanning, use it
		if (scan_info_->substate == HANDOVER || scan_info_->substate == SCANNING) {
			if (scan_info_->substate == SCANNING) {
				if (scan_info_->nbr == NULL || scan_info_->nbr->getID()!=frame->bsid) {
	  //check if an entry already exist in the database
					scan_info_->nbr = nbr_db_->getNeighbor (frame->bsid);
					if (scan_info_->nbr == NULL) {
	    //create entry
						debug2 ("Creating nbr info for node %d\n", frame->bsid);
						scan_info_->nbr = new WimaxNeighborEntry (frame->bsid);
						nbr_db_->addNeighbor (scan_info_->nbr);
					} else {
						debug2 ("loaded nbr info\n");
						if (scan_info_->nbr->isDetected ()) {
 	      //we already synchronized with this AP...skip channel
							nextChannel();
							lost_synch ();
							return;
						} 
					}
				}
			}//if HANDOVER, scan_info_->nbr is already set

			bool error = false;
      //we check if we can read the DL_MAP
			mac802_16_dcd_frame *dcd = scan_info_->nbr->getDCD();
			if (dcd!=NULL) {
				debug2 ("Check if we can decode stored dcd\n");
	//check if we can decode dl_map with previously acquired dcd      
				bool found;
				for (int i = 0 ; !error && i < getMap()->getDlSubframe()->getPdu()->getNbBurst() ; i++) {
					int diuc = getMap()->getDlSubframe()->getPdu()->getBurst(i)->getIUC();
					if (diuc == DIUC_END_OF_MAP)
						continue;
					found = false;
					for (u_int32_t j = 0 ; !found && j < dcd->nb_prof; j++) {
						found = dcd->profiles[j].diuc==diuc;	    
					}
					error = !found;
				}
				if (!error)
					process_dcd (dcd);
			} else {
				debug2 ("No DCD information found\n");
			}
		}
	} else {
    //maintain synchronization
		assert (lostDLMAPtimer_->busy());
		lostDLMAPtimer_->stop();
    //printf ("update dlmap timer\n");
		lostDLMAPtimer_->start (macmib_.lost_dlmap_interval);

		if (getMacState()!= MAC802_16_WAIT_DL_SYNCH_DCD
				  && getMacState()!=MAC802_16_UL_PARAM) {

      //since the map may have changed, we need to adjust the timer 
      //for the DLSubframe
			double stime = getMap()->getStarttime();
			stime += getMap()->getDlSubframe()->getPdu()->getBurst(1)->getStarttime()*getPhy()->getSymbolTime();
      //printf ("received dl..needs to update expiration to %f, %f,%f\n", stime, NOW,getMap()->getStarttime());
			getMap()->getDlSubframe()->getTimer()->resched (stime-NOW);
			dl_timer_->resched (getMap()->getStarttime()+getFrameDuration()-NOW);
				  }
	}
}

/**
 * Process a DCD message
 * @param frame The dcd information
 */
void Mac802_16SS::process_dcd (mac802_16_dcd_frame *frame)
{
	if (getMacState()==MAC802_16_WAIT_DL_SYNCH) {
    //we are waiting for DL_MAP, ignore this message
		return;
	}

	getMap()->parseDCDframe (frame);
	if (getMacState()==MAC802_16_WAIT_DL_SYNCH_DCD) {
		debug ("At %f in %d, received DCD for synch\n", NOW, addr());
    //now I have all information such as frame duration
    //adjust timing in case the frame we received the DL_MAP
    //and the DCD is different
		while (NOW - getMap()->getStarttime () > getFrameDuration()) {
			getMap()->setStarttime (getMap()->getStarttime()+getFrameDuration());
		}
    
    //store information to be used during potential handoff
		if (scan_info_->substate == SCANNING) {
			mac802_16_dcd_frame *tmp = (mac802_16_dcd_frame *) malloc (sizeof (mac802_16_dcd_frame));
			memcpy (tmp, frame, sizeof (mac802_16_dcd_frame));
			mac802_16_dcd_frame *old = scan_info_->nbr->getDCD(); 
			if (frame == old)
				frame = tmp;
			if (old)
				free (old); //free previous entry
			scan_info_->nbr->setDCD(tmp);    //set new one
		}

		setMacState(MAC802_16_UL_PARAM);
    //we can schedule next frame
    //printf ("SS schedule next frame at %f\n", getMap()->getStarttime()+getFrameDuration());
    //dl_timer_->sched (getMap()->getStarttime()+getFrameDuration()-NOW);
	}

	if (t1timer_->busy()!=0) {
    //we were waiting for this packet
		t1timer_->stop();
		t1timer_->start (macmib_.t1_timeout);
	}
}

/**
 * Process a UCD message
 * @param frame The ucd information
 */
void Mac802_16SS::process_ucd (mac802_16_ucd_frame *frame)
{
	if (getMacState()==MAC802_16_WAIT_DL_SYNCH
		   ||getMacState()==MAC802_16_WAIT_DL_SYNCH_DCD) {
    //discard the packet
		return;
		   }
		   assert (t12timer_->busy()!=0); //we are waiting for this packet

		   if (getMacState()==MAC802_16_UL_PARAM) {
    //check if uplink channel usable
			   debug ("At %f in %d, received UL(UCD) parameters\n", NOW, addr());
    //start T2: ranging
			   t2timer_->start (macmib_.t2_timeout);
    //start Lost UL-MAP
			   lostULMAPtimer_->start (macmib_.lost_ulmap_interval);

    //store information to be used during potential handoff
			   if (scan_info_->substate == SCANNING) {
				   mac802_16_ucd_frame *tmp = (mac802_16_ucd_frame *) malloc (sizeof (mac802_16_ucd_frame));
				   memcpy (tmp, frame, sizeof (mac802_16_ucd_frame));
				   mac802_16_ucd_frame *old = scan_info_->nbr->getUCD(); 
				   if (frame == old)
					   frame = tmp;
				   if (old) 
					   free (old); //free previous entry
				   scan_info_->nbr->setUCD(tmp);    //set new one            
      
			   }

    //change state
			   setMacState (MAC802_16_RANGING);
		   }

  //reset T12
		   t12timer_->stop();
		   t12timer_->start (macmib_.t12_timeout);

		   getMap()->parseUCDframe (frame);
}

/**
 * Process a UL_MAP message
 * @param frame The ul_map information
 */
void Mac802_16SS::process_ul_map (mac802_16_ul_map_frame *frame)
{
	if (getMacState()==MAC802_16_WAIT_DL_SYNCH 
		   || getMacState()==MAC802_16_WAIT_DL_SYNCH_DCD) {
    //discard the packet
		return;
		   }

  //debug ("At %f in %d, received UL_MAP for synch (substate=%d)\n",
  // NOW, addr(),scan_info_->substate);


		   if (getMacState()==MAC802_16_UL_PARAM) {
			   if (scan_info_->substate == HANDOVER || scan_info_->substate==SCANNING) {
				   FrameMap *tmpMap = new FrameMap (this);
				   tmpMap->parseULMAPframe (frame); 
      //printf ("Checking if we can read UL_MAP\n");
				   bool error = false;
      //we check if we can read the UL_MAP
				   mac802_16_ucd_frame *ucd = scan_info_->nbr->getUCD();
				   if (ucd!=NULL) {
	//check if we can decode ul_map with previously acquired ucd      
					   bool found;
					   for (int i = 0 ; !error && i < tmpMap->getUlSubframe()->getNbPdu() ; i++) {
						   UlBurst *b = (UlBurst*)tmpMap->getUlSubframe()->getPhyPdu(i)->getBurst(0);
						   int uiuc = b->getIUC();
						   if (uiuc == UIUC_END_OF_MAP)
							   continue;
						   if (uiuc == UIUC_EXT_UIUC && b->getExtendedUIUC ()== UIUC_FAST_RANGING)
							   uiuc = b->getFastRangingUIUC();	  
						   found = false;
						   for (u_int32_t j = 0 ; !found && j < ucd->nb_prof; j++) {
	    //printf ("\t prof=%d, search=%d\n", ucd->profiles[j].uiuc, uiuc);
							   found = ucd->profiles[j].uiuc==uiuc;	    
						   }
						   error = !found;
					   }
					   if (!error)
						   process_ucd (ucd);
				   }
				   delete (tmpMap);
				   if (error) {
	//we cannot read message
					   return;
				   }
			   } else
				   return;
		   }

		   if (scan_info_->substate == SCANNING) {
    //TBD: add checking scanning type for the given station
			   u_char scanning_type = 0;
			   for (int i = 0 ; i < scan_info_->rsp->n_recommended_bs_full ; i++) {
				   if (scan_info_->rsp->rec_bs_full[i].recommended_bs_id == scan_info_->nbr->getID()) {
					   scanning_type = scan_info_->rsp->rec_bs_full[i].scanning_type;
					   break;
				   }
			   }
			   if (scanning_type == 0) {
      //store information about possible base station and keep scanning
				   scan_info_->nbr->getState()->state_info= backup_state();
				   debug ("At %f in Mac %d bs %d detected during scanning\n", NOW, addr(), scan_info_->nbr->getID());
				   scan_info_->nbr->setDetected (true);
				   nextChannel();
				   lost_synch ();
				   return;
			   }
		   }

		   getMap()->parseULMAPframe (frame);  
		   if (getMacState()==MAC802_16_RANGING) {
      //execute ranging
			   assert (t2timer_->busy()!=0); //we are waiting for this packet
			   init_ranging ();
		   }

  //schedule when to take care of outgoing packets
		   double start = getMap()->getStarttime();
		   start += getMap()->getUlSubframe()->getStarttime()*getPhy()->getPS(); //offset for ul subframe
		   start -= NOW; //works with relative time not absolute
		   debug2 ("Uplink starts in %f (framestate=%f) %f %f\n", 
			   start, 
      getMap()->getStarttime(),
	     getFrameDuration()/getPhy()->getPS(), 
			      getFrameDuration()/getPhy()->getSymbolTime());
  
		   ul_timer_->resched (start);

  //reset Lost UL-Map
		   lostULMAPtimer_->stop();
		   lostULMAPtimer_->start (macmib_.lost_ulmap_interval);
}

/**
 * Process a ranging response message 
 * @param frame The ranging response frame
 */
void Mac802_16SS::process_ranging_rsp (mac802_16_rng_rsp_frame *frame)
{
  //check the destination
	if (frame->ss_mac_address != addr())
		return;
  
	Connection *basic, *primary;
	PeerNode *peer;

  //TBD: add processing for periodic ranging

  //check status 
	switch (frame->rng_status) {
		case RNG_SUCCESS:
			debug ("Ranging response: status = Success.Basic=%d, Primary=%d\n",
			       frame->basic_cid, frame->primary_cid);

			peer = getPeerNode_head();
			assert (peer);
			getMap()->getUlSubframe()->getRanging()->removeRequest ();

			if (scan_info_->substate == SCANNING) {
      //store information about possible base station and keep scanning
				scan_info_->nbr->getState()->state_info= backup_state();
				scan_info_->nbr->setDetected (true);
      //keep the information for later
				mac802_16_rng_rsp_frame *tmp = (mac802_16_rng_rsp_frame *) malloc (sizeof (mac802_16_rng_rsp_frame));
				memcpy (tmp, frame, sizeof (mac802_16_rng_rsp_frame));
				scan_info_->nbr->setRangingRsp (tmp);
				nextChannel();
				lost_synch ();
				return;
			}

    //ranging worked, now we must register
			basic = peer->getBasic(IN_CONNECTION);
			primary = peer->getPrimary(IN_CONNECTION);
			if (basic!=NULL && basic->get_cid ()==frame->basic_cid) {
      //duplicate response
				assert (primary->get_cid () == frame->primary_cid);
			} else {
				if (basic !=NULL) {
	//we have been allocated new cids..clear old ones
					getCManager ()->remove_connection (basic->get_cid());
					getCManager ()->remove_connection (primary->get_cid());
					if (peer->getSecondary(IN_CONNECTION)!=NULL) {
						getCManager ()->remove_connection (peer->getSecondary(IN_CONNECTION));
						getCManager ()->remove_connection (peer->getSecondary(OUT_CONNECTION));
					}
					if (peer->getOutData()!=NULL)
						getCManager ()->remove_connection (peer->getOutData());
					if (peer->getInData()!=NULL)
						getCManager ()->remove_connection (peer->getInData());
				} 

				basic = new Connection (CONN_BASIC, frame->basic_cid);
				Connection *upbasic = new Connection (CONN_BASIC, frame->basic_cid);
				primary = new Connection (CONN_PRIMARY, frame->primary_cid);
				Connection *upprimary = new Connection (CONN_PRIMARY, frame->primary_cid);

      //a SS should only have one peer, the BS
				peer->setBasic (basic, upbasic); //set outgoing
				peer->setPrimary (primary, upprimary); //set outgoing
				getCManager()->add_connection (upbasic, OUT_CONNECTION);
				getCManager()->add_connection (basic, IN_CONNECTION);
				getCManager()->add_connection (upprimary, OUT_CONNECTION);
				getCManager()->add_connection (primary, IN_CONNECTION);
			}

    //registration must be sent using Primary Management CID
			setMacState (MAC802_16_REGISTER);
    //stop timeout timer
			t2timer_->stop ();
			nb_reg_retry_ = 0; //first time sending
			send_registration();

			break;
		case RNG_ABORT:
		case RNG_CONTINUE:
		case RNG_RERANGE:
			break;
		default:
			fprintf (stderr, "Unknown status reply\n");
			exit (-1);
	}
}

/**
 * Schedule a ranging
 */
void Mac802_16SS::init_ranging ()
{
  //check if there is a ranging opportunity
	UlSubFrame *ulsubframe = getMap()->getUlSubframe();
	DlSubFrame *dlsubframe = getMap()->getDlSubframe();

  // If I am doing a Handoff, check if I already associated 
  // with the target AP
	if (scan_info_->substate == HANDOVER && scan_info_->nbr->getRangingRsp()!=NULL) {
		debug ("At %f in Mac %d MN already executed ranging during scanning\n", NOW, addr());
		process_ranging_rsp (scan_info_->nbr->getRangingRsp());
		return;
	}

  //check if there is Fast Ranging IE
	for (PhyPdu *p = getMap()->getUlSubframe ()->getFirstPdu(); p ; p= p ->next_entry()) {
		UlBurst *b = (UlBurst*) p->getBurst(0);
		if (b->getIUC() == UIUC_EXT_UIUC && 
				  b->getExtendedUIUC ()== UIUC_FAST_RANGING &&
				  b->getFastRangingMacAddr ()==addr()) {
			debug2 ("Found fast ranging\n");
      //we should put the ranging request in that burst
			Packet *p= getPacket();
			hdr_cmn* ch = HDR_CMN(p);
			HDR_MAC802_16(p)->header.cid = INITIAL_RANGING_CID;

			p->allocdata (sizeof (struct mac802_16_rng_req_frame));
			mac802_16_rng_req_frame *frame = (mac802_16_rng_req_frame*) p->accessdata();
			frame->type = MAC_RNG_REQ;
			frame->dc_id = dlsubframe->getChannelID();
			frame->ss_mac_address = addr();
      //other elements??      
			frame->req_dl_burst_profile = default_diuc_ & 0xF; //we use lower bits only
			ch->size() += RNG_REQ_SIZE;
      //compute when to send message
			double txtime = getPhy()->getTrxTime (ch->size(), ulsubframe->getProfile (b->getFastRangingUIUC ())->getEncoding());
			ch->txtime() = txtime;
      //starttime+backoff
			ch->timestamp() = NOW; //add timestamp since it bypasses the queue
			b->enqueue(p);
			setMacState(MAC802_16_WAIT_RNG_RSP);
			return;
				  }
	}


	for (PhyPdu *pdu = ulsubframe->getFirstPdu(); pdu ; pdu = pdu->next_entry()) {
		if (pdu->getBurst(0)->getIUC()==UIUC_INITIAL_RANGING) {
			debug ("At %f SS Mac %d found ranging opportunity\n", NOW, addr());
			Packet *p= getPacket();
			hdr_cmn* ch = HDR_CMN(p);
			HDR_MAC802_16(p)->header.cid = INITIAL_RANGING_CID;

			p->allocdata (sizeof (struct mac802_16_rng_req_frame));
			mac802_16_rng_req_frame *frame = (mac802_16_rng_req_frame*) p->accessdata();
			frame->type = MAC_RNG_REQ;
			frame->dc_id = dlsubframe->getChannelID();
			frame->ss_mac_address = addr();
      //other elements??      
			frame->req_dl_burst_profile = default_diuc_ & 0xF; //we use lower bits only
			ch->size() += RNG_REQ_SIZE;
      //compute when to send message
			double txtime = getPhy()->getTrxTime (ch->size(), ulsubframe->getProfile (pdu->getBurst(0)->getIUC())->getEncoding());
			ch->txtime() = txtime;
      //starttime+backoff
			getMap()->getUlSubframe()->getRanging()->addRequest (p);
			setMacState(MAC802_16_WAIT_RNG_RSP);

			return;
		}
	}
}

/**
 * Prepare to send a registration message
 */
void Mac802_16SS::send_registration ()
{
	Packet *p;
	struct hdr_cmn *ch;
	hdr_mac802_16 *wimaxHdr;
	mac802_16_reg_req_frame *reg_frame;
	PeerNode *peer;

  //create packet for request
	p = getPacket ();
	ch = HDR_CMN(p);
	wimaxHdr = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct mac802_16_reg_req_frame));
	reg_frame = (mac802_16_reg_req_frame*) p->accessdata();
	reg_frame->type = MAC_REG_REQ;
	ch->size() += REG_REQ_SIZE;

	peer = getPeerNode_head();  
	wimaxHdr->header.cid = peer->getPrimary(OUT_CONNECTION)->get_cid();
	peer->getPrimary(OUT_CONNECTION)->enqueue (p);

  //start reg timeout
	if (t6timer_==NULL) {
		t6timer_ = new WimaxT6Timer (this);
	}
	t6timer_->start (macmib_.t6_timeout);
	nb_reg_retry_++;
}

/**
 * Process a registration response message 
 * @param frame The registration response frame
 */
void Mac802_16SS::process_reg_rsp (mac802_16_reg_rsp_frame *frame)
{
  //check the destination
	PeerNode *peer = getPeerNode_head();

	if (frame->response == 0) {
    //status OK
		debug ("At %f in Mac %d, registration sucessful (nbretry=%d)\n", NOW, addr(),
		       nb_reg_retry_);
		Connection *secondary = peer->getSecondary(IN_CONNECTION);
		if (!secondary) {
			Connection *secondary = new Connection (CONN_SECONDARY, frame->sec_mngmt_cid);
			Connection *upsecondary = new Connection (CONN_SECONDARY, frame->sec_mngmt_cid);
			getCManager()->add_connection (upsecondary, OUT_CONNECTION);
			getCManager()->add_connection (secondary, IN_CONNECTION);
			peer->setSecondary (secondary, upsecondary);
		}
    //cancel timeout
		t6timer_->stop ();

    //update status
		setMacState(MAC802_16_CONNECTED);

    //we need to setup a data connection (will be moved to service flow handler)
		getServiceHandler ()->sendFlowRequest (peer->getAddr(), OUT_CONNECTION);
		getServiceHandler ()->sendFlowRequest (peer->getAddr(), IN_CONNECTION);

#ifdef USE_802_21
		if (scan_info_->substate==HANDOVER) {
			debug ("At %f in Mac %d link handoff complete\n", NOW, addr());      
			send_link_handover_complete (addr(), scan_info_->serving_bsid, peer->getAddr());
			scan_info_->handoff_timeout = -1;
		}
		debug ("At %f in Mac %d, send link up\n", NOW, addr());
		send_link_up (addr(), peer->getAddr(), -1);
#endif
    
	} else {
    //status failure
		debug ("At %f in Mac %d, registration failed (nbretry=%d)\n", NOW, addr(),
		       nb_reg_retry_);
		if (nb_reg_retry_ == macmib_.reg_req_retry) {
#ifdef USE_802_21
			if (scan_info_ && scan_info_->handoff_timeout == -2) {
				debug ("At %f in Mac %d link handoff failure\n", NOW, addr());      
	//send_link_handoff_failure (addr(), scan_info_->serving_bsid, peer->getAddr());
				scan_info_->handoff_timeout = -1;
			}
#endif
			lost_synch ();
		} else {
			send_registration();
		}
	}
}

/**
 * Send a scanning message to the serving BS
 */
void Mac802_16SS::send_scan_request ()
{
	Packet *p;
	struct hdr_cmn *ch;
	hdr_mac802_16 *wimaxHdr;
	mac802_16_mob_scn_req_frame *req_frame;
	PeerNode *peer;

  //if the mac is not connected, cannot send the request
	if (getMacState() != MAC802_16_CONNECTED) {
		debug ("At %f in Mac %d scan request invalid because MAC is disconnected\n", NOW, addr());
		return;
	}


	debug ("At %f in Mac %d enqueue scan request\n", NOW, addr());

  //create packet for request
	p = getPacket ();
	ch = HDR_CMN(p);
	wimaxHdr = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct mac802_16_mob_scn_req_frame));
	req_frame = (mac802_16_mob_scn_req_frame*) p->accessdata();
	req_frame->type = MAC_MOB_SCN_REQ;

	req_frame->scan_duration = macmib_.scan_duration;
	req_frame->interleaving_interval = macmib_.interleaving;
	req_frame->scan_iteration = macmib_.scan_iteration;
	req_frame->n_recommended_bs_index = 0;
	req_frame->n_recommended_bs_full = 0;

	ch->size() += Mac802_16pkt::getMOB_SCN_REQ_size(req_frame);
	peer = getPeerNode_head();  
	wimaxHdr->header.cid = peer->getPrimary(OUT_CONNECTION)->get_cid();
	peer->getPrimary(OUT_CONNECTION)->enqueue (p);

  //start reg timeout
	if (t44timer_==NULL) {
		t44timer_ = new WimaxT44Timer (this);
	}
	t44timer_->start (macmib_.t44_timeout);
	nb_scan_req_++;
}

/**
 * Process a scanning response message 
 * @param frame The scanning response frame
 */
void Mac802_16SS::process_scan_rsp (mac802_16_mob_scn_rsp_frame *frame)
{
  //PeerNode *peer = getPeerNode_head();
	if (!t44timer_->busy()) {
    //we are receiving the response too late..ignore
		debug ("At %f in Mac %d, scan response arrives too late\n", NOW, addr());
		return;
	}


	if (frame->scan_duration != 0) {
    //scanning accepted
		debug ("At %f in Mac %d, scanning accepted (dur=%d it=%d)\n", NOW, addr(), frame->scan_duration,frame->scan_iteration );
    //allocate data for scanning
    //scan_info_ = (struct scanning_structure *) malloc (sizeof (struct scanning_structure));
    //store copy of frame
    
		scan_info_->rsp = (struct mac802_16_mob_scn_rsp_frame *) malloc (sizeof (struct mac802_16_mob_scn_rsp_frame));
		memcpy (scan_info_->rsp, frame, sizeof (struct mac802_16_mob_scn_rsp_frame));
		scan_info_->iteration = 0;
		scan_info_->count = frame->start_frame;
		scan_info_->substate = SCAN_PENDING;
		scan_info_->handoff_timeout = 0; 
		scan_info_->serving_bsid = getPeerNode_head()->getAddr();
		scan_info_->nb_rdv_timers = 0;

    //mark all neighbors as not detected
		for (int i = 0 ; i < nbr_db_->getNbNeighbor() ; i++) {
			nbr_db_->getNeighbors()[i]->setDetected(false);
		}

    //schedule timer for rdv time (for now just use full)
    //TBD: add rec_bs_index
		debug ("\tstart scan in %d frames (%f)\n",frame->start_frame,NOW+frame->start_frame*getFrameDuration());
		for (int i = 0 ; i < scan_info_->rsp->n_recommended_bs_full ; i++) {
			if (scan_info_->rsp->rec_bs_full[i].scanning_type ==SCAN_ASSOC_LVL1 
						 || scan_info_->rsp->rec_bs_full[i].scanning_type==SCAN_ASSOC_LVL2) {
				debug2 ("Creating timer for bs=%d at time %f\n", 
					scan_info_->rsp->rec_bs_full[i].recommended_bs_id, 
     NOW+getFrameDuration()*scan_info_->rsp->rec_bs_full[i].rdv_time);
				assert (nbr_db_->getNeighbor (scan_info_->rsp->rec_bs_full[i].recommended_bs_id));
	//get the channel
				int ch = getChannel (nbr_db_->getNeighbor (scan_info_->rsp->rec_bs_full[i].recommended_bs_id)->getDCD ()->frequency*1000);
				assert (ch!=-1);
				WimaxRdvTimer *timer = new WimaxRdvTimer (this, ch);
				scan_info_->rdv_timers[scan_info_->nb_rdv_timers++] = timer;
				timer->start(getFrameDuration()*scan_info_->rsp->rec_bs_full[i].rdv_time);
						 }
		}

	} else {
		debug ("At %f in Mac %d, scanning denied\n", NOW, addr());
    //what do I do???
	}

	t44timer_->stop();
	nb_scan_req_ = 0;
}

/**
 * Send a MSHO-REQ message to the BS
 */
void Mac802_16SS::send_msho_req ()
{
	Packet *p;
	struct hdr_cmn *ch;
	hdr_mac802_16 *wimaxHdr;
	mac802_16_mob_msho_req_frame *req_frame;
	double rssi;

	PeerNode *peer = getPeerNode_head();

	int nbPref = 0;
	for (int i = 0 ; i < nbr_db_->getNbNeighbor() ; i++) {
		WimaxNeighborEntry *entry = nbr_db_->getNeighbors()[i];
		if (entry->isDetected()) {
			debug ("At %f in Mac %d Found new AP %d..need to send HO message\n",NOW, addr(), entry->getID());
			nbPref++;
		}  
	}

	if (nbPref==0)
		return; //no other BS found

  //create packet for request
	p = getPacket ();
	ch = HDR_CMN(p);
	wimaxHdr = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct mac802_16_mob_msho_req_frame)+nbPref*sizeof (mac802_16_mob_msho_req_bs_index));
	req_frame = (mac802_16_mob_msho_req_frame*) p->accessdata();
	memset (req_frame, 0, sizeof (mac802_16_mob_msho_req_bs_index));
	req_frame->type = MAC_MOB_MSHO_REQ;
  
	req_frame->report_metric = 0x2; //include RSSI
	req_frame->n_new_bs_index = 0;
	req_frame->n_new_bs_full = nbPref;
	req_frame->n_current_bs = 1;
	rssi = getPeerNode_head()->getStatWatch()->average();
	debug2 ("RSSI=%e, %d, bs=%d\n", rssi, (u_char)((rssi+103.75)/0.25), getPeerNode_head()->getAddr());
	req_frame->bs_current[0].temp_bsid = getPeerNode_head()->getAddr();
	req_frame->bs_current[0].bs_rssi_mean = (u_char)((rssi+103.75)/0.25);
	for (int i = 0, j=0; i < nbr_db_->getNbNeighbor() ; i++) {
		WimaxNeighborEntry *entry = nbr_db_->getNeighbors()[i];
    //TBD: there is an error measuring RSSI for current BS during scanning
    //anyway, we don't put it in the least, so it's ok for now
		if (entry->isDetected() && entry->getID()!= getPeerNode_head()->getAddr()) {
			req_frame->bs_full[j].neighbor_bs_index = entry->getID();
			rssi = entry->getState()->state_info->peer_list->lh_first->getStatWatch()->average();
			debug2 ("RSSI=%e, %d, bs=%d\n", rssi, (u_char)((rssi+103.75)/0.25), entry->getID());
			req_frame->bs_full[j].bs_rssi_mean = (u_char)((rssi+103.75)/0.25);
      //the rest of req_frame->bs_full is unused for now..
			req_frame->bs_full[j].arrival_time_diff_ind = 0;
			j++;
		}
	}
  
	ch->size() += Mac802_16pkt::getMOB_MSHO_REQ_size(req_frame);
	wimaxHdr->header.cid = peer->getPrimary(OUT_CONNECTION)->get_cid();
	peer->getPrimary(OUT_CONNECTION)->enqueue (p);
}

/**
 * Process a BSHO-RSP message 
 * @param frame The handover response frame
 */
void Mac802_16SS::process_bsho_rsp (mac802_16_mob_bsho_rsp_frame *frame)
{
	debug ("At %f in Mac %d, received handover response\n", NOW, addr());
 
  //go and switch to the channel recommended by the BS
	int targetBS = frame->n_rec[0].neighbor_bsid;
	PeerNode *peer = getPeerNode_head();      

	if (peer->getAddr ()==targetBS) {
		debug ("\tDecision to stay in current BS\n");
		return;
	}
	scan_info_->nbr = nbr_db_->getNeighbor (targetBS);

	Packet *p;
	struct hdr_cmn *ch;
	hdr_mac802_16 *wimaxHdr;
	mac802_16_mob_ho_ind_frame *ind_frame;
  
  
	p = getPacket ();
	ch = HDR_CMN(p);
	wimaxHdr = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct mac802_16_mob_ho_ind_frame));
	ind_frame = (mac802_16_mob_ho_ind_frame*) p->accessdata();
	ind_frame->type = MAC_MOB_HO_IND;
  
	ind_frame->mode = 0; //HO
	ind_frame->ho_ind_type = 0; //Serving BS release
	ind_frame->rng_param_valid_ind = 0;
	ind_frame->target_bsid = targetBS;
  
	ch->size() += Mac802_16pkt::getMOB_HO_IND_size(ind_frame);
	wimaxHdr->header.cid = peer->getPrimary(OUT_CONNECTION)->get_cid();
	peer->getPrimary(OUT_CONNECTION)->enqueue (p);
  
#ifdef USE_802_21
	send_link_handover_imminent (addr(), peer->getAddr(), targetBS);
	debug ("At %f in Mac %d link handover imminent\n", NOW, addr());
#endif 
  
	debug ("\tHandover to BS %d\n", targetBS);
	scan_info_->handoff_timeout = 20;
	scan_info_->substate = HANDOVER_PENDING;
  
  //setChannel (scan_info_->bs_infos[i].channel);
  //lost_synch ();  
}


/**
 * Process a NBR_ADV message 
 * @param frame The handover response frame
 */
void Mac802_16SS::process_nbr_adv (mac802_16_mob_nbr_adv_frame *frame)
{
	debug ("At %f in Mac %d, received neighbor advertisement\n", NOW, addr());

  //mac802_16_mob_nbr_adv_frame *copy;
  //copy  = (mac802_16_mob_nbr_adv_frame *) malloc (sizeof (mac802_16_mob_nbr_adv_frame));
  //memcpy (copy, frame, sizeof (mac802_16_mob_nbr_adv_frame));
  
  //all we need is to store the information. We will process that only
  //when we will look for another station
	for (int i = 0 ; i < frame->n_neighbors ; i++) {
		int nbrid = frame->nbr_info[i].nbr_bsid;
		mac802_16_nbr_adv_info *info = (mac802_16_nbr_adv_info *) malloc (sizeof(mac802_16_nbr_adv_info));
		WimaxNeighborEntry *entry = nbr_db_->getNeighbor (nbrid);
		if (entry==NULL){
			entry = new WimaxNeighborEntry (nbrid);
			nbr_db_->addNeighbor (entry);
		}
		memcpy(info, &(frame->nbr_info[i]), sizeof(mac802_16_nbr_adv_info));
		if (entry->getNbrAdvMessage ())
			free (entry->getNbrAdvMessage());
		entry->setNbrAdvMessage(info);
		if (info->dcd_included) {
      //set DCD 
			mac802_16_dcd_frame *tmp = (mac802_16_dcd_frame *)malloc (sizeof(mac802_16_dcd_frame));
			memcpy(tmp, &(info->dcd_settings), sizeof(mac802_16_dcd_frame));
			entry->setDCD(tmp);
		}
		else 
			entry->setDCD(NULL);
		if (info->ucd_included) {
      //set DCD 
			mac802_16_ucd_frame *tmp = (mac802_16_ucd_frame *)malloc (sizeof(mac802_16_ucd_frame));
			memcpy(tmp, &(info->ucd_settings), sizeof(mac802_16_ucd_frame));
			entry->setUCD(tmp);
#ifdef DEBUG_WIMAX
			debug2 ("Dump information nbr in Mac %d for nbr %d %lx\n", addr(), nbrid, (long)tmp);
			int nb_prof = tmp->nb_prof;
			mac802_16_ucd_profile *profiles = tmp->profiles;
			for (int i = 0 ; i < nb_prof ; i++) {
				debug2 ("\t Reading ul profile %i: f=%d, rate=%d, iuc=%d\n", i, 0, profiles[i].fec, profiles[i].uiuc);
			}
#endif
		}
		else
			entry->setUCD(NULL);
	}  

}

/**** Internal methods ****/

#ifdef USE_802_21

/*
 * Connect to the PoA
 */
void Mac802_16SS::link_connect(int poa)
{
	debug ("At %f in Mac %d, received link connect to BS %d\n", NOW, addr(), poa);
 
	set_mode (NORMAL_MODE);

  //go and switch to the channel recommended by the BS
	int targetBS = poa;
	PeerNode *peer = getPeerNode_head();      

	if (peer->getAddr ()==targetBS) {
		debug ("\tDecision to stay in current BS\n");
		return;
	}
	scan_info_->nbr = nbr_db_->getNeighbor (targetBS);

	Packet *p;
	struct hdr_cmn *ch;
	hdr_mac802_16 *wimaxHdr;
	mac802_16_mob_ho_ind_frame *ind_frame;
  
  
	p = getPacket ();
	ch = HDR_CMN(p);
	wimaxHdr = HDR_MAC802_16(p);
	p->allocdata (sizeof (struct mac802_16_mob_ho_ind_frame));
	ind_frame = (mac802_16_mob_ho_ind_frame*) p->accessdata();
	ind_frame->type = MAC_MOB_HO_IND;
  
	ind_frame->mode = 0; //HO
	ind_frame->ho_ind_type = 0; //Serving BS release
	ind_frame->rng_param_valid_ind = 0;
	ind_frame->target_bsid = targetBS;
  
	ch->size() += Mac802_16pkt::getMOB_HO_IND_size(ind_frame);
	wimaxHdr->header.cid = peer->getPrimary(OUT_CONNECTION)->get_cid();
	peer->getPrimary(OUT_CONNECTION)->enqueue (p);
  
#ifdef USE_802_21
	send_link_handover_imminent (addr(), peer->getAddr(), targetBS);
	debug ("At %f in Mac %d link handover imminent\n", NOW, addr());
  
#endif 
  
	debug ("\tHandover to BS %d\n", targetBS);
	scan_info_->handoff_timeout = 20;
	scan_info_->substate = HANDOVER_PENDING;
  //setChannel (scan_info_->bs_infos[i].channel);
  //lost_synch ();    
}


/*
 * Disconnect from the PoA
 */
void Mac802_16SS::link_disconnect ()
{
  //force losing synchronization
	lost_synch ();
	set_mode (POWER_DOWN); //not sure if we should turn it off
}


/*
 * Set the operation mode
 * @param mode The new operation mode
 * @return true if transaction succeded
 */
bool Mac802_16SS::set_mode (mih_operation_mode_t mode)
{
	switch (mode) {
		case NORMAL_MODE:
			if (op_mode_ != NORMAL_MODE) {
				getPhy()->node_on(); //turn on phy
				debug ("Turning on mac\n");
			}
			op_mode_ = mode;
			return true;
			break;
		case POWER_SAVING:
    //not yet supported 
			return false;
			break;
		case POWER_DOWN:
			if (op_mode_ != POWER_DOWN) {
				getPhy()->node_off(); //turn off phy
				debug ("Turning off mac\n");
			}
			op_mode_ = mode;
			return true;
			break;
		default:
			return false;
	}
}

/*
 * Scan chanels
 */
void Mac802_16SS::link_scan (void *req)
{
	if(!isScanRunning()){
		setScanFlag(true);  
		send_scan_request ();
	}
}

#endif

/**
 * Update the given timer and check if thresholds are crossed
 * @param watch the stat watch to update
 * @param value the stat value
 */
void Mac802_16SS::update_watch (StatWatch *watch, double value)
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

		send_link_parameters_report (addr(), getPeerNode_head()->getAddr(), param, old_value, new_value);      
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
void Mac802_16SS::update_throughput (ThroughputWatch *watch, double size)
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

		send_link_parameters_report (addr(), getPeerNode_head()->getAddr(), param, old_value, new_value);      
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
 * Start a new frame
 */
void Mac802_16SS::start_dlsubframe ()
{
	debug2 ("At %f in Mac %d SS scheduler dlsubframe expires %d\n", NOW, addr(), scan_info_->substate);

	frame_number_++;

	switch (scan_info_->substate) {
		case SCAN_PENDING: 
			if (scan_info_->count == 0) {
				resume_scanning();
				return;
			} 
			scan_info_->count--;
			break;
		case HANDOVER_PENDING:
			if (scan_info_->handoff_timeout == 0) {
				assert (scan_info_->nbr);
#ifdef USE_802_21
      //debug ("At %f in Mac %d link handoff proceeding\n", NOW, addr());
      //send_link_handoff_proceeding (addr(), getPeerNode_head()->getAddr(), scan_info_->nbr->getID());
#endif 
				scan_info_->substate = HANDOVER;
      //restore previous state 
      //restore_state (scan_info_->nbr->getState()->state_info);
				setChannel (scan_info_->nbr->getState()->state_info->channel);
				lost_synch ();
      //add target as peer
				addPeerNode (new PeerNode(scan_info_->nbr->getID()));
				return;
			}
			scan_info_->handoff_timeout--;
			break;
		default:
			break;
	}
    
  //this is the begining of new frame
	map_->setStarttime (NOW);

  //start handler of dlsubframe
	map_->getDlSubframe()->getTimer()->sched (0);

  //reschedule for next frame
	dl_timer_->resched (getFrameDuration());
}

/**
 * Start a new frame
 */
void Mac802_16SS::start_ulsubframe ()
{
	debug ("At %f in Mac %d SS scheduler ulsubframe expires\n", NOW, addr());
  
  //change state of PHY: even though it should have been done before
  //there are some cases where it does not (during scanning)
	getPhy()->setMode (OFDM_SEND);

	scheduler_->schedule();

  //start handler for ulsubframe
	if (getMap()->getUlSubframe()->getNbPdu ()>0) {
		Burst *b = getMap()->getUlSubframe()->getPhyPdu (0)->getBurst (0);
		getMap()->getUlSubframe()->getTimer()->sched (b->getStarttime()*getPhy()->getSymbolTime());
	}//else there is no uplink phy pdu defined

  //reschedule for next frame
	ul_timer_->resched (getFrameDuration());     
 
}

/**
 * Called when lost synchronization
 */
void Mac802_16SS::lost_synch ()
{
#ifdef USE_802_21
	int poa = -1;
	bool disconnect = false;
#endif

  //reset timers
	if (t1timer_->busy()!=0)
		t1timer_->stop();
	if (t12timer_->busy()!=0)
		t12timer_->stop();
	if (t21timer_->busy()!=0)
		t21timer_->stop();
	if (lostDLMAPtimer_->busy()!=0)
		lostDLMAPtimer_->stop(); 
	if (lostULMAPtimer_->busy()!=0)
		lostULMAPtimer_->stop(); 
	if (t2timer_->busy()!=0)
		t2timer_->stop(); 
	if (t44timer_ && t44timer_->busy()!=0)
		t44timer_->stop();

  //we need to go to receiving mode
  //printf ("Set phy to recv %x\n", getPhy());
	getPhy()->setMode (OFDM_RECV);
	if (getMacState()==MAC802_16_CONNECTED) {
    //remove possible pending requests
		map_->getUlSubframe()->getBw_req()->removeRequests(); 

#ifdef USE_802_21
		poa = getPeerNode_head()->getAddr ();
		disconnect = true;
#endif
	}

  //remove information about peer node
	if (getPeerNode_head())
		removePeerNode (getPeerNode_head());

  //start waiting for DL synch
	setMacState (MAC802_16_WAIT_DL_SYNCH);
	t21timer_->start (macmib_.t21_timeout);
	if (dl_timer_->status()==TIMER_PENDING)
		dl_timer_->cancel();
	map_->getDlSubframe()->getTimer()->reset();
	if (ul_timer_->status()==TIMER_PENDING)
		ul_timer_->cancel();
	map_->getUlSubframe()->getTimer()->reset();

#ifdef USE_802_21
	if (disconnect) {
		debug ("At %f in Mac %d, send link down\n", NOW, addr());
		send_link_down (addr(), poa, LD_RC_FAIL_NORESOURCE);
	}
#endif

	if (scan_info_->substate == HANDOVER_PENDING || scan_info_->substate == SCAN_PENDING) {
    //we have lost synch before scanning/handover is complete
		for (int i=0 ; i < scan_info_->nb_rdv_timers ; i++) {
      //debug ("canceling rdv timer\n");
			if (scan_info_->rdv_timers[i]->busy()) {
				scan_info_->rdv_timers[i]->stop();
			}
			delete (scan_info_->rdv_timers[i]);
		}
		scan_info_->nb_rdv_timers = 0;
	}

	if (scan_info_->substate == HANDOVER_PENDING) {
		debug ("Lost synch during pending handover (%d)\n", scan_info_->handoff_timeout);
    //since we lost connection, let's execute handover immediately 
		scan_info_->substate = HANDOVER;
		setChannel (scan_info_->nbr->getState()->state_info->channel);
    //add target as peer
		addPeerNode (new PeerNode(scan_info_->nbr->getID()));
		return;
	} 
	if (scan_info_->substate == SCAN_PENDING) {
		debug ("Lost synch during pending scan (%d)\n", scan_info_->count);
    //we must cancel the scanning
		scan_info_->substate = NORMAL;
	}
}

/**
 * Start/Continue scanning
 */
void Mac802_16SS::resume_scanning ()
{
	if (scan_info_->iteration == 0) 
		debug ("At %f in Mac %d, starts scanning\n", NOW, addr());
	else 
		debug ("At %f in Mac %d, resume scanning\n", NOW, addr());
  
	scan_info_->substate = SCANNING;

  //backup current state
	scan_info_->normal_state.state_info = backup_state();
	if (t1timer_->busy())
		t1timer_->pause();
	scan_info_->normal_state.t1timer = t1timer_;
	if (t2timer_->busy())
		t2timer_->pause();
	scan_info_->normal_state.t2timer = t2timer_;
	if (t6timer_->busy())
		t6timer_->pause();
	scan_info_->normal_state.t6timer = t6timer_;
	if (t12timer_->busy())
		t12timer_->pause();
	scan_info_->normal_state.t12timer = t12timer_;
	if (t21timer_->busy())
		t21timer_->pause();
	scan_info_->normal_state.t21timer = t21timer_;
	if (lostDLMAPtimer_->busy())
		lostDLMAPtimer_->pause();
	scan_info_->normal_state.lostDLMAPtimer = lostDLMAPtimer_;
	if (lostULMAPtimer_->busy())
		lostULMAPtimer_->pause();
	scan_info_->normal_state.lostULMAPtimer = lostULMAPtimer_;
	scan_info_->normal_state.map = map_;

	if (scan_info_->iteration == 0) {
    //reset state
		t1timer_ = new WimaxT1Timer (this);
		t2timer_ = new WimaxT2Timer (this);
		t6timer_ = new WimaxT6Timer (this);
		t12timer_ = new WimaxT12Timer (this);
		t21timer_ = new WimaxT21Timer (this);
		lostDLMAPtimer_ = new WimaxLostDLMAPTimer (this);
		lostULMAPtimer_ = new WimaxLostULMAPTimer (this);
    
		map_ = new FrameMap (this);
    
		nextChannel();

		scan_info_->scn_timer_ = new WimaxScanIntervalTimer (this);

    //start waiting for DL synch
		setMacState (MAC802_16_WAIT_DL_SYNCH);
		t21timer_->start (macmib_.t21_timeout);
		if (dl_timer_->status()==TIMER_PENDING)
			dl_timer_->cancel();
		map_->getDlSubframe()->getTimer()->reset();
		if (ul_timer_->status()==TIMER_PENDING)
			ul_timer_->cancel();
		map_->getUlSubframe()->getTimer()->reset();
    

	}else{
    //restore where we left
    //restore previous timers
		restore_state(scan_info_->scan_state.state_info);
		t1timer_ = scan_info_->scan_state.t1timer;
		if (t1timer_->paused())
			t1timer_->resume();
		t2timer_ = scan_info_->scan_state.t2timer;
		if (t2timer_->paused())
			t2timer_->resume();
		t6timer_ = scan_info_->scan_state.t6timer;
		if (t6timer_->paused())
			t6timer_->resume();
		t12timer_ = scan_info_->scan_state.t12timer;
		if (t12timer_->paused())
			t12timer_->resume();
		t21timer_ = scan_info_->scan_state.t21timer;
		if (t21timer_->paused())
			t21timer_->resume();
		lostDLMAPtimer_ = scan_info_->scan_state.lostDLMAPtimer;
		if (lostDLMAPtimer_->paused())
			lostDLMAPtimer_->resume();
		lostULMAPtimer_ = scan_info_->scan_state.lostULMAPtimer;
		if (lostULMAPtimer_->paused())
			lostULMAPtimer_->resume();
		map_ = scan_info_->scan_state.map;
    
		getPhy()->setMode (OFDM_RECV);

		if (ul_timer_->status()==TIMER_PENDING)
			ul_timer_->cancel();
	}
	setNotify_upper (false);
  //printf ("Scan duration=%d, frameduration=%f\n", scan_info_->rsp->scan_duration, getFrameDuration());
	scan_info_->scn_timer_->start (scan_info_->rsp->scan_duration*getFrameDuration());
	scan_info_->iteration++;
  
}

/**
 * Pause scanning
 */
void Mac802_16SS::pause_scanning ()
{
	if (scan_info_->iteration < scan_info_->rsp->scan_iteration)
		debug ("At %f in Mac %d, pause scanning\n", NOW, addr());
	else 
		debug ("At %f in Mac %d, stop scanning\n", NOW, addr());

  //return to normal mode
	if (scan_info_->iteration < scan_info_->rsp->scan_iteration) {
    //backup current state
		scan_info_->scan_state.state_info = backup_state();
		if (t1timer_->busy())
			t1timer_->pause();
		scan_info_->scan_state.t1timer = t1timer_;
		if (t2timer_->busy())
			t2timer_->pause();
		scan_info_->scan_state.t2timer = t2timer_;
		if (t6timer_->busy())
			t6timer_->pause();
		scan_info_->scan_state.t6timer = t6timer_;
		if (t12timer_->busy())
			t12timer_->pause();
		scan_info_->scan_state.t12timer = t12timer_;
		if (t21timer_->busy())
			t21timer_->pause();
		scan_info_->scan_state.t21timer = t21timer_;
		if (lostDLMAPtimer_->busy())
			lostDLMAPtimer_->pause();
		scan_info_->scan_state.lostDLMAPtimer = lostDLMAPtimer_;
		if (lostULMAPtimer_->busy())
			lostULMAPtimer_->pause();
		scan_info_->scan_state.lostULMAPtimer = lostULMAPtimer_;
		scan_info_->scan_state.map = map_;

		scan_info_->count = scan_info_->rsp->interleaving_interval;

	} else {
    //else scanning is over, no need to save data
    //reset timers
		if (t1timer_->busy()!=0)
			t1timer_->stop();
		delete (t1timer_);
		if (t12timer_->busy()!=0)
			t12timer_->stop();
		delete (t12timer_);
		if (t21timer_->busy()!=0)
			t21timer_->stop();
		delete (t21timer_);
		if (lostDLMAPtimer_->busy()!=0)
			lostDLMAPtimer_->stop(); 
		delete (lostDLMAPtimer_);
		if (lostULMAPtimer_->busy()!=0)
			lostULMAPtimer_->stop(); 
		delete (lostULMAPtimer_);
		if (t2timer_->busy()!=0)
			t2timer_->stop(); 
		delete (t2timer_);
	}
  //restore previous timers
	restore_state(scan_info_->normal_state.state_info);
	t1timer_ = scan_info_->normal_state.t1timer;
	if (t1timer_->paused())
		t1timer_->resume();
	t2timer_ = scan_info_->normal_state.t2timer;
	if (t2timer_->paused())
		t2timer_->resume();
	t6timer_ = scan_info_->normal_state.t6timer;
	if (t6timer_->paused())
		t6timer_->resume();
	t12timer_ = scan_info_->normal_state.t12timer;
	if (t12timer_->paused())
		t12timer_->resume();
	t21timer_ = scan_info_->normal_state.t21timer;
	if (t21timer_->paused())
		t21timer_->resume();
	lostDLMAPtimer_ = scan_info_->normal_state.lostDLMAPtimer;
	if (lostDLMAPtimer_->paused())
		lostDLMAPtimer_->resume();
	lostULMAPtimer_ = scan_info_->normal_state.lostULMAPtimer;
	if (lostULMAPtimer_->paused())
		lostULMAPtimer_->resume();
	map_ = scan_info_->normal_state.map;

	setNotify_upper (true);
	dl_timer_->resched (0);

	if (scan_info_->iteration == scan_info_->rsp->scan_iteration) {
		scan_info_->substate = NORMAL;
    
		/** here we check if there is a better BS **/
#ifdef USE_802_21
		if(mih_){
			int nbDetected = 0;
			for (int i = 0 ; i < nbr_db_->getNbNeighbor() ; i++) {
				if (nbr_db_->getNeighbors()[i]->isDetected()) {
					nbDetected++;
				}
			}
			int *listOfPoa = new int[nbDetected];
			int itr = 0;
			for (int i = 0 ; i < nbr_db_->getNbNeighbor() ; i++) {
				WimaxNeighborEntry *entry = nbr_db_->getNeighbors()[i];
				if (entry->isDetected()) {
					listOfPoa[itr] = entry->getID();
					itr++;
				}  
			}  
			send_scan_result (listOfPoa, itr*sizeof(int));	
		}else
#endif
		{
			send_msho_req();
		}

		setScanFlag(false);
		scan_info_->count--; //to avoid restarting scanning
	} else {
		scan_info_->substate = SCAN_PENDING;
	}
}

/**
 * Set the scan flag to true/false
 * param flag The value for the scan flag
 */
void Mac802_16SS::setScanFlag(bool flag)
{
	scan_flag_ = flag;
}

/**
 * return scan flag
 * @return the scan flag
 */
bool Mac802_16SS::isScanRunning()
{
	return scan_flag_;
}



