/* -*- Mode:C++ -*- */

/*
 * Copyright (c) 2007 Regents of the SIGNET lab, University of Padova.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Padova (SIGNET lab) nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 
/**
 * @file   mmac-aloha-adv.h
 * @author Federico Guerra
 * @date   Feb 2009
 * 
 * @brief  
 * 
 * 
 */


#ifndef MMAC_ALOHA_ADV_H 
#define MMAC_ALOHA_ADV_H

#include <mmac.h>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <queue>
// #include <utility>
// #include "mtrand.h"
#include <fstream>

#include <mphy.h>

#define ALOHA_DROP_REASON_WRONG_STATE "WST"
#define ALOHA_DROP_REASON_WRONG_RECEIVER "WRCV"
#define ALOHA_DROP_REASON_UNKNOWN_TYPE "UPT"
#define ALOHA_DROP_REASON_BUFFER_FULL "DBF"
#define ALOHA_DROP_REASON_ERROR "ERR"

extern packet_t PT_MMAC_ACK;


class MMacAlohaAdv : public MMac {

  
  public:

    
  MMacAlohaAdv();
  virtual ~MMacAlohaAdv();

  virtual int command(int argc, const char*const* argv);
  virtual int crLayCommand(ClMessage* m);

  
  protected:


  static const double prop_speed;
  
  enum ALOHA_STATUS {
    ALOHA_STATE_IDLE = 1, ALOHA_STATE_BACKOFF, ALOHA_STATE_TX_DATA, ALOHA_STATE_TX_ACK, ALOHA_STATE_WAIT_ACK, 
    ALOHA_STATE_DATA_RX, ALOHA_STATE_ACK_RX, ALOHA_STATE_NOT_SET, ALOHA_STATE_LISTEN,
    ALOHA_STATE_CHK_ACK_TIMEOUT, ALOHA_STATE_CHK_LISTEN_TIMEOUT, ALOHA_STATE_CHK_BACKOFF_TIMEOUT, ALOHA_STATE_RX_IDLE,
    ALOHA_STATE_RX_LISTEN, ALOHA_STATE_RX_BACKOFF, ALOHA_STATE_RX_WAIT_ACK, ALOHA_STATE_WRONG_PKT_RX
  };

  enum ALOHA_REASON_STATUS {
    ALOHA_REASON_DATA_PENDING, ALOHA_REASON_DATA_RX, ALOHA_REASON_DATA_TX, ALOHA_REASON_ACK_TX, 
    ALOHA_REASON_ACK_RX, ALOHA_REASON_BACKOFF_TIMEOUT, ALOHA_REASON_ACK_TIMEOUT, ALOHA_REASON_DATA_EMPTY, 
    ALOHA_REASON_NOT_SET, ALOHA_REASON_MAX_TX_TRIES, ALOHA_REASON_BACKOFF_PENDING, ALOHA_REASON_LISTEN,
    ALOHA_REASON_LISTEN_TIMEOUT, ALOHA_REASON_LISTEN_PENDING, ALOHA_REASON_START_RX, 
    ALOHA_REASON_PKT_NOT_FOR_ME, ALOHA_REASON_WAIT_ACK_PENDING, ALOHA_REASON_PKT_ERROR
  };

  enum ALOHA_PKT_TYPE {
    ALOHA_ACK_PKT = 1, ALOHA_DATA_PKT, ALOHA_DATAMAX_PKT
  };

  enum ALOHA_ACK_MODES {
    ALOHA_ACK_MODE = 1, ALOHA_NO_ACK_MODE
  };

  enum ALOHA_TIMER_STATUS {
    ALOHA_IDLE = 1, ALOHA_RUNNING, ALOHA_FROZEN, ALOHA_EXPIRED
  };


  class AlohaTimer : public TimerHandler {
    
    
    public:

      
    AlohaTimer(MMacAlohaAdv* m) : TimerHandler(), start_time(0.0), left_duration(0.0), counter(0), module(m), timer_status(ALOHA_IDLE) { 
      assert(m != NULL); }
    
    virtual ~AlohaTimer() { }
    
    
    virtual void freeze() { assert(timer_status == ALOHA_RUNNING); left_duration -= (NOW - start_time); 
                            if (left_duration <= 0.0) 
                              left_duration = module->mac2phy_delay_; 
                            force_cancel();
                            timer_status = ALOHA_FROZEN; }
    
    virtual void unFreeze() { assert(timer_status == ALOHA_FROZEN); start_time = NOW; assert(left_duration > 0);
                              sched(left_duration); timer_status = ALOHA_RUNNING; }
    
    
    virtual void stop() { timer_status = ALOHA_IDLE; force_cancel(); }
    
    virtual void schedule( double val ) { start_time = NOW; left_duration = val; timer_status = ALOHA_RUNNING; resched(val); }
    
    
    bool isIdle() { return ( timer_status == ALOHA_IDLE ); }
    
    bool isRunning() { return (timer_status == ALOHA_RUNNING); }
    
    bool isExpired() { return (timer_status == ALOHA_EXPIRED); }
    
    bool isFrozen() { return (timer_status == ALOHA_FROZEN); }
       
    bool isActive() { return (timer_status == ALOHA_FROZEN || timer_status == ALOHA_RUNNING ); }
    
    
    void resetCounter() { counter = 0; }
    
    void incrCounter() { ++counter; }
    
    int getCounter() { return counter; }
    
    
    double getDuration() { return left_duration; }
    
    
    protected:

      
    double start_time;
    
