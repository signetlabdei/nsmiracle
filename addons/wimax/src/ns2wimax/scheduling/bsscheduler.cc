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
 * $Id: bsscheduler.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "bsscheduler.h"
#include "burst.h"
#include "dlburst.h"
#include "ulburst.h"
#include "random.h"
#include "wimaxctrlagent.h"
#include "mac802_16BS.h"

/**
 * Bridge to TCL for BSScheduler
 */
static class BSSchedulerClass : public TclClass {
public:
  BSSchedulerClass() : TclClass("WimaxScheduler/BS") {}
  TclObject* create(int, const char*const*) {
    return (new BSScheduler());
    
  }
} class_bsscheduler;

/*
 * Create a scheduler
 */
BSScheduler::BSScheduler () 
{
  debug2 ("BSScheduler created\n");
  default_mod_ = OFDM_BPSK_1_2;
  bind ("dlratio_", &dlratio_);

  //dlratio_ = DEFAULT_DL_RATIO;

  contention_size_ = MIN_CONTENTION_SIZE;

  nextDL_ = -1;
  nextUL_ = -1;
}
 
/**
 * Return the MAC casted to BSScheduler
 * @return The MAC casted to BSScheduler
 */
Mac802_16BS* BSScheduler::getMac()
{
  return (Mac802_16BS*)mac_;
}

/*
 * Interface with the TCL script
 * @param argc The number of parameter
 * @param argv The list of parameters
 */
int BSScheduler::command(int argc, const char*const* argv)
{
  if (argc == 3) {
    if (strcmp(argv[1], "set-default-modulation") == 0) {
      if (strcmp(argv[2], "OFDM_BPSK_1_2") == 0)
	default_mod_ = OFDM_BPSK_1_2;
      else if (strcmp(argv[2], "OFDM_QPSK_1_2") == 0)
	default_mod_ = OFDM_QPSK_1_2;
      else if (strcmp(argv[2], "OFDM_QPSK_3_4") == 0)
	default_mod_ = OFDM_QPSK_3_4;
      else if (strcmp(argv[2], "OFDM_16QAM_1_2") == 0)
	default_mod_ = OFDM_16QAM_1_2;
      else if (strcmp(argv[2], "OFDM_16QAM_3_4") == 0)
	default_mod_ = OFDM_16QAM_3_4;
      else if (strcmp(argv[2], "OFDM_64QAM_2_3") == 0)
	default_mod_ = OFDM_64QAM_2_3;
      else if (strcmp(argv[2], "OFDM_64QAM_3_4") == 0)
	default_mod_ = OFDM_64QAM_3_4;
      else
	return TCL_ERROR;
      return TCL_OK;
    }
    else if (strcmp(argv[1], "set-contention-size") == 0) {
      contention_size_ = atoi (argv[2]);
      assert (contention_size_>=0);
      return TCL_OK;      
    }
  }
  return TCL_ERROR;
}

/**
 * Initializes the scheduler
 */
