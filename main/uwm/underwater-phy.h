/* **************************************************
 * underwater ns2 code
 * by: Al Harris (thanks to Robin S. for making it go in ns2)
 *
 * Work done while at the University of Padova, Italy with
 * Michele Zorzi
 ******************************************************/

#ifndef ns_UnderwaterPhy_h
#define ns_UnderwaterPhy_h

#include <propagation.h>
#include <modulation.h>
#include <omni-antenna.h>
#include <phy.h>
#include <mobilenode.h>
#include <timer-handler.h>

#include "underwater.h"


class Underwater_Sleep_Timer : public TimerHandler {
 public:
  Underwater_Sleep_Timer(UnderwaterPhy *a);
 protected:
  virtual void expire(Event *e);
  UnderwaterPhy *a_;
};


class UnderwaterPhy : public Phy {
public:
	UnderwaterPhy();
	virtual ~UnderwaterPhy() { }

	void sendDown(Packet *p);
	int sendUp(Packet *p);
	
	inline double getL() const {return L_;}
	inline double getLambda() const {return lambda_;}
	inline Node* node(void) const { return node_; }
	inline double getPtconsume() { return Pt_consume_; }

	double getDist(double Pr, double Pt, double Gt, double Gr, double hr,
		       double ht, double L, double lambda);
  
	virtual int command(int argc, const char*const* argv);
	virtual void dump(void) const;
	
	//void setnode (MobileNode *node) { node_ = node; }
	void node_on();
	void node_off();	

        /* -NEW- */
        inline double getAntennaZ() { return ant_->getZ(); }
        inline double getPt() { return Pt_; }
        inline double getRXThresh() { return RXThresh_; }
        inline double getCSThresh() { return CSThresh_; }
        inline double getFreq() { return freq_; }
        /* End -NEW- */

	void node_sleep();
	void node_wakeup();
	inline bool& Is_node_on() { return node_on_; }
	inline bool Is_sleeping() { if (status_==SLEEP) return(1); else return(0); }

protected:
	double Pt_;		// transmitted signal power (W)
	double Pt_consume_;	// power consumption for transmission (W)
	double Pr_consume_;	// power consumption for reception (W)
	double P_idle_;         // idle power consumption (W)
	double P_sleep_;	// sleep power consumption (W)
	double P_transition_;	// power consumed when transiting from SLEEP mode to IDLE mode and vice versa.
	double T_transition_;	// time period to transit from SLEEP mode to IDLE mode and vice versa.
//

	double last_send_time_;	// the last time the node sends somthing.
	double channel_idle_time_;	// channel idle time.
	double update_energy_time_;	// the last time we update energy.

	double freq_;           // frequency
	double lambda_;		// wavelength (m)
	double L_;		// system loss factor
  
	double RXThresh_;	// receive power threshold (W)
	double CSThresh_;	// carrier sense threshold (W)
	double CPThresh_;	// capture threshold (db)
  
	Antenna *ant_;
	Underwater *propagation_;	// Propagation Model
	Modulation *modulation_;	// Modulation Schem

	// Why phy has a node_ and this guy has it all over again??
//  	MobileNode* node_;         	// Mobile Node to which interface is attached .

 	enum ChannelStatus { SLEEP, IDLE, RECV, SEND };	
	bool node_on_; // on-off status of this node
	Underwater_Sleep_Timer sleep_timer_;
	int status_;

private:
	inline int initialized() {
		return (node_ && uptarget_ && downtarget_ && propagation_);
	}
	void UpdateIdleEnergy();
	void UpdateSleepEnergy();

	// Convenience method
	EnergyModel* em() { return node()->energy_model(); }

	friend class Underwater_Sleep_Timer;

};

#endif /* !ns_UnderwaterPhy_h */
