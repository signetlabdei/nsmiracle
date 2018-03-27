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
 * $Id: mac802_16.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "mac802_16.h"
#include "ns2wimax/scheduling/wimaxscheduler.h"
//we use mac 802_11 for trace
#include "mac.h"
#include "mac-802_11.h"

/* Defines frequencies for 3.5 GHz band and 7 Mhz freqency bandwidth */
/* Will be removed when a dynamic way is added */
static const int nbFreq = 5;
static const double frequencies[] = { 3.486e+9, 3.493e+9, 3.5e+9, 3.507e+9, 3.514e+9};

extern ClMessage_t MAC_CLMSG_GET_ADDR;

int hdr_mac802_16::offset_;
/**
 * TCL Hooks for the simulator for wimax packets
 */
static class Mac802_16HeaderClass : public PacketHeaderClass
{
public:
	Mac802_16HeaderClass() : PacketHeaderClass("PacketHeader/802_16",
					     sizeof(hdr_mac802_16))
	{
		bind_offset(&hdr_mac802_16::offset_);
		bind();
	}
} class_hdr_mac802_16;

static class Mac802_16Class : public TclClass {
	public:
		Mac802_16Class() : TclClass("Module/802_16/MAC") {}
		TclObject* create(int, const char*const*) {
			return (new Mac802_16());
		}
} class_mac802_16;

Phy802_16MIB::Phy802_16MIB(Mac802_16 *parent)
{
	parent->bind ("channel_", &channel );
	parent->bind ("fbandwidth_", &fbandwidth );
	parent->bind ("ttg_", &ttg );
	parent->bind ("rtg_", &rtg );
}

/**
 * TCL Hooks for the simulator for wimax mac
 */
Mac802_16MIB::Mac802_16MIB(Mac802_16 *parent)
{
	parent->bind ("queue_length_", &queue_length );
	parent->bind ("frame_duration_", &frame_duration );
	parent->bind ("dcd_interval_", &dcd_interval );
	parent->bind ("ucd_interval_", &ucd_interval );
	parent->bind ("init_rng_interval_", &init_rng_interval );
	parent->bind ("lost_dlmap_interval_", &lost_dlmap_interval );
	parent->bind ("lost_ulmap_interval_", &lost_ulmap_interval );
	parent->bind ("t1_timeout_", &t1_timeout );
	parent->bind ("t2_timeout_", &t2_timeout );
	parent->bind ("t3_timeout_", &t3_timeout );
	parent->bind ("t6_timeout_", &t6_timeout );
	parent->bind ("t12_timeout_", &t12_timeout );
	parent->bind ("t16_timeout_", &t16_timeout );
	parent->bind ("t17_timeout_", &t17_timeout );
	parent->bind ("t21_timeout_", &t21_timeout );
	parent->bind ("contention_rng_retry_", &contention_rng_retry );
	parent->bind ("invited_rng_retry_", &invited_rng_retry );
	parent->bind ("request_retry_", &request_retry );
	parent->bind ("reg_req_retry_", &reg_req_retry );
	parent->bind ("tproc_", &tproc );
	parent->bind ("dsx_req_retry_", &dsx_req_retry );
	parent->bind ("dsx_rsp_retry_", &dsx_rsp_retry );

	parent->bind ("rng_backoff_start_", &rng_backoff_start);
	parent->bind ("rng_backoff_stop_", &rng_backoff_stop);
	parent->bind ("bw_backoff_start_", &bw_backoff_start);
	parent->bind ("bw_backoff_stop_", &bw_backoff_stop);
 
	//mobility extension
	parent->bind ("scan_duration_", &scan_duration );
	parent->bind ("interleaving_interval_", &interleaving );
	parent->bind ("scan_iteration_", &scan_iteration );
	parent->bind ("t44_timeout_", &t44_timeout );
	parent->bind ("max_dir_scan_time_", &max_dir_scan_time );
	parent->bind ("nbr_adv_interval_", &nbr_adv_interval );
	parent->bind ("scan_req_retry_", &scan_req_retry );

	parent->bind ("client_timeout_", &client_timeout );
	parent->bind ("rxp_avg_alpha_", &rxp_avg_alpha);
	parent->bind ("lgd_factor_", &lgd_factor_);
}

/**
 * Creates a Mac 802.16
 * The MAC 802.16 is a superclass for the BS and SS MAC
 * It should not be instanciated
 */