void BSScheduler::init ()
{
  WimaxScheduler::init();

  // If the user did not set the profiles by hand, let's do it
  // automatically
  if (getMac()->getMap()->getDlSubframe()->getProfile (DIUC_PROFILE_1)==NULL) {
    Profile *p = getMac()->getMap()->getDlSubframe()->addProfile ((int)round((getMac()->getPhy()->getFreq()/1000)), OFDM_BPSK_1_2);
    p->setIUC (DIUC_PROFILE_1);
    p = getMac()->getMap()->getDlSubframe()->addProfile ((int)round((getMac()->getPhy()->getFreq()/1000)), OFDM_QPSK_1_2);
    p->setIUC (DIUC_PROFILE_2);
    p = getMac()->getMap()->getDlSubframe()->addProfile ((int)round((getMac()->getPhy()->getFreq()/1000)), OFDM_QPSK_3_4);
    p->setIUC (DIUC_PROFILE_3);
    p = getMac()->getMap()->getDlSubframe()->addProfile ((int)round((getMac()->getPhy()->getFreq()/1000)), OFDM_16QAM_1_2);
    p->setIUC (DIUC_PROFILE_4);
    p = getMac()->getMap()->getDlSubframe()->addProfile ((int)round((getMac()->getPhy()->getFreq()/1000)), OFDM_16QAM_3_4);
    p->setIUC (DIUC_PROFILE_5);
    p = getMac()->getMap()->getDlSubframe()->addProfile ((int)round((getMac()->getPhy()->getFreq()/1000)), OFDM_64QAM_2_3);
    p->setIUC (DIUC_PROFILE_6);
    p = getMac()->getMap()->getDlSubframe()->addProfile ((int)round((getMac()->getPhy()->getFreq()/1000)), OFDM_64QAM_3_4);
    p->setIUC (DIUC_PROFILE_7);

    p = getMac()->getMap()->getUlSubframe()->addProfile (0, default_mod_);
    p->setIUC (UIUC_INITIAL_RANGING);
    p = getMac()->getMap()->getUlSubframe()->addProfile (0, default_mod_);
    p->setIUC (UIUC_REQ_REGION_FULL);  

    p = getMac()->getMap()->getUlSubframe()->addProfile (0, OFDM_BPSK_1_2);
    p->setIUC (UIUC_PROFILE_1);
    p = getMac()->getMap()->getUlSubframe()->addProfile (0, OFDM_QPSK_1_2);
    p->setIUC (UIUC_PROFILE_2);
    p = getMac()->getMap()->getUlSubframe()->addProfile (0, OFDM_QPSK_3_4);
    p->setIUC (UIUC_PROFILE_3);
    p = getMac()->getMap()->getUlSubframe()->addProfile (0, OFDM_16QAM_1_2);
    p->setIUC (UIUC_PROFILE_4);
    p = getMac()->getMap()->getUlSubframe()->addProfile (0, OFDM_16QAM_3_4);
    p->setIUC (UIUC_PROFILE_5);
    p = getMac()->getMap()->getUlSubframe()->addProfile (0, OFDM_64QAM_2_3);
    p->setIUC (UIUC_PROFILE_6);
    p = getMac()->getMap()->getUlSubframe()->addProfile (0, OFDM_64QAM_3_4);
    p->setIUC (UIUC_PROFILE_7);
  }

  //init contention slots
  ContentionSlot *slot = getMac()->getMap()->getUlSubframe()->getRanging ();
  slot->setSize (getInitRangingopportunity ());
  slot->setBackoff_start (getMac()->macmib_.rng_backoff_start);
  slot->setBackoff_stop (getMac()->macmib_.rng_backoff_stop);
  
  slot = getMac()->getMap()->getUlSubframe()->getBw_req ();
  slot->setSize (getBWopportunity ());
  slot->setBackoff_start (getMac()->macmib_.bw_backoff_start);
  slot->setBackoff_stop (getMac()->macmib_.rng_backoff_stop);

}

/**
 * Compute and return the bandwidth request opportunity size
 * @return The bandwidth request opportunity size
 */
int BSScheduler::getBWopportunity ()
{
  int nbPS = BW_REQ_PREAMBLE * getMac()->getPhy()->getSymbolPS();
  //add PS for carrying header
  nbPS += (int) round((getMac()->getPhy()->getTrxTime (HDR_MAC802_16_SIZE, getMac()->getMap()->getUlSubframe()->getProfile(UIUC_REQ_REGION_FULL)->getEncoding())/getMac()->getPhy()->getPS ()));
  //printf ("BWopportunity size=%d\n", nbPS);
  return nbPS;
}

/**
 * Compute and return the initial ranging opportunity size
 * @return The initial ranging opportunity size
 */
int BSScheduler::getInitRangingopportunity ()
{
  int nbPS = INIT_RNG_PREAMBLE * getMac()->getPhy()->getSymbolPS();
  //add PS for carrying header
  nbPS += (int) round((getMac()->getPhy()->getTrxTime (RNG_REQ_SIZE+HDR_MAC802_16_SIZE, getMac()->getMap()->getUlSubframe()->getProfile(UIUC_INITIAL_RANGING)->getEncoding())/getMac()->getPhy()->getPS ()));
  //printf ("Init ranging opportunity size=%d\n", nbPS);
  return nbPS;  
}

/**
 * Schedule bursts/packets
 */
