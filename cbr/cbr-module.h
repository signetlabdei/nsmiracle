/* -*-	Mode:C++ -*- */
//
// Copyright (c) 2006 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the 
//    names of its contributors may be used to endorse or promote products 
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
 

#ifndef CBR_MODULE_H
#define CBR_MODULE_H

#include <module.h>

#define CBR_DROP_REASON_UNKNOWN_TYPE "UKT"
#define CBR_DROP_REASON_RECEIVED "RCV"
#define CBR_DROP_REASON_OUT_OF_SEQUENCE "OOS"
#define CBR_DROP_REASON_WRONG_SADDR "WSA"
#define CBR_DROP_REASON_WRONG_SPORT "WSP"
#define CBR_DROP_REASON_EXCEEDED_PLAYOUT "EPB"


#define HDR_CBR(P)      (hdr_cbr::access(P))

extern packet_t PT_MCBR;

typedef struct hdr_cbr {

  double ts;  /**< packet timestamp, i.e., its generation time) */
  int sn;     /**< sequence number of this packet */  
  bool rftt_valid; /**< whether the rftt value is valid (a packet must
		    * have been received in the opposite direction) 
		    */ 
  double rftt;    /**< Forward Trip Time seen in the opposite direction
		* (opposite with respect to the direction of this
		* packet). This is used to calculate Round Trip Time 
		*/
  

  static int offset_;
  inline static int& offset() { return offset_; }
  inline static struct hdr_cbr* access(const Packet* p) {
    return (struct hdr_cbr*)p->access(offset_);
  }
  
} hdr_cbr;


class CbrModule;


/**
 * Timer used by CbrModule to schedule packet transmission
 * 
 */
class SendTimer : public TimerHandler
{
public:
  SendTimer(CbrModule *m) : TimerHandler() { module = m; }
	
protected:
  virtual void expire(Event *e);
  CbrModule* module;
};


/**
 * Implements a Constant Bit Rate packet sender/receiver
 * 
 */
class CbrModule : public Module
{
  friend class SendTimer;

public:

  CbrModule();
  ~CbrModule();		
		
  virtual void recv(Packet*);
  virtual void recv(Packet* p, Handler* h);
  virtual int command(int argc, const char*const* argv);
  virtual int crLayCommand(ClMessage* m);

  virtual double GetRTT();
  virtual double GetFTT();
  virtual double GetFTTpob();


  virtual double GetPER();  
  virtual double GetTHR(); 

  virtual double GetPER_PB();  
    
  virtual double GetRTTstd();
  virtual double GetFTTstd();

  virtual void resetStats(); 


protected:

  static int uidcnt_;  /**< simulation-unique packet ID */
  nsaddr_t dstPort_;
  nsaddr_t dstAddr_;


  /**
   * Generate a packet and transmit it
   */ 
  virtual void sendPkt();
  /**
   * Call sendPkt and schedule a new transmission after period_ seconds
   */
  virtual void transmit();

  /** 
   * initialize a packet before transmission
   * 
   * @param p packet to be initialized
   */	
  virtual void initPkt(Packet* p);

  /** 
   * start sending packets
   * 
   */
  virtual void start();
		

  /** 
   * stop sending packets
   * 
   */
  virtual void stop();

  virtual void updateRTT(double rtt);
  virtual void updateFTT(double ftt);
  virtual void updateThroughput(int bytes, double dt);

  virtual void incrPktLost(int npkts);
  virtual void incrPktRecv();
  virtual void incrPktOoseq();
  virtual void incrPktInvalid();

 virtual void incrPktOutBuff();

  virtual double getTimeBeforeNextPkt();


  double alpha_;      /**< smoothing factor used for statistics using
			first order IIR filter */
	       
  int txsn;          /**< sequence number of next new packet to be transmitted */
  int hrsn;          /**< highest received sequence number */

  int pkts_recv;     /**< total number of received packets. Packet out
			of sequence are not counted here. */
  int pkts_ooseq;    /**< total number of packets received out of
			sequence  */

  // playout buffer -------------

  virtual double getMOS(double Pdrop, double delay, double rsource);

 virtual void updateMOS(double Pdrop, double delay, double rsource);
   virtual double GetMOS();
  bool addfd_;
  double fd_;

  double cr_;
  double MOS;

  int pob_; // if 1 then playout buffer is used

  double pob_time; // playout buffer delay

  int pkts_opt;     /**< total number of packets received out of plyout buffer timeout
			  */
  double first_pkt_rftt; // reference froward trip time

  int first_pkt_; // first packet

  //----------------------

  int pkts_lost;     /**< total number of lost packets, including
			packets received out of sequence   */

  int pkts_invalid;  /**< total number of invalid packets received */

  int pkts_last_reset; /**< for error checking after stats are
			  reset. Set to pkts_lost+pkts_recv each time
			  resetStats is called. */ 

  double rftt;       /**< Forward Trip Time seen for last received
			packet */ 

  double srtt;       /**< Smoothed Round Trip Time, calculated as for
			TCP */

  double sftt;       /**< Smoothed Forward Trip Time, calculated as srtt */

  double lrtime;       /**< Time of last packet reception */

  double sthr;       /**< Smoothed throughput calculation */


  double period_;      /**< period between two consecutive packet transmissions */
  int pktSize_;        /**< packet size */
  SendTimer sendTmr_;  /**< timer which schedules packet transmissions */


 /* Cumulative statistics */

  double sumrtt;       /**< sum of RTT samples */
  double sumrtt2;      /**< sum of (RTT^2) */
  int rttsamples;      /**< num of RTT samples */  
  
  double sumftt;       /**< sum of FTT samples */
  double sumftt2;      /**< sum of (FTT^2) */
  int fttsamples;      /**< num of FTT samples */  

  double sumbytes;
  double sumdt;



  int PoissonTraffic_;  

  
};




#endif /* CBR_MODULE_H */
