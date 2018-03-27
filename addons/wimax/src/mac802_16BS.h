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
 * $Id: mac802_16BS.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef MAC802_16BS_H
#define MAC802_16BS_H

#include "mac802_16.h"
#include "ns2wimax/scheduling/wimaxctrlagent.h"
#include "ns2wimax/scheduling/scanningstation.h"

/** Information about a new client */
struct new_client_t {
  int cid; //primary cid of new client
  new_client_t *next;
};

class T17Element;
LIST_HEAD (t17element, T17Element);
/** Object to handle timer t17 */
class T17Element {
 public:
  T17Element (Mac802_16 *mac, int index) {
    index_ = index;
    timer = new WimaxT17Timer (mac, index);
    timer->start (mac->macmib_.t17_timeout);
  }

  ~T17Element () { delete (timer); }

  int index () { return index_; }
  int paused () { return timer->paused(); }
  void cancel () { timer->stop(); }

  // Chain element to the list
  inline void insert_entry(struct t17element *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Return next element in the chained list
  T17Element* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }
 protected:
  
  /*
   * Pointer to next in the list
   */
  LIST_ENTRY(T17Element) link;
  //LIST_ENTRY(T17Element); //for magic draw

 private:
  int index_;
  WimaxT17Timer *timer;
};

class FastRangingInfo;
LIST_HEAD (fastRangingInfo, FastRangingInfo);
/** Store information about a fast ranging request to send */
class FastRangingInfo {
 public:
  FastRangingInfo (int frame, int macAddr) {
    frame_ = frame;
    macAddr_ = macAddr;
  }

  int frame () { return frame_; }
  int macAddr () { return macAddr_; }

  // Chain element to the list
  inline void insert_entry(struct fastRangingInfo *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Return next element in the chained list
  FastRangingInfo* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }
 protected:
  
  /*
   * Pointer to next in the list
   */
  LIST_ENTRY(FastRangingInfo) link;
  //LIST_ENTRY(FastRangingInfo); //for magic draw

 private:
  int frame_;
  int macAddr_;
};


/**
 * Class implementing IEEE 802_16 State machine at the Base Station (BS)
 */ 
class Mac802_16BS : public Mac802_16 {
  friend class WimaxCtrlAgent;
  friend class BSScheduler;
 public:

  Mac802_16BS();

  /**
   * Interface with the TCL script
   * @param argc The number of parameter
   * @param argv The list of parameters
   */
  int command(int argc, const char*const* argv);

  /**
   * Process packets going out
   * @param p The packet to transmit
   */
  void tx(Packet *p);

  /**
   * Process packets going out
   * @param p The packet to transmit
   */
  void transmit(Packet *p);
        
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
   /* 
    * Configure/Request configuration
    * The upper layer sends a config object with the required 
    * new values for the parameters (or PARAMETER_UNKNOWN_VALUE).
    * The MAC tries to set the values and return the new setting.
    * For examples if a MAC does not support a parameter it will
    * return  PARAMETER_UNKNOWN_VALUE
    * @param config The configuration object
    */ 
   void link_configure (link_parameter_config_t* config);

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

  /**
   * Start a new DL subframe
   */
  virtual void start_dlsubframe ();

  /**
   * Start a new UL subframe
   */
  virtual void start_ulsubframe ();

  /** 
   * Finds out if the given station is currently scanning
   * @param nodeid The MS id
   */
  bool isPeerScanning (int nodeid);

    /**
   * Set the control agent
   * @param agent The control agent
   */
  void setCtrlAgent (WimaxCtrlAgent *agent);

  /** Add a new Fast Ranging allocation
   * @param time The time when to allocate data
   * @param macAddr The MN address
   */
  void addNewFastRanging (double time, int macAddr);

  /**
   * Send a scan response to the MN
   * @param rsp The response from the control
   * @cid The CID for the MN
   */
  void send_scan_response (mac802_16_mob_scn_rsp_frame *rsp, int cid);

  /**
   * Indicates if it is time to send a DCD message
   */
  bool sendDCD;

  /**
   * DL configuration change count
   */
  int dlccc_;
  
  /**
   * Indicates if it is time to send a UCD message
   */
  bool sendUCD;

  /**
   * UL configuration change count
   */
  int ulccc_;

 private:

  /**
    * Process a MAC type packet
    * @param con The connection by which it arrived
    * @param p The packet to process
    */
  void process_mac_packet (Connection *con, Packet *p);

  /**
   * Process a RNG-REQ message
   * @param frame The ranging request information
   */
  void process_ranging_req (Packet *p);

  /**
   * Process bandwidth request
   * @param p The request
   */
  void process_bw_req (Packet *p);

  /**
   * Process bandwidth request
   * @param p The request
   */
  void process_reg_req (Packet *p);

  /**
   * Process handover request
   * @param req The request
   */
  void process_msho_req (Packet *req);
 
  /**
   * Process handover indication
   * @param p The indication
   */
  void process_ho_ind (Packet *p);
 
  /**
   * Send a neighbor advertisement message
   */
  void send_nbr_adv ();

   /**
   * Add a new timer 17 for client
   * @param index The client address
   */
  void addtimer17 (int index);
  
  /**
   * Cancel and remove the timer17 associated with the node
   * @param index The client address
   */
  void removetimer17 (int index);

  
  /**
   * Pointer to the head of the list of nodes that should
   * perform registration
   */
  struct t17element t17_head_;

  /**
   * Pointer to the head of the list of clients
   */
  struct new_client_t *cl_head_;

  /**
   * Pointer to the tail of the list of clients
   */
  struct new_client_t *cl_tail_;

  /**
   * The index of the last SS that had bandwidth allocation
   */
  int bw_node_index_;
  
  /**
   * The node that had the last bandwidth allocation
   */
  PeerNode *bw_peer_; 

  /**
   * Timer for DCD
   */
  WimaxDCDTimer *dcdtimer_; 

  /**
   * Timer for UCD
   */
  WimaxUCDTimer *ucdtimer_;

  /**
   * Timer for MOB-NBR_ADV messages
   */
  WimaxMobNbrAdvTimer *nbradvtimer_;

  /** 
   * List of station in scanning 
   */
  struct scanningStation scan_stations_;
  
  /**
   * The Wimax control for BS synchronization
   */
  WimaxCtrlAgent *ctrlagent_;
  
  /**
   * List of the upcoming Fast Ranging allocation 
   */
  struct fastRangingInfo fast_ranging_head_;

};

#endif //MAC802_16BS_H

