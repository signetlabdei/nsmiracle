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
 * @file   mmac-csma.cc
 * @author Nicola Baldo
 * @date   Sun Jul 27 15:03:51 2008
 * 
 * @brief  
 * 
 * 
 */

#include<mac.h>
#include"mmac-csma.h"
#include<iostream>

#define DEBUG_UNEXPECTED_EVENT { std::cerr << showpoint << NOW << " (" << addr << ") unexpected event " << __PRETTY_FUNCTION__ << " in state " << state_name_array[state].name << std::endl ; exit(1); }
#define DEBUG_EVENT {   if (debug_states_) std::cerr << showpoint << NOW << " MMacCsma " << addr << " : event " << __FUNCTION__ << std::endl; }

// packet_t PT_MMAC_ACK;

struct state_name { 
  enum MMAC_CSMA_STATE state;
  const char* name;
} state_name_array[NUM_MMAC_CSMA_STATES] = {
  {STATE_IDLE, "IDLE"},
  {STATE_RX_DATA, "RX_DATA"},
  {STATE_TX_ACK, "TX_ACK"},
  {STATE_CHK_PENDING_PKT, "CHK_PENDING_PKT"},
  {STATE_CHK_FROZEN_BACKOFF, "CHK_FROZEN_BACKOFF"},
  {STATE_BACKOFF, "BACKOFF"},
  {STATE_FREEZE_BACKOFF, "FREEZE_BACKOFF"},
  {STATE_TX_DATA, "TX_DATA"},
  {STATE_START_ACK_TIMEOUT, "START_ACK_TIMEOUT"},
  {STATE_WAIT_ACK, "WAIT_ACK"},
  {STATE_RX_ACK, "RX_ACK"},
  {STATE_CHK_ACK_TIMEOUT_EXPIRED, "CHK_ACK_TIMEOUT_EXPIRED"},
  {STATE_TX_SUCCESSFUL, "TX_SUCCESSFUL"},
  {STATE_TX_FAILED, "TX_FAILED"}
};


static class MMacCsmaClass : public TclClass {
public:
  MMacCsmaClass() : TclClass("Module/MMac/CSMA") {}
  TclObject* create(int, const char*const*) {
    return (new MMacCsma);
  }
} class_MMacCsma;



int MMacCsma::uidcnt = 1000;

MMacCsma::MMacCsma()
  : backoff_timer(this),
    ack_timer(this),
    ConsecutiveFailedTxAttempts(0),
    PktTx(0),
    PktRx(0),
    state(STATE_IDLE)
{
  bind("HeaderSize_",&HeaderSize_);
  bind("BaseBackoffTime_",&BaseBackoffTime_);
  bind("AckTimeout_",&AckTimeout_);
  bind("debug_states_",&debug_states_);
  //  bind("",&);
}


bool MMacCsma::FrozenBackoff()
{
  return backoff_timer.isFrozen();
}


bool MMacCsma::PendingPacket()
{
  if (PktTx)
    return true;
  else if (!Q.empty())
    {
      PktTx = Q.front();
      Q.pop();
      return true;
    }
  else
    return false;
}

void MMacCsma::recvFromUpperLayers(Packet* p)
{  
  DEBUG_EVENT;
  switch (state) 
    {
    case STATE_IDLE: 
      assert(Q.empty());
      assert(PktTx == NULL);
      assert(PendingPacket() == false);
      PktTx = p;      
      enterState_TX_DATA();
      break;            
    default: 
      if (PktTx == NULL)
	PktTx = p;
      else	
	Q.push(p);      
      break;
    }
}

void MMacCsma::Phy2MacEndTx(const Packet* p)
{
  DEBUG_EVENT;   
  switch (state) 
    {
    case STATE_TX_DATA: 
      enterState_START_ACK_TIMEOUT();
      break;            
    case STATE_TX_ACK: 
      enterState_CHK_PENDING_PKT();
      break;      
    default:
      DEBUG_UNEXPECTED_EVENT;
      break;
    }
}

void MMacCsma::Phy2MacStartRx(const Packet* p)
{
  DEBUG_EVENT;
  assert(PktRx == NULL);  
  PktRx = p;
  switch (state) 
    {
    case STATE_IDLE: 
      enterState_RX_DATA();
      break;            
    case STATE_WAIT_ACK: 
      enterState_RX_ACK();
      break;  
    case STATE_BACKOFF: 
      enterState_FREEZE_BACKOFF();
      break;    
    default:
      DEBUG_UNEXPECTED_EVENT;
      break;
    }
}