Mac802_16::Mac802_16() : macmib_(this), phymib_(this), rxTimer_(this)
{
	//init variable
	LIST_INIT(&classifier_list_);
	peer_list_ = (struct peerNode *) malloc (sizeof(struct peerNode));
	LIST_INIT(peer_list_);
	nb_peer_=0;

	collision_ = false;
	pktRx_ = NULL;
	pktBuf_ = NULL;

	connectionManager_ = new ConnectionManager (this);
	scheduler_ = NULL;
	/* the following will be replaced by dynamic adding of service flow */
	serviceFlowHandler_ = new ServiceFlowHandler ();
	serviceFlowHandler_->setMac (this);
	frame_number_ = 0;
	notify_upper_ = true;

	last_tx_time_ = 0;
	last_tx_duration_ = 0;

	Tcl& tcl = Tcl::instance();
	tcl.evalf ("Module/MPhy/OFDM set RXThresh_");
	macmib_.RXThreshold_ = atof (tcl.result());
  
	nbr_db_ = new NeighborDB ();
  
#ifdef USE_802_21
	linkType_ = LINK_802_16;
	eventList_ = 0x1BF;
	commandList_ = 0xF;
#endif

	/* Initialize Stats variables */
	bind_bool ("print_stats_", &print_stats_);
	last_tx_delay_ = 0;
	double tmp;
	bind ("delay_avg_alpha_", &tmp);
	delay_watch_.set_alpha(tmp);
	bind ("jitter_avg_alpha_", &tmp);
	jitter_watch_.set_alpha(tmp);
	bind ("loss_avg_alpha_", &tmp);
	loss_watch_.set_alpha(tmp);
	bind ("throughput_avg_alpha_", &tmp);
	rx_data_watch_.set_alpha(tmp);
	rx_data_watch_.set_pos_gradient (false);
	rx_traffic_watch_.set_alpha(tmp);
	rx_traffic_watch_.set_pos_gradient (false);
	tx_data_watch_.set_alpha(tmp);
	tx_data_watch_.set_pos_gradient (false);
	tx_traffic_watch_.set_alpha(tmp);
	tx_traffic_watch_.set_pos_gradient (false);
	bind ("throughput_delay_", &tmp);
	rx_data_watch_.set_delay (tmp);
	rx_traffic_watch_.set_delay (tmp);
	tx_data_watch_.set_delay (tmp);
	tx_traffic_watch_.set_delay (tmp);
  //timers for stats
	rx_data_timer_ = new StatTimer (this, &rx_data_watch_);
	rx_traffic_timer_ = new StatTimer (this, &rx_traffic_watch_);
	tx_data_timer_ = new StatTimer (this, &tx_data_watch_);
	tx_traffic_timer_ = new StatTimer (this, &tx_traffic_watch_);

	initTimer_ = new InitTimer (this);
	initTimer_->sched (0.001);
}

Mac802_16::~Mac802_16()
{
}

int Mac802_16::getFrameNumber () {
	return frame_number_;
}

station_type_t Mac802_16::getNodeType()
{
	return type_;
}

int Mac802_16::command (int argc, const char *const *argv)
{
	if (argc == 2) {
		if (strcmp(argv[1], "reset-classifiers") == 0) {
			for (SDUClassifier *n=classifier_list_.lh_first;n;n=classifier_list_.lh_first) {
				printf ("Classifier priority=%d\n",n->getPriority());
				n->remove_entry();
				delete (n);
			}
			return TCL_OK;
		}
	}
	else if (argc == 3) {
		if (strcmp(argv[1], "add-classifier") == 0) {
			SDUClassifier *clas = (SDUClassifier*) TclObject::lookup(argv[2]);
			if (clas == 0)
				return TCL_ERROR;
			//add classifier to the list
			addClassifier (clas);
			return TCL_OK;
		} else if (strcmp(argv[1], "set-scheduler") == 0) {
			if (scheduler_) {
				printf ("Replacing scheduler\n");
				delete (scheduler_); //free old/default scheduler
			}
			scheduler_ = (WimaxScheduler*) TclObject::lookup(argv[2]);
			if (scheduler_ == 0)
				return TCL_ERROR;
			scheduler_->setMac (this); //register the mac
			return TCL_OK;
		} else if (strcmp(argv[1], "set-servicehandler") == 0) {
			serviceFlowHandler_ = (ServiceFlowHandler*) TclObject::lookup(argv[2]);
			if (serviceFlowHandler_ == 0)
				return TCL_ERROR;
			serviceFlowHandler_->setMac (this);
			return TCL_OK;
		} else if (strcmp(argv[1], "set-channel") == 0) {
			phymib_.channel = atoi (argv[2]);
			double tmp = frequencies[phymib_.channel];
			getPhy ()->setFrequency (tmp);
			return TCL_OK;
		} else if (strcmp(argv[1], "log-target") == 0) {
			logtarget_ = (NsObject*) TclObject::lookup(argv[2]);
			if(logtarget_ == 0)
				return TCL_ERROR;
			return TCL_OK;
		}
	}
	return MMacModule::command(argc, argv);
}

