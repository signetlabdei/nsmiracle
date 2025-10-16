/* **************************************************
 * underwater ns2 code
 * by: Al Harris (thanks to Robin S. for making it go in ns2)
 *
 * Work done while at the University of Padova, Italy with
 * Michele Zorzi
 ******************************************************/

#include <math.h>

#include <packet.h>

#include <mobilenode.h>
#include <phy.h>
#include <propagation.h>
#include <modulation.h>
#include <omni-antenna.h>
#include <underwater-phy.h>
#include <packet.h>
#include <ip.h>
#include <agent.h>
#include <trace.h>

#include <diffusion/diff_header.h>
#include "underwater.h"

#define MAX(a,b) (((a)<(b))?(b):(a))
#define MIN(a,b) (((a)>(b))?(b):(a))


Underwater_Sleep_Timer::Underwater_Sleep_Timer(UnderwaterPhy *a) 
  : TimerHandler()
{ 
  a_ = a; 
}

void Underwater_Sleep_Timer::expire(Event *) 
{
  a_->UpdateSleepEnergy();
}


/* NOTE: I did not use ns to track energy so none of the settings match
 * cards, they are easy to change however */

/* ======================================================================
   UnderwaterPhy Interface
   ====================================================================== */
static class UnderwaterPhyClass: public TclClass {
public:
        UnderwaterPhyClass() : TclClass("Phy/UnderwaterPhy") {}
        TclObject* create(int, const char*const*) {
                return (new UnderwaterPhy);
        }
} class_UnderwaterPhy;


UnderwaterPhy::UnderwaterPhy() : Phy(), sleep_timer_(this), status_(IDLE)
{
fprintf(stderr,"underwaterphy: ()\n");
	/*
	 *  It sounds like 10db should be the capture threshold.
	 *
	 *  If a node is presently receiving a packet a a power level
	 *  Pa, and a packet at power level Pb arrives, the following
	 *  comparion must be made to determine whether or not capture
	 *  occurs:
	 *
	 *    10 * log(Pa) - 10 * log(Pb) > 10db
	 *
	 *  OR equivalently
	 *
	 *    Pa/Pb > 10.
	 *
	 */
	bind("CPThresh_", &CPThresh_);
	bind("CSThresh_", &CSThresh_);
	bind("RXThresh_", &RXThresh_);
	//bind("bandwidth_", &bandwidth_);
	bind("Pt_", &Pt_);
	bind("freq_", &freq_);
	bind("L_", &L_);
	
	lambda_ = SPEED_OF_LIGHT / freq_;

	node_ = 0;
	ant_ = 0;
	propagation_ = 0;
	modulation_ = 0;

	//  These should be set to correspond to your card
	
	Pt_consume_ = 0.660;  
	Pr_consume_ = 0.395; 


	P_idle_ = 0.0;
	P_sleep_ = 0.00;
	P_transition_ = 0.00;
	node_on_=1;
	
	channel_idle_time_ = NOW;
	update_energy_time_ = NOW;
	last_send_time_ = NOW;
	
	sleep_timer_.resched(1.0);

}

