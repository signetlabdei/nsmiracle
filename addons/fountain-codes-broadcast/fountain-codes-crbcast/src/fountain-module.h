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
 

#ifndef FOUNTAIN_MODULE_H
#define FOUNTAIN_MODULE_H

#include <iostream>
#include <module.h>
#include "codec.h" // so we can use the codec libary
#include <time.h>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <string>
#include <node-core.h>

#define FC_DROP_REASON_UNKNOWN_TYPE "UKT"
#define FC_DROP_REASON_RECEIVED "RCV"
#define FC_DROP_REASON_WRONG_RECEIVER "WRV"
#define FC_DROP_REASON_WRONG_SADDR "WSA"
#define FC_DROP_REASON_WRONG_SPORT "WSP"
#define FC_DROP_REASON_WRONG_BLOCK_NUMBER "WBN"
#define FC_DROP_REASON_ALREADY_SYNC "ASY"
#define FC_DROP_REASON_DECODED_DATA "ADD"
#define FC_DROP_REASON_ALREADY_NEWBLOCK "ANB"
#define FC_DROP_REASON_BLOCK_DONE "BKD"
#define FC_DROP_REASON_ALREADY_FINISHED "AFI"

#define HDR_FC(P)      (hdr_fc::access(P))

class FCModule;

class FCMMultihopController;

class FCMCRBController;

struct hdr_fc;

typedef vector<FCModule*>::iterator Fcm_iter;
typedef vector<hdr_fc>::iterator hdr_iter;
typedef map<int, set<int> > SMap;
typedef pair<int, set<int> > Entry;
typedef pair<int,int> Mu_entry;
typedef pair<int, int> Stats;
typedef map<int, double> PerMap;
typedef SMap::iterator SMapiter;
typedef set<int>::iterator Setiter;
typedef PerMap::iterator PerMapIter;
typedef uint16_t**** Mu;

extern packet_t PT_FCM;

typedef struct hdr_fc {

  union {
      set<int>* rxs_id;
      int rx_id;
  }; 

  double ts;  /**< packet timestamp, i.e., its generation time) */
  int sn;     /**< sequence number of this packet */  
  bool rftt_valid; /**< whether the rftt value is valid (a packet must *
		    * have been received in the opposite direction)    *
		    */ 
  double rftt;    /**< Forward Trip Time seen in the opposite direction * 
		* (opposite with respect to the direction of this       *
		* packet). This is used to calculate Round Trip Time    *
		*/

  uint8_t* pkt_pointer; /* pointer to the location of THIS pkt in the *
              		 * encodedPks array                           */
  int block_id; // curr block
  uint16_t pktid; // pkt id of curr block	
  int sender_id; // sender id   
  int total_ack; // no of acked pkts
  int res_rank; //residual rank
  int command;  // command 
  int seed;

  static int offset_;
  inline static int& offset() { return offset_; }
  inline static struct hdr_fc* access(const Packet* p) {
    return (struct hdr_fc*)p->access(offset_);
  }
  
} hdr_fc;

class FCMTimer : public TimerHandler
{
public:
  FCMTimer(FCModule *m) : TimerHandler() { module = m; action=1; } //default action is SEND
  void setAction(int action); // set sending or decoding timers
  int getAction();

protected:
  enum {
  ACTION_DATA = 1, ACTION_DECODE = 2, ACTION_SYNC = 3, ACTION_WAIT_STATS = 4, ACTION_NEWBLOCK = 5,
  ACTION_WAIT_NEWBLOCK = 6 
  };
  uint8_t action; //1 = send action, 2 = decode action
  virtual void expire(Event *e);
  FCModule* module;
};

class FCModule : public Module
{
  friend class FCMTimer;
  friend class FCMMultihopController;
  friend class FCMCRBController;

  public:

  /**metodi pubblici*******************/
  FCModule();
  virtual ~FCModule();		
		
  virtual void recv(Packet*);
  virtual void recv(Packet* p, Handler* h); 
  virtual int command(int argc, const char*const* argv);
  virtual int crLayCommand(ClMessage* m);
  
  virtual void finish();
  virtual void startMasterEncoder();
  virtual void setInitialReceivers(const set<int>* receivers);

  virtual double GetRTT();
  virtual double GetFTT();
  virtual PerMap GetPER();  
  virtual double GetTHR(); 