void Mac802_16::setChannel (int channel)
{
	assert (channel < nbFreq);
	phymib_.channel = channel;
	double tmp = frequencies[phymib_.channel];
	getPhy ()->setFrequency (tmp);
}

int Mac802_16::getChannel ()
{
	return phymib_.channel;
}

int Mac802_16::getChannel (double freq)
{
	for (int i = 0 ; i < nbFreq ; i++) {
		if (frequencies[i]==freq)
			return i;
	}
	return -1;
}

void Mac802_16::nextChannel ()
{
	debug ("At %f in Mac %d Going to channel %d\n", NOW, addr(), (phymib_.channel+1)%nbFreq);
	setChannel ((phymib_.channel+1)%nbFreq);
}

void Mac802_16::tx(Packet *p)
{
	//handle by subclass
}

void Mac802_16::transmit(Packet *p)
{
	//handle by subclass
}

void Mac802_16::rx (Packet *p)
{
 	//handle by subclass
}

void Mac802_16::receive ()
{
	//handle by subclass
}

void Mac802_16::setNotify_upper (bool notify) {
	notify_upper_ = notify;
	if (notify_upper_ && pktBuf_) {
		sendDown (pktBuf_);
		pktBuf_ = NULL;
	}
}

PeerNode* Mac802_16::getPeerNode (int index)
{
	for (PeerNode *p=peer_list_->lh_first;p;p=p->next_entry()) {
		if (p->getAddr ()==index)
			return p;
	}
	return NULL;
}

void Mac802_16::addPeerNode (PeerNode *node)
{
	node->insert_entry (peer_list_);
	nb_peer_++;
	//update Rx time so for default value
	node->setRxTime(NOW);
	node->getStatWatch()->set_alpha(macmib_.rxp_avg_alpha);
}

void Mac802_16::removePeerNode (PeerNode *peer)
{
	//when removing, we give the CID and it removes IN and OUT connections
	if (peer->getBasic(IN_CONNECTION)) {
		getCManager()->remove_connection (peer->getBasic(IN_CONNECTION)->get_cid());
		delete (peer->getBasic(IN_CONNECTION));
		delete (peer->getBasic(OUT_CONNECTION));
	}
	if (peer->getPrimary(IN_CONNECTION)) {
		getCManager()->remove_connection (peer->getPrimary(IN_CONNECTION)->get_cid());
		delete (peer->getPrimary(IN_CONNECTION));
		delete (peer->getPrimary(OUT_CONNECTION));
	}
	if (peer->getSecondary(IN_CONNECTION)) {
		getCManager()->remove_connection (peer->getSecondary(IN_CONNECTION)->get_cid());
		delete (peer->getSecondary(IN_CONNECTION));
		delete (peer->getSecondary(OUT_CONNECTION));
	}
	if (peer->getInData()) {
		getCManager()->remove_connection (peer->getInData()->get_cid());
		delete (peer->getInData());
	}
	if (peer->getOutData()) {
		getCManager()->remove_connection (peer->getOutData()->get_cid());
		delete (peer->getOutData());
	}
	peer->remove_entry ();
	nb_peer_--;
	delete (peer);
}

int Mac802_16::getNbPeerNodes ()
{
	return nb_peer_;
}

void Mac802_16::start_dlsubframe ()
{
	//handle by subclass
}

void Mac802_16::start_ulsubframe ()
{
	//handle by subclass
}

void Mac802_16::expire (timer_id id)
{
	//handle by subclass
}

Mac802_16* Mac802_16::getPhy()
{
	return this;
}

#ifdef USE_802_21

