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
 * $Id: mac802_16.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef _802_16_MAC_H
#define _802_16_MAC_H

#include <mmac-module.h>

#include "ns2wimax/sduclassifier.h"
#include "ns2wimax/connectionmanager.h"
#include "ns2wimax/serviceflowhandler.h"
#include "ns2wimax/serviceflowqos.h"
#include "ns2wimax/peernode.h"
#include "mac802_16pkt.h"
#include "ns2wimax/mac802_16timer.h"
#include "ns2wimax/scheduling/framemap.h"
#include "ns2wimax/scheduling/contentionslot.h"
#include "ns2wimax/scheduling/dlsubframetimer.h"
#include "ns2wimax/scheduling/ulsubframetimer.h"
#include "ns2wimax/neighbordb.h"
#include "ns2wimax/wimaxneighborentry.h"

#include "wimax-clmsg.h"

//Define new debug function for cleaner code
#ifdef DEBUG_WIMAX
#define debug2 printf
#else
#define debug2(arg1,...)
#endif

#define BS_NOT_CONNECTED -1 //bs_id when MN is not connected

#define DL_PREAMBLE 3  //preamble+fch
#define INIT_RNG_PREAMBLE 2
#define BW_REQ_PREAMBLE 1

/** Defines different types of nodes */
enum station_type_t {
	STA_UNKNOWN,
 STA_MN,
 STA_BS
};

/** MAC MIB */
class Mac802_16MIB {
	public:
		Mac802_16MIB (Mac802_16 *parent);
 
		int queue_length;
		double frame_duration;

		double dcd_interval;
		double ucd_interval;
		double init_rng_interval;
		double lost_dlmap_interval;
		double lost_ulmap_interval;
  
		double t1_timeout;
		double t2_timeout;
		double t3_timeout;
		double t6_timeout;
		double t12_timeout;
		double t16_timeout;
		double t17_timeout;
		double t21_timeout;
		double t44_timeout;

		u_int32_t contention_rng_retry;
		u_int32_t invited_rng_retry;
		u_int32_t request_retry;
		u_int32_t reg_req_retry;
		double    tproc;
		u_int32_t dsx_req_retry;
		u_int32_t dsx_rsp_retry;

		u_int32_t rng_backoff_start;
		u_int32_t rng_backoff_stop;
		u_int32_t bw_backoff_start;
		u_int32_t bw_backoff_stop;

		//mobility extension
		u_int32_t scan_duration;
		u_int32_t interleaving;
		u_int32_t scan_iteration;
		u_int32_t max_dir_scan_time;
		double    nbr_adv_interval;
		u_int32_t scan_req_retry;

		//miscalleous
		double rxp_avg_alpha;  //for measurements
		double lgd_factor_;
		double RXThreshold_;
		double client_timeout; //used to clear information on BS side
};

/** PHY MIB */
class Phy802_16MIB {
	public:
		Phy802_16MIB (Mac802_16 *parent);
 
		int channel; //current channel
		double fbandwidth;
		u_int32_t ttg;
		u_int32_t rtg;
};

class WimaxScheduler;
class FrameMap;
class StatTimer;
class DlTimer;
class UlTimer;

/**
 * Class implementing IEEE 802_16
 */
class Mac802_16 : public MMacModule
{
	friend class PeerNode;
	friend class SDUClassifier;
	friend class WimaxFrameTimer;
	friend class FrameMap;
	friend class WimaxScheduler;
	friend class BSScheduler;
	friend class SSscheduler;
	friend class ServiceFlowHandler;
	friend class Connection;
	friend class StatTimer;
	friend class InitTimer;

	public:
		/** Constructor */
		Mac802_16();

		/** Destructor */
		virtual ~Mac802_16();

		  /**
		 * Return the connection manager
		 * @return the connection manager
		   */
		inline ConnectionManager *  getCManager () { return connectionManager_; }
  
		/**
		 * Return The Service Flow handler
		 * @return The Service Flow handler
		 */
		inline ServiceFlowHandler *  getServiceHandler () { return serviceFlowHandler_; }
  