void BSScheduler::schedule ()
{
  //The scheduler will perform the following steps:
  //1-Clear DL map
  //2-Allocate Burst for Broadcast message
  //3-Pick Unicast bursts for Downlink (Best Effort) in a RR fashion
  //4-Clear UL map
  //5-Allocate contention slots for initial ranging
  //6-Allocate contention slots for bandwidth requests
  //7-Allocate unicast burst for Uplink according to bandwidth requests received (RR fashion)

  Packet *p;
  struct hdr_cmn *ch;
  double txtime; //tx time for some data (in second)
  int txtime_s;  //number of symbols used to transmit the data
  DlBurst *db;
  PeerNode *peer;

  Mac802_16 *phy = mac_->getPhy();
  //variables for scheduling
  FrameMap *map = mac_->getMap();
  int nbPS = (int) floor((mac_->getFrameDuration()/phy->getPS()));
  assert (nbPS*phy->getPS()<=mac_->getFrameDuration()); //check for rounding errors
  int nbPS_left = nbPS - mac_->phymib_.rtg - mac_->phymib_.ttg;
  int nbSymbols = (int) floor((phy->getPS()*nbPS_left)/phy->getSymbolTime());
  assert (nbSymbols*phy->getSymbolTime()+(mac_->phymib_.rtg + mac_->phymib_.ttg)*phy->getPS() < mac_->getFrameDuration());
  int maxdlduration = (int) (nbSymbols / (1.0/dlratio_)); //number of symbols for downlink
  int dlduration = DL_PREAMBLE;                             //number of symbols currently used for downlink
  int nbdlbursts = 0;
  int maxulduration = nbSymbols - maxdlduration;            //number of symbols for uplink
  int ulduration = 0;                                       //number of symbols currently used for uplink
  int nbulpdus = 0;

  //printf ("%d %9f\n",phy->getSymbolPS(), (nbSymbols*phy->getSymbolPS()+mac_->phymib_.rtg + mac_->phymib_.ttg)*phy->getPS());

  assert ((nbSymbols*phy->getSymbolPS()+mac_->phymib_.rtg + mac_->phymib_.ttg)*phy->getPS()< mac_->getFrameDuration());

  assert (maxdlduration*phy->getSymbolTime()+mac_->phymib_.rtg*phy->getPS()+maxulduration*phy->getSymbolTime()+mac_->phymib_.ttg*phy->getPS() < mac_->getFrameDuration());
  

  debug2 ("Frame: duration=%f, PSduration=%e, symboltime=%e, nbPS=%d, rtg=%d, ttg=%d, PSleft=%d, nbSymbols=%d, ", \
	  mac_->getFrameDuration(), phy->getPS(), phy->getSymbolTime(), nbPS, mac_->phymib_.rtg, mac_->phymib_.ttg, nbPS_left, nbSymbols);
  debug2 ("maxdlduration=%d, maxulduration=%d\n",maxdlduration, maxulduration);

  map->getDlSubframe()->getPdu()->removeAllBursts();
  //1-First burst for broadcast messages are using DIUC_PROFILE_4
  db = (DlBurst*) map->getDlSubframe()->getPdu ()->addBurst (nbdlbursts++);
  db->setCid ( BROADCAST_CID );
  db->setIUC (map->getDlSubframe()->getProfile (DIUC_PROFILE_4)->getIUC());
  db->setStarttime (dlduration); //after preamble and fch
  db->setPreamble(true); //this is the first burst after preamble
  
  //time to send DL_MAP and UL_MAP
  txtime = phy->getTrxSymbolTime (2*sizeof (struct gen_mac_header_t)+sizeof (struct mac802_16_dl_map_frame)+sizeof (struct mac802_16_ul_map_frame), map->getDlSubframe()->getProfile (db->getIUC())->getEncoding());
  txtime_s = (int) round(txtime/phy->getSymbolTime ()); //in units of symbol
  if ((dlduration+txtime_s) > maxdlduration) {
    fprintf (stderr, "Error: Not enough symbols on the downlink subframe to send synchronization messages\n");
    fprintf (stderr, "       maxdlduration=%d, needed=%d\n", maxdlduration, dlduration+txtime_s);
    fprintf (stderr, "       Consider incrementing the dlratio_ or using a better modulation\n");
    exit(0);
  }
  
  db->setDuration (txtime_s); //enough to send DL_MAP...
  
  //check if we have to add DCD and/or UCD to transmit and increase burst size if
  //necessary
  if (getMac()->sendDCD || map->getDlSubframe()->getCCC()!= getMac()->dlccc_) {
    p = map->getDCD();
    ch = HDR_CMN(p);
    txtime = phy->getTrxTime (ch->size(), map->getDlSubframe()->getProfile (db->getIUC())->getEncoding());
    ch->txtime() = txtime;
    txtime_s = (int) ceil(txtime/phy->getSymbolTime ()); //in units of symbol
    assert ((dlduration+db->getDuration()+txtime_s) <= maxdlduration); //if this occurs, increase dlratio
    db->setDuration (db->getDuration()+txtime_s);
  }
  if (getMac()->sendUCD || map->getUlSubframe()->getCCC()!= getMac()->ulccc_) {
    p = map->getUCD();
    ch = HDR_CMN(p);
    txtime = phy->getTrxTime (ch->size(), map->getDlSubframe()->getProfile (db->getIUC())->getEncoding());
    ch->txtime() = txtime;
    txtime_s = (int) ceil(txtime/phy->getSymbolTime ()); //in units of symbol
    assert ((dlduration+db->getDuration()+txtime_s) <= maxdlduration); //if this occurs, increase dlratio
    db->setDuration (db->getDuration()+txtime_s);
  }
  
  //check if we have additional packets in the broadcast queue
  if (mac_->getCManager()->get_connection (BROADCAST_CID, OUT_CONNECTION)->queueByteLength()>0) {
    //printf ("Additional %d packet(s) in Broadcast\n", mac_->getCManager()->get_connection (BROADCAST_CID, OUT_CONNECTION)->queueLength());
    //other packets to send, increase the broadcast duration
    txtime = phy->getTrxSymbolTime (mac_->getCManager()->get_connection (BROADCAST_CID, OUT_CONNECTION)->queueByteLength(), map->getDlSubframe()->getProfile (db->getIUC())->getEncoding());
    txtime_s = (int) round(txtime/phy->getSymbolTime ()); //in units of symbol. We add one to create a small gap between bursts
    if (txtime_s <= maxdlduration-dlduration-db->getDuration()) {
      db->setDuration (db->getDuration()+txtime_s);
    } else {
      //fill up the rest
      db->setDuration (maxdlduration-dlduration);
    }
  }
  dlduration += db->getDuration ()+1; //add 1 OFDM symbol between padding (to avoid collisions)

  //First implementation, return the CIDs in a RR fashion
  peer = mac_->getPeerNode (nextDL_);
  if (peer == NULL) //if the node is not here, lets start from the beginning
    peer = mac_->getPeerNode_head();
  if (peer) {
    for (int i=0; i<mac_->getNbPeerNodes() && dlduration < maxdlduration ;i++) {
      //for each node, the order is as follows:
      //basic, primary, secondary, data
      if (peer->getBasic(OUT_CONNECTION) && peer->getBasic(OUT_CONNECTION)->queueByteLength()>0) {
	dlduration = addDlBurst (nbdlbursts++, peer->getBasic(OUT_CONNECTION), peer->getDIUC(), dlduration, maxdlduration);
      }
      if (peer->getPrimary(OUT_CONNECTION) && peer->getPrimary(OUT_CONNECTION)->queueByteLength()>0 && dlduration < maxdlduration) {
	dlduration = addDlBurst (nbdlbursts++, peer->getPrimary(OUT_CONNECTION), peer->getDIUC(), dlduration, maxdlduration);
      }
      if (peer->getSecondary(OUT_CONNECTION) && peer->getSecondary(OUT_CONNECTION)->queueByteLength()>0 && dlduration < maxdlduration) {
	dlduration = addDlBurst (nbdlbursts++, peer->getSecondary(OUT_CONNECTION), peer->getDIUC(), dlduration, maxdlduration);
      }
      if (peer->getOutData() && peer->getOutData()->queueByteLength()>0 && dlduration < maxdlduration) {
	dlduration = addDlBurst (nbdlbursts++, peer->getOutData(), peer->getDIUC(), dlduration, maxdlduration);
      }
      peer = peer->next_entry();
      if (peer == NULL)
        peer = mac_->getPeerNode_head();

      if (peer)
	nextDL_ = peer->getAddr(); //go to next one
      else
	nextDL_ = -1; //will go the the beginning of the list
    }
  }
  assert (dlduration <= maxdlduration);

  //3-Add the End of map element
  db = (DlBurst*) map->getDlSubframe()->getPdu ()->addBurst (nbdlbursts);
  db->setIUC (DIUC_END_OF_MAP);
  db->setStarttime (maxdlduration);

  //Start the Uplink allocation

  //4-Set start time for uplink subframe and clean previous allocation
  mac_->getMap()->getUlSubframe()->setStarttime (maxdlduration*phy->getSymbolPS()+mac_->phymib_.rtg);
  while (mac_->getMap()->getUlSubframe()->getNbPdu()>0) {
    PhyPdu *pdu = mac_->getMap()->getUlSubframe()->getPhyPdu(0);
    pdu->removeAllBursts();
    mac_->getMap()->getUlSubframe()->removePhyPdu(pdu);
    delete (pdu);
  }

  //5-Set contention slots for initial ranging
  int rangingduration =(int) ceil(((mac_->getPhy()->getPS()*contention_size_*getInitRangingopportunity())/mac_->getPhy()->getSymbolTime()));
  assert (rangingduration < maxulduration-ulduration); //check there is enough space
  UlBurst *ub = (UlBurst*)mac_->getMap()->getUlSubframe()->addPhyPdu (nbulpdus++,0)->addBurst (0);
  ub->setIUC (UIUC_INITIAL_RANGING);
  ub->setDuration (rangingduration);
  ub->setStarttime (ulduration); //we put the contention at the begining
  ulduration += ub->getDuration()+1;

  //5-Set contention slots for bandwidth request
  int bwduration =(int) ceil(((mac_->getPhy()->getPS()*contention_size_*getBWopportunity())/mac_->getPhy()->getSymbolTime()));
  assert (bwduration < maxulduration-ulduration);
  ub = (UlBurst*)mac_->getMap()->getUlSubframe()->addPhyPdu (nbulpdus++,0)->addBurst (0);
  ub->setIUC (UIUC_REQ_REGION_FULL);
  ub->setDuration (bwduration);
  ub->setStarttime (ulduration); //we put the contention at the begining
  ulduration += ub->getDuration()+1;

  //check if there is Fast Ranging allocation to do
  FastRangingInfo *next_info = NULL;
  for (FastRangingInfo *info = getMac()->fast_ranging_head_.lh_first ; info ; info=next_info) {
    //get next info before the entry is removed from list
    next_info = info->next_entry();
    if (info->frame() == mac_->getFrameNumber()) {
      //we need to include a fast ranging allocation
      ub = (UlBurst*)mac_->getMap()->getUlSubframe()->addPhyPdu (nbulpdus++,0)->addBurst (0);
      int tmp =(int) ceil(((mac_->getPhy()->getPS()*getInitRangingopportunity())/mac_->getPhy()->getSymbolTime()));
      ub->setIUC (UIUC_EXT_UIUC);
      ub->setDuration (tmp);
      ub->setStarttime (ulduration); //start after previous slot
      ub->setFastRangingParam (info->macAddr(), UIUC_INITIAL_RANGING);
      ulduration += ub->getDuration()+1;
      mac_->debug ("At %f in Mac %d adding fast ranging for %d\n", NOW, mac_->addr(), info->macAddr());
      info->remove_entry();
    }
  }

  //First implementation, return the CIDs in a RR fashion
  peer = mac_->getPeerNode (nextUL_);
  if (peer == NULL) //if the node is not here, lets start from the beginning
    peer = mac_->getPeerNode_head();
  if (peer) {
    //printf ("We have %d macs\n", mac_->getNbPeerNodes());
    for (int i=0; i<mac_->getNbPeerNodes() && ulduration < maxulduration ;i++) {

      //The following code uses BW information to allocate data
      if (peer->getReqBw()>0 && !getMac()->isPeerScanning (peer->getAddr())) {
	mac_->debug ("At %f Allocating UL burst for Node %d:", NOW, peer->getAddr());
	//add a pdu and burst for this node
	ub = (UlBurst*) mac_->getMap()->getUlSubframe()->addPhyPdu (nbulpdus++,0)->addBurst (0);
	ub->setCid (peer->getBasic(IN_CONNECTION)->get_cid());
	//get the profile (we do a simple translation so that DL_PROFILE_1 = UL_PROFILE_1
	//printf ("DIUC_1=%d UIUC_1=%d, DIUCreq=%d, UIUC=%d\n", DIUC_PROFILE_1, UIUC_PROFILE_1, peer->getDIUC(), DIUC_PROFILE_1+UIUC_PROFILE_1-peer->getDIUC());
	ub->setIUC (mac_->getMap()->getUlSubframe()->getProfile (peer->getDIUC()-DIUC_PROFILE_1+UIUC_PROFILE_1)->getIUC());
	ub->setStarttime (ulduration);
	
	txtime = phy->getTrxSymbolTime (peer->getReqBw(), mac_->getMap()->getUlSubframe()->getProfile (ub->getIUC())->getEncoding());
	txtime_s = (int) round(txtime/phy->getSymbolTime ()); //in units of symbol
	if (txtime_s <= maxulduration-ulduration) {
	  ub->setDuration (txtime_s);
	  ulduration += ub->getDuration()+1; //add 1 OFDM symbol between bursts
	} else {
	  //fill up the rest
	  ub->setDuration (maxulduration-ulduration);
	  ulduration = maxulduration;
	}
	mac_->debug ("duration= %d\n", ub->getDuration());
      }
      
      //The following allocates all the bandwidth to 1 node.
      /*
      ub = (UlBurst*) mac_->getMap()->getUlSubframe()->addPhyPdu (nbulpdus++,0)->addBurst (0);
      ub->setCid (peer->getBasic(IN_CONNECTION)->get_cid());
      //get the profile (we do a simple translation so that DL_PROFILE_1 = UL_PROFILE_1
      //printf ("DIUC_1=%d UIUC_1=%d, DIUCreq=%d, UIUC=%d\n", DIUC_PROFILE_1, UIUC_PROFILE_1, peer->getDIUC(), DIUC_PROFILE_1+UIUC_PROFILE_1-peer->getDIUC());
      ub->setIUC (mac_->getMap()->getUlSubframe()->getProfile (peer->getDIUC()-DIUC_PROFILE_1+UIUC_PROFILE_1)->getIUC());
      ub->setStarttime (ulduration);
      ub->setDuration (maxulduration-ulduration);
      ulduration = maxulduration;
      */

      peer = peer->next_entry();
      if (peer == NULL)
        peer = mac_->getPeerNode_head();

      if (peer)
        nextUL_ = peer->getAddr(); //go to next one
      else
        nextUL_ = -1; //will go the the beginning of the list
    }      
  }
   
  //end of map
  ub = (UlBurst*)mac_->getMap()->getUlSubframe()->addPhyPdu (nbulpdus,0)->addBurst (0);
  ub->setIUC (UIUC_END_OF_MAP);
  ub->setStarttime (maxulduration);

  assert (mac_->getFrameDuration() > map->getUlSubframe()->getStarttime()*phy->getPS()+maxulduration*phy->getSymbolTime ());
  //printf ("Start %f end=%f\n",NOW+map->getUlSubframe()->getStarttime()*phy->getPS(), NOW+map->getUlSubframe()->getStarttime()*phy->getPS()+maxulduration*phy->getSymbolTime ());
  //printf ("End2=%f\n", NOW+map->getUlSubframe()->getStarttime()*phy->getPS()+ub->getStarttime()*mac_->getPhy()->getSymbolTime());

  //now transfert the packets to the bursts starting with broadcast messages
  Burst *b = map->getDlSubframe()->getPdu ()->getBurst (0);
  int b_data = 0;
  int max_data = phy->getMaxPktSize (b->getDuration(), mac_->getMap()->getDlSubframe()->getProfile (b->getIUC())->getEncoding())-b_data;

  p = map->getDL_MAP();
  ch = HDR_CMN(p);
  txtime = phy->getTrxTime (ch->size(), map->getDlSubframe()->getProfile (b->getIUC())->getEncoding());
  ch->txtime() = txtime;
  assert (b_data+ch->size() <= max_data);
  ch->timestamp() = NOW; //add timestamp since it bypasses the queue
  b->enqueue(p);      //enqueue into burst
  b_data += ch->size();

  p = map->getUL_MAP();
  ch = HDR_CMN(p);
  txtime = phy->getTrxTime (ch->size(), map->getDlSubframe()->getProfile (b->getIUC())->getEncoding());
  ch->txtime() = txtime;
  assert (b_data+ch->size() <= max_data);
  ch->timestamp() = NOW; //add timestamp since it bypasses the queue
  b->enqueue(p);      //enqueue into burst
  b_data += ch->size();

  if (getMac()->sendDCD || map->getDlSubframe()->getCCC()!= getMac()->dlccc_) {
    p = map->getDCD();
    ch = HDR_CMN(p);
    txtime = phy->getTrxTime (ch->size(), map->getDlSubframe()->getProfile (b->getIUC())->getEncoding());
    ch->txtime() = txtime;
    assert (b_data+ch->size() <= max_data);
    ch->timestamp() = NOW; //add timestamp since it bypasses the queue
    b->enqueue(p);      //enqueue into burst
    b_data += ch->size();
  }

  if (getMac()->sendUCD || map->getUlSubframe()->getCCC()!= getMac()->ulccc_) {
    p = map->getUCD();
    ch = HDR_CMN(p);
    txtime = phy->getTrxTime (ch->size(), map->getDlSubframe()->getProfile (b->getIUC())->getEncoding());
    ch->txtime() = txtime;
    assert (b_data+ch->size() <= max_data);
    ch->timestamp() = NOW; //add timestamp since it bypasses the queue
    b->enqueue(p);      //enqueue into burst
    b_data += ch->size();
  }

  //Get other broadcast messages
  Connection *c=mac_->getCManager ()->get_connection (b->getCid(),OUT_CONNECTION);
  b_data = transfer_packets (c, b, b_data);

  //Now get the other bursts
  for (int index = 1 ; index < map->getDlSubframe()->getPdu ()->getNbBurst() -1 ; index++) {
    Burst *b = map->getDlSubframe()->getPdu ()->getBurst (index);
    int b_data = 0;

    Connection *c=mac_->getCManager ()->get_connection (b->getCid(),OUT_CONNECTION);
    assert (c);
    //printf ("Transferring data to burst %d\n", b->getCid());
    b_data = transfer_packets (c, b, b_data);
  }

}