void Mac802_16::link_configure (link_parameter_config_t* config)
{
	assert (config);
	config->bandwidth = 15000000; //TBD use phy (but depend on modulation)
	config->type = LINK_802_16;
	//we set the rest to PARAMETER_UNKNOWN_VALUE
	config->ber = PARAMETER_UNKNOWN_VALUE;
	config->delay = PARAMETER_UNKNOWN_VALUE;
	config->macPoA = PARAMETER_UNKNOWN_VALUE;
}

void Mac802_16::link_disconnect ()
{
	//handle by subclass
}


void Mac802_16::link_connect (int poa)
{
	//handle by subclass
}

bool Mac802_16::set_mode (mih_operation_mode_t mode)
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

void Mac802_16::link_scan (void *req)
{
	//handle by subclass
}

struct link_param_th_status * Mac802_16::link_configure_thresholds (int numLinkParameter, struct link_param_th *linkThresholds)
{
	struct link_param_th_status *result = (struct link_param_th_status *) malloc(numLinkParameter * sizeof (struct link_param_th_status));
	StatWatch *watch=NULL;
	for (int i=0 ; i < numLinkParameter ; i++) {
		result[i].parameter = linkThresholds[i].parameter;
		result[i].status = 1; //accepted..default
		switch (linkThresholds[i].parameter.parameter_type){
			case LINK_GEN_FRAME_LOSS:
				watch = &loss_watch_;
				break;
			case LINK_GEN_PACKET_DELAY:
				watch = &delay_watch_;
				break;
			case LINK_GEN_PACKET_JITTER:
				watch = &jitter_watch_;
				break;
			case LINK_GEN_RX_DATA_THROUGHPUT:
				watch = &rx_data_watch_;
				break;
			case LINK_GEN_RX_TRAFFIC_THROUGHPUT:
				watch = &rx_traffic_watch_;
				break;
			case LINK_GEN_TX_DATA_THROUGHPUT:
				watch = &tx_data_watch_;
				break;
			case LINK_GEN_TX_TRAFFIC_THROUGHPUT:
				watch = &tx_traffic_watch_;
				break;
			default:
				fprintf (stderr, "Parameter type not supported %d/%d\n",
					 linkThresholds[i].parameter.link_type,
      linkThresholds[i].parameter.parameter_type);
				result[i].status = 0; //rejected
		}
		watch->set_thresholds (linkThresholds[i].initActionTh.data_d,
				       linkThresholds[i].rollbackActionTh.data_d ,
	   linkThresholds[i].exectActionTh.data_d);
	}
	return result;
}
 
#endif

void Mac802_16::init ()
{
	//define by subclass
}

Packet *Mac802_16::getPacket ()
{
	Packet *p = Packet::alloc ();
  
	hdr_mac802_16 *wimaxHdr= HDR_MAC802_16(p);

  //set header information
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
	wimaxHdr->header.cid = BROADCAST_CID; //default
	wimaxHdr->header.hcs = 0;
	HDR_CMN(p)->ptype() = PT_MAC;

	HDR_CMN(p)->size() = HDR_MAC802_16_SIZE;

	return p;
}

int Mac802_16::getFrameDurationCode () {
	if (macmib_.frame_duration == 0.0025)
		return 0;
	else if (macmib_.frame_duration == 0.004)
		return 1;
	else if (macmib_.frame_duration == 0.005)
		return 2;
	else if (macmib_.frame_duration == 0.008)
		return 3;
	else if (macmib_.frame_duration == 0.01)
		return 4;
	else if (macmib_.frame_duration == 0.0125)
		return 5;
	else if (macmib_.frame_duration == 0.02)
		return 6;
	else {
		fprintf (stderr, "Invalid frame duration %f\n", macmib_.frame_duration);
		exit (1);
	}
}

void Mac802_16::setFrameDurationCode (int code) 
{
	switch (code) {
		case 0:
			macmib_.frame_duration = 0.0025;
			break;
		case 1:
			macmib_.frame_duration = 0.004;
			break;
		case 2:
			macmib_.frame_duration = 0.005;
			break;
		case 3:
			macmib_.frame_duration = 0.008;
			break;
		case 4:
			macmib_.frame_duration = 0.01;
			break;
		case 5:
			macmib_.frame_duration = 0.0125;
			break;
		case 6:
			macmib_.frame_duration = 0.02;
			break;
		default:
			fprintf (stderr, "Invalid frame duration code %d\n", code);
			exit (1);
	}
}