		/**
		 * Return the Scheduler
		 * @return the Scheduler
		 */
		inline WimaxScheduler * getScheduler () { return scheduler_; }

		/**
		 * Return the frame duration (in s)
		 * @return the frame duration (in s)
		 */
		double  getFrameDuration () { return macmib_.frame_duration; }
  
		/**
		 * Set the frame duration
		 * @param duration The frame duration (in s)
		 */
		void  setFrameDuration (double duration) { macmib_.frame_duration = duration; }

		/**
		 * Return the current frame number
		 * @return the current frame number
		 */
		int getFrameNumber ();

		/**
		 * Return the type of MAC
		 * @return the type of node
		 */
		station_type_t getNodeType();

		/**
		 * Interface with the TCL script
		 * @param argc The number of parameter
		 * @param argv The list of parameters
		 */
		virtual int command (int argc, const char *const *argv);

		/**
		 * Change the channel
		 * @param channel The new channel
		 */
		void setChannel (int channel);

		/**
		 * Return the channel index
		 * @return The channel
		 */
		int getChannel ();

		/**
		 * Return the channel number for the given frequency
		 * @param freq The frequency
		 * @return The channel number of -1 if the frequency does not match
		 */
		int getChannel (double freq);

		/**
		 * Set the channel to the next from the list
		 * Used at initialisation and when loosing signal
		 */
		void nextChannel ();

		/**
		 * Process packets going out
		 * @param p The packet to transmit
		 */
		virtual void tx(Packet *p);

		/**
		 * Process packets going out
		 * @param p The packet to transmit
		 */
		virtual void transmit(Packet *p);

		/**
		 * Process incoming packets
		 * @param p The received packet
		 */
		virtual void rx(Packet *p);

		/**
		 * Process the packet after receiving last bit
		 */
		virtual void receive();

		/**
		 * Set the variable used to find out if upper layers
		 * must be notified to send packets. During scanning we
		 * do not want upper layers to send packet to the mac.
		 * @param notify Value indicating if we want to receive packets
		 * from upper layers
		 */
		void setNotify_upper (bool notify);

		/**
		 * Return the head of the peer nodes list
		 * @return the head of the peer nodes list
		 */
		PeerNode * getPeerNode_head () { return peer_list_->lh_first; }

		/**
		 * Return the peer node that has the given address
		 * @param index The address of the peer
		 * @return The peer node that has the given address
		 */
		PeerNode *getPeerNode (int index);

		/**
		 * Add the peer node
		 * @param The peer node to add
		 */
		void addPeerNode (PeerNode *node);

		/**
		 * Remove a peer node
		 * @param The peer node to remove
		 */
		void removePeerNode (PeerNode *node);

		/**
		 * Return the number of peer nodes
		 */
		int getNbPeerNodes ();

		/**
		 * Start a new DL subframe
		 */
		virtual void start_dlsubframe ();
  
		/**
		 * Start a new UL subframe
		 */
		virtual void start_ulsubframe ();

		/**
		 * Called when a timer expires
		 * @param The timer ID
		 */
		virtual void expire (timer_id id);

		/**
		 * Return the MAP of the current frame
		 * @return the MAP of the current frame
		 */
		FrameMap *getMap () { return map_;}

		/**
		 * WILD HACK: Minimizing the changes in original code
		 * We don't use phy directly anymore, but with wrappers via ClMessages
		 * @return pointer to Mac802_16 pointer (our self)
		 */
		virtual Mac802_16* getPhy();
		
		/**
		 * The MAC MIB
		 */
		Mac802_16MIB macmib_;

		/**
		 * The Physical layer MIB
		 */
		Phy802_16MIB phymib_;

#ifdef USE_802_21 //Switch to activate when using 802.21 modules (external package)
		/**
		 * Configure/Request configuration
		 * The upper layer sends a config object with the required
		 * new values for the parameters (or PARAMETER_UNKNOWN_VALUE).
		 * The MAC tries to set the values and return the new setting.
		 * For examples if a MAC does not support a parameter it will
		 * return  PARAMETER_UNKNOWN_VALUE
		 * @param config The configuration object
		 */
		void link_configure (link_parameter_config_t* config);

