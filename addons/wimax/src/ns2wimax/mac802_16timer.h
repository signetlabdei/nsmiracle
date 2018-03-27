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

#ifndef MAC802_16TIMER_H
#define MAC802_16TIMER_H

#include "scheduler.h"
#include "timer-handler.h"

/** Define the ID for each timer **/
enum timer_id {
  WimaxFrameTimerID,
  WimaxRxTimerID,
  WimaxDCDTimerID,
  WimaxUCDTimerID,
  WimaxRngIntTimerID,
  WimaxLostDLMAPTimerID,
  WimaxLostULMAPTimerID,
  WimaxT1TimerID,
  WimaxT2TimerID,
  WimaxT3TimerID,
  WimaxT6TimerID,
  WimaxT9TimerID,
  WimaxT12TimerID,
  WimaxT16TimerID,
  WimaxT17TimerID,
  WimaxT21TimerID,
  WimaxT44TimerID,
  //mobility extension
  WimaxMobNbrAdvTimerID,

  WimaxScanIntervalTimerID,
  WimaxRdvTimerID
};


class Mac802_16;

/**
 * Super class for timers used in wimax 
 */
class WimaxTimer : public Handler {
public:
	WimaxTimer(Mac802_16* m) : mac(m) {
		busy_ = paused_ = 0; stime = rtime = 0.0;
	}

	virtual void handle(Event *e) = 0;

	virtual void start(double time);
	virtual void stop(void);
	void pause(void); /*{ assert(0); }*/
	void resume(void);/* { assert(0); }*/

	inline int busy(void) { return busy_; }
	inline int paused(void) { return paused_; }
	inline double expire(void) {
		return ((stime + rtime) - Scheduler::instance().clock());
	}

protected:
	Mac802_16	*mac;
	int		busy_;
	int		paused_;
	Event		intr;
	double		stime;	// start time
	double		rtime;	// remaining time
};

/** Timer for receiving a packet */
class WimaxRxTimer : public WimaxTimer {
 public:
  WimaxRxTimer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for DCD interval */
class WimaxDCDTimer : public WimaxTimer {
 public:
  WimaxDCDTimer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for UCD interval */
class WimaxUCDTimer : public WimaxTimer {
 public:
  WimaxUCDTimer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for initial ranging regions interval */
class WimaxRngIntTimer : public WimaxTimer {
 public:
  WimaxRngIntTimer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for Lost DL-MAP interval */
class WimaxLostDLMAPTimer : public WimaxTimer {
 public:
  WimaxLostDLMAPTimer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for Lost UL-MAP interval */
class WimaxLostULMAPTimer : public WimaxTimer {
 public:
  WimaxLostULMAPTimer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T1 : wait for DCD timeout */
class WimaxT1Timer : public WimaxTimer {
 public:
  WimaxT1Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T2 : wait for broadcast ranging timeout */
class WimaxT2Timer : public WimaxTimer {
 public:
  WimaxT2Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T3 : ranging response timeout */
class WimaxT3Timer : public WimaxTimer {
 public:
  WimaxT3Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T6 : wait for registration response */
class WimaxT6Timer : public WimaxTimer {
 public:
  WimaxT6Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T9 : registration timeout */
class WimaxT9Timer : public WimaxTimer {
 public:
  WimaxT9Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T12 : wait for UCD descriptor */
class WimaxT12Timer : public WimaxTimer {
 public:
  WimaxT12Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T16 : wait for bw request grant */
class WimaxT16Timer : public WimaxTimer {
 public:
  WimaxT16Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T17 : wait for SS to register */
class WimaxT17Timer : public WimaxTimer {
 public:
  WimaxT17Timer(Mac802_16 *m, int peerIndex) : WimaxTimer(m) { peerIndex_ = peerIndex;}
  
  void	handle(Event *e);
 private:
  int peerIndex_;
};

/** Timer for T21 : time the station searches for DL-MAP on a channel */
class WimaxT21Timer : public WimaxTimer {
 public:
  WimaxT21Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for T44 : wait for BS to send MOB_SCN-REP */
class WimaxT44Timer : public WimaxTimer {
 public:
  WimaxT44Timer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for scan interval : timer for scanning */
class WimaxScanIntervalTimer : public WimaxTimer {
 public:
  WimaxScanIntervalTimer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for neighbor advertisement interval */
class WimaxMobNbrAdvTimer : public WimaxTimer {
 public:
  WimaxMobNbrAdvTimer(Mac802_16 *m) : WimaxTimer(m) {}
  
  void	handle(Event *e);
};

/** Timer for rendez-vous with target BSs */
class WimaxRdvTimer : public WimaxTimer {
 public:
  WimaxRdvTimer(Mac802_16 *m, int channel) : WimaxTimer(m) 
    {
      channel_ = channel;
    }
  
  void	handle(Event *e);
 private:
  int channel_;
};

/** Timer to indicate a new Downlink frame */
class DlTimer : public TimerHandler {
 public:
  DlTimer(Mac802_16 *m) : TimerHandler() {m_=m;}
  
  void	expire(Event *e);
 private:
  Mac802_16 *m_;
}; 

/** Timer to indicate a new uplink frame */
class UlTimer : public TimerHandler {
 public:
  UlTimer(Mac802_16 *m) : TimerHandler() {m_=m;}
  
  void	expire(Event *e);
 private:
  Mac802_16 *m_;
}; 

/** Timer to indicate a new uplink frame */
class InitTimer : public TimerHandler {
 public:
  InitTimer(Mac802_16 *m) : TimerHandler() {m_=m;}
  
  void	expire(Event *e);
 private:
  Mac802_16 *m_;
}; 


#endif
