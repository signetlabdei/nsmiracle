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
 * $Id: mac802_16SS.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef MAC802_16SS_H
#define MAC802_16Ss_H

#include "mac802_16.h"

#define BS_NOT_CONNECTED -1 //bs_id when MN is not connected

/** Defines the state of the MAC */
enum Mac802_16State {
	MAC802_16_DISCONNECTED,
	MAC802_16_WAIT_DL_SYNCH,
	MAC802_16_WAIT_DL_SYNCH_DCD,
	MAC802_16_UL_PARAM,
	MAC802_16_RANGING,
	MAC802_16_WAIT_RNG_RSP,
	MAC802_16_REGISTER,
	MAC802_16_SCANNING,
	MAC802_16_CONNECTED
};

/** Data structure to store MAC state */
struct state_info {
	Mac802_16State state;
	int bs_id;
	double frameduration;
	int frame_number;
	int channel;
	ConnectionManager * connectionManager;
	ServiceFlowHandler * serviceFlowHandler;
	struct peerNode *peer_list;
	int nb_peer;
};

/** The sub state (while connected) */
enum ss_sub_state {
	NORMAL,           //Normal state
	SCAN_PENDING,     //Normal period but pending scanning to start/resume
	SCANNING,         //Currently scanning
	HANDOVER_PENDING, //Normal state but handover to start
	HANDOVER          //Executing handover
};

/** Data structure to store scanning information */
struct scanning_structure {
	struct mac802_16_mob_scn_rsp_frame *rsp; //response from BS
	struct sched_state_info scan_state;     //current scanning state
	struct sched_state_info normal_state;   //backup of normal state
	int iteration;                          //current iteration
	WimaxScanIntervalTimer *scn_timer_;     //timer to notify end of scanning period
	int count;                              //number of frame before switching to scanning
	ss_sub_state substate;
	WimaxNeighborEntry *nbr; //current neighbor during scanning or handover
	//arrays of rdv timers
	WimaxRdvTimer *rdv_timers[2*MAX_NBR];
	int nb_rdv_timers;
	//handoff information
	int serving_bsid;
	int handoff_timeout; //number frame to wait before executing handoff
};

/**
 * Class implementing IEEE 802_16 State Machine at the Subscriber Station (SS)
 */
class Mac802_16SS : public Mac802_16 {
	friend class BwRequest;

	public:
		/** Constructor*/
		Mac802_16SS();
		
		/**
		 * Interface with the TCL script
		 * @param argc The number of parameter
		 * @param argv The list of parameters
		 */
		int command(int argc, const char*const* argv);

		/**
		 * Set the mac state
		 * @param state The new mac state
		 */
		void setMacState (Mac802_16State state);

		/**
		 * Return the mac state
		 * @return The new mac state
		 */
		Mac802_16State getMacState ();

		/**
		 * Creates a snapshot of the MAC's state and reset it
		 * @return The snapshot of the MAC's state
		 */
		state_info *backup_state ();

		/**
		 * Restore the state of the Mac
		 * @param state The state to restore
		 */
		void restore_state (state_info *state);

		/**
		 * Process packets going out
		 * @param p The packet to transmit
		 */
		void transmit(Packet *p);

		/**
		 * Process packets going out
		 * @param p The packet to transmit
		 */
		void tx(Packet *p);

		/**
		 * Process incoming packets
		 * @param p The received packet
		 */
		void rx(Packet *p);

		/**
		 * Process the packet after receiving last bit
		 */
		void receive();
		
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
		 * Disconnect from the PoA
		 */
		void link_disconnect ();

		/**
		 * Connect to the PoA
		 * @param poa The address of PoA
		 */
		void link_connect (int poa);

		/**
		 * Scan channel
		 * @param req the scan request information
		 */
		void link_scan (void *req);

		/**
		 * Set the operation mode
		 * @param mode The new operation mode
		 * @return true if transaction succeded
		 */
		bool set_mode (mih_operation_mode_t mode);
#endif

	protected:

		/**
		 * init the timers and state
		 */
		void init ();

		/**
		 * Initialize default connection
		 */
		void init_default_connections ();

		/**
		 * Called when lost synchronization
		 */
		void lost_synch ();

		/**
		 * Start a new DL subframe
		 */
		void start_dlsubframe ();

		/**
		 * Start a new UL subframe
		 */
		void start_ulsubframe ();