		/**
		 * Configure the threshold values for the given parameters
		 * @param numLinkParameter number of parameter configured
		 * @param linkThresholds list of parameters and thresholds
		 */
		struct link_param_th_status * link_configure_thresholds (int numLinkParameter, struct link_param_th *linkThresholds); //configure threshold
        
		/**
		 * Disconnect from the PoA
		 */
		virtual void link_disconnect ();

		/**
		 * Connect to the PoA
		 * @param poa The address of PoA
		 */
		virtual void link_connect (int poa);
   
		/**
		 * Scan channel
		 * @param req the scan request information
		 */
		virtual void link_scan (void *req);

		/**
		 * Set the operation mode
		 * @param mode The new operation mode
		 * @return true if transaction succeded
		 */
		bool set_mode (mih_operation_mode_t mode);
#endif
		/**
		 * Return the duration of a PS (physical slot), unit for allocation time.
		 * Use Frame duration / PS to find the number of available slot per frame
		 */
		virtual double getPS();
		
		/**
		 * Return the number of PS used by an OFDM symbol
		 */
		virtual int getSymbolPS();

		/**
		 * Return the OFDM symbol duration time
		 */
		virtual double getSymbolTime();

		/**
		 * Get frequency at which the phy is operating
		 * @return frequency as a double
		 */
		virtual double getFreq();

		/**
		 * Compute the transmission time for a packet of size sdusize and
		 * using the given modulation rate
		 * @param sdusize Size in bytes of the data to send
		 * @param mod The modulation to use
		 */
		virtual double getTrxTime(int sdusize, Ofdm_mod_rate mod);

		/**
		 * Compute the transmission time using OFDM symbol as
		 * minimum allocation for a packet of size sdusize and
		 * using the given modulation rate
		 * @param sdusize Size in bytes of the data to send
		 * @param mod The modulation to use
		 */
		virtual double getTrxSymbolTime(int sdusize, Ofdm_mod_rate mod);

		/**
		 * Return the maximum size in bytes that can be sent for the given
		 * nb of symbols and modulation
		 */
		virtual int getMaxPktSize(double nbsymbols, Ofdm_mod_rate mod);

		/**
		 * Change the frequency at which the phy is operating
		 * @param freq The new frequency
		 */
		virtual void setFrequency(double freq);

		/**
		 * Set the new modulation for the physical layer
		 * @param modulation The new physical modulation
		 */
		virtual void setModulation(Ofdm_mod_rate modulation);

		/** 
		 * Set the mode for physical layer
		 */
		virtual void setMode(Ofdm_phy_state mode);

		/**
		 * Activate node
		 */
		virtual void node_on();
		
		/**
		 * Deactivate node
		 */
		virtual void node_off();

	protected:
		/**
		 * Init the MAC
		 */
		virtual void init ();

		/**
		 * The packet scheduler
		 */
		WimaxScheduler * scheduler_;

		/**
		 * Return a new allocated packet
		 * @return A newly allocated packet
		 */
		Packet * getPacket();

		/**
		 * Return the code for the frame duration
		 * @return the code for the frame duration
		 */
		int getFrameDurationCode ();

		/**
		 * Set the frame duration using code
		 * @param code The frame duration code
		 */
		void setFrameDurationCode (int code);

		/**
		 * Add a classifier
		 */
		void addClassifier (SDUClassifier *);

		/**
		 * Run the packet through the classifiers
		 * to find the proper connection
		 * @param p the packet to classify
		 */
		int classify (Packet *p);

		/**
		 * Timer to init the MAC
		 */
		InitTimer *initTimer_;

		/**
		 * The map of the frame
		 */
		FrameMap *map_;

		/**
		 * Current frame number
		 */
		int frame_number_;

		/**
		 * Timer used to mark the begining of downlink subframe (i.e new frame)
		 */
		DlTimer *dl_timer_;

		/**
		 * Timer used to mark the begining of uplink subframe
		 */
		UlTimer *ul_timer_;

		/**
		 * Statistics for queueing delay
		 */
		StatWatch delay_watch_;

		/**
		 * Delay for last packet
		 */
		double last_tx_delay_;