void Mac802_16::addClassifier (SDUClassifier *clas) 
{
	SDUClassifier *n=classifier_list_.lh_first;
	SDUClassifier *prev=NULL;
	int i = 0;
	if (!n || (n->getPriority () >= clas->getPriority ())) {
		//the first element
		//debug ("Add first classifier\n");
		clas->insert_entry_head (&classifier_list_);
	} else {
		while ( n && (n->getPriority () < clas->getPriority ()) ) {
			prev=n;
			n=n->next_entry();
			i++;
		}
		//debug ("insert entry at position %d\n", i);
		clas->insert_entry (prev);
	}
	//Register this mac with the classifier
	clas->setMac (this);
}

int Mac802_16::classify (Packet *p)
{
	int cid = -1;
	for (SDUClassifier *n=classifier_list_.lh_first; n && cid==-1; n=n->next_entry()) {
		cid = n->classify (p);
	}
	return cid;
}

void Mac802_16::update_watch (StatWatch *watch, double value)
{
	//handle by subclass
}

void Mac802_16::update_throughput (ThroughputWatch *watch, double size)
{
	//handle by subclass
}

double Mac802_16::txtime(Packet *p)
{
	hdr_ofdmphy* oh = HDR_OFDMPHY(p);
	hdr_cmn* ch = HDR_CMN(p);

	return getTrxTime(ch->size(), oh->modulation_);
}

double Mac802_16::getPS()
{
	WimaxClMsgGetPHYPS *c = new WimaxClMsgGetPHYPS();
	sendSyncClMsgDown(c);
	double val = c->getPS();
	delete c;
	return val;
}

int Mac802_16::getSymbolPS()
{
	WimaxClMsgGetSymbolPs *c = new WimaxClMsgGetSymbolPs();
	sendSyncClMsgDown(c);
	int val = c->getSymbolPs();
	delete c;
	return val;
}

double Mac802_16::getSymbolTime()
{
	WimaxClMsgGetPHYSymbolTime *c = new WimaxClMsgGetPHYSymbolTime();
	sendSyncClMsgDown(c);
	double val = c->getSymbolTime();
	delete c;
	return val;
}

double Mac802_16::getFreq()
{
	WimaxClMsgGetFreq *c = new WimaxClMsgGetFreq();
	sendSyncClMsgDown(c);
	double val = c->getFreq();
	delete c;
	return val;
}

double Mac802_16::getTrxTime(int sdusize, Ofdm_mod_rate mod)
{
	WimaxClMsgGetPHYTrxTime *c = new WimaxClMsgGetPHYTrxTime(sdusize, mod);
	sendSyncClMsgDown(c);
	double val = c->getTrxTime();
	delete c;
	return val;
}

double Mac802_16::getTrxSymbolTime(int sdusize, Ofdm_mod_rate mod)
{
	WimaxClMsgGetPHYTrxSymbolTime *c = new WimaxClMsgGetPHYTrxSymbolTime(sdusize, mod);
	sendSyncClMsgDown(c);
	double val = c->getTrxSymbolTime();
	delete c;
	return val;
}

void Mac802_16::setMode(Ofdm_phy_state mode)
{
	WimaxClMsgSetPHYMode *c = new WimaxClMsgSetPHYMode(mode);
	sendSyncClMsgDown(c);
	delete c;
}

void Mac802_16::setModulation(Ofdm_mod_rate modulation)
{
	WimaxClMsgSetPHYModulation *c = new WimaxClMsgSetPHYModulation(modulation);
	sendSyncClMsgDown(c);
	delete c;
}

void Mac802_16::setFrequency(double freq)
{
	WimaxClMsgSetPHYFreq *c = new WimaxClMsgSetPHYFreq(freq);
	sendSyncClMsgDown(c);
	delete c;
}

int Mac802_16::getMaxPktSize(double nbsymbols, Ofdm_mod_rate mod)
{	
	WimaxClMsgGetMaxPktSize *c = new WimaxClMsgGetMaxPktSize(nbsymbols, mod);
	sendSyncClMsgDown(c);
	int maxPktSize = c->getMaxPktSize();
	delete c;
	return maxPktSize;
}

void Mac802_16::node_on()
{
	WimaxClMsgSetNodeOn *c = new WimaxClMsgSetNodeOn();
	sendSyncClMsgDown(c);
	delete c;
}

void Mac802_16::node_off()
{
	WimaxClMsgSetNodeOff *c = new WimaxClMsgSetNodeOff();
	sendSyncClMsgDown(c);
	delete c;
}
