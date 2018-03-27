/* This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 File * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * <BR>
 * We would appreciate acknowledgement if the software is used.
 * <BR>
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * </PRE></P>
 * @author  rouil
 */


#include "framemap.h"
#include "contentionslot.h"
#include "wimaxscheduler.h"
#include "ulburst.h"
#include "dlburst.h"

/*
 * Creates a map of the frame
 * @param mac Pointer to the mac layer
 */
FrameMap::FrameMap (Mac802_16 *mac): dlsubframe_(this), ulsubframe_(this)
{
  assert (mac);
  mac_ = mac; 

  //retreive information from mac
  rtg_ = mac_->phymib_.rtg;
  ttg_ = mac_->phymib_.ttg;
  duration_ = mac_->getFrameDuration();
}

/**
 * Compute the DL_MAP packet based on the information contained in the structure
 */
Packet* FrameMap::getDL_MAP( )
{
  Packet *p = mac_->getPacket();
  hdr_cmn* ch = HDR_CMN(p);

  //printf ("Creating DL_MAP:");
  int nbies = dlsubframe_.getPdu()->getNbBurst();
  //printf ("nbies=%d\n",nbies);

  //allocate data for DL_MAP
  p->allocdata (sizeof (struct mac802_16_dl_map_frame));
  mac802_16_dl_map_frame *frame = (mac802_16_dl_map_frame*) p->accessdata();

  frame->type = MAC_DL_MAP;
  frame->bsid = mac_->addr();
  frame->nb_ies = nbies;
  
  //allocate IEs
  mac802_16_dlmap_ie *ies = frame->ies;

  for (int i = 0 ; i < nbies ; i++) {
    Burst *b = dlsubframe_.getPdu()->getBurst(i);
    ies[i].diuc = b->getIUC();
    ies[i].start_time = b->getStarttime();
    if (b->getIUC()!=DIUC_END_OF_MAP) {
      ies[i].cid =  b->getCid();
      if (i==0)
	ies[i].preamble = dlsubframe_.getPdu()->getPreamble();
      else
	ies[i].preamble = 0;
    }
  }
  
  ch->size() += GET_DL_MAP_SIZE(nbies);

  return p;
}

/**
 * Compute and return the DCD frame
 */
Packet* FrameMap::getDCD( )
{
  Packet *p = mac_->getPacket ();
  hdr_cmn* ch = HDR_CMN(p);

  //allocate data for DL_MAP
  //printf ("getDCD...nbprofile=%d\n", dlsubframe_.getNbProfile());
  p->allocdata (sizeof (struct mac802_16_dcd_frame));
  mac802_16_dcd_frame *frame = (mac802_16_dcd_frame*) p->accessdata();

  frame->type = MAC_DCD;
  frame->dcid = mac_->addr(); //to check if needs to be different from ucid
  frame->config_change_count = dlsubframe_.getCCC(); 
  frame->frame_duration_code = mac_->getFrameDurationCode ();
  frame->frame_number = mac_->frame_number_;
  frame->nb_prof = dlsubframe_.getNbProfile();
  frame->ttg = mac_->phymib_.ttg;
  frame->rtg = mac_->phymib_.rtg;
  frame->frequency = (int) (mac_->getPhy()->getFreq()/1000); 

  //allocate IEs
  mac802_16_dcd_profile *profiles = frame->profiles;

  int i=0;
  for (Profile *p = dlsubframe_.getFirstProfile() ; p ; p=p->next_entry()) {
    //set data for first burst
    profiles[i].diuc = p->getIUC(); 
    profiles[i].frequency = p->getFrequency();
    profiles[i].fec = p->getEncoding(); 
    i++;
  }
  //the end of map is already included in the frame length
  
  ch->size() += GET_DCD_SIZE(dlsubframe_.getNbProfile());
  return p;
}

/**
 * Compute and return the UL_MAP frame
 */