int
UnderwaterPhy::command(int argc, const char*const* argv)
{
fprintf(stderr,"underwaterphy: command\n");
	TclObject *obj; 

	if (argc==2) {
		if (strcasecmp(argv[1], "NodeOn") == 0) {
			node_on();

			if (em() == NULL) 
				return TCL_OK;
			if (NOW > update_energy_time_) {
				update_energy_time_ = NOW;
			}
			return TCL_OK;
		} else if (strcasecmp(argv[1], "NodeOff") == 0) {
			node_off();

			if (em() == NULL) 
				return TCL_OK;
			if (NOW > update_energy_time_) {
				em()->DecrIdleEnergy(NOW-update_energy_time_,
						     P_idle_);
				update_energy_time_ = NOW;
			}
			return TCL_OK;
		}
	} else if(argc == 3) {
		if (strcasecmp(argv[1], "setTxPower") == 0) {
			Pt_consume_ = atof(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setRxPower") == 0) {
			Pr_consume_ = atof(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setIdlePower") == 0) {
			P_idle_ = atof(argv[2]);
			return TCL_OK;
		}else if (strcasecmp(argv[1], "setSleepPower") == 0) {
			P_sleep_ = atof(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setTransitionPower") == 0) {
			P_transition_ = atof(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setTransitionTime") == 0) {
			T_transition_ = atof(argv[2]);
			return TCL_OK;
		}else if( (obj = TclObject::lookup(argv[2])) == 0) {
			fprintf(stderr,"UnderwaterPhy: %s lookup of %s failed\n", 
				argv[1], argv[2]);
			return TCL_ERROR;
		}else if (strcmp(argv[1], "propagation") == 0) {
			assert(propagation_ == 0);
			propagation_ = (Underwater*) obj;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "antenna") == 0) {
			ant_ = (Antenna*) obj;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "node") == 0) {
			assert(node_ == 0);
			node_ = (Node *)obj;
			return TCL_OK;
		}
	}
	return Phy::command(argc,argv);
}
 
void 
UnderwaterPhy::sendDown(Packet *p)
{
fprintf(stderr,"underwaterphy: senddown\n");
	/*
	 * Sanity Check
	 */
	assert(initialized());
	
	if (em()) {
			//node is off here...
			if (Is_node_on() != true ) {
			Packet::free(p);
			return;
			}
			if(Is_node_on() == true && Is_sleeping() == true){
			em()-> DecrSleepEnergy(NOW-update_energy_time_,
							P_sleep_);
			update_energy_time_ = NOW;

			}

	}
	/*
	 * Decrease node's energy
	 */
	if(em()) {
		if (em()->energy() > 0) {

		    double txtime = hdr_cmn::access(p)->txtime();
		    double start_time = MAX(channel_idle_time_, NOW);
		    double end_time = MAX(channel_idle_time_, NOW+txtime);
		    double actual_txtime = end_time-start_time;

		    if (start_time > update_energy_time_) {
			    em()->DecrIdleEnergy(start_time - 
						 update_energy_time_, P_idle_);
			    update_energy_time_ = start_time;
		    }

		    /* It turns out that MAC sends packet even though, it's
		       receiving some packets.
		    
		    if (txtime-actual_txtime > 0.000001) {
			    fprintf(stderr,"Something may be wrong at MAC\n");
			    fprintf(stderr,"act_tx = %lf, tx = %lf\n", actual_txtime, txtime);
		    }
		    */

		   // Sanity check
		   double temp = MAX(NOW,last_send_time_);

		   /*
		   if (NOW < last_send_time_) {
			   fprintf(stderr,"Argggg !! Overlapping transmission. NOW %lf last %lf temp %lf\n", NOW, last_send_time_, temp);
		   }
		   */
		   
		   double begin_adjust_time = MIN(channel_idle_time_, temp);
		   double finish_adjust_time = MIN(channel_idle_time_, NOW+txtime);
		   double gap_adjust_time = finish_adjust_time - begin_adjust_time;
		   if (gap_adjust_time < 0.0) {
			   fprintf(stderr,"What the heck ! negative gap time.\n");
		   }

		   if ((gap_adjust_time > 0.0) && (status_ == RECV)) {
			   em()->DecrTxEnergy(gap_adjust_time,
					      Pt_consume_-Pr_consume_);
		   }

		   em()->DecrTxEnergy(actual_txtime,Pt_consume_);
//		   if (end_time > channel_idle_time_) {
//			   status_ = SEND;
//		   }
//
		   status_ = IDLE;

		   last_send_time_ = NOW+txtime;
		   channel_idle_time_ = end_time;
		   update_energy_time_ = end_time;

		   if (em()->energy() <= 0) {
			   em()->setenergy(0);
			   ((MobileNode*)node())->log_energy(0);
		   }

		} else {

			// log node energy
			if (em()->energy() > 0) {
				((MobileNode *)node_)->log_energy(1);
			} 
//
			Packet::free(p);
			return;
		}
	}

	/*
	 *  Stamp the packet with the interface arguments
	 */
	p->txinfo_.stamp((MobileNode*)node(), ant_->copy(), Pt_, lambda_);
	// Send the packet


	channel_->recv(p, this);
}

int 
UnderwaterPhy::sendUp(Packet *p)
{
fprintf(stderr,"underwaterphy: sendup\n");
	/*
	 * Sanity Check
	 */
	assert(initialized());

	PacketStamp s;
	double Pr;
	int pkt_recvd = 0;

	Pr = p->txinfo_.getTxPr();
	
	// if the node is in sleeping mode, drop the packet simply
	if (em()) {
			if (Is_node_on()!= true){
			pkt_recvd = 0;
			goto DONE;
			}

			if (Is_sleeping()==true && (Is_node_on() == true)) {
				pkt_recvd = 0;
				goto DONE;
			}
			
	}
	// if the energy goes to ZERO, drop the packet simply
	if (em()) {
		if (em()->energy() <= 0) {
			pkt_recvd = 0;
			goto DONE;
		}
	}

	if(propagation_) {
		s.stamp((MobileNode*)node(), ant_, 0, lambda_);
		Pr = propagation_->Pr(&p->txinfo_, &s, this);
		fprintf(stderr,"phy: Pr: %f, %d\n",Pr,node_->address());
		if (Pr < CSThresh_) {
			//fprintf(stderr,"failed: below CSThresh\n");
			pkt_recvd = 0;
			goto DONE;
		}
		if (Pr < RXThresh_) {
			/*
			 * We can detect, but not successfully receive
			 * this packet.
			 */
			//fprintf(stderr,"failed: below RXThresh\n");
			hdr_cmn *hdr = HDR_CMN(p);
			hdr->error() = 1;
#if DEBUG > 3
			printf("SM %f.9 _%d_ drop pkt from %d low POWER %e/%e\n",
			       Scheduler::instance().clock(), node()->index(),
			       p->txinfo_.getNode()->index(),
			       Pr,RXThresh);
#endif
		}
		else
		{

			// stuff for debugging, etc:

		  //shannon for now
		  double bitr,pdelay,bandw,power,totdelay,energycon;
				pdelay = propagation_->get_pdelay(&p->txinfo_, &s, this);
				bandw = propagation_->bandwidth(&p->txinfo_, &s, this);
		  bitr = bandw * 100 * log2(1 + pow(10,2));
		  power = ((20.0/1000000.0 * pow(10,Pt_/20.0))/20.0) * 50.0;
		  power = power < 10?10:power;
		  energycon = (HDR_CMN(p)->size() / bitr) * power;
		  totdelay = (HDR_CMN(p)->size() / bitr) + pdelay;
			fprintf(stderr,"RECV: packet size: %d recv'd: %d -> %d: prop: %f, band: %f, bitrate: %f, power: %f, total delay: %f energy: %f\n",
					HDR_CMN(p)->size(),
					p->txinfo_.getNode()->address(),node()->address(),
					pdelay,bandw,bitr/1000,power,totdelay,energycon);

		}
	}
	if(modulation_) {
		/* to do it right... noise should be taken here, not earlier.. just
		 * so you know ;) */
		hdr_cmn *hdr = HDR_CMN(p);
		hdr->error() = modulation_->BitError(Pr);
	}
	
	/*
	 * The MAC layer must be notified of the packet reception
	 * now - ie; when the first bit has been detected - so that
	 * it can properly do Collision Avoidance / Detection.
	 */
	
	//fprintf(stderr,"success\n");
	pkt_recvd = 1;

DONE:
	p->txinfo_.getAntenna()->release();

	/* WILD HACK: The following two variables are a wild hack.
	   They will go away in the next release...
	   They're used by the mac-802_11 object to determine
	   capture.  This will be moved into the net-if family of 
	   objects in the future. */
	p->txinfo_.RxPr = Pr;
	p->txinfo_.CPThresh = CPThresh_;

	/*
	 * Decrease energy if packet successfully received
	 */
	if(pkt_recvd && em()) {

		double rcvtime = hdr_cmn::access(p)->txtime();
		// no way to reach here if the energy level < 0
		
		double start_time = MAX(channel_idle_time_, NOW);
		double end_time = MAX(channel_idle_time_, NOW+rcvtime);
		double actual_rcvtime = end_time-start_time;

		if (start_time > update_energy_time_) {
			em()->DecrIdleEnergy(start_time-update_energy_time_,
					     P_idle_);
			update_energy_time_ = start_time;
		}
		
		em()->DecrRcvEnergy(actual_rcvtime,Pr_consume_);
/*
  if (end_time > channel_idle_time_) {
  status_ = RECV;
  }
*/
		channel_idle_time_ = end_time;
		update_energy_time_ = end_time;

		status_ = IDLE;

		/*
		  hdr_diff *dfh = HDR_DIFF(p);
		  printf("Node %d receives (%d, %d, %d) energy %lf.\n",
		  node()->address(), dfh->sender_id.addr_, 
		  dfh->sender_id.port_, dfh->pk_num, node()->energy());
		*/

		// log node energy
		if (em()->energy() > 0) {
		((MobileNode *)node_)->log_energy(1);
        	} 

		if (em()->energy() <= 0) {  
			// saying node died
			em()->setenergy(0);
			((MobileNode*)node())->log_energy(0);
		}
	}
	
	return pkt_recvd;
}

void
UnderwaterPhy::node_on()
{
fprintf(stderr,"underwaterphy: node_on\n");

        node_on_= TRUE;
	status_ = IDLE;

       if (em() == NULL)
 	    return;	
   	if (NOW > update_energy_time_) {
      	    update_energy_time_ = NOW;
   	}
}

void 
UnderwaterPhy::node_off()
{
fprintf(stderr,"underwaterphy: node_off\n");

        node_on_= FALSE;
	status_ = SLEEP;

	if (em() == NULL)
            return;
        if (NOW > update_energy_time_) {
            em()->DecrIdleEnergy(NOW-update_energy_time_,
                                P_idle_);
            update_energy_time_ = NOW;
	}
}

void 
UnderwaterPhy::node_wakeup()
{
fprintf(stderr,"underwaterphy: node_wakeup\n");

	if (status_== IDLE)
		return;

	if (em() == NULL)
            return;

        if ( NOW > update_energy_time_ && (status_== SLEEP) ) {
		//the power consumption when radio goes from SLEEP mode to IDLE mode
		em()->DecrTransitionEnergy(T_transition_,P_transition_);
		
		em()->DecrSleepEnergy(NOW-update_energy_time_,
				      P_sleep_);
		status_ = IDLE;
	        update_energy_time_ = NOW;
		
		// log node energy
		if (em()->energy() > 0) {
			((MobileNode *)node_)->log_energy(1);
	        } else {
			((MobileNode *)node_)->log_energy(0);   
	        }
	}
}

void 
UnderwaterPhy::node_sleep()
{
fprintf(stderr,"underwaterphy: node_sleep\n");
//
//        node_on_= FALSE;
//
	if (status_== SLEEP)
		return;

	if (em() == NULL)
            return;

        if ( NOW > update_energy_time_ && (status_== IDLE) ) {
	//the power consumption when radio goes from IDLE mode to SLEEP mode
	    em()->DecrTransitionEnergy(T_transition_,P_transition_);

            em()->DecrIdleEnergy(NOW-update_energy_time_,
                                P_idle_);
		status_ = SLEEP;
	        update_energy_time_ = NOW;

	// log node energy
		if (em()->energy() > 0) {
			((MobileNode *)node_)->log_energy(1);
	        } else {
			((MobileNode *)node_)->log_energy(0);   
	        }
	}
}
//
void
UnderwaterPhy::dump(void) const
{
fprintf(stderr,"underwaterphy: node_dump\n");
	Phy::dump();
	fprintf(stdout,
		"\tPt: %f, Gt: %f, Gr: %f, lambda: %f, L: %f\n",
		Pt_, ant_->getTxGain(0,0,0,lambda_), ant_->getRxGain(0,0,0,lambda_), lambda_, L_);
	//fprintf(stdout, "\tbandwidth: %f\n", bandwidth_);
	fprintf(stdout, "--------------------------------------------------\n");
}


void UnderwaterPhy::UpdateIdleEnergy()
{
fprintf(stderr,"underwaterphy: updateidleenergy\n");
	if (em() == NULL) {
		return;
	}
	if (NOW > update_energy_time_ && (Is_node_on()==TRUE && status_ == IDLE ) ) {
		  em()-> DecrIdleEnergy(NOW-update_energy_time_,
					P_idle_);
		  update_energy_time_ = NOW;
	}

	// log node energy
	if (em()->energy() > 0) {
		((MobileNode *)node_)->log_energy(1);
        } else {
		((MobileNode *)node_)->log_energy(0);   
        }

//	idle_timer_.resched(10.0);
}

double UnderwaterPhy::getDist(double Pr, double Pt, double Gt, double Gr,
			    double hr, double ht, double L, double lambda)
{
fprintf(stderr,"underwaterphy: getdist\n");
	if (propagation_) {
		return propagation_->getDist(Pr, Pt, Gt, Gr, hr, ht, L,
					     lambda);
	}
	return 0;
}

//
void UnderwaterPhy::UpdateSleepEnergy()
{
fprintf(stderr,"underwaterphy: updatesleepenergy\n");
	if (em() == NULL) {
		return;
	}
	if (NOW > update_energy_time_ && ( Is_node_on()==TRUE  && Is_sleeping() == true) ) {
		  em()-> DecrSleepEnergy(NOW-update_energy_time_,
					P_sleep_);
		  update_energy_time_ = NOW;
		// log node energy
		if (em()->energy() > 0) {
			((MobileNode *)node_)->log_energy(1);
        	} else {
			((MobileNode *)node_)->log_energy(0);   
        	}
	}
	
	//A hack to make states consistent with those of in Energy Model for AF
	int static s=em()->sleep();
	if(em()->sleep()!=s){

		s=em()->sleep();	
		if(s==1)
			node_sleep();
		else
			node_wakeup();			
		printf("\n AF hack %d\n",em()->sleep());	
	}	
	
	sleep_timer_.resched(10.0);
}