		/**
		 * Statistics for delay jitter
		 */
		StatWatch jitter_watch_;

		/**
		 * Stats for packet loss
		 */
		StatWatch loss_watch_;

		/**
		 * Stats for incoming data throughput
		 */
		ThroughputWatch rx_data_watch_;

		/**
		 * Stats for incoming traffic throughput (data+management)
		 */
		ThroughputWatch rx_traffic_watch_;


		/**
		 * Stats for outgoing data throughput
		 */
		ThroughputWatch tx_data_watch_;

		/**
		 * Stats for outgoing traffic throughput (data+management)
		 */
		ThroughputWatch tx_traffic_watch_;

		/**
		 * Timers to continuously poll stats in case it is not updated by
		 * sending or receiving packets
		 */
		StatTimer *rx_data_timer_;
		StatTimer *rx_traffic_timer_;
		StatTimer *tx_data_timer_;
		StatTimer *tx_traffic_timer_;

		/**
		 * Indicates if the stats must be printed
		 */
		int print_stats_;

		/**
		 * Update the given timer and check if thresholds are crossed
		 * @param watch the stat watch to update
		 * @param value the stat value
		 */
		virtual void update_watch (StatWatch *watch, double value);

		/**
		 * Update the given timer and check if thresholds are crossed
		 * @param watch the stat watch to update
		 * @param size the size of packet received
		 */
		virtual void update_throughput (ThroughputWatch *watch, double size);

#ifdef USE_802_21 //Switch to activate when using 802.21 modules (external package)
		/**
		 * Poll the given stat variable to check status  (not implemented)
		 * @param type The link parameter type
		 */
		void poll_stat (link_parameter_type_s type);
#endif

		/**
		 * Object to log received packets. Private in Mac so we need to redefine it
		 */
		NsObject*	logtarget_;

		/**
		 * Packet being received
		 */
		Packet *pktRx_;

		/**
		 * A packet buffer used to temporary store a packet
		 * received by upper layer. Used during scanning
		 */
		Packet *pktBuf_;

		/**
		 * Set the node type (not implemented)
		 * @param type The station type
		 */
		void setStationType (station_type_t type);
		
		/**
		 * Get the txtime (wrapper for getTrxTime)
		 * @param p the given packet
		 * @return the txtime
		 */
		virtual double txtime(Packet *p);

		/**
		* The type of station (MN or BS)
		*/
		station_type_t type_;

		/**
		 * Receiving timer
		 */
		WimaxRxTimer rxTimer_;

		/**
		 * Indicates if a collision occured
		 */
		bool collision_;

		/**
		 * Indicate if upper layer must be notified to send more packets
		 */
		bool notify_upper_;

		/**
		 * Last time a packet was sent
		 */
		double last_tx_time_;

		/**
		 * Last transmission duration
		 */
		double last_tx_duration_;

		/**
		 * The class to handle connections
		 */
		ConnectionManager * connectionManager_;

		/**
		 * The module that handles flow requests
		 */
		ServiceFlowHandler * serviceFlowHandler_;

		/**
		 * List of connected peer nodes. Only one for SSs.
		 */
		struct peerNode *peer_list_;

		/**
		 * Number of peer in the list
		 */
		int nb_peer_;

		/**
		 * Database of neighboring BS
		 */
		NeighborDB *nbr_db_;


	private:
		/**
		 * The list of classifier
		 */
		struct sduClassifier classifier_list_;
};


/** Class to poll stats */
class StatTimer : public TimerHandler {
	public:
		StatTimer (Mac802_16 *mac, ThroughputWatch *watch) : TimerHandler() {
			mac_ = mac;
			watch_ = watch;
			timer_interval_ = 0.100000000001; //default 100ms+a little off to avoid synch
			resched (timer_interval_);
		}
		void expire (Event *) {
			mac_->update_throughput (watch_, 0);
			//double tmp = watch_->get_timer_interval();
			//resched(tmp > 0? tmp: timer_interval_);
		}
		inline void set_timer_interval(double ti) { timer_interval_ = ti; }
	private:
		Mac802_16 *mac_;
		ThroughputWatch *watch_;
		double timer_interval_;
};


#endif
