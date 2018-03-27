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

#include "dlsubframetimer.h"
#include "framemap.h"
#include "subframe.h"
#include "wimaxscheduler.h"
#include "contentionslot.h"

/**
 * Creates a timer to handle the subframe transmission
 * @param subframe The DlSubframe
 */
DlSubFrameTimer::DlSubFrameTimer (DlSubFrame *subframe): burstIndex_(0), newburst_(true), mac_(0)
{
  assert (subframe);
  subframe_ = subframe;
}

/**
 * Reset the timer
 */
void DlSubFrameTimer::reset ()
{
  burstIndex_ = 0;
  newburst_ = true;
  if (status()==TIMER_PENDING)
    cancel();
}

/**
 * When it expires, the timer will handle the next packet to send
 * @param e not used
 */
void DlSubFrameTimer::expire( Event* e )
{
  if (!mac_) {
    mac_= subframe_->map_->getMac();
  }
  
  //printf ("At %f in Mac %d DlsubFrameTimer expires\n", NOW, mac_->addr());
  int iuc;
  Burst *b = subframe_->getPdu()->getBurst(burstIndex_);
  if (newburst_) {
    //printf ("\tburst=%x type=%d\n", b,b->getIUC());
    if (b->getIUC()==DIUC_END_OF_MAP) {
      //printf ("\tend of subframe\n");
      burstIndex_=0;//reset for next frame
      if (mac_->getNodeType()==STA_MN) {
	mac_->getPhy()->setMode (OFDM_SEND);
      } else {
	mac_->getPhy()->setMode (OFDM_RECV);
      }
      return; //end of subframe
    }
    //change modulation
    iuc = b->getIUC();
    Ofdm_mod_rate rate = subframe_->getProfile (iuc)->getEncoding();
    mac_->getPhy()->setModulation (rate);
  }
  //check if packet to send
  Packet *p = b->dequeue();
  if (p) {
    newburst_ = false;
    double txtime = HDR_CMN(p)->txtime();
    //schedule for next packet
    mac_->transmit (p);
    if (b->getQueueLength()!=0) {
      //printf ("\tNext packet at %f(in %f)\n", NOW+txtime, txtime);
      resched (txtime); //wait transmition time + GAP
      return;
    }
  }
  
  //no packet to send...schedule for next phypdu
  newburst_= true;
  burstIndex_++;
  double stime=0.0;
  assert (b->next_entry());
  
  stime = subframe_->map_->getStarttime();
  stime += b->next_entry()->getStarttime()*mac_->getPhy()->getSymbolTime();
  //printf ("\tMap start time=%f Next burst at %f\n", subframe_->map_->getStarttime(), stime);
  resched (stime-NOW);
  
}
