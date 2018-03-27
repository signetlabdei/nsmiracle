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
 * $Id: contentionrequest.cc 92 2008-03-25 19:35:54Z emil $
 */

//#include "contentionrequest.h"
#include "contentionslot.h"
#include "framemap.h"
#include "wimaxscheduler.h"
#include "random.h"
#include "mac802_16SS.h"

/*
 * Handling function for WimaxFrameTimer
 * @param e The event that occured
 */
void WimaxBackoffTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  HDR_CMN(c_->p_)->timestamp() = NOW; //add timestamp since it bypasses the queue
  mac->transmit (c_->p_->copy());

  //start timeout trigger
  c_->starttimeout ();
}

void WimaxBackoffTimer::pause()
{
	Scheduler &s = Scheduler::instance();

	//the caculation below make validation pass for linux though it
	// looks dummy

	double st = s.clock();
	double rt = stime;
	double sr = st - rt;

	assert(busy_ && ! paused_);

	paused_ = 1;
	rtime -= sr;

	assert(rtime >= 0.0);

	s.cancel(&intr);
}


void WimaxBackoffTimer::resume()
{
	Scheduler &s = Scheduler::instance();

	assert(busy_ && paused_);

	paused_ = 0;
	stime = s.clock();

	assert(rtime >= 0.0);
       	s.schedule(this, &intr, rtime);
}

/*
 * Creates a contention slot for the given frame
 * @param s The contention slot 
 * @param p The packet to send
 */
ContentionRequest::ContentionRequest (ContentionSlot *s, Packet *p)
{
  assert (s);
  assert (p);
  s_=s;
  mac_ = s_->map_->getMac();
  window_ = s_->getBackoff_start();
  nb_retry_ = 0;
  p_=p;
  backoff_timer_ = new WimaxBackoffTimer (this, mac_);
  timeout_timer_ = new ContentionTimer (this);
  int result = Random::random() % ((int)(pow (2, window_)+1));
  mac_->debug ("At %f in Mac %d Start contention in %f(backoff=%d, size=%d, ps=%f)\n", NOW, mac_->addr(), result*s_->getSize()*mac_->getPhy()->getPS(),result,s_->getSize(),mac_->getPhy()->getPS());
  backoff_timer_->start (result*s_->getSize()*mac_->getPhy()->getPS());
  backoff_timer_->pause();
}

ContentionRequest::~ContentionRequest ()
{
  //printf ("canceling timeout\n");
  //the timeout timer need not be triggered 
  //this can happen when the STA received bw allocation
  //when it is not waiting for one (or it's still in backoff)
  if (timeout_timer_->status()==TIMER_PENDING)
    timeout_timer_->cancel();
  if (backoff_timer_->busy())
    backoff_timer_->stop();
  delete backoff_timer_;
  delete timeout_timer_;
  assert (p_);
  Packet:: free (p_);
}

/*
 * Called when timeout expired
 */
void ContentionRequest::expire ()
{

}

/*
 * Called when timeout expired
 */
void ContentionRequest::starttimeout ()
{
  timeout_timer_->sched (timeout_);
}

/* 
 * Pause the backoff timer
 */
void ContentionRequest::pause () 
{
  if (backoff_timer_->busy() && !backoff_timer_->paused())
    backoff_timer_->pause(); 
}

/*
 * Resume the backoff timer
 */
void ContentionRequest::resume () 
{ 
  if (backoff_timer_->paused() && timeout_timer_->status()==TIMER_IDLE)
    backoff_timer_->resume(); 
}

/*
 * Creates a contention slot for the given frame
 * @param frame The frame map 
 */
RangingRequest::RangingRequest (ContentionSlot *s, Packet *p) : ContentionRequest (s,p)
{
  type_ = WimaxT3TimerID;
  timeout_ = mac_->macmib_.t3_timeout;
}

/*
 * Called when timeout expired
 */
void RangingRequest::expire ()
{
  mac_->debug ("Ranging request expires\n");
  if (nb_retry_ == (int)mac_->macmib_.contention_rng_retry) {
    //max retries reached, inform the scheduler
    mac_->expire (type_);
  } else {
    if (window_ < s_->getBackoff_stop())
      window_++;
    nb_retry_++;
    int result = Random::random() % ((int)(pow (2, window_)+1));
    mac_->debug ("Start Ranging contention in %f(backoff=%d, size=%d, ps=%f)\n", result*s_->getSize()*mac_->getPhy()->getPS(),result,s_->getSize(),mac_->getPhy()->getPS());
    backoff_timer_->start (result*s_->getSize()*mac_->getPhy()->getPS());
    backoff_timer_->pause();
  }
}

/*
 * Creates a contention slot for the given frame
 * @param frame The frame map 
 */
BwRequest::BwRequest (ContentionSlot *s, Packet *p, int cid, int len) : ContentionRequest (s,p)
{
  type_ = WimaxT16TimerID;
  timeout_ = mac_->macmib_.t16_timeout;
  cid_ = cid;
  size_ = len;
}

/*
 * Called when timeout expired
 */ 
void BwRequest::expire ()
{
  debug2 ("At %f in Mac %d Bw request expires (%d/%d)\n", NOW, mac_->addr(), nb_retry_, (int)mac_->macmib_.request_retry);
  if (nb_retry_ == (int)mac_->macmib_.request_retry) {
    //max retries reached, delete the pdu that were waiting
    Connection *c = mac_->getCManager()->get_connection (cid_, true);
    int len = 0;
    debug2 ("Dropping packet because bw req exceeded\n");
    while (len < size_) {
      Packet *p = c->dequeue();
      assert (p);
      len += HDR_CMN(p)->size();
      //We want to know when the packet is dropped. Create a new entry
      ((Mac802_16SS*)mac_)->drop(p, 1, "BWR");
      //Packet::free (p);
    }
    //must remove the request from the list
    ((BwContentionSlot*)s_)->removeRequest (cid_);
  } else {
    if (window_ < s_->getBackoff_stop())
      window_++;
    nb_retry_++;
    int result = Random::random() % ((int)(pow (2, window_)+1));
    debug2 ("Start BW contention in %f(backoff=%d, size=%d, ps=%f)\n", result*s_->getSize()*mac_->getPhy()->getPS(),result,s_->getSize(),mac_->getPhy()->getPS());
    backoff_timer_->start (result*s_->getSize()*mac_->getPhy()->getPS());
    backoff_timer_->pause();
  }
}
