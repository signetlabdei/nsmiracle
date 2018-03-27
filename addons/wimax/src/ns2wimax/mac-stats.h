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
 * @author woon
 */

#ifndef mac_stats_h
#define mac_stats_h

#include <timer-handler.h>

enum threshold_action_t {
  NO_ACTION_TH,
  INIT_ACTION_TH,
  ROLLBACK_ACTION_TH,
  EXEC_ACTION_TH
};

/*
 * NIST: Stats Watch
 * Used to maintain statistics of a particular parameter
 *
 * Positive gradient means that when the value measured increases, it will trigger INIT_ACTION_TH 
 * A positive gradient is used for measurements such as packet loss, where as negative gradient
 * is used for RSSI.
 */
class StatWatch : public TimerHandler {
public:
  StatWatch(double ti=1, double alpha=1, bool pos_gradient=true) : TimerHandler() { 
		timer_interval_ = ti;
		alpha_ = alpha;
		pos_gradient_ = pos_gradient;
		current_ = 0;
		average_ = 0;
		instant_ = 0;
		total_ = 0;
		sample_number_ = 0;
		th_set_ = false;
		init_sent_ = false;
		init_th_ = 0;
		rollback_th_ = 0;
		exec_th_ = 0;
		pending_action_ = NO_ACTION_TH;
		delay_ = 0;
	}
	virtual void expire(Event*) { schedule_next(); }
		
	inline void reset() {
		average_ = 0;
		instant_ = 0;
		total_ = 0;
		sample_number_ = 0;
	}

	inline void set_thresholds (double init_th, double rollback_th, double exec_th) {
	  assert ( (pos_gradient_ && rollback_th <= init_th && init_th <= exec_th)
		   || (!pos_gradient_ && rollback_th >= init_th && init_th >= exec_th));
	  init_th_ = init_th;
	  rollback_th_ = rollback_th;
	  exec_th_ = exec_th;
	  th_set_ = true;
	}
		
	inline void set_timer_interval(double ti) { timer_interval_ = ti; }
	inline void set_alpha(double a) { alpha_= a; }
	inline void set_current(double c) { current_= c; }
	inline void set_pos_gradient(bool b) { pos_gradient_ = b; }
	inline void set_delay (double d) { delay_ = d; }
	virtual threshold_action_t update(double new_val) {
	  old_average_ = average_;
	  average_ = alpha_*new_val + (1-alpha_)*average_;
	  total_ += new_val;
	  sample_number_++;
	  instant_ = new_val;
	  
	  //evaluate if threshold has been crossed
	  if (th_set_) {
	    if (pos_gradient_) {
	      //check if threshold is crossed
	      if (old_average_ > rollback_th_ && average_ <= rollback_th_ && init_sent_) {
		pending_action_ = ROLLBACK_ACTION_TH;
		ts_ = NOW+delay_;
	      } else if (old_average_ < exec_th_ && average_ >= exec_th_) {
		pending_action_ = EXEC_ACTION_TH;
		ts_ = NOW+delay_;
	      } else if (old_average_ < init_th_ && average_ >= init_th_) {
		pending_action_ = INIT_ACTION_TH;
		ts_ = NOW+delay_;
	      } else {
		//check if threshold is canceled
		if (pending_action_ == ROLLBACK_ACTION_TH && average_ > rollback_th_
		    || pending_action_ == EXEC_ACTION_TH && average_ < exec_th_
		    || pending_action_ == INIT_ACTION_TH && average_ < init_th_) {
		  pending_action_ = NO_ACTION_TH;
		}
	      }
	      //check if action is still valid
	      if (pending_action_ != NO_ACTION_TH && NOW >= ts_) {
		threshold_action_t tmp = pending_action_;
		pending_action_ = NO_ACTION_TH;
		return tmp;
	      }
	      
	    } else {	    
	      //check if threshold is crossed
	      if (old_average_ < rollback_th_ && average_ >= rollback_th_ && init_sent_) {
		pending_action_ = ROLLBACK_ACTION_TH;
		ts_ = NOW+delay_;
	      } else if (old_average_ > exec_th_ && average_ <= exec_th_) {
		pending_action_ = EXEC_ACTION_TH;
		ts_ = NOW+delay_;
	      } else if (old_average_ > init_th_ && average_ <= init_th_) {
		pending_action_ = INIT_ACTION_TH;
		ts_ = NOW+delay_;
	      } else {
		//check if threshold is canceled
		if (pending_action_ == ROLLBACK_ACTION_TH && average_ < rollback_th_
		    || pending_action_ == EXEC_ACTION_TH && average_ > exec_th_
		    || pending_action_ == INIT_ACTION_TH && average_ > init_th_) {
		  pending_action_ = NO_ACTION_TH;
		}
	      }
	      //check if action is still valid
	      if (pending_action_ != NO_ACTION_TH && NOW >= ts_) {
		threshold_action_t tmp = pending_action_;
		pending_action_ = NO_ACTION_TH;
		  return tmp;	      
	      }
	    }
	  }
	  return NO_ACTION_TH;
	}
	inline void schedule_next() { resched(timer_interval_); }

