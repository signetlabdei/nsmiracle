/*-------------------------------------------------------------------------
* Author: Federico Guerra
* Date: June 2010
* Department of Information Engineering (DEI), University of Padova
*-------------------------------------------------------------------------
*
* Copyright (c) 2010 Regents of the SIGNET lab, University of Padova.
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


#ifndef MARQ_DEFINITIONS_H
#define MARQ_DEFINITIONS_H


#define HDR_MARQ(P)      (hdr_marq::access(P))

#define MARQ_MIN_DELAY   (1e-10)


#include <vector>
#include <climits>
#include <packet.h>
#include <module.h>


class MArq;


class MArqTimer : public TimerHandler {
  
  
  public:


  enum MARQ_TIMER_STATUS {
    IDLE = 1, RUNNING, FROZEN, EXPIRED
  };    
    
  
  MArqTimer(MArq *m) : TimerHandler(), start_time(0.0), left_duration(0.0), counter(0), module(m), timer_status(IDLE) { 
    assert(m != NULL); }
  
  virtual ~MArqTimer() { }
  
  
  virtual void freeze() { assert(timer_status == RUNNING); left_duration = NOW - start_time; 
                          if (left_duration <= 0.0) 
                            left_duration = 1e-15; 
                          force_cancel();
                          timer_status = FROZEN; }
  
  virtual void unFreeze() { assert(timer_status == FROZEN); start_time = NOW; assert(left_duration > 0);
                            sched(left_duration); timer_status = RUNNING; }
  
  
  virtual void stop() { timer_status = IDLE; force_cancel(); }
  
  virtual void schedule( double val ) { start_time = NOW; left_duration = val; timer_status = RUNNING; resched(val); }
  
  
  bool isIdle() { return ( timer_status == IDLE ); }
  
  bool isRunning() { return (timer_status == RUNNING); }
  
  bool isExpired() { return (timer_status == EXPIRED); }
  
  bool isFrozen() { return (timer_status == FROZEN); }
      
  bool isActive() { return (timer_status == FROZEN || timer_status == RUNNING ); }
  
  
  void resetCounter() { counter = 0; }
  
  void incrCounter() { ++counter; }
  
  int getCounter() { return counter; }
  
  
  double getDuration() { return left_duration; }
  
  
  protected:

    
  double start_time;
  
  double left_duration;
      
  int counter;
  
  
  MArq* module;
  
  MARQ_TIMER_STATUS timer_status;
  
  
};


class MArq {

  
  public:
    
    
  class PacketWindow;
  
  
  typedef int marqsn_t;
  
  typedef ::std::vector< Packet* > PktVect;
  
  typedef ::std::vector< marqsn_t > PktSnVect;

  
  class PacketWindow {
    
    
    public:
      
      
    PacketWindow() : win_vector(), window_sn(0) { }
    
    PacketWindow( const PktVect& win, int sn ) : win_vector(win), window_sn(sn) { }
    
    
    void clear() { win_vector.clear(); window_sn++; }
    
    void reset() { win_vector.clear(); window_sn = 0; }
    
    int size() const { return win_vector.size(); }
    
    
    PktVect& access() { return win_vector; }
    
    Packet*& operator[]( int key ) { return win_vector[key]; }
    
    
    void incrWindowSn() { ++window_sn; }
    
    int getWindowSn() { return window_sn; }
    
    void resetWindowSn() { window_sn = 0; }
    
    
    protected:
      
    
    PktVect win_vector;
  
    int window_sn;
    
    
  };
    
  
  class GiveTxWindowEvent : public Event {

    
    public:
      
      
    GiveTxWindowEvent( const PacketWindow& tx ) : tx_window(tx)  {  }
    
       
    const PacketWindow& tx_window;
    

  };


  class GiveRxWindowEvent : public Event {

      
    public:
      
      
    GiveRxWindowEvent( const PacketWindow& rx ) : rx_window(rx)  {  }
    
       
    const PacketWindow& rx_window;
    
    
  };


  class GiveAckWindowEvent : public Event {

      
    public:
      
      
    GiveAckWindowEvent( const PktSnVect& ack ) : ack_window(ack)  {  }
    
        
    const PktSnVect& ack_window;
    

  };
  
  
  class GiveTxWindowHandler : public Handler {

    
    public:
      
         
    GiveTxWindowHandler(MArq* ptr) : marq(ptr)  {  }

    virtual ~GiveTxWindowHandler() { }


    virtual void handle(Event* e) = 0;

    
    protected:
      
      
    MArq* marq;

    
  };


  class GiveRxWindowHandler : public Handler {

      
    public:
      
         
    GiveRxWindowHandler(MArq* ptr) : marq(ptr)  {  }

    virtual ~GiveRxWindowHandler() { }


    virtual void handle(Event* e) = 0;

    
    protected:
      
      
    MArq* marq;
    
    
  };


  class GiveAckWindowHandler : public Handler {

      
    public:
      
         
    GiveAckWindowHandler(MArq* ptr) : marq(ptr)  {  }

    virtual ~GiveAckWindowHandler() { }


    virtual void handle(Event* e) = 0;  

    
    protected:
      
      
    MArq* marq;
    

  };  
      
    
  class TxWinTimer : public MArqTimer { 

    
    public:
    
      
    TxWinTimer(MArq* m) : MArqTimer(m) { }
    
    virtual ~TxWinTimer() { }
    
      
    protected:

      
    virtual void expire(Event *e) = 0;
    

  }; 


  class RxWinTimer : public MArqTimer { 

    
    public:
    
      
    RxWinTimer(MArq* m) : MArqTimer(m) { }
    
    virtual ~RxWinTimer() { }
    
      
    protected:

      
    virtual void expire(Event *e) = 0;
    

  };   
    

  class AckWinTimer : public MArqTimer { 

    
    public:
    
      
    AckWinTimer(MArq* m) : MArqTimer(m) { }
    
    virtual ~AckWinTimer() { }
    
      
    protected:

      
    virtual void expire(Event *e) = 0;
    

  };   

  
  MArq() 
    : tx_win_timer(NULL), rx_win_timer(NULL), ack_win_timer(NULL), 
      tx_win_handler(NULL), rx_win_handler(NULL), ack_win_handler(NULL),
      curr_window_size(1), max_window_size(1), max_tx_tries(INT_MAX), 
      curr_tx_window(), curr_rx_window() 
    { }
  
  virtual ~MArq() { }
    
    
  virtual void processTxPacket( Packet* p ) = 0;
        
  virtual void processRxPacket( Packet* p ) = 0;
  
  virtual void processAckWindow( const PktSnVect& ack_window ) = 0;
  
  virtual void processAck( marqsn_t ack_sn ) = 0;

  
  virtual void removeFromCurrentTxWindow( const PktSnVect& failed_pkt_ids ) = 0;
  
  virtual void removeFromCurrentRxWindow( const PktSnVect& failed_pkt_ids ) = 0;

  
  virtual void resetCurrentTxWindow() = 0;
    
  virtual void resetCurrentRxWindow() = 0;  
    
  
  virtual bool TxEmpty() const = 0;
  
  virtual bool RxEmpty() const = 0;
  
  
  virtual int TxSize() const = 0;
  
  virtual int RxSize() const = 0;

  
  void setMaxWindowSize( int size ) { max_window_size = size; }
  
  void setMaxTxTries( int tries ) { max_tx_tries = tries; }
  
  
  virtual void updateCurrentWindowSize( int size ) {
    if ( size >= 1 && size <= max_window_size ) curr_window_size = size;
    else if ( size > max_window_size ) curr_window_size = max_window_size;
    else if ( size < 1 ) curr_window_size = 1;
  }
  
  
  int getMaxWindowSize() const { return max_window_size; }
  
  int getMaxTxTries() const { return max_tx_tries; }
  
  
  void setGiveTxWindowHandler( GiveTxWindowHandler* h ) { tx_win_handler = h; }
  
  void setGiveRxWindowHandler( GiveRxWindowHandler* h ) { rx_win_handler = h; }
  
  void setGiveAckWindowHandler( GiveAckWindowHandler* h ) { ack_win_handler = h; }
  
    
  protected:
    
  TxWinTimer* tx_win_timer; 
  
  RxWinTimer* rx_win_timer;
  
  AckWinTimer* ack_win_timer;

  
  GiveTxWindowHandler* tx_win_handler;
  
  GiveRxWindowHandler* rx_win_handler;
  
  GiveAckWindowHandler* ack_win_handler;
  

  int curr_window_size;
  
  int max_window_size;

  int max_tx_tries;
  
  
  PacketWindow curr_tx_window;
  
  PacketWindow curr_rx_window;
  
  PktSnVect curr_ack_window;
  
  
  virtual marqsn_t getNextWinPktSn() = 0;

  
  void giveCurrentTxWindow( GiveTxWindowEvent* event, double delay = MARQ_MIN_DELAY );

  void giveCurrentRxWindow( GiveRxWindowEvent* event, double delay = MARQ_MIN_DELAY );

  void giveCurrentAckWindow( GiveAckWindowEvent* event, double delay = MARQ_MIN_DELAY );
  
  
};


inline void MArq::giveCurrentTxWindow( GiveTxWindowEvent* event, double delay ) {
  Scheduler::instance().schedule( tx_win_handler, event, delay );
}


inline void MArq::giveCurrentRxWindow( GiveRxWindowEvent* event, double delay ) { 
  Scheduler::instance().schedule( rx_win_handler, event, delay );
}


inline void MArq::giveCurrentAckWindow( GiveAckWindowEvent* event, double delay ) {
  Scheduler::instance().schedule( ack_win_handler, event, delay );
}


typedef struct hdr_marq { 
   
  
  int curr_tx_tries;
  
  bool has_been_acked;
  
  
  int pkt_pos;
  
  int pkt_sn;
  
  
  int window_size;
  
  int window_sn;
    
  
  MArq::PacketWindow* tx_window;
  
  MArq::PktSnVect* rx_window;
  
  
  static int offset_;
  
  
  inline static int& offset() { return offset_; }
  
  inline static struct hdr_marq* access(const Packet* p) {
    return (struct hdr_marq*)p->access(offset_);
  }    


} hdr_marq;


#endif // MARQ_DEFINITIONS_H