		/**
		 * Start/Continue scanning
		 */
		void resume_scanning ();

		/**
		 * Pause scanning
		 */
		void pause_scanning ();
		
		/**
		 * Update the given timer and check if thresholds are crossed
		 * @param watch the stat watch to update
		 * @param value the stat value
		 */
		void update_watch (StatWatch *watch, double value);

		/**
		 * Update the given timer and check if thresholds are crossed
		 * @param watch the stat watch to update
		 * @param size the size of packet received
		 */
		void update_throughput (ThroughputWatch *watch, double size);

#ifdef USE_802_21 //Switch to activate when using 802.21 modules (external package)
		/**
		 * Poll the given stat variable to check status
		 * @param type The link parameter type
		 */
		void poll_stat (link_parameter_type_s type);
  
#endif
		/**
		 * Called when a timer expires
		 * @param The timer ID
		 */
		virtual void expire (timer_id id);

	private:
		/**
		 * Process a MAC type packet
		 * @param p The packet to process
		 */
		void process_mac_packet (Packet *p);

		/**
		 * Process a DL_MAP message
		 * @param frame The dl_map information
		 */
		void process_dl_map (mac802_16_dl_map_frame *frame);

		/**
		 * Process a DCD message
		 * @param frame The dcd information
		 */
		void process_dcd (mac802_16_dcd_frame *frame);

		/**
		 * Process a UL_MAP message
		 * @param frame The ul_map information
		 */
		void process_ul_map (mac802_16_ul_map_frame *frame);

		/**
		 * Process a UCD message
		 * @param frame The ucd information
		 */
		void process_ucd (mac802_16_ucd_frame *frame);

		/**
		 * Process a ranging response message
		 * @param frame The ranging response frame
		 */
		void process_ranging_rsp (mac802_16_rng_rsp_frame *frame);

		/**
		 * Process a registration response message
		 * @param frame The registration response frame
		 */
		void process_reg_rsp (mac802_16_reg_rsp_frame *frame);

		/**
		 * Schedule a ranging
		 */
		void init_ranging ();

		/**
		 * Prepare to send a registration message
		 */
		void send_registration ();

		/**
		 * Send a scanning message to the serving BS
		 */
		void send_scan_request ();

		/**
		 * Process a scanning response message
		 * @param frame The scanning response frame
		 */
		void process_scan_rsp (mac802_16_mob_scn_rsp_frame *frame);

		/**
		 * Process a BSHO-RSP message
		 * @param frame The handover response frame
		 */
		void process_bsho_rsp (mac802_16_mob_bsho_rsp_frame *frame);

		/**
		 * Process a BSHO-RSP message
		 * @param frame The handover response frame
		 */
		void process_nbr_adv (mac802_16_mob_nbr_adv_frame *frame);

		/**
		 * Send a MSHO-REQ message to the BS
		 */
		void send_msho_req ();

		/**
		 * Check rdv point when scanning
		 */
		void check_rdv ();

		/**
		 * Set the scan flag to true/false
		 * param flag The value for the scan flag
		 */
		void setScanFlag(bool flag);

		/**
		 * return scan flag
		 * @return the scan flag
		 */
		bool isScanRunning();

		/**
		 * A packet buffer used to temporary store a packet
		 * received by upper layer. Used during scanning
		 */
		Packet *pktBuf_;

		/**
		 * The state of the MAC
		 */
		Mac802_16State state_;

		/**
		 * Current number of registration retry
		 */
		u_int32_t nb_reg_retry_;

		/**
		 * Current number of scan request retry
		 */
		u_int32_t nb_scan_req_;

		/**
		 * Timers
		 */
		WimaxT1Timer  *t1timer_;
		WimaxT2Timer  *t2timer_;
		WimaxT6Timer  *t6timer_;
		WimaxT12Timer *t12timer_;
		WimaxT21Timer *t21timer_;
		WimaxLostDLMAPTimer *lostDLMAPtimer_;
		WimaxLostULMAPTimer *lostULMAPtimer_;
		WimaxT44Timer *t44timer_;

		/**
		 * The scanning information
		 */
		struct scanning_structure *scan_info_;

		/**
		 * Indicates if scan has been requested
		 */
		bool scan_flag_;

		/**
		 * Default DIUC to use.
		 */
		u_char default_diuc_;
};

#endif  //MAC802_16SS_H