void MMacCsma::Phy2MacEndRx(Packet* p)
{
  DEBUG_EVENT;

  hdr_cmn* ch = hdr_cmn::access(p);
  hdr_mac *mh = HDR_MAC(p);

  assert(PktRx == p); // otherwise we have a strange PHY underneath
  PktRx = NULL;       // Rx ended

  if (ch->error())
    {
      recvWhateverElse(p);
    }
  else
    {
      // packet is received correctly
      if(mh->macDA() == addr)
	{
	  if(ch->ptype() == PT_MMAC_ACK)
	    recvAck4Me(p);
	  else
	    recvData4Me(p);
	}
      else
	{
	  // not for me
	  recvWhateverElse(p);
	}
    }
}

void MMacCsma::recvData4Me(Packet* p)
{
  DEBUG_EVENT;
  hdr_mac *mh = HDR_MAC(p); 
  switch (state) 
    {
    case STATE_RX_DATA:        
      enterState_TX_ACK(mh->macSA());
      sendUp(p);
      break;      
    default: 
      recvWhateverElse(p);                      
      break;
    }
}

void MMacCsma::recvAck4Me(Packet* p)
{
  DEBUG_EVENT;
  switch (state) 
    {
    case STATE_RX_ACK: 
      Packet::free(p);
      enterState_TX_SUCCESSFUL();
      break;      
    default: 
      recvWhateverElse(p);                      
      break;
    }
}


void MMacCsma::recvWhateverElse(Packet* p)
{
  DEBUG_EVENT;
  Packet::free(p);
  switch (state) 
    {
    case STATE_RX_DATA: 
      enterState_CHK_PENDING_PKT();
      break;
    case STATE_RX_ACK: 
      enterState_CHK_ACK_TIMEOUT_EXPIRED();
      break;      
    default:
      DEBUG_UNEXPECTED_EVENT;
      break;
    }
}


void MMacCsma::AckTimeout()
{
  DEBUG_EVENT;
  switch (state) 
    {
    case STATE_WAIT_ACK: 
      enterState_TX_FAILED();
      break;
    case STATE_RX_ACK: 
      // do nothing, ack_timer.isExpired() will return true
      break;      
    default:
      DEBUG_UNEXPECTED_EVENT;
      break;
    }
}

void MMacCsma::BackoffEnded()
{
  DEBUG_EVENT;
  switch (state) 
    {
    case STATE_BACKOFF: 
      enterState_TX_DATA();
      break;
    default:
      DEBUG_UNEXPECTED_EVENT;
      break;
    }
}

void MMacCsma::setState(enum MMAC_CSMA_STATE s)
{
  state = s;
  if (debug_states_)
    std::cerr << showpoint << NOW 
	      << " MMacCsma " << addr << " : entering state " 
	      << state_name_array[s].name 
      //<< " " << s
      //<< " (" << state_name_array[s].state << ")"
	      << std::endl;  
}

void MMacCsma::enterState_IDLE()
{
  assert(STATE_CHK_PENDING_PKT == state); 
  assert(PktRx == NULL);
  assert(PktTx == NULL);
  setState(STATE_IDLE);
}


void MMacCsma::enterState_RX_DATA()
{
  assert((STATE_IDLE == state)
	 ||(STATE_FREEZE_BACKOFF == state));
  setState(STATE_RX_DATA);
  assert(PktRx != NULL);
}

void MMacCsma::enterState_TX_ACK(int macSA_of_data_pkt)
{
  assert(STATE_RX_DATA == state);
  setState(STATE_TX_ACK);
  Packet* p = Packet::alloc();
  hdr_cmn* ch = hdr_cmn::access(p);
  //  ch->uid() = 0;
  // ch->ptype() = PT_MMAC_ACK;
  ch->size() = HeaderSize_; // no payload
  assert(ch->size() > 0);

  hdr_mac *ack_mh = HDR_MAC(p);
  ack_mh->macSA() = addr;
  ack_mh->macDA() = macSA_of_data_pkt;  
  Mac2PhyStartTx(p);
}


void MMacCsma::enterState_CHK_PENDING_PKT()
{
  assert((STATE_RX_DATA == state)
	 ||(STATE_TX_ACK == state)
	 ||(STATE_TX_SUCCESSFUL == state)); 
  setState(STATE_CHK_PENDING_PKT);
  if(PendingPacket())
    enterState_CHK_FROZEN_BACKOFF();
  else
    enterState_IDLE();
}