  virtual double GetRTTstd();
  virtual double GetFTTstd();
  virtual int GetFCMId();
  virtual bool isComplete();

  virtual void resetStats(); 
  /************************************/

  protected:

  /*constants***************/
  static const int XIMIN = 1;
  static const int INF = 65535; 
  static const int MAXDIST = 100000;
  static const int ALL = 21;
  /***************************/

  /*Timer Coder e MultiHopControl Pointers*/
  FCMTimer FCMTmr_;  /**< timer which schedules packet transmissions */
  Codec* coder;   // pointer to the encoder/decoder
  FCMMultihopController* multihop_ctrl;
  FCMCRBController* crbcast_ctrl;
  /***************/
  
  /***protocol stack id**************/
  int ctrl_module_id;  //id del canale di controllo
  int send_module_id; //id del canale di trasmissione
  
  nsaddr_t dstPort_;
  nsaddr_t dstAddr_;
  /**********************************/
  
  /********metodi interni************/ 
  virtual void sendPkt(int command, double delay = 0, int receiver = ALL);
  virtual void transmit();
  virtual void initPkt(Packet* p,int command, int receiver = ALL);
  virtual void stop();
  virtual void setDist();
  virtual void getOptPolicy(); // gets the Optimal Policy
  virtual int xxmax(int curr_round); // get the max pkts to send at the current round
  virtual void resetTxStats(); // resets Acks and Ranks numbers to zero
  virtual void allocateData(); // allocates data in memory
  virtual void encodeData(); // passes the data to the encoder
  virtual void deleteData(); // delete the data
  virtual void deleteEncodedData(); // delete the encoded data
  virtual void createMu(Mu& input_mu, Mu_entry entry); // creates the mu pointer-matrix for the optimal policy
  virtual void deleteMu(Mu& input_mu, Mu_entry entry); // deletes the mu
  virtual void initialize(); // create the coder
  virtual void computeClass(); //compute the class of the FC Module
  virtual void startDiscovery();
  virtual void discovery();
  //virtual void newBlock();
  virtual void doNewBlock();
  virtual void abortCurrBlock();
  virtual void ackStats(int sender);
  virtual void newBlockAck(int sender);
  virtual void changePolicy(); //collect ack pkts and ranks statistics and continues the sending process if necessary
  virtual void tryToDecode(); // try to decode and refresh min ack and rank statitics
  virtual void getTxPolicy(int class_id = 0, int curr_stats = 0); // update  curr_policy 
  //virtual void processAck(const hdr_fc* fc_header); //modulo da sovrascrivere quando lo si eredita per adattarlo al MAC
  virtual void sendNewBlock(); //new block è default
  virtual void controlNewBlock();
  virtual void incrDataPktReceived();
  virtual void loadEncodedPkt();
  virtual char* printCommand(int command);
  virtual void setToCoder();
  virtual void setToDecoder();
  virtual void increaseBlock();

  /***************************************/

  /*metodi per inserire e accedere alle mappe*/
  virtual void insertAck(int class_id, int ack_value);
  virtual void insertRank(int class_id, int rank_value);
  virtual Stats getMinAck();
  virtual Stats getMaxRank();
  virtual void resetRankStats();
  virtual void resetAckStats();

  /*************************************/

  /*******metodi di servizio**********/
  virtual void printStats();
  virtual void waitForUser();
  /*********************************/

  /*** metodi di CBRMiracle*********/
  virtual void updateRTT(double rtt);
  virtual void updateFTT(double ftt);
  virtual void updateThroughput(int bytes, double dt);

  virtual void incrPktLost(int npkts);
  virtual void incrPktRecv();
  virtual void incrPktInvalid();
  virtual void incrPktDupl();
  virtual void incrPktWrblock();
  virtual void incrPktWrdata();
  /**********************************/

  /***variabili di CBRMiracle********/
  double alpha_;      /**< smoothing factor used for statistics using
			first order IIR filter */
  int txsn;          /**< sequence number of next new packet to be transmitted */
  int hrsn;          /**< highest received sequence number */
  int pkts_recv;     /**< total number of received packets */
  int pkts_lost;     /**< total number of lost packets,  */
  int pkts_invalid;  /**< total number of invalid packets received */
  int pkts_dupl; 
  int pkts_wrblock;
  int pkts_wrdata;
  int pkts_last_reset; /**< for error checking after stats are
			  reset. Set to pkts_lost+pkts_recv each time
			  resetStats is called. */ 

