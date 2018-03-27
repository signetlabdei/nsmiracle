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
 * $Id: wimaxctrlagent.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef WIMAXCTRLAGENT_H
#define WIMAXCTRLAGENT_H

#include "module.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"
#include "random.h"
#include "mac802_16pkt.h"

#define MAX_MAP_ENTRY 10
#define MAX_MN_REQ    10  //maximum number of concurrent MN requests 
#define UPDATE_JITTER 0.5 //jitter at start up to avoid synchronization
#define START_FRAME_OFFSET 2 //in the response we set teh start_frame to this value

/* 
 * Packet structure for 
 */
#define HDR_WIMAXBS(p)    ((struct hdr_wimaxbs*)(p)->access(hdr_wimaxbs::offset_))

/** Types of message */
enum wimaxbs_type {
  WIMAX_BS_ADV,
  WIMAX_BS_SYNCH_REQ,
  WIMAX_BS_SYNCH_RSP,
};

/** Structure of packet exchanged between BSs */
struct hdr_wimaxbs {
  wimaxbs_type subtype_; //type of message

  uint32_t macaddr_;     //address of the MAC of interest in this message

  //data for BS association request
  int cid; //to know for which connection the message is
  wimax_scanning_type scanning_type;
  int current_frame;
  int rdvt; //rendez-vous time in units of frame
  double rendezvous_time; //rendez-vous time in seconds

  static int offset_;
  inline static int& offset() { return offset_; }
  inline static hdr_wimaxbs* access(Packet* p) {
    return (hdr_wimaxbs*) p->access(offset_);
  }
  inline wimaxbs_type& getType() {return subtype_;}
  inline uint32_t& macAddr() { return macaddr_; }
};

class WimaxCtrlAgent;

class Scan_req;
LIST_HEAD (scan_req, Scan_req);
//structure to handle scan requests

/** 
 * Timer to send an update to neighboring BSs
 */
class ScanRspTimer : public TimerHandler {
 public:
        ScanRspTimer(Scan_req *a) : TimerHandler() 
	  { a_ = a;}
 protected:
        void expire(Event *);
        Scan_req *a_;
};

/**
 * Store information about a pending scan request
 */
class Scan_req {
 public:
  Scan_req (WimaxCtrlAgent *agent, double delay, int cid, mac802_16_mob_scn_req_frame *req) {
    agent_ = agent;
    timer_ = new ScanRspTimer (this);
    timer_->sched (delay);
    cid_ = cid;
    memcpy (&req_, req, sizeof (mac802_16_mob_scn_req_frame)); 
  }

  int cid() { return cid_; }
  int& start_frame() { return start_frame_; }
  int& pending_rsp () { return pending_rsp_; }
  WimaxCtrlAgent *agent() {return agent_; }
  mac802_16_mob_scn_req_frame *request() { return &req_; }
  mac802_16_mob_scn_rsp_frame *response() { return &rsp_; }
 
  inline void cancel_timer () {
    if (timer_->status()==TIMER_PENDING)
      timer_->cancel();
  }

  inline void sched_timer (double time) {
    timer_->sched (time);
  }
  // Chain element to the list
  inline void insert_entry(struct scan_req *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Return next element in the chained list
  Scan_req* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    cancel_timer ();
    LIST_REMOVE(this, link); 
  }
 protected:
  /*
   * Pointer to next in the list
   */
  LIST_ENTRY(Scan_req) link;
  //LIST_ENTRY(Scan_req); //for magic draw

 private:
  int cid_;                         //CID of the connection the request came from
  mac802_16_mob_scn_req_frame req_; //store request data
  mac802_16_mob_scn_rsp_frame rsp_; //store response data
  int pending_rsp_;                 //number of pending responses
  int start_frame_;                  //frame number when the serving BS decided the rendez-vous time
  ScanRspTimer *timer_;
  WimaxCtrlAgent *agent_;
};

/** 
 * Timer to send an update to neighboring BSs
 */
class UpdateTimer : public TimerHandler {
 public:
        UpdateTimer(WimaxCtrlAgent *a) : TimerHandler() 
	  { a_ = a;}
 protected:
        void expire(Event *);
        WimaxCtrlAgent *a_;
};

class Mac802_16BS;
/** 
 * Agnet to handle communication between BSs
 */
class WimaxCtrlAgent : public Module {
  
 public:
  /**
   * Constructor
   */
  WimaxCtrlAgent();

  /* 
   * Interface with TCL interpreter
   * @param argc The number of elements in argv
   * @param argv The list of arguments
   * @return TCL_OK if everything went well else TCL_ERROR
   */
  int command(int argc, const char*const* argv);

  /* 
   * Process received packet
   * @param p The packet received
   * @param h The handler that sent the packet
   */
  void recv(Packet*);

  /*
   * Send an update (DCD/UCD) to all neighboring BSs
   */
  void sendUpdate ();

  /**
   * Process a request from a MN
   * @param req The request
   */
  virtual void process_scan_request (Packet *req);

  /**
   * Process synchronization request
   * @param p The request
   */
  virtual void process_synch_request (Packet *p);

  /**
   * Process synchronization response
   * @param p The response
   */
  virtual void process_synch_response (Packet *p);

  /**
   * Send a scan response to the MN that has the given CID
   * @param cid The CID of the MN
   */
  virtual void send_scan_response (int cid);

 protected:
  /**
   * Process an update message
   */
  void processUpdate(Packet* p);

  Mac802_16BS *mac_;

  /**
   * Timer to send update messages to neighbor BSs
   */
  UpdateTimer updatetimer_;

  /**
   * time interval between updates
   */
  double adv_interval_;

  /**
   * Table mapping neighbor IP address and Mac address
   */
  int maptable_[MAX_MAP_ENTRY][2]; 

  /**
   * number of element in the mapping table
   */
  int nbmapentry_;

  /**
   * Default association level
   */
  int defaultlevel_; 

  /**
   * Contains list of requests
   */
  struct scan_req scan_req_head_;

  /**
   * Synchronization delay in unit of frame (i.e time we wait for synchronization with BSs)
   */
  int synch_frame_delay_;

  /**
   * Allocate a new packet
   */
   Packet* allocpkt();
  
   /**
    * Return the ip address of the node
    */
   int getNodeIp() { return nodeIp; }
   
   private:
	nsaddr_t nodeIp;	
};


#endif
