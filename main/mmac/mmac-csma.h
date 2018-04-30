/* -*- Mode:C++ -*- */

/*
 * Copyright (c) 2008 Regents of the SIGNET lab, University of Padova.
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
 * @file   mmac-csma.h
 * @author Nicola Baldo
 * @date   Sun Jul 27 15:03:44 2008
 * 
 * @brief  
 * 
 * 
 */

 
#ifndef MMAC_CSMA_H 
#define MMAC_CSMA_H

#include<module.h>
#include<mmac.h>
#include<timer-handler.h>
#include<queue>



extern packet_t PT_MMAC_ACK;

class MMacCsma;

enum TIMER_STATUS{
  STATUS_STOPPED = 1,
  STATUS_RUNNING,
  STATUS_FROZEN
};


class Backoff_Timer : public TimerHandler
{
public:
  Backoff_Timer(MMacCsma *m);
  void start(double duration);
  void freeze();
  void defrost();
  bool isFrozen();
  
protected:
  virtual void expire(Event *e);
  MMacCsma* mac;
  enum TIMER_STATUS status;
  double start_time;
  double left_duration;
};

class Ack_Timer : public TimerHandler
{
public:
  Ack_Timer(MMacCsma *m);
  void start(double duration);
  void stop();
  bool isExpired();
	
protected:
  virtual void expire(Event *e);
  MMacCsma* mac;
  enum TIMER_STATUS status;
};




enum MMAC_CSMA_STATE {
  STATE_IDLE = 0,
  STATE_RX_DATA,
  STATE_TX_ACK,
  STATE_CHK_PENDING_PKT,
  STATE_CHK_FROZEN_BACKOFF,
  STATE_BACKOFF,
  STATE_FREEZE_BACKOFF,
  STATE_TX_DATA,
  STATE_START_ACK_TIMEOUT,
  STATE_WAIT_ACK,
  STATE_RX_ACK,
  STATE_CHK_ACK_TIMEOUT_EXPIRED,
  STATE_TX_SUCCESSFUL,
  STATE_TX_FAILED,
  NUM_MMAC_CSMA_STATES
};


class MMacCsma : public MMac
{
public: 
  MMacCsma();
  
protected:

  // events
  virtual void recvFromUpperLayers(Packet* p);
  virtual void Phy2MacEndTx(const Packet* p);
  virtual void Phy2MacStartRx(const Packet* p);
  virtual void Phy2MacEndRx(Packet* p);
  virtual void recvData4Me(Packet* p);
  virtual void recvAck4Me(Packet* p);
  virtual void recvWhateverElse(Packet* p);

public:
  virtual void AckTimeout();
  virtual void BackoffEnded();

protected:

  // actions to be performed when entering a given state

  virtual void enterState_IDLE();
  virtual void enterState_RX_DATA();
  virtual void enterState_TX_ACK(int macSA_of_data_pkt);
  virtual void enterState_CHK_PENDING_PKT();
  virtual void enterState_CHK_FROZEN_BACKOFF();
  virtual void enterState_BACKOFF();
  virtual void enterState_FREEZE_BACKOFF();
  virtual void enterState_TX_DATA();
  virtual void enterState_START_ACK_TIMEOUT();
  virtual void enterState_WAIT_ACK();
  virtual void enterState_RX_ACK();
  virtual void enterState_CHK_ACK_TIMEOUT_EXPIRED();
  virtual void enterState_TX_SUCCESSFUL();
  virtual void enterState_TX_FAILED();

  void setState(enum MMAC_CSMA_STATE s);

  enum MMAC_CSMA_STATE state;


  // status info to be kept across different states -- we try to have
  // as few of these variables as possible, since they complicate the
  // design of the protocol
  bool PendingPacket();
  bool FrozenBackoff();
  bool AckTimeoutExpired();

  const Packet* PktRx;
  Packet* PktTx;
  std::queue<Packet*> Q;   /// MAC queue used for packet scheduling 
  int ConsecutiveFailedTxAttempts;
  
  // timers
  Backoff_Timer backoff_timer;
  Ack_Timer ack_timer;

  // other stuff  
  int HeaderSize_; 
  double BaseBackoffTime_;
  double AckTimeout_;

  int debug_states_;

  static int uidcnt;

};




#endif /* MMAC_CSMA_H */