  double rftt;       /**< Forward Trip Time seen for last received packet */ 
	
  double srtt;       /**< Smoothed Round Trip Time, calculated as for
			TCP */
  double sftt;       /**< Smoothechard Forward Trip Time, calculated as srtt */

  double lrtime;       /**< Time of last packet reception */
  double sthr;       /**< Smoothed throughput calculation */

  /*dati input *************/
  double period_;      /**< period between two consecutive packet transmissions */
  int pktSize_;        /**< packet size */
  int blockSize_; // size of the encoding block
  int symSize_;   // size of symbols
  int totBlock_ ; // total tx block number
  int genType_; // generator type
  int debug_;
  int seed_; //seed for random generator
  int max_rounds_; // max no of tx rounds
  double propSpeed_ ; // propagation speed
  double TxRadius_ ;
  /***************************/

  /*variabili della topologia per le classi*/
  int n_class; // no of total classes
  double d_lower; // lower bound: if distance <= d_lower => class_id = 1;
  double d_max; //simulation max distance
  double d_upper; //upper bound: if distance >= d_upper =>class_id = n_class;
  double max_RTT; // computed max RTT based on max dist from sender / prop speed
  /*******************************************/

  /* counters */
  int ximax; // max pcks combined  

  static int uidcnt_;  /**< simulation-unique packet ID */
  static int uidfc_;   /* simulation-unique FCModule ID */
  int idcurrfc_ ;      /* id of the FCMODULE */

  int curr_round; // current tx round
  int curr_policy; //no of pkt to tx for the curr round
  int curr_class_min; //current policy class
  int curr_block;
  int curr_ack_stat; // the min current acked pkts among classes
  int curr_rank_stat; // the max current rank among classes

  int pkts_tx; // number of pkts sent in curr block
  int total_data_pkt;
  int total_ctrl_pkt;
  int maxPktsToSend; // number of max overhead encoded pck that the decoder can receive

  int block_failed; // da togliere static
  int total_good_rounds; 

  double N_tx;
  double P_fail;
  double N_rounds;

  
  /*************************/

  /*nomi files*/
  //char distFile[STRLEN]; // Distribution File name
  //char optPolicyFile[STRLEN]; // Optimal Policy File name
  string distFile;
  string optPolicyFile;
  /*************************/
  
  /*vettori e set per le statistiche*/
  uint8_t** data; // data to be encoded
  uint8_t** encodedPkts; // encoded pkt
  static map< Mu_entry , Mu > mu_map; // optimal policy
  static map< Mu_entry , bool> mu_map_done;
  static vector<FCModule*> fcm_pointers; // Array of pointers to all FC Modules present in the simulation
  static vector<double> dist_vector;
  vector<int> class_id;
  map<int,int> tot_nds_classes;
  double class_dist;
  set<int> fcm_peers; // contiene i ricevitori (se encoder) o i trasmettitori (se decoder)
  set<int> stats_peers; // contiene i ricevitori che hanno spedito correttamente gli stats
  set<int> newblock_peers; // contienei i ricevitori che hanno spedito correttamente i newblock_ack
  static set<int> complete_peers;
  static set<int> incomplete_peers;
  static list<int> fcm_lcomp;
  //static set<int> current_compl_peers;
  SMap acks_stats;
  SMap rank_stats;

  list<int> crb_phase_I_index;
  int crb_phase_I_seed;
  /************************************/

  /*bool e flag **/
  int crb_phase;
  bool mhop;
  bool printstats;
  bool do_retx;
  bool isdecoder; // flag to tag a decoder
  bool is_master; 
  bool done;
  bool newblock_done;
  bool finished;
  volatile bool decoding; //flag to tag decoding activities
  int stats_mode; // we work with ack mode or with rank mode
  /*****************/

 /* Cumulative statistics */
  time_t start_time;
  time_t stop_time;
  double sumrtt;       /**< sum of RTT samples */
  double sumrtt2;      /**< sum of (RTT^2) */
  int rttsamples;      /**< num of RTT samples */  
  
  double sumftt;       /**< sum of FTT samples */
  double sumftt2;      /**< sum of (FTT^2) */
  int fttsamples;      /**< num of FTT samples */  

  double sumbytes;
  double sumdt;
  /***************************/  
};

#endif /* FOUNTAIN_MODULE_H */