Packet* FrameMap::getUL_MAP( )
{
  Packet *p = mac_->getPacket ();
  hdr_cmn* ch = HDR_CMN(p);

  int nbies = ulsubframe_.getNbPdu(); //there is one burst per UL phy PDU
  //printf ("getUL_MAP, nbies=%d\n", nbies);
  //allocate data for DL_MAP
  p->allocdata (sizeof (struct mac802_16_ul_map_frame));
  mac802_16_ul_map_frame *frame = (mac802_16_ul_map_frame*) p->accessdata();

  frame->type = MAC_UL_MAP;
  frame->ucid = mac_->addr();  //set channel ID to index_ to be unique
  frame->ucd_count = ulsubframe_.getCCC(); 
  frame->allocation_start = ulsubframe_.getStarttime(); //the subframe starts with the contention slot
  frame->nb_ies = nbies; 
  
  //allocate IEs
  mac802_16_ulmap_ie *ies = frame->ies;

  int i=0;
  for (PhyPdu *p = ulsubframe_.getFirstPdu(); p ; p= p ->next_entry()) {
    UlBurst *b = (UlBurst*) p->getBurst(0);
    ies[i].uiuc = b->getIUC(); //end of map
    ies[i].start_time = b->getStarttime();
    if (b->getIUC()!=UIUC_END_OF_MAP) {
      ies[i].cid =  b->getCid();
      ies[i].midamble_rep = b->getMidamble();
      ies[i].duration = b->getDuration();
      if (b->getIUC() == UIUC_EXT_UIUC) {
	ies[i].extended_uiuc = b->getExtendedUIUC();
	if (b->getExtendedUIUC ()== UIUC_FAST_RANGING) {
	  ies[i].fast_ranging.mac_addr = b->getFastRangingMacAddr ();
	  ies[i].fast_ranging.uiuc = b->getFastRangingUIUC ();
	}
      }
    }
    i++;
  }

  ch->size() += GET_UL_MAP_SIZE(nbies);
  return p;
}

/**
 * Compute and return the UCD frame
 */
Packet* FrameMap::getUCD( )
{
  Packet *p = mac_->getPacket ();
  hdr_cmn* ch = HDR_CMN(p);

  //allocate data for DL_MAP
  p->allocdata (sizeof (struct mac802_16_ucd_frame));
  mac802_16_ucd_frame *frame = (mac802_16_ucd_frame*) p->accessdata();

  frame->type = MAC_UCD;
  frame->config_change_count = 0; //changed by scheduler
  frame->rng_backoff_start = ulsubframe_.getRanging()->getBackoff_start();
  frame->rng_backoff_end = ulsubframe_.getRanging()->getBackoff_stop();
  frame->rng_req_size = ulsubframe_.getRanging()->getSize();
  frame->req_backoff_start = ulsubframe_.getBw_req()->getBackoff_start();
  frame->req_backoff_end = ulsubframe_.getBw_req()->getBackoff_stop()+1;
  frame->bw_req_size = ulsubframe_.getBw_req()->getSize();

  frame->nb_prof = ulsubframe_.getNbProfile();
  //allocate IEs
  mac802_16_ucd_profile *profiles = frame->profiles;

  int i=0;
  for (Profile *p = ulsubframe_.getFirstProfile() ; p ; p=p->next_entry()) {
    //set data for first burst
    profiles[i].uiuc = p->getIUC(); 
    profiles[i].fec = p->getEncoding(); 
    i++;
  }

  //the end of map is already included in the frame length
  ch->size() += GET_UCD_SIZE(ulsubframe_.getNbProfile());
  return p;
}

/**
 * Parse a DL_MAP message and create the data structure
 * @param frame The DL frame information
 */
void FrameMap::parseDLMAPframe (mac802_16_dl_map_frame *frame)
{
  //printf ("parse DL-MAP in %d\n", mac_->addr());
  // Clear previous information  
  while (dlsubframe_.getPdu()->getNbBurst()>0) {
    Burst *b = dlsubframe_.getPdu()->getBurst (0);
    dlsubframe_.getPdu()->removeBurst (b);
    delete b;
  }

  int nbies = frame->nb_ies;
  mac802_16_dlmap_ie *ies = frame->ies;

  for (int i = 0 ; i < nbies ; i++) {
    Burst *b = dlsubframe_.getPdu()->addBurst(i);
    b->setIUC(ies[i].diuc);
    b->setStarttime(ies[i].start_time);
    if (b->getIUC()!=DIUC_END_OF_MAP) {
      b->setCid(ies[i].cid);
      if (i==0) //first burst contains preamble
	dlsubframe_.getPdu()->setPreamble(ies[i].preamble);
    }
    //printf ("\t Adding burst %d: cid=%d, iuc=%d start=%d\n", i, b->getCid(), b->getIUC(),b->getStarttime());
  }
  //should we parse end of map too?
}

