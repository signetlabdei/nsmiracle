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

#include "mac802_16timer.h"
#include "mac802_16.h"
#include "scheduling/wimaxscheduler.h"

/* 
 * Starts a timer for the given duration
 * @param time The timer duration
 */
void WimaxTimer::start(double time)
{
  Scheduler &s = Scheduler::instance();
  assert(busy_ == 0);
  busy_ = 1;
  paused_ = 0;
  stime = s.clock();
  rtime = time;
  assert(rtime >= 0.0);
  s.schedule(this, &intr, rtime); //schedule the event
}

/*
 * Stop the timer
 */
void WimaxTimer::stop(void)
{
  Scheduler &s = Scheduler::instance();
  
  assert(busy_);
  
  if(paused_ == 0)
    s.cancel(&intr); //cancel the event
  
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;
}

void WimaxTimer::pause(void)
{
	Scheduler &s = Scheduler::instance();

	assert(busy_ && ! paused_);

	paused_ = 1;
	rtime -= s.clock()-stime;

	assert(rtime >= 0.0);

	s.cancel(&intr);
}


void WimaxTimer::resume(void)
{
	Scheduler &s = Scheduler::instance();

	assert(busy_ && paused_);

	paused_ = 0;
	stime = s.clock();

	assert(rtime >= 0.0);
       	s.schedule(this, &intr, rtime );
}



/*
 * Handling function for WimaxFrameTimer
 * @param e The event that occured
 */
void WimaxRxTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->receive ();
}

/*
 * Handling function for WimaxFrameTimer
 * @param e The event that occured
 */
void WimaxT1Timer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxT1TimerID);
}

/*
 * Handling function for WimaxFrameTimer
 * @param e The event that occured
 */
void WimaxT2Timer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxT2TimerID);
}

/*
 * Handling function for WimaxT3Timer
 * @param e The event that occured
 */
void WimaxT3Timer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxT3TimerID);
}

/*
 * Handling function for WimaxT6Timer
 * @param e The event that occured
 */
void WimaxT6Timer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxT6TimerID);
}

/*
 * Handling function for WimaxT12Timer
 * @param e The event that occured
 */
void WimaxT12Timer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxT12TimerID);
}

/*
 * Handling function for WimaxT17Timer
 * @param e The event that occured
 */
void WimaxT17Timer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  /** The node did not send a registration: release and
   *  age out Basic and Primary CIDs
   */
  PeerNode * peer = mac->getPeerNode (peerIndex_);
  mac->debug ("At %f in Mac %d did not register on time...release CIDs\n", NOW, mac->addr(),peerIndex_);
  mac->removePeerNode (peer);
}


/*
 * Handling function for WimaxT21Timer
 * @param e The event that occured
 */
void WimaxT21Timer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxT21TimerID);
}

/*
 * Handling function for WimaxFrameTimer
 * @param e The event that occured
 */
void WimaxLostDLMAPTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxLostDLMAPTimerID);
}

/*
 * Handling function for WimaxFrameTimer
 * @param e The event that occured
 */
void WimaxLostULMAPTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxLostULMAPTimerID);
}

/*
 * Handling function for WimaxDCDTimer
 * @param e The event that occured
 */
void WimaxDCDTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxDCDTimerID);
}

/*
 * Handling function for WimaxUCDTimer
 * @param e The event that occured
 */
void WimaxUCDTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxUCDTimerID);
}

/*
 * Handling function for WimaxScanIntervalTimer
 * @param e The event that occured
 */
void WimaxScanIntervalTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxScanIntervalTimerID);
}

/*
 * Handling function for WimaxT44Timer
 * @param e The event that occured
 */
void WimaxT44Timer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxT44TimerID);
}

/*
 * Handling function for WimaxMobNbrAdvTimer
 * @param e The event that occured
 */
void WimaxMobNbrAdvTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxMobNbrAdvTimerID);
}

/*
 * Handling function for WimaxRdvTimer
 * @param e The event that occured
 */
void WimaxRdvTimer::handle(Event *e)
{
  busy_ = 0;
  paused_ = 0;
  stime = 0.0;
  rtime = 0.0;

  mac->expire(WimaxRdvTimerID);
  printf ("Rdv timeout going to channel %d\n", channel_);
  mac->setChannel (channel_);
}

/*
 * Indicates the beginning of DL subframe
 */
void DlTimer::expire (Event *e)
{
  m_->start_dlsubframe();
}

/*
 * Indicates the beginning of DL subframe
 */
void UlTimer::expire (Event *e)
{
  m_->start_ulsubframe();
}

/*
 * Initialize the MAC
 */
void InitTimer::expire (Event *e)
{
  m_->init();
}
