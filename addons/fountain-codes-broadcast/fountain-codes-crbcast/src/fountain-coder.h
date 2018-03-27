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
 

#ifndef FOUNTAIN_CODER_H
#define FOUNTAIN_CODER_H

#include <module.h>
#include "codec.h" // so we can use the codec libary
#include <vector>
#include <set>
#include <map>
#include <string>

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

#define HDR_FCODER(P)      (hdr_fcoder::access(P))

class FCoder;

struct hdr_fcoder;

typedef vector<FCoder*>::iterator Fcm_iter;
typedef vector<hdr_fcoder>::iterator hdr_iter;
typedef map<int, set<int> > SMap;
typedef pair<int, set<int> > Entry;
typedef pair<int, int> Stats;
typedef map<int, double> PerMap;
typedef SMap::iterator SMapiter;
typedef set<int>::iterator Setiter;
typedef PerMap::iterator PerMapIter;

extern packet_t PT_FC;

typedef struct hdr_fcoderoder {

  double ts;  /**< packet timestamp, i.e., its generation time) */
  
  int sn;     /**< sequence number of this packet */  

  int sender_id;

  int command;

  uint8_t* pkt_pointer; /** pointer to the location of THIS pkt in the *
              		 * encodedPks array                           */
  int block_id; // curr block

  uint16_t pktid; // pkt id of curr block	
 
  int total_ack; // no of acked pkts

  int res_rank; //residual rank

  static int offset_;
  inline static int& offset() { return offset_; }
  inline static struct hdr_fcoder* access(const Packet* p) {
    return (struct hdr_fcoder*)p->access(offset_);
  }
  
} hdr_fcoderoder;



class FCoder : public Module
{
  public:

  /**metodi pubblici*******************/
  FCoder();
  virtual ~FCoder();		
		
  virtual void recv(Packet*);
  virtual void recv(Packet* p, Handler* h); 
  virtual int command(int argc, const char*const* argv);
  virtual int crLayCommand(ClMessage* m);
  virtual Packet* getDataPkt();
  virtual void doNewBlock();
  virtual void initialize(); // create the coder
  virtual void tryToDecode();
  virtual int getTxPolicy(int class_id = 0, int curr_stats = 0); // update  curr_policy 
  virtual void setToCoder();
  virtual void setToDecoder();

  /*metodi per inserire e accedere alle mappe*/
  virtual void insertAck(int class_id, int ack_value);
  virtual void insertRank(int class_id, int rank_value);
  virtual Stats getMinAck();
  virtual Stats getMaxRank();
  virtual void resetRankStats();
  virtual void resetAckStats();
 
  virtual PerMap GetPER();  
  virtual int getFcId();
  virtual int getCurrAck();
  virtual int getCurrRank();
  virtual void printStats();
  virtual bool isDecoder();
  virtual double getMaxRTT();

  /*************************************/

  protected:

  /*constants***************/
  static const int XIMIN = 1;
  static const int INF = 65535; 
  static const int MAXDIST = 20000;
  /***************************/

  /*Timer e Coder*/
  Codec* coder;   // pointer to the encoder/decoder
  /***************/
  
  /***protocol stack id**************/
  nsaddr_t dstPort_;
  nsaddr_t dstAddr_;
  /**********************************/
  
  /********metodi interni************/ 
  virtual void computeClass(); //compute the class of the FC Module
  virtual void sendPkt(double delay = 0);
  virtual void initPkt(Packet* p);
  virtual void setDist();
  virtual void getOptPolicy(const char* optPolicyFile); // gets the Optimal Policy
  virtual int xxmax(int curr_round); // get the max pkts to send at the current round
  virtual void resetTxStats(); // resets Acks and Ranks numbers to zero
  virtual void allocateData(); // allocates data in memory
  virtual void encodeData(); // passes the data to the encoder
  virtual void deleteData(); // delete the data
  virtual void deleteEncodedData(); // delete the encoded data
  virtual void createMu(); // creates the mu pointer-matrix for the optimal policy
  virtual void deleteMu(); // deletes the mu
  virtual void incrDataPktReceived();
  virtual void loadEncodedPkt();
  /***************************************/
  
  /*******metodi di servizio**********/
  virtual void waitForUser();
  /*********************************/

  /*** metodi di CBRMiracle*********/
  virtual void incrPktLost(int npkts);
  virtual void incrPktRecv();
  virtual void incrPktInvalid();
  virtual void incrPktDupl();
  virtual void incrPktWrblock();
  virtual void incrPktWrdata();
  /**********************************/

  /***variabili di CBRMiracle********/
  int txsn;          /**< sequence number of next new packet to be transmitted */
  int hrsn;          /**< highest received sequence number */
  int pkts_recv;     /**< total number of received packets */
  int pkts_lost;     /**< total number of lost packets,  */
  int pkts_invalid;  /**< total number of invalid packets received */
  int pkts_dupl; 
  int pkts_wrblock;
  int pkts_wrdata;

  /*dati input *************/
  int pktSize_;        /**< packet size */
  int blockSize_; // size of the encoding block
  int symSize_;   // size of symbols
  int genType_; // generator type
  int debug_;
  int seed_; //seed for random generator
  int max_rounds_; // max no of tx rounds
  double propSpeed_ ; // propagation speed
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
  static int uidfc_;   /* simulation-unique FCoder ID */
  int idcurrfc_ ;      /* id of the FCoder */

  int curr_round; // current tx round
  int curr_policy; //no of pkt to tx for the curr round
  int curr_class_min; //current policy class
  int curr_block;
  int curr_ack_stat; // the min current acked pkts among classes
  int curr_rank_stat; // the max current rank among classes

  int pkts_tx; // number of pkts sent in curr block

  int total_data_pkt;

  int maxPktsToSend; // number of max overhead encoded pck that the decoder can receive

  int block_failed; // da togliere static
  int total_good_rounds; 
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
  uint16_t**** mu; // optimal policy
 
  static vector<FCoder*> fcm_pointers; // Array of pointers to all FC Modules present in the simulation
 
  set<int> stats_peers; // contiene i ricevitori che hanno spedito correttamente gli stats
  set<int> fcm_peers; // contiene i ricevitori che hanno spedito correttamente gli stats

  vector<int> class_id;

  SMap acks_stats;
  SMap rank_stats;
  /************************************/

  /*bool e flag **/
  
  static bool initialized;
  bool isdecoder; // flag to tag a decoder
  bool done;
  bool newblock_done;
  volatile bool decoding; //flag to tag decoding activities
  int stats_mode; // we work with ack mode or with rank mode
  /*****************/

};

#endif /* FOUNTAIN_MODULE_H */