/**
 * Add a downlink burst with the given information
 * @param burstid The burst number
 * @param c The connection to add
 * @param iuc The profile to use
 * @param dlduration current allocation status
 * @param the new allocation status
 */
int BSScheduler::addDlBurst (int burstid, Connection *c, int iuc, int dlduration, int maxdlduration)
{
  double txtime; //tx time for some data (in second)
  int txtime_s;  //number of symbols used to transmit the data
  Mac802_16 *phy = mac_->getPhy();

  //add a burst for this node
  DlBurst *db = (DlBurst*) mac_->getMap()->getDlSubframe()->getPdu ()->addBurst (burstid);
  db->setCid (c->get_cid());
  db->setIUC (iuc);
  db->setStarttime (dlduration);

  txtime = phy->getTrxSymbolTime (c->queueByteLength(), mac_->getMap()->getDlSubframe()->getProfile (db->getIUC())->getEncoding());
  txtime += c->queueLength() * TX_GAP; //add small gaps between packets to send
  txtime_s = (int) ceil(txtime/phy->getSymbolTime ()); //in units of symbol
  if (txtime_s < maxdlduration-dlduration) {
    db->setDuration (txtime_s);
    dlduration += db->getDuration ()+1; //add 1 OFDM symbol between bursts
  } else {
    //fill up the rest
    db->setDuration (maxdlduration-dlduration);
    dlduration = maxdlduration;
  }
  return dlduration;
}