void MMacCsma::enterState_CHK_FROZEN_BACKOFF()
{
  assert(STATE_CHK_PENDING_PKT == state); 
  setState(STATE_CHK_FROZEN_BACKOFF);
  if(FrozenBackoff())
    enterState_BACKOFF();
  else
    enterState_TX_DATA();    
}


void MMacCsma::enterState_BACKOFF()
{
  assert((STATE_CHK_FROZEN_BACKOFF == state)
	 ||(STATE_TX_FAILED == state)); 
  setState(STATE_BACKOFF);
  if (backoff_timer.isFrozen())
    backoff_timer.defrost();
  else
    backoff_timer.start(BaseBackoffTime_*pow(2,ConsecutiveFailedTxAttempts));
}


void MMacCsma::enterState_FREEZE_BACKOFF()
{
  assert(STATE_BACKOFF == state); 
  setState(STATE_FREEZE_BACKOFF);
  backoff_timer.freeze();
  enterState_RX_DATA();
}

void MMacCsma::enterState_TX_DATA()
{
  assert((STATE_BACKOFF == state)
	 ||(STATE_CHK_FROZEN_BACKOFF == state)
	 ||(STATE_IDLE == state)); 
  setState(STATE_TX_DATA);
  assert(PktTx);
  Mac2PhyStartTx(PktTx->copy());
}


void MMacCsma::enterState_START_ACK_TIMEOUT()
{
  assert(STATE_TX_DATA == state); 
  setState(STATE_START_ACK_TIMEOUT);
  ack_timer.start(AckTimeout_);
  enterState_WAIT_ACK();
}


void MMacCsma::enterState_WAIT_ACK()
{
  assert((STATE_START_ACK_TIMEOUT == state)
	 ||(STATE_CHK_ACK_TIMEOUT_EXPIRED == state));
  setState(STATE_WAIT_ACK);         
}

void MMacCsma::enterState_RX_ACK()
{
  assert(STATE_WAIT_ACK == state);
  setState(STATE_RX_ACK);  
}

void MMacCsma::enterState_CHK_ACK_TIMEOUT_EXPIRED()
{
  assert(STATE_RX_ACK == state); 
  setState(STATE_CHK_ACK_TIMEOUT_EXPIRED);
  if(ack_timer.isExpired())
    enterState_TX_FAILED();
  else
    enterState_WAIT_ACK();
}

void MMacCsma::enterState_TX_SUCCESSFUL()
{
  assert(STATE_RX_ACK == state); 
  setState(STATE_TX_SUCCESSFUL);
  ack_timer.stop();
  PktTx = NULL;
  ConsecutiveFailedTxAttempts = 0;
  enterState_CHK_PENDING_PKT();
}

void MMacCsma::enterState_TX_FAILED()
{
  assert((STATE_WAIT_ACK == state)
	 ||(STATE_WAIT_ACK == state)); 
  setState(STATE_TX_FAILED);
  //  ack_timer.stop();
  ConsecutiveFailedTxAttempts++;
  enterState_BACKOFF();
}



Backoff_Timer::Backoff_Timer(MMacCsma *m)
  : status(STATUS_STOPPED),
    mac(m)
{  
}

void Backoff_Timer::start(double duration)
{
  assert(status == STATUS_STOPPED);
  start_time = NOW;
  sched(duration);
  status = STATUS_RUNNING;
}

void Backoff_Timer::freeze()
{
  assert(status == STATUS_RUNNING);
  left_duration = NOW - start_time;
  cancel();
  status = STATUS_FROZEN;
}


void Backoff_Timer::defrost()
{ 
  assert(status == STATUS_FROZEN);
  start_time = NOW;
  assert(left_duration > 0);
  sched(left_duration);
  status = STATUS_RUNNING;
}

void Backoff_Timer::expire(Event *e)
{
  assert(status == STATUS_RUNNING);  
  status = STATUS_STOPPED;
  mac->BackoffEnded();

}

bool Backoff_Timer::isFrozen()
{
  return (STATUS_FROZEN == status);
}





Ack_Timer::Ack_Timer(MMacCsma *m)
  : status(STATUS_STOPPED),
    mac(m)
{  
}

void Ack_Timer::start(double duration)
{
  assert(status == STATUS_STOPPED);
  sched(duration);
  status = STATUS_RUNNING;
}

void Ack_Timer::expire(Event *e)
{
  assert(status == STATUS_RUNNING);  
  mac->AckTimeout();
  status = STATUS_STOPPED;
}

bool Ack_Timer::isExpired()
{
  return (STATUS_STOPPED == status);
}

void Ack_Timer::stop()
{
  assert(status == STATUS_RUNNING);  
  cancel();
  status = STATUS_STOPPED;
}
