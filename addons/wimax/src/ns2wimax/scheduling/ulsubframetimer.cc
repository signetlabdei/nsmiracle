/* This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
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

#include "ulsubframetimer.h"
#include "framemap.h"
#include "subframe.h"
#include "wimaxscheduler.h"
#include "contentionslot.h"

/**
 * Creates a timer to handle the subframe transmission
 * @param subframe The UlSubframe
 */
UlSubFrameTimer::UlSubFrameTimer (UlSubFrame *subframe): pdu_(0), newphy_(true), mac_(0)
{
  assert (subframe);
  subframe_ = subframe;
}

/**
 * Reset the timer
 */
void UlSubFrameTimer::reset ()
{
  pdu_= NULL;
  newphy_ = true;
  if (status()==TIMER_PENDING)
    cancel();
}

/**
 * When it expires, the timer will handle the next packet to send
 * @param e not used
 */
void UlSubFrameTimer::expire( Event* e )
{
  if (!mac_) {
    mac_= subframe_->map_->getMac();
  }

  //mac_->debug ("At %f in Mac %d UlsubFrameTimer expires\n", NOW, mac_->addr());
  int iuc;
  if (newphy_) {
    if (pdu_==NULL){
      //printf ("\ttake first pdu\n");
      //get the first pdu
      pdu_ = subframe_->getFirstPdu();
      if (!pdu_)
	return; //this means there was no uplink burst allocated
    } else {
      //printf ("\tcontinue pdu\n");
      iuc = pdu_->getBurst(0)->getIUC();
      //check if this is a contention slot
      if (iuc == UIUC_INITIAL_RANGING) {
	//stop ranging timer
	//printf ("\tpause ranging\n");
	subframe_->ranging_->pauseTimers ();
      } else if (iuc == UIUC_REQ_REGION_FULL) {
	//stop bw request timers
	//printf ("\tpause bw requests\n");
	subframe_->bw_req_->pauseTimers();
      }
      pdu_ = pdu_->next_entry();
    }  
  
    if (pdu_->getBurst(0)->getIUC()==UIUC_END_OF_MAP){
      pdu_=NULL; //reset for next frame
      //mac_->debug ("\tend of map\n");
      if (mac_->getNodeType()==STA_BS) {
	mac_->getPhy()->setMode (OFDM_SEND);
      } else {
	mac_->getPhy()->setMode (OFDM_RECV);
      }
      return; //end of subframe
    }
    
    //change the modulation
    UlBurst *burst =  (UlBurst*)pdu_->getBurst(0);
    iuc = burst->getIUC();
    //printf ("Searching for IUC=%d\n", iuc);
    if (iuc == UIUC_EXT_UIUC && burst->getExtendedUIUC()== UIUC_FAST_RANGING) {
      iuc = burst->getFastRangingUIUC ();
      //printf ("Searching for IUC=%d\n", iuc);
    }
    Ofdm_mod_rate rate = subframe_->getProfile (iuc)->getEncoding();
    mac_->getPhy()->setModulation (rate);
    //check if this is a contention slot
    if (iuc == UIUC_INITIAL_RANGING) {
      //resume ranging timer
      //printf ("\tresume ranging\n");
      subframe_->ranging_->resumeTimers();
    } else if (iuc == UIUC_REQ_REGION_FULL) {
      //resume bw request timers
      //printf ("\tresume bw requests\n");
      subframe_->bw_req_->resumeTimers();
    }
  }
  //check if packet to send
  Packet *p = pdu_->getBurst(0)->dequeue();

  if (p) {
    newphy_= false;
    double txtime = HDR_CMN(p)->txtime();
    //printf ("\tPacket to send\n");
    //schedule for next packet
    mac_->transmit (p);
    if (pdu_->getBurst(0)->getQueueLength()!=0) {
      //mac_->debug ("\treschedule in %f (%f)\n", txtime, NOW+txtime);
      resched (txtime); //wait transmition time + GAP
      return;
    }
  }

  newphy_= true;
  double stime=0.0;
  assert (pdu_->next_entry());

  stime = subframe_->map_->getStarttime();
  //mac_->debug("\tstart frame=%f\n", stime);
  stime += subframe_->getStarttime()*mac_->getPhy()->getPS();
  //mac_->debug ("\tulstart = %f\n", stime);
  stime += pdu_->next_entry()->getBurst(0)->getStarttime()*mac_->getPhy()->getSymbolTime();
  //mac_->debug ("\tnext pdu start=%d\n", pdu_->next_entry()->getBurst(0)->getStarttime());
  //mac_->debug ("\t%f Next burst %d at %f\n", NOW, pdu_->next_entry()->getBurst(0)->getIUC(), stime);
  resched (stime-NOW);

}