/**
 * Parse a DCD message and create the data structure
 * @param frame The DL frame information
 */
void FrameMap::parseDCDframe (mac802_16_dcd_frame *frame)
{
  //clear previous profiles
  dlsubframe_.removeProfiles();

  int nb_prof = frame->nb_prof;
  mac_->frame_number_ = frame->frame_number;
  mac802_16_dcd_profile *profiles = frame->profiles;
  mac_->setFrameDurationCode (frame->frame_duration_code);

  for (int i = 0 ; i < nb_prof ; i++) {
    Profile *p = dlsubframe_.addProfile (profiles[i].frequency, (Ofdm_mod_rate)profiles[i].fec);
    p->setIUC (profiles[i].diuc);
    //printf ("\t Adding dl profile %i: f=%d, rate=%d, iuc=%d\n", i, p->getFrequency(), p->getEncoding(), p->getIUC());
  }
}

/**
 * Parse a UL_MAP message and create the data structure
 * @param frame The UL frame information
 */
void FrameMap::parseULMAPframe (mac802_16_ul_map_frame *frame)
{
  //printf ("parse UL-MAP\n");
  // Clear previous information
  for (PhyPdu *p = ulsubframe_.getFirstPdu(); p ; p = ulsubframe_.getFirstPdu()) {
    ulsubframe_.removePhyPdu(p);
    delete (p);
  }
  
  int nbies = frame->nb_ies;
  mac802_16_ulmap_ie *ies = frame->ies;

  ulsubframe_.setStarttime(frame->allocation_start);
  //mac_->debug ("\tul start time = %d %f\n", frame->allocation_start, frame->allocation_start*mac_->getPhy()->getPS());

  for (int i = 0 ; i < nbies ; i++) {
    UlBurst *b = (UlBurst*)(ulsubframe_.addPhyPdu(i,0))->addBurst(0);
    b->setIUC(ies[i].uiuc);
    b->setStarttime(ies[i].start_time);
    if (b->getIUC()!=UIUC_END_OF_MAP) {
      b->setCid(ies[i].cid);
      b->setMidamble(ies[i].midamble_rep);
      b->setDuration(ies[i].duration);
      if (b->getIUC() == UIUC_EXT_UIUC) {
	if(ies[i].extended_uiuc== UIUC_FAST_RANGING) {
	  b->setFastRangingParam (ies[i].fast_ranging.mac_addr, ies[i].fast_ranging.uiuc);
	}
      }
    }
    /*mac_->debug ("\t Adding burst %d: cid=%d, iuc=%d start=%d (%f) duration=%d\n", \
      i, b->getCid(), b->getIUC(),b->getStarttime(), starttime_+frame->allocation_start*mac_->getPhy()->getPS()+b->getStarttime()*mac_->getPhy()->getSymbolTime(), b->getDuration());*/
  }
}

/**
 * Parse a UCD message and create the data structure
 * @param frame The DL frame information
 */
void FrameMap::parseUCDframe (mac802_16_ucd_frame *frame)
{
  assert (frame);
  //clear previous profiles
  ulsubframe_.removeProfiles();
  
  /*printf ("parse UCD..rng_start=%d, rng_stop=%d, req_start=%d, req_stop=%d\n",\
    frame->rng_backoff_start, frame->rng_backoff_end, frame->req_backoff_start,
    frame->req_backoff_end);*/
  ulsubframe_.getRanging()->setBackoff_start(frame->rng_backoff_start);
  ulsubframe_.getRanging()->setBackoff_stop(frame->rng_backoff_end);
  ulsubframe_.getRanging()->setSize(frame->rng_req_size);
  ulsubframe_.getBw_req()->setBackoff_start(frame->req_backoff_start);
  ulsubframe_.getBw_req()->setBackoff_stop(frame->req_backoff_end);  
  ulsubframe_.getBw_req()->setSize(frame->bw_req_size);

  int nb_prof = frame->nb_prof;
  mac802_16_ucd_profile *profiles = frame->profiles;
  for (int i = 0 ; i < nb_prof ; i++) {
    Profile *p = ulsubframe_.addProfile (0, (Ofdm_mod_rate)(profiles[i].fec));
    p->setIUC (profiles[i].uiuc);
    //printf ("\t Adding ul profile %i: f=%d, rate=%d, iuc=%d\n", i, p->getFrequency(), p->getEncoding(), p->getIUC());
  }
}