	inline double current() { return current_; }
	inline double total() { return total_; }
	inline double sample_number() { return sample_number_; }
	inline double average() { return average_; }
	inline double old_average() { return old_average_; }
	inline double instant() { return instant_; }
	inline double simple_average() { return total_/sample_number_; }

protected:
	double timer_interval_;
	double alpha_;
	double current_;
	double average_;
	double old_average_;
	double instant_;
	double total_;
	int sample_number_;
	bool pos_gradient_;
	double delay_;
	threshold_action_t pending_action_;
	double ts_;
	bool th_set_;
	bool init_sent_;
	double init_th_;
	double rollback_th_;
	double exec_th_;
};

/**
 * Class to handle throughput measurements
 */
class ThroughputWatch: public StatWatch  {
 public:
  /**
   * Constructor
   */
  ThroughputWatch() { }

  /**
   * Virtual desctructor
   */
  virtual ~ThroughputWatch() {};

  inline void set_alpha(double a) { size_.set_alpha(a); time_.set_alpha(a); }
  inline double get_timer_interval () { return 0.01000000001; }

  threshold_action_t update(double size, double time) {
    size_.update (size);    
    time_.update (time-time_.current());
    time_.set_current (time);
    old_average_ = average_;
    average_ = size_.average()/time_.average();

    //evaluate if threshold has been crossed
    if (th_set_) {
      if (pos_gradient_) {
	//check if threshold is crossed
	if (old_average_ > rollback_th_ && average_ <= rollback_th_ && init_sent_) {
	  pending_action_ = ROLLBACK_ACTION_TH;
	  ts_ = NOW+delay_;
	} else if (old_average_ < exec_th_ && average_ >= exec_th_) {
	  pending_action_ = EXEC_ACTION_TH;
	  ts_ = NOW+delay_;
	} else if (old_average_ < init_th_ && average_ >= init_th_) {
	  pending_action_ = INIT_ACTION_TH;
	  ts_ = NOW+delay_;
	} else {
	  //check if threshold is canceled
	  if (pending_action_ == ROLLBACK_ACTION_TH && average_ > rollback_th_
	      || pending_action_ == EXEC_ACTION_TH && average_ < exec_th_
	      || pending_action_ == INIT_ACTION_TH && average_ < init_th_) {
	    pending_action_ = NO_ACTION_TH;
	  }
	}
	//check if action is still valid
	if (pending_action_ != NO_ACTION_TH && NOW >= ts_) {
	  threshold_action_t tmp = pending_action_;
	  pending_action_ = NO_ACTION_TH;
	  return tmp;
	}
	
      } else {	    
	//check if threshold is crossed
	if (old_average_ < rollback_th_ && average_ >= rollback_th_ && init_sent_) {
	  pending_action_ = ROLLBACK_ACTION_TH;
	  ts_ = NOW+delay_;
	} else if (old_average_ > exec_th_ && average_ <= exec_th_) {
	  pending_action_ = EXEC_ACTION_TH;
	  ts_ = NOW+delay_;
	} else if (old_average_ > init_th_ && average_ <= init_th_) {
	  pending_action_ = INIT_ACTION_TH;
	  ts_ = NOW+delay_;
	} else {
	  //check if threshold is canceled
	  if (pending_action_ == ROLLBACK_ACTION_TH && average_ < rollback_th_
	      || pending_action_ == EXEC_ACTION_TH && average_ > exec_th_
	      || pending_action_ == INIT_ACTION_TH && average_ > init_th_) {
	    pending_action_ = NO_ACTION_TH;
	  }
	}
	//check if action is still valid
	if (pending_action_ != NO_ACTION_TH && NOW >= ts_) {
	  threshold_action_t tmp = pending_action_;
	  pending_action_ = NO_ACTION_TH;
	  printf ("Action triggered %d\n", tmp);
	  return tmp;	      
	}
      }
    }
    return NO_ACTION_TH;
  }

 protected:
  /**
   * Watch for packet size
   */
  StatWatch size_;
  
  /**
   * Watch for inter arrival time
   */
  StatWatch time_;

};



#endif //mac_stats_h