    double left_duration;
       
    int counter;
    
    
    MMacAlohaAdv* module;
    
    ALOHA_TIMER_STATUS timer_status;
    
    
  };
  
  
  class BackOffTimer : public AlohaTimer { 
  
    
    public: 
    
      
    BackOffTimer(MMacAlohaAdv* m) : AlohaTimer(m) { }
    
    virtual ~BackOffTimer() { }
    
    
    protected:
      
      
    virtual void expire(Event *e);

  
  };
  
  
  class AckTimer : public AlohaTimer { 
  
    
    public:
    
      
    AckTimer(MMacAlohaAdv* m) : AlohaTimer(m) { }
    
    virtual ~AckTimer() { }
    
      
    protected:

      
    virtual void expire(Event *e);
    

  };
  

  class ListenTimer : public AlohaTimer { 
  
    
    public:
    
      
    ListenTimer(MMacAlohaAdv* m) : AlohaTimer(m) { }
    
    virtual ~ListenTimer() { }
    
      
    protected:

      
    virtual void expire(Event *e);

    
  };
  
  
  virtual void recvFromUpperLayers(Packet* p);

  virtual void Mac2PhyStartTx(Packet* p);
  virtual void Phy2MacEndTx(const Packet* p);
  virtual void Phy2MacStartRx(const Packet* p);
  virtual void Phy2MacEndRx(Packet* p);

  virtual double computeTxTime(ALOHA_PKT_TYPE type);
  virtual void initPkt( Packet* p, ALOHA_PKT_TYPE pkt_type, int dest_addr = 0);
  virtual double getBackoffTime();

  virtual void txData();
  virtual void txAck(int dest_addr);
  
  
  virtual void stateIdle();
  virtual void stateRxIdle();
  virtual void stateTxData();
  virtual void stateTxAck(int dest_addr);
  virtual void stateBackoff();
  virtual void stateRxBackoff();
  virtual void stateWaitAck();
  virtual void stateRxWaitAck();
  virtual void stateListen();
  virtual void stateRxListen();


  virtual void stateCheckListenExpired();
  virtual void stateCheckAckExpired();
  virtual void stateCheckBackoffExpired();
  
  virtual void stateRxData(Packet* p);
  virtual void stateRxAck(Packet* p);
  virtual void stateRxPacketNotForMe(Packet* p);

  virtual void printStateInfo(double delay = 0);
  virtual void initInfo();
  virtual void refreshState(ALOHA_STATUS state) { prev_prev_state = prev_state; prev_state = curr_state; curr_state = state; }
  virtual void refreshReason(ALOHA_REASON_STATUS reason) { last_reason = reason; }
  virtual void exitBackoff();

  virtual void setSessionDistance(double distance) { session_distance = distance; }
  virtual bool keepDataPkt(int serial_number);
  virtual void incrCurrTxRounds() { curr_tx_rounds++; }
  virtual void resetCurrTxRounds() { curr_tx_rounds = 0; }
  virtual void updateRTT(double rtt);
  virtual double getRTT() { return (rttsamples>0) ? sumrtt/rttsamples : 0 ; }
  virtual void updateAckTimeout(double rtt);
  virtual void updateLastDataIdRx(int id) { last_data_id_rx = id; }
  virtual void queuePop(bool flag = true) { Packet::free(Q.front()); Q.pop(); waitEndTime(flag); data_sn_queue.pop(); }
  virtual void resetSession();
  virtual void waitForUser();

  // stats functions

  virtual int getRemainingPkts() { return(up_data_pkts_rx - Q.size()); }

  virtual void incrUpperDataRx() {up_data_pkts_rx++;}

  ///////////// input
  int max_tx_tries;
  double backoff_tuner;
  double wait_costant;
  int max_payload; // number of bytes
  int HDR_size;
  int ACK_size;
  double ACK_timeout;
  int buffer_pkts;
  double alpha_;      
  double max_backoff_counter;
  double listen_time;
 /////////////////////////////

  std::queue<Packet*> Q;   /// MAC queue used for packet scheduling 
  std::queue<int> data_sn_queue; /// data sn queue used for retx tries

  static bool initialized;
//   queue<double> queue_wait_time; 

  static int u_pkt_id;  /**< simulation-unique packet ID */
  int u_data_id; // DATA pkt id
  int last_sent_data_id;

  bool TxActive;
  bool RxActive;
  bool session_active;
  bool print_transitions;
  bool has_buffer_queue;

  double start_tx_time;        
  double srtt;       /**< Smoothed Round Trip Time, calculated as for TCP */
  double sumrtt;       /**< sum of RTT samples */
  double sumrtt2;      /**< sum of (RTT^2) */
  int rttsamples;      /**< num of RTT samples */  

  int curr_tx_rounds;
  int last_data_id_rx;

  Packet* curr_data_pkt;

  double session_distance;

  int txsn;

  AckTimer ack_timer; 
  BackOffTimer backoff_timer;
  ListenTimer listen_timer;
  
  ALOHA_REASON_STATUS last_reason;
  ALOHA_STATUS curr_state;
  ALOHA_STATUS prev_state;
  ALOHA_STATUS prev_prev_state;
  
  ALOHA_ACK_MODES ack_mode;

  static map< ALOHA_STATUS , string > status_info;
  static map< ALOHA_REASON_STATUS, string> reason_info;
  static map< ALOHA_PKT_TYPE, string> pkt_type_info;

  ofstream fout;
};

#endif /* MMAC_ALOHA_ADV_H */
