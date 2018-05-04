/*-------------------------------------------------------------------------
* Author: Federico Guerra
* Date: 12 February 2009
* Department of Information Engineering (DEI), University of Padova
*-------------------------------------------------------------------------
*
* Copyright (c) 2009 Regents of the SIGNET lab, University of Padova.
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


#include "mmac-aloha-adv.h"
#include <mac.h>
#include <cmath>
#include <climits>
#include <iomanip>
#include <rng.h>


enum {
    NOT_SET = -1, SESSION_DISTANCE_NOT_SET = 0
};

static class ALOHAModuleClass : public TclClass {
public:
  ALOHAModuleClass() : TclClass("Module/MMac/ALOHA/ADV") {}
  TclObject* create(int, const char*const*) {
    return (new MMacAlohaAdv());
  }
} class_module_aloha;


void MMacAlohaAdv::AckTimer::expire(Event *e) {
  timer_status = ALOHA_EXPIRED;
  if (module->curr_state == ALOHA_STATE_WAIT_ACK) {


        if (module->debug_) cout << NOW << "  MMacAlohaAdv("<< module->addr << ") timer expire() current state = " 
                         << module->status_info[module->curr_state] << "; ACK not received, next state = " 
                         << module->status_info[ALOHA_STATE_BACKOFF] << endl;

        module->refreshReason(ALOHA_REASON_ACK_TIMEOUT);
        module->stateBackoff(); 
  }
  else {
    if (module->debug_ ) cout << NOW << "  MMacAlohaAdv("<< module->addr << ")::AckTimer::expired() " << endl;
  }  
}


void MMacAlohaAdv::BackOffTimer::expire(Event *e) {
  timer_status = ALOHA_EXPIRED;
  if (module->curr_state == ALOHA_STATE_BACKOFF ) {
    
    if (module->debug_) cout << NOW << "  MMacAlohaAdv("<< module->addr << ") timer expire() current state = " 
                             << module->status_info[module->curr_state] << "; backoff expired, next state = " 
                             << module->status_info[ALOHA_STATE_IDLE] << endl;

    module->refreshReason(ALOHA_REASON_BACKOFF_TIMEOUT);
    module->exitBackoff();
    module->stateIdle();
  }
  else {
    if (module->debug_ ) cout << NOW << "  MMacAlohaAdv("<< module->addr << ")::BackoffTimer::expired() " << endl;
  }  
}


void MMacAlohaAdv::ListenTimer::expire(Event *e) {   
  timer_status = ALOHA_EXPIRED;

  if (module->curr_state == ALOHA_STATE_LISTEN ) {

    if (module->debug_) cout << NOW << "  MMacAlohaAdv("<< module->addr << ") timer expire() current state = " 
                      << module->status_info[module->curr_state] << "; listening period expired, next state = " 
                      << module->status_info[ALOHA_STATE_TX_DATA] << endl;

    module->refreshReason(ALOHA_REASON_LISTEN_TIMEOUT);
    module->stateTxData();
  }
  else {
    if (module->debug_ ) cout << NOW << "  MMacAlohaAdv("<< module->addr << ")::ListenTimer::expired() " << endl;
  }  
}


const double MMacAlohaAdv::prop_speed = 1500.0;
int MMacAlohaAdv::u_pkt_id;
bool MMacAlohaAdv::initialized = false;


map< MMacAlohaAdv::ALOHA_STATUS , string> MMacAlohaAdv::status_info;
map< MMacAlohaAdv::ALOHA_REASON_STATUS, string> MMacAlohaAdv::reason_info;
map< MMacAlohaAdv::ALOHA_PKT_TYPE, string> MMacAlohaAdv::pkt_type_info;

MMacAlohaAdv::MMacAlohaAdv() 
: 
  max_tx_tries(),
  backoff_tuner(),
  wait_costant(),
  max_payload(), 
  HDR_size(),
  ACK_size(),
  ACK_timeout(),
  buffer_pkts(),
  alpha_(),
  max_backoff_counter(),
  listen_time(),
  Q(),
  data_sn_queue(),
  u_data_id(0),
  last_sent_data_id(0),
  TxActive(false),
  RxActive(false),
  session_active(false),
  print_transitions(false),
  has_buffer_queue(true),
  start_tx_time(0),   
  srtt(0),      
  sumrtt(0),      
  sumrtt2(0),     
  rttsamples(0),
  curr_tx_rounds(0),
  last_data_id_rx(NOT_SET),
  curr_data_pkt(0),
  session_distance(SESSION_DISTANCE_NOT_SET),
  txsn(1),
  ack_timer(this),
  backoff_timer(this),
  listen_timer(this),
  last_reason(ALOHA_REASON_NOT_SET),
  curr_state(ALOHA_STATE_IDLE), 
  prev_state(ALOHA_STATE_IDLE),
  prev_prev_state(ALOHA_STATE_IDLE), 
  ack_mode(ALOHA_ACK_MODE),
  fout() 
{ 
  u_pkt_id = 0;
  mac2phy_delay_ = 1e-19;
  
  bind("HDR_size_", (int*)& HDR_size);
  bind("ACK_size_", (int*)& ACK_size);
  bind("max_tx_tries_", (int*)& max_tx_tries);
  bind("wait_costant_", (double*)& wait_costant);
  bind("debug_", (double*)&debug_); //degug mode
  bind("max_payload_", (int*)&max_payload);
  bind("ACK_timeout_", (double*)& ACK_timeout);
  bind("alpha_", (double*)&alpha_);
  bind("backoff_tuner_", (double*)&backoff_tuner);
  bind("buffer_pkts_", (int*)&buffer_pkts);
  bind("max_backoff_counter_", (int*)&max_backoff_counter);
  bind("listen_time_", &listen_time);
                     
  if (max_tx_tries <= 0) max_tx_tries = INT_MAX;
  if (buffer_pkts > 0) has_buffer_queue = true;
  if ( listen_time <= 0.0 ) listen_time = 1e-19;
}

MMacAlohaAdv::~MMacAlohaAdv()
{

}

// TCL command interpreter
int MMacAlohaAdv::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if (argc==2)
    {
      if(strcasecmp(argv[1], "setAckMode") == 0)
	{
	  ack_mode = ALOHA_ACK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setNoAckMode") == 0)	
	{
          ack_mode = ALOHA_NO_ACK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "initialize") == 0)	
	{
          if (initialized == false) initInfo();          
          if (print_transitions) fout.open("/tmp/ALOHAstateTransitions.txt",ios_base::app);
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "printTransitions") == 0)	
	{
          print_transitions = true;
	  return TCL_OK;
	}
      // stats functions
      else if(strcasecmp(argv[1], "getQueueSize") == 0)	
	{
	  tcl.resultf("%d",Q.size());
      	  return TCL_OK;
        }
    }
  return MMac::command(argc, argv);
}


int MMacAlohaAdv::crLayCommand(ClMessage* m)
{
  switch (m->type()) 
    {
    
      //case whatever: 
      //    return 0;
      //    break;
    
    default:
      return Module::crLayCommand(m);    
    }  
}


void MMacAlohaAdv::initInfo()
{

  initialized = true;

  if ( (print_transitions) && (system(NULL)) ) {
      int ret_val = system("rm -f /tmp/ALOHAstateTransitions.txt");
      if (ret_val != 0) {
          if (debug_) cout << NOW << "  MMacAlohaAdv(" << addr << ")::initInfo() error with rm" << endl;
      }
      ret_val = system("touch /tmp/ALOHAstateTransitions.txt");
      if (ret_val != 0) {
         if (debug_) cout << NOW << "  MMacAlohaAdv(" << addr << ")::initInfo() error with touch" << endl; 
      }
  }

  status_info[ALOHA_STATE_IDLE] = "Idle state";
  status_info[ALOHA_STATE_BACKOFF] = "Backoff state"; 
  status_info[ALOHA_STATE_TX_DATA] = "Transmit DATA state"; 
  status_info[ALOHA_STATE_TX_ACK] = "Transmit ACK state";
  status_info[ALOHA_STATE_WAIT_ACK] = "Wait for ACK state"; 
  status_info[ALOHA_STATE_DATA_RX] = "DATA received state"; 
  status_info[ALOHA_STATE_ACK_RX] = "ACK received state"; 
  status_info[ALOHA_STATE_LISTEN] = "Listening channel state";
  status_info[ALOHA_STATE_RX_IDLE] = "Start rx Idle state";
  status_info[ALOHA_STATE_RX_BACKOFF] = "Start rx Backoff state";
  status_info[ALOHA_STATE_RX_LISTEN] = "Start rx Listen state";
  status_info[ALOHA_STATE_RX_WAIT_ACK] = "Start rx Wait ACK state";
  status_info[ALOHA_STATE_CHK_LISTEN_TIMEOUT] = "Check Listen timeout state";
  status_info[ALOHA_STATE_CHK_BACKOFF_TIMEOUT] = "Check Backoff timeout state";
  status_info[ALOHA_STATE_CHK_ACK_TIMEOUT] = "Check Wait ACK timeout state";
  status_info[ALOHA_STATE_WRONG_PKT_RX] = "Wrong Pkt Rx state";
  
  reason_info[ALOHA_REASON_DATA_PENDING] = "DATA pending from upper layers"; 
  reason_info[ALOHA_REASON_DATA_RX] = "DATA received";
  reason_info[ALOHA_REASON_DATA_TX] = "DATA transmitted"; 
  reason_info[ALOHA_REASON_ACK_TX] = "ACK tranmsitted";
  reason_info[ALOHA_REASON_ACK_RX] = "ACK received"; 
  reason_info[ALOHA_REASON_BACKOFF_TIMEOUT] = "Backoff expired"; 
  reason_info[ALOHA_REASON_ACK_TIMEOUT] = "ACK timeout"; 
  reason_info[ALOHA_REASON_DATA_EMPTY] = "DATA queue empty";
  reason_info[ALOHA_REASON_MAX_TX_TRIES] = "DATA dropped due to max tx rounds";
  reason_info[ALOHA_REASON_LISTEN] = "DATA pending, listening to channel";
  reason_info[ALOHA_REASON_LISTEN_TIMEOUT] = "DATA pending, end of listening period";
  reason_info[ALOHA_REASON_START_RX] = "Start rx pkt";
  reason_info[ALOHA_REASON_PKT_NOT_FOR_ME] = "Received an erroneous pkt";
  reason_info[ALOHA_REASON_BACKOFF_PENDING] = "Backoff timer pending";
  reason_info[ALOHA_REASON_WAIT_ACK_PENDING] = "Wait for ACK timer pending";
  reason_info[ALOHA_REASON_LISTEN_PENDING] = "Listen to channel pending";
  reason_info[ALOHA_REASON_PKT_ERROR] = "Erroneous pkt";
  
  pkt_type_info[ALOHA_ACK_PKT] = "ACK pkt";
  pkt_type_info[ALOHA_DATA_PKT] = "DATA pkt"; 
  pkt_type_info[ALOHA_DATAMAX_PKT] = "MAX payload DATA pkt";
}

void MMacAlohaAdv::resetSession()
{   
  session_distance = SESSION_DISTANCE_NOT_SET; 
//   curr_tx_rounds = 0;
}

void MMacAlohaAdv::updateRTT(double curr_rtt)
{
  srtt = alpha_ * srtt + (1-alpha_) * curr_rtt;
  sumrtt += curr_rtt;
  sumrtt2 += curr_rtt*curr_rtt;
  rttsamples++;
}

void MMacAlohaAdv::updateAckTimeout(double rtt) {
  updateRTT(rtt);
//   double curr_rtt = getRTT();

//   if (curr_rtt > 0) ACK_timeout = min(ACK_timeout, getRTT() );

  if (debug_) cout << NOW << "  MMacAlohaAdv(" << addr << ")::updateAckTimeout() curr ACK_timeout = " 
                   << ACK_timeout << endl;
//   waitForUser();
}

bool MMacAlohaAdv::keepDataPkt(int serial_number) {
  bool keep_packet;
  if (serial_number > last_data_id_rx) {
    keep_packet = true;
    last_data_id_rx = serial_number;
  }
  else keep_packet = false;
  return keep_packet;
}

double MMacAlohaAdv::computeTxTime(ALOHA_PKT_TYPE type)
{
  double duration;
  Packet* temp_data_pkt;

  if (type == ALOHA_DATA_PKT) {
     if (!Q.empty()) {
        temp_data_pkt = (Q.front())->copy();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = HDR_size + ch->size();
     }
     else { 
        temp_data_pkt = Packet::alloc();
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = HDR_size + max_payload;
     }
  }
  else if (type == ALOHA_ACK_PKT) {
        temp_data_pkt = Packet::alloc();  
        hdr_cmn *ch = HDR_CMN(temp_data_pkt);
        ch->size() = ACK_size;
  }
  duration = Mac2PhyTxDuration(temp_data_pkt );
  Packet::free(temp_data_pkt);
  return(duration);
}


void MMacAlohaAdv::exitBackoff()
{
  backoff_timer.stop();
}


double MMacAlohaAdv::getBackoffTime()
{
  incrTotalBackoffTimes();
  double random = RNG::defaultrng()->uniform_double();

//   while (random == 0) { // se random == 0 lo rigenero
//      random = RNG::defaultrng()->uniform_double();
//   }

  backoff_timer.incrCounter();
  double counter = backoff_timer.getCounter();
  if ( counter > max_backoff_counter ) counter = max_backoff_counter;

  double backoff_duration = backoff_tuner * random * 2.0 * ACK_timeout * pow( 2.0, counter );
  //random = ceil(random);
  backoffSumDuration(backoff_duration);

  if (debug_){
       cout << NOW << "  MMacAlohaAdv("<< addr <<")::getBackoffTime() backoff time = " 
            << backoff_duration << " s" << endl;
//        waitForUser();  
  }
  return backoff_duration;
}

void MMacAlohaAdv::recvFromUpperLayers(Packet* p)
{ 
  if ( ((has_buffer_queue == true) && (Q.size() < (unsigned int)buffer_pkts)) || (has_buffer_queue == false) ) {
     initPkt(p , ALOHA_DATA_PKT);
     Q.push(p);
     incrUpperDataRx();
     waitStartTime();

     if ( curr_state == ALOHA_STATE_IDLE ) 
       {
         refreshReason(ALOHA_REASON_DATA_PENDING);
         stateListen();

//          if (debug_) 
// 	   cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
// 	       << " mac busy => enqueueing packet" << endl;
       }
     else
       {
//          if (debug_) 
// 	   cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
// 	        << " transmitting packet" << endl;
//       
       }
  }
  else {
     incrDiscardedPktsTx();
     drop(p, 1, ALOHA_DROP_REASON_BUFFER_FULL);
  }
}

void MMacAlohaAdv::initPkt( Packet* p, ALOHA_PKT_TYPE type, int dest_addr ) {
  hdr_cmn* ch = hdr_cmn::access(p);
//   hdr_aloha* alohah = HDR_ALOHA(p);
  hdr_mac* mach = HDR_MAC(p);
// 
//   cout << "MMacAlohaAdv initPkt() dest_addr " << dest_addr << endl;

  int curr_size = ch->size();

  switch(type) {
  
    case(ALOHA_DATAMAX_PKT): // fallthrough
    case(ALOHA_DATA_PKT): {
      ch->size() = curr_size + HDR_size;
      data_sn_queue.push(u_data_id);
      u_data_id++;
    } 
    break;

    case(ALOHA_ACK_PKT): {
      ch->ptype() = PT_MMAC_ACK;
      ch->size() = ACK_size;
      ch->uid() = u_pkt_id++;
      mach->macSA() = addr;
      mach->macDA() = dest_addr;
    }
    break;

  }

}

void MMacAlohaAdv::Mac2PhyStartTx(Packet* p) {
  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr <<")::Mac2PhyStartTx() start tx packet" << endl;
  
//   TxActive = true;
//   RxActive = false;
  MMac::Mac2PhyStartTx(p);
}


void MMacAlohaAdv::Phy2MacEndTx(const Packet* p) { 
  // si occupa dei cambi stato

//   TxActive = false;

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr <<")::Phy2MacEndTx() end tx packet" << endl;

  switch(curr_state) {

    case(ALOHA_STATE_TX_DATA): {
      refreshReason(ALOHA_REASON_DATA_TX);
      if (ack_mode == ALOHA_ACK_MODE) {

        if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr <<")::Phy2MacEndTx() DATA sent,from "
                        << status_info[curr_state] << " to " 
                        << status_info[ALOHA_STATE_WAIT_ACK] << endl;

        stateWaitAck(); 
      }
      else{

        if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr <<")::Phy2MacEndTx() DATA sent, from " 
                        << status_info[curr_state] << " to " << status_info[ALOHA_STATE_IDLE] << endl;
    
        stateIdle();
      }
    }
    break;

    case(ALOHA_STATE_TX_ACK): {
      refreshReason(ALOHA_REASON_ACK_TX);

      if ( prev_prev_state == ALOHA_STATE_RX_BACKOFF ) {
        if (debug_) cout << NOW  << "  MMacAlohaAdv("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                          << status_info[curr_state] << " to " << status_info[ALOHA_STATE_CHK_BACKOFF_TIMEOUT] << endl;
          
        stateCheckBackoffExpired();
      }
      else if ( prev_prev_state == ALOHA_STATE_RX_LISTEN ) {
        if (debug_) cout << NOW  << "  MMacAlohaAdv("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                          << status_info[curr_state] << " to " << status_info[ALOHA_STATE_CHK_LISTEN_TIMEOUT] << endl;
        
        stateCheckListenExpired();
      }
      else if ( prev_prev_state == ALOHA_STATE_RX_IDLE ) {

        if (debug_) cout << NOW  << "  MMacAlohaAdv("<< addr <<")::Phy2MacEndTx() ack sent, from " 
                         << status_info[curr_state] << " to " << status_info[ALOHA_STATE_IDLE] << endl;

        stateIdle();
      } 
      else {
      
        cout << NOW << "  MMacAlohaAdv("<< addr <<")::Phy2MacEndTx() logical error in timers, current state = " 
              << status_info[curr_state] << endl;
        exit(1);
      }
    }
    break;

    default: {
        cout << NOW << "  MMacAlohaAdv("<< addr <<")::Phy2MacEndTx() logical error, current state = " 
             << status_info[curr_state] << endl;
        exit(1);
    }
    break;

  }

}


void MMacAlohaAdv::Phy2MacStartRx(const Packet* p) {
  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr <<")::Phy2MacStartRx() rx Packet " << endl; 

//   RxActive = true;

  refreshReason(ALOHA_REASON_START_RX);

  switch(curr_state) { 
    
    case(ALOHA_STATE_IDLE): 
      stateRxIdle();
    break;
    
    case(ALOHA_STATE_LISTEN): 
      stateRxListen();
    break;
      
    case(ALOHA_STATE_BACKOFF): 
      stateRxBackoff();
    break;
      
    case(ALOHA_STATE_WAIT_ACK): 
      stateRxWaitAck();
    break;
    
    default: {
      cerr << NOW << "  MMacAlohaAdv("<< addr << ")::Phy2MacStartRx() logical warning, current state = " 
           << status_info[curr_state] << endl;
//       exit(1);
    }
    
  }
  

}


void MMacAlohaAdv::Phy2MacEndRx(Packet* p) {
//   RxActive = false;

//   if (debug_) cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ << endl;  
  hdr_cmn* ch = HDR_CMN(p);
  packet_t rx_pkt_type = ch->ptype();
  hdr_mac* mach = HDR_MAC(p);
  hdr_MPhy* ph = HDR_MPHY(p);

  int dest_mac = mach->macDA();

  double gen_time = ph->txtime;
  double received_time = ph->rxtime;
  double diff_time = received_time - gen_time;

  double distance = diff_time * prop_speed;

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::Phy2MacEndRx() " 
                   << status_info[curr_state] << ", received a pkt type = " 
                   << ch->ptype() << ", src addr = " << mach->macSA() 
                   << " dest addr = " << mach->macDA() 
                   << ", estimated distance between nodes = " << distance << " m " << endl;

  if ( ch->error() || (curr_state == ALOHA_STATE_TX_DATA) || (curr_state == ALOHA_STATE_TX_ACK) ) {

    if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::Phy2MacEndRx() dropping corrupted pkt " << endl;
    incrErrorPktsRx();

    refreshReason(ALOHA_REASON_PKT_ERROR);
    drop(p, 1, ALOHA_DROP_REASON_ERROR);
    stateRxPacketNotForMe(NULL);
  }
  else {
    if ( dest_mac == addr || dest_mac == (int)MAC_BROADCAST ) {
      if ( rx_pkt_type == PT_MMAC_ACK ) {
        refreshReason(ALOHA_REASON_ACK_RX);
        stateRxAck(p);
      }
      else if ( curr_state != ALOHA_STATE_RX_WAIT_ACK ) {
        refreshReason(ALOHA_REASON_DATA_RX);
        stateRxData(p);
      }
      else {
        refreshReason(ALOHA_REASON_PKT_NOT_FOR_ME);
        stateRxPacketNotForMe(p);
      }
    }
    else {
      refreshReason(ALOHA_REASON_PKT_NOT_FOR_ME);
      stateRxPacketNotForMe(p);
    }
  }
}

void MMacAlohaAdv::txData()
{ 
  Packet* data_pkt = curr_data_pkt->copy();      // copio pkt 
 
  if ( (ack_mode == ALOHA_NO_ACK_MODE) ) {
     queuePop();
  }                                           
 
//   TxActive = true;
//   RxActive = false;
  
  incrDataPktsTx();
  incrCurrTxRounds();
  Mac2PhyStartTx(data_pkt); 
}

void MMacAlohaAdv::txAck( int dest_addr )
{
  Packet* ack_pkt = Packet::alloc();
  initPkt( ack_pkt , ALOHA_ACK_PKT, dest_addr );

  incrAckPktsTx();
  Mac2PhyStartTx(ack_pkt);
}

void MMacAlohaAdv::stateRxPacketNotForMe(Packet* p) {
  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateRxPacketNotForMe() pkt for another address. Dropping pkt" << endl;
  if ( p != NULL ) Packet::free(p);
  
  refreshState( ALOHA_STATE_WRONG_PKT_RX );
  
  switch( prev_state ) {
  
    case ALOHA_STATE_RX_IDLE:
      stateIdle();
      break;
      
    case ALOHA_STATE_RX_LISTEN:
      stateCheckListenExpired();
      break;
      
    case ALOHA_STATE_RX_BACKOFF:
      stateCheckBackoffExpired();
      break;
      
    case ALOHA_STATE_RX_WAIT_ACK:
      stateCheckAckExpired();
      break;
      
    default:
      cerr << NOW << "  MMacAlohaAdv("<< addr << ")::stateRxPacketNotForMe() logical error, current state = " 
           << status_info[curr_state] << endl;
      exit(1);
      
  }
}


void MMacAlohaAdv::stateCheckListenExpired() { 
  refreshState(ALOHA_STATE_CHK_LISTEN_TIMEOUT);

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateCheckListenExpired()" << endl;
  if (print_transitions) printStateInfo();
  if ( listen_timer.isActive() ) {
    refreshReason( ALOHA_REASON_LISTEN_PENDING );
    refreshState( ALOHA_STATE_LISTEN );
  }
  else if ( listen_timer.isExpired() ) {
    refreshReason( ALOHA_REASON_LISTEN_TIMEOUT );
    if ( !( prev_state == ALOHA_STATE_TX_ACK || prev_state == ALOHA_STATE_WRONG_PKT_RX
         || prev_state == ALOHA_STATE_ACK_RX || prev_state == ALOHA_STATE_DATA_RX ) ) stateTxData();
    else stateListen();
  }
  else {
    cerr << NOW << "  MMacAlohaAdv("<< addr << ")::stateCheckListenExpired() listen_timer logical error, current timer state = " 
         << status_info[curr_state] << endl;
    exit(1);  
  }
}


void MMacAlohaAdv::stateCheckAckExpired() {
  refreshState(ALOHA_STATE_CHK_ACK_TIMEOUT);

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateCheckAckExpired()" << endl;
  if (print_transitions) printStateInfo();
  if ( ack_timer.isActive() ) {
    refreshReason( ALOHA_REASON_WAIT_ACK_PENDING );
    refreshState( ALOHA_STATE_WAIT_ACK );
  }
  else if ( ack_timer.isExpired() ) {
    refreshReason( ALOHA_REASON_ACK_TIMEOUT );
    stateBackoff();
  }
  else {
    cerr << NOW << "  MMacAlohaAdv("<< addr << ")::stateCheckAckExpired() ack_timer logical error, current timer state = " 
         << status_info[curr_state] << endl;
    exit(1);  
  }
}


void MMacAlohaAdv::stateCheckBackoffExpired() {
  refreshState(ALOHA_STATE_CHK_BACKOFF_TIMEOUT);

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateCheckBackoffExpired()" << endl;
  if (print_transitions) printStateInfo();
  if ( backoff_timer.isActive() ) {
    refreshReason( ALOHA_REASON_BACKOFF_PENDING );
    stateBackoff();
  }
  else if ( backoff_timer.isExpired() ) {
    refreshReason( ALOHA_REASON_BACKOFF_TIMEOUT );
    exitBackoff();
    stateIdle();
  }
  else {
    cerr << NOW << "  MMacAlohaAdv("<< addr << ")::stateCheckBackoffExpired() backoff_timer logical error, current timer state = " 
         << status_info[curr_state] << endl;
    exit(1);  
  }
}
  
  
void MMacAlohaAdv::stateIdle() {
  ack_timer.stop();
  backoff_timer.stop();
  listen_timer.stop();
  resetSession();
  
  refreshState(ALOHA_STATE_IDLE);
//   session_active = false;

  if (print_transitions) printStateInfo();

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateIdle() queue size = " << Q.size() << endl;

  if ( !Q.empty() ) {
    refreshReason(ALOHA_REASON_LISTEN);
    stateListen();
  }
}


void MMacAlohaAdv::stateRxIdle() {
//   refreshReason(ALOHA_REASON_START_RX);
  refreshState(ALOHA_STATE_RX_IDLE);

  if (print_transitions) printStateInfo();
}


void MMacAlohaAdv::stateListen() {
  listen_timer.stop();
  refreshState(ALOHA_STATE_LISTEN);

  listen_timer.incrCounter();
  
  double time = listen_time * RNG::defaultrng()->uniform_double() + wait_costant;

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateListen() listen time = " << time << endl;

  if (print_transitions) printStateInfo();

  listen_timer.schedule( time );
}


void MMacAlohaAdv::stateRxListen() {
//   refreshReason(ALOHA_REASON_START_RX);
  refreshState(ALOHA_STATE_RX_LISTEN);

  if (print_transitions) printStateInfo();
}


void MMacAlohaAdv::stateBackoff() {
//   backoff_timer.force_cancel();
  refreshState(ALOHA_STATE_BACKOFF);

  if ( backoff_timer.isFrozen() ) backoff_timer.unFreeze();
  else backoff_timer.schedule( getBackoffTime() );

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateBackoff() " << endl;
  if (print_transitions) printStateInfo(backoff_timer.getDuration());
}


void MMacAlohaAdv::stateRxBackoff() {
  backoff_timer.freeze();
//   refreshReason(ALOHA_REASON_START_RX);
  refreshState(ALOHA_STATE_RX_BACKOFF);

  if (print_transitions) printStateInfo();
}


void MMacAlohaAdv::stateTxData()
{
//   timer.stop();
  refreshState(ALOHA_STATE_TX_DATA);

//   session_active = true;

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateTxData() " << endl;
  if (print_transitions) printStateInfo();

  curr_data_pkt = Q.front();
                     
//   cout << "data_sn_queue front = " << data_sn_queue.front() << "; last id = " << last_sent_data_id << endl;

  if ( data_sn_queue.front() != last_sent_data_id) {
     resetCurrTxRounds();
     ack_timer.resetCounter();
     listen_timer.resetCounter();
     backoff_timer.resetCounter(); // se è nuovo pacchetto dati riazzero il backoff counter
//      session_distance = SESSION_DISTANCE_NOT_SET;
  }
  if ( curr_tx_rounds < max_tx_tries ) { 
//     hdr_mac* mach = HDR_MAC(curr_data_pkt);
//      setDestAddr(mach->macDA()); // indirizzo destinazione
     start_tx_time = NOW; // we set curr RTT
     last_sent_data_id = data_sn_queue.front();
     txData();
  }
  else {
    queuePop(false);
    incrDroppedPktsTx();

    refreshReason(ALOHA_REASON_MAX_TX_TRIES);

    if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateTxData() curr_tx_rounds " << curr_tx_rounds
                     << " > max_tx_tries = " << max_tx_tries << endl;
//     waitForUser();

    stateIdle();
  }
}

void MMacAlohaAdv::stateWaitAck() {
  ack_timer.stop();
  refreshState(ALOHA_STATE_WAIT_ACK);

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateWaitAck() " << endl;
  if (print_transitions) printStateInfo();
  
  ack_timer.incrCounter();
  ack_timer.schedule(ACK_timeout + wait_costant); 
}


void MMacAlohaAdv::stateRxWaitAck() {
  refreshState(ALOHA_STATE_RX_WAIT_ACK);

  if (print_transitions) printStateInfo();
}


void MMacAlohaAdv::stateTxAck( int dest_addr ) {
//   timer.stop();
  refreshState(ALOHA_STATE_TX_ACK);

  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateTxAck() dest addr " << dest_addr << endl;
  if (print_transitions) printStateInfo();
 
  txAck( dest_addr );
}


void MMacAlohaAdv::stateRxData(Packet* data_pkt) {
//   ack_timer.stop();
  refreshState( ALOHA_STATE_DATA_RX );
  
  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateRxData() " << endl;
  refreshReason( ALOHA_REASON_DATA_RX );

  hdr_mac* mach = HDR_MAC(data_pkt);
  int dst_addr = mach->macSA();
  
  switch( prev_state ) {
    
    case ALOHA_STATE_RX_IDLE: {
      hdr_cmn* ch = hdr_cmn::access(data_pkt);
      ch->size() = ch->size() - HDR_size;
      incrDataPktsRx();
      sendUp(data_pkt); // mando agli strati superiori il pkt

      if (ack_mode == ALOHA_ACK_MODE) stateTxAck(dst_addr);
      else stateIdle();
    }
    break;
      
    case ALOHA_STATE_RX_LISTEN: {
      hdr_cmn* ch = hdr_cmn::access(data_pkt);
      ch->size() = ch->size() - HDR_size;
      incrDataPktsRx();
      sendUp(data_pkt); // mando agli strati superiori il pkt

      if (ack_mode == ALOHA_ACK_MODE) stateTxAck(dst_addr);
      else stateCheckListenExpired();
    }
    break;
   
    case ALOHA_STATE_RX_BACKOFF: {
      hdr_cmn* ch = hdr_cmn::access(data_pkt);
      ch->size() = ch->size() - HDR_size;
      incrDataPktsRx();
      sendUp(data_pkt); // mando agli strati superiori il pkt
      
      if (ack_mode == ALOHA_ACK_MODE) stateTxAck(dst_addr);
      else stateCheckBackoffExpired();
    }
    break;
      
      
    default: 

      cerr << NOW << " MMacAlohaAdv("<< addr << ")::stateRxData() logical error, prev state = " << status_info[prev_state]
           << endl;

  }
}


void MMacAlohaAdv::stateRxAck(Packet* p) {
  ack_timer.stop();
  refreshState(ALOHA_STATE_ACK_RX);
  if (debug_) cout << NOW << "  MMacAlohaAdv("<< addr << ")::stateRxAck() " << endl;

  Packet::free(p);

  refreshReason(ALOHA_REASON_ACK_RX);

  switch( prev_state ) {
    
    case ALOHA_STATE_RX_IDLE:
      stateIdle();
      break;
      
    case ALOHA_STATE_RX_LISTEN:
      stateCheckListenExpired();
      break;
      
    case ALOHA_STATE_RX_BACKOFF:
      stateCheckBackoffExpired();
      break;
      
    case ALOHA_STATE_RX_WAIT_ACK:
      queuePop();
      updateAckTimeout(NOW - start_tx_time + wait_costant);
      incrAckPktsRx();
      stateIdle();      
      break;
      
    default: 

      cerr << NOW << " MMacAlohaAdv("<< addr << ")::stateRxAck() logical error, prev state = " << status_info[prev_state]
           << endl;

  }
}

void MMacAlohaAdv::printStateInfo(double delay)
{
  if (debug_) cout << NOW << " MMacAlohaAdv("<< addr << ")::printStateInfo() " << "from " << status_info[prev_state] 
                   << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;

  if (curr_state == ALOHA_STATE_BACKOFF) {
      fout <<left << setw(10) << NOW << "  MMacAlohaAdv("<< addr << ")::printStateInfo() " 
           << "from " << status_info[prev_state] 
           << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] 
           << ". Backoff duration = " << delay << endl;
  }
  else {
      fout << left << setw(10) << NOW << "  MMacAlohaAdv("<< addr << ")::printStateInfo() " 
           << "from " << status_info[prev_state] 
           << " to " << status_info[curr_state] << ". Reason: " << reason_info[last_reason] << endl;
  }
}

void MMacAlohaAdv::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
} 

