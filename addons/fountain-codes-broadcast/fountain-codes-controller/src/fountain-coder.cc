/* -*-	Mode:C++ -*- */
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



/***************************************************************************
* TO DO LIST:
* 
*  cerca DA QUI x partire
*  DA FARE: rftt vettoriale
*
****************************************************************************/

#include <iostream>
#include <cmath>
#include <node-core.h>
#include <ip.h>
#include <stdio.h>
#include <fstream>

#include "fountain-coder.h"


 // aliases for timer actions and for stats_mode
enum
{
  ACK_MODE = 7, RANK_MODE = 8,
  DECODE = 9, DATA = 10, RESEND_STATS =11, SYNC = 12, SYNC_ACK = 13, STATS = 14, NEW_BLOCK = 16,
  NEW_BLOCK_ACK = 17 , SYNC_COOP = 18, SYNC_COOP_ACK = 19, RESEND_NEWBLOCKACK = 20,
  CTRL = 0, ALL = 21
};

extern packet_t PT_FCM;

int hdr_fcoder::offset_;

static class FCPktClass : public PacketHeaderClass {
public:
  FCPktClass() : PacketHeaderClass("PacketHeader/FC", sizeof(hdr_fcoder)) {
    this->bind();
    bind_offset(&hdr_fcoder::offset_);
  }
} class_fc_pkt; 



static class FCoderClass : public TclClass {
public:
  FCoderClass() : TclClass("Module/FCM/Coder") {}
  TclObject* create(int, const char*const*) {
    return (new FCoder());
  }
} class_module_fcoder;

vector<FCoder*> FCoder::fcm_pointers; // metto tutti a zero

int FCoder::uidcnt_;		// unique id of the packet generated
int FCoder::uidfc_ = 0;	// simulation-unique id of the FCModules
bool FCoder::initialized = false;

FCoder::FCoder() 
  : txsn(1),
    hrsn(0),
    pkts_recv(0),
    pkts_lost(0),
    pkts_wrblock(0),
    pkts_dupl(0),
    pkts_tx(0),
    pkts_wrdata(0),
    total_data_pkt(0),
    curr_ack_stat(0), // the min current acked pkts among classes
    curr_rank_stat(0), // the max current rank among classes
    curr_round(1),
    curr_class_min(0),
    curr_policy(0),
    curr_block(0),
    total_good_rounds(0),
    block_failed(0),
    isdecoder(true),
    decoding(false),
    done(true),
    newblock_done(false),
    mu(0),
    data(0),
    encodedPkts(0),
    coder(0),
    debug_(0),
    n_class(0)
  {
  // binding to TCL variables
  bind("destPort_", (int*)&dstPort_);
  bind("destAddr_", (int*)&dstAddr_);
  bind("packetSize_", &pktSize_);
  bind("blockSize_", (int*)&blockSize_); // size of the encoding block
  bind("symSize_", (int*)&symSize_);   // size of symbols
  bind("genType_", (int*)&genType_); // generator type
  bind("seed_",(int*)&seed_); //seed for encoder and decoder. MUST BE THE SAME
  bind("max_rounds_", (int*)&max_rounds_); // MAX no of tx rounds
  bind("debug_", (int*)&debug_); //degug mode
  bind("propSpeed_", (double*)&propSpeed_); //propagation speed
  bind("n_class", (int*)&n_class); // no of classes
  bind("d_max", (double*)&d_max);
  bind("d_lower", (double*)&d_lower);
  bind("d_upper", (double*)&d_upper);

  idcurrfc_ = uidfc_; // every FCoder Object stores its own ID
  fcm_pointers.push_back(this);
  uidfc_++;	
  acks_stats.clear();
  rank_stats.clear();
  fcm_peers.clear();
}

FCoder::~FCoder()
{
  delete coder;
  deleteMu();
  deleteData();
  deleteEncodedData();
}

void FCoder::initialize()
{
  ximax = blockSize_ / 2;
  maxPktsToSend = blockSize_ + max_rounds_*ximax;
  bool verbose;
  if (debug_ > 0) verbose = true;
  else verbose = false;
  coder = new Codec( blockSize_, symSize_, genType_, verbose, false);
  setDist();
  if (isdecoder == false) {
    getOptPolicy( optPolicyFile.c_str() ); // gets the Optimal Policy
    allocateData();
    encodeData();
    loadEncodedPkt();
  }
  if (initialized == false) {
     computeClass();
     initialized == true;
  }
  if(debug_ > 0) {
    printf("FCoder(%d)::initialize() coder created %d\n", idcurrfc_,coder);
    printf("FCoder(%d)::initialize() coder parameters: blockS %d symS %d gentT %d \n", idcurrfc_,blockSize_,symSize_,genType_);
    //waitForUser();
  }
}

//const char*const*
// TCL command interpreter
int FCoder::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc==3)
  {
      if(strcasecmp(argv[1], "setDistFile") == 0)
        {
          distFile.clear();
          distFile = argv[2];
          if (debug_ >0) printf("FCoder(%d)::command() setDistFile set to %s\n", idcurrfc_,distFile.c_str());
          return TCL_OK;
        }
      else if(strcasecmp(argv[1], "setOptFile") == 0)
        {
          optPolicyFile.clear();
          optPolicyFile = argv[2];
          if(debug_ >0) printf("FCoder(%d)::command() setOptFile set to %s\n", idcurrfc_,optPolicyFile.c_str());
          return TCL_OK;
        }
  }
  else if(argc==2)
    {
      if(strcasecmp(argv[1], "setAckMode") == 0)	// TCL command to start the packet generation and transmission
	{
	  stats_mode = ACK_MODE;
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "setRankMode") == 0)	// TCL command to start the packet generation and transmission
	{
	  stats_mode = RANK_MODE;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "initialize") == 0)	// TCL command to stop the packet generation
	{
	  initialize();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setToCoder") == 0) // TCL command to set the encoding activity
	{
	  isdecoder = false;
	  coder->setCoder(); 
	  return TCL_OK;
	  if(debug_ >0) printf("FCoder(%d)::command() setToCoder called \n", idcurrfc_);
	}
      else if(strcasecmp(argv[1], "setToDecoder") == 0)	// TCL command to set the decoding activity
	{
	  isdecoder = true;
          coder->setDecoder((maxPktsToSend - blockSize_));
          if(debug_ >0) printf("FCoder(%d)::command() setToDecoder called \n", idcurrfc_);
          return TCL_OK;
	}
    }  
  return Module::command(argc, argv);
}


int FCoder::crLayCommand(ClMessage* m)
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

inline int FCoder::xxmax(int curr_round) 
{
  return(blockSize_ + (curr_round*ximax));
}


void FCoder::createMu() 
{
  if (n_class == 0) {
     printf("FCoder(%d)::createMu() error; call setOptFile command first!!\n",idcurrfc_);
     exit(1);
  }
  else if (mu == 0) {
     mu = new uint16_t***[n_class];
		
     for (int cl=0; cl < n_class; cl++){
   	   mu[cl] = new uint16_t**[max_rounds_+1];
	   for (int i=0; i<=max_rounds_; i++) {
		   mu[cl][i] = new uint16_t*[xxmax(max_rounds_)+2];
		   for (int j=0; j<=xxmax(max_rounds_)+1; j++) {
			   mu[cl][i][j] = new uint16_t[xxmax(max_rounds_)+2]; // from 0 -> L
		   }
	   }
     }
  }
}

void FCoder::deleteMu() 
{
  if ( mu == 0) delete mu;
  else {
     for (int cl=0; cl < n_class; cl++) {
	   for (int i=0; i<=max_rounds_; i++) {
	   	for (int j=0; j<=xxmax(max_rounds_)+1; j++) {
		     delete[] mu[cl][i][j]; // from 0 -> L
		}
	        delete[] mu[cl][i];		
	}
        delete[] mu[cl];	
      }
      delete[] mu;
  }
}

void FCoder::insertAck(int class_id, int ack_value)
{
   acks_stats[class_id].insert(ack_value);

}

void FCoder::insertRank(int class_id, int rank_value)
{
  rank_stats[class_id].insert(rank_value);
}

Stats FCoder::getMinAck() 
{
 Stats result;
 int min_ack = INF;
 int class_min;

 for (SMapiter i = acks_stats.begin(); i != acks_stats.end(); i++) {

    int curr_ack = *(((*i).second).begin());
    if (curr_ack < min_ack) {
        min_ack = curr_ack;
        class_min = (*i).first;
    }
 }  
 result.first=class_min;
 result.second=min_ack;
 return(result);
}

Stats FCoder::getMaxRank()
{
 Stats result;
 int max_rank = -1;
 int class_max;

 for (SMapiter i = rank_stats.begin(); i != rank_stats.end(); i++) {

    Setiter curr_iter = ((*i).second).end(); // la fine
    curr_iter--; // meno uno ==> max
    int curr_rank = *curr_iter; 

    if (curr_rank > max_rank) {
        max_rank = curr_rank;
        class_max = (*i).first;
    }
 }  
 result.first = class_max;
 result.second = max_rank;
 return(result);
}

inline void FCoder::resetRankStats()
{
  rank_stats.clear();
}

inline void FCoder::resetAckStats(){
  acks_stats.clear();
}

inline void FCoder::setDist()
{
  coder->setDistribution(distFile.c_str());
}

void FCoder::getOptPolicy(const char* PolicyFileName)
{
  using namespace std;

  ifstream in(PolicyFileName);
  
  if (!in) {
    printf("\e[1;31mFCoder(%d) empty or wrong optimal policy file: you provided \"%s\"\e[0m\n",idcurrfc_,PolicyFileName);
    exit(1);
  }

  int 	c, x, m, l, op;
 
 /*************************************************************************
  *gets topology infos. 1st line has to be: n_class d_max d_lower d_upper *
  *************************************************************************/
	
  if(debug_ >0) printf("FCoder(%d)::getOptPolicy() Opening optimal policy file: \"%s\" \n",idcurrfc_, PolicyFileName);
     
  createMu();

  while (!in.eof())
  {

	in >> c;
	in >> l;
	in >> x;
	in >> m; 
	in >> op; 	// optimal policy value

	//if(debug_>0) printf("Read: c=%d\tl=%d\tx=%d\tm=%d\top=%d\t\n", c, l, x, m, op);

	if (m == 65535 && op == 65535) // to detect class change
	{
		mu[c-1][1][0][0] = x; // here the optimal policy value is x, not op
		//if (debug_ > 0) printf("mu[%d][%d][%d][%d] given1 value: %d\n", c-1,1,0,0, mu[c-1][1][0][0]);
	}
	else
	{
	   if ( (l <= max_rounds_) && ( x <= xxmax(max_rounds_)+2) && (m <= xxmax(max_rounds_)+1)) {
	      mu[c-1][l][x][m] = op;
	      //if (debug_ > 0) printf("mu[%d][%d][%d][%d] given value: %d\n", c-1,l,x,m, mu[c-1][l][x][m]);
	   }	
	   else printf("\e[1;31mFCoder(%d)::getOptFile() incorrect format of policy \"%s\"...truncating...\e[0m\n",idcurrfc_,PolicyFileName);
	} 
  }
}

void FCoder::allocateData()
{
  data = new uint8_t*[blockSize_];
   
  // Initialize data structure
  for(int i=0; i<blockSize_; i++) {
    data[i] = new uint8_t[symSize_]; 
    for(int j=0; j<symSize_; j++) {
       data[i][j] = i;
       //if(debug_>0) printf("FCoder(%d)::allocateData() data[%d][%d] = %d; \n",idcurrfc_,i,j,data[i][j]); 
    }
  }
  //if (debug_ > 0) waitForUser();
}

void FCoder::deleteData()
{
  if (data == 0) delete data;
  else {
     for (int i=0; i<blockSize_; i++) delete[] data[i] ;
     delete[] data;
  }
}

void FCoder::encodeData()
{
  encodedPkts = new uint8_t*[maxPktsToSend];

  for (int i=0; i< maxPktsToSend; i++) {
    encodedPkts[i] = new uint8_t[symSize_];
    //if(debug_>0) printf("FCoder(%d)::encodeData() created encodedPkts[%d])\n",idcurrfc_,i);
  }
  //if (debug_ > 0) waitForUser();
  
}

void FCoder::deleteEncodedData()
{
  if (encodedPkts == 0) delete encodedPkts;
  else {
     for (int i=0; i<blockSize_; i++) delete[] encodedPkts[i] ;
     delete[] encodedPkts;
  }
}

void FCoder::loadEncodedPkt() 
{
   for (int j = 0; j < blockSize_; j++) {
       coder->setToCoder(data[j]);
       //if(debug_>0) printf("FCoder(%d)::loadEncodedPkt() loaded data[%d]\n",idcurrfc_,j);
   }
   //if (debug_ > 0) waitForUser();
}

void FCoder::computeClass() 
{
  Position* p_sender = getPosition();
  double dist;
  double max_dist = 0.0;

  for (int i = 0; i < fcm_pointers.size() ; i++) {
       Position* p_receiver = fcm_pointers.at(i)->getPosition();
       dist = p_sender->getDist(p_receiver);
       if (dist > max_dist) max_dist = dist;
       if (debug_ > 0) printf("FCoder(%d)::computeClass() user(%d) has distance: %f \n",idcurrfc_,i,dist);
       if (n_class == 1) {
	   class_id.push_back(0);
       }
       if (n_class == 2) {
          if (dist > d_max) {
	     printf("\e[1;31mFCoder(%d)::computeClass() ERROR!! user(%d) has distance %f > d_max %f! \e[0m\n",idcurrfc_,i,dist,d_max);
             exit(1);
          }
          else if (dist <= d_lower) class_id.push_back(0); // caso d< d_lower
          else class_id.push_back(1);
       }
       else{ 
         if (dist <= d_lower) class_id.push_back(0); // caso d< d_lower
         else if ((dist > d_lower) && (dist < d_upper)) { // caso d_lower < d < d_upper
      	    double class_dist = (d_upper-d_lower)/(n_class-2);
            int class_user = int(ceil((dist-d_lower)/class_dist));
            class_id.push_back(class_user);
         }
         else if (( dist >= d_upper) && (dist <= d_max)) { // caso d_upper <= d <= d_max
	    class_id.push_back(n_class-1);
         }
         else { // caso d > d_max
	     printf("\e[1;31mFCoder(%d)::computeClass() ERROR!! user(%d) has distance %f > d_max %f! \e[0m\n",idcurrfc_,i,dist,d_max);
             exit(1);
	 }       
      }
  if (debug_ > 0) printf("FCoder(%d)::computeclass() user(%d) has class: %d\n",idcurrfc_,i,class_id.at(i));
  }
  max_RTT = 2*(max_dist/propSpeed_);
  if (debug_ > 0) printf("FCoder(%d)::computeclass() max_RTT is: %f\n",idcurrfc_,max_RTT);
}

int FCoder::getTxPolicy(int class_input, int curr_stats)
{
  if(curr_round == 1) {
     curr_policy = 0;
     int id;
     int class_no;
     for (Setiter i = fcm_peers.begin(); i != fcm_peers.end() ; i++) {
         id = (*i); 
         class_no = class_id.at(id); 
         if ( mu[class_no][1][0][0] > curr_policy) curr_policy = mu[class_no][1][0][0];
     }
  }
  else {
     curr_policy = curr_policy + mu[class_input][curr_round][pkts_tx][curr_stats];
  }
  if(debug_>0) printf("FCoder(%d)::getTxPolicy(): the new policy is %d \n",idcurrfc_,curr_policy);

  return curr_policy;
}

inline void FCoder::incrDataPktReceived()
{
   curr_ack_stat++;
   if (debug_ > 0) printf("\e[35m FCoder(%d)::incrDataPktReceived(): added a DATA pkt. current DATA total is %d\e[0m\n",idcurrfc_,curr_ack_stat);
}

void FCoder::resetTxStats()
{
   pkts_tx = 0;
   curr_ack_stat = 0;
   curr_rank_stat = 0;
   curr_round = 1;
   curr_policy = 0;

   acks_stats.clear();
   rank_stats.clear();
}

void FCoder::setToCoder()
{
  coder->setCoder();
}

void FCoder::setToDecoder()
{
  coder->setDecoder(maxPktsToSend - blockSize_);
}

void FCoder::doNewBlock() 
{
  done = false;
  curr_block++;
  coder->startNewBlock(seed_++);
  if (isdecoder == false) {
     loadEncodedPkt();
  }
  resetTxStats(); 
  if (debug_>0) {
     printf("\e[32m FCoder(%d)::doNewBlock() started a new block %d of total %d with seed: %d \e[0m\n",idcurrfc_,curr_block,totBlock_,seed_-1);
     //waitForUser();
  }
}


void FCoder::initPkt(Packet* p )
{
 
  hdr_cmn* ch = hdr_cmn::access(p);
  ch->uid() = uidcnt_++;
  ch->ptype() = PT_FC;
  ch->size() = pktSize_;
  
  hdr_ip* iph = hdr_ip::access(p);
  iph->daddr() = dstAddr_;
  iph->dport() = dstPort_;

  hdr_fcoder* fch = HDR_FCODER(p);
    
  fch->pkt_pointer = encodedPkts[pkts_tx];  
  fch->block_id = curr_block; // curr block
  fch->pktid = pkts_tx+1; // pkt id of curr block	
  fch->total_ack = CTRL; // no of acked pkts
  fch->res_rank = CTRL; //residual rank
  pkts_tx++; // aumento il contatore dei pacchetti spediti per il blocco corrente
   
  /*comuni a tutti*/
  fch->sn = txsn;  // initialize serial number
  fch->sender_id = idcurrfc_; // initialize sender id
  fch->ts = Scheduler::instance().clock();
  ch->timestamp() = fch->ts;
  
  txsn++; // aumento il serial number

  /*if (debug_ > 0 ) {
    printf("FCoder(%d)::initPkt() sn=%d, pktid=%d, TS=%f \n",idcurrfc_,fch->sn,fch->pktid,fch->ts);  
    waitForUser();

  }*/
 
}

Packet* FCoder::getDataPkt()
{

  Packet* p = Packet::alloc();

  coder->getCodedPck( encodedPkts[pkts_tx], (pkts_tx+1));

  total_data_pkt++; //aumento il contatore dei tx pkts totali 

  initPkt(p);

  hdr_cmn* ch = hdr_cmn::access(p);

  if(debug_>0) 
     printf("\e[36m FCoder(%d)::sendPkt(), send a pkt (%d) \e[0m\n",idcurrfc_,ch->uid());
 
  return(p);
}

void FCoder::recv(Packet* p, Handler* h)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  recv(p);
}

// DA QUI
void FCoder::recv(Packet* p)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  //if (debug_>0) printf("FCoder(%d)::recv(Packet*p,Handler*) pktId %d\n",idcurrfc_, ch->uid());
  hdr_fcoder* fch = HDR_FCODER(p);
    
  if (ch->ptype() != PT_FC)
    {
      drop(p,1,FC_DROP_REASON_UNKNOWN_TYPE);
      incrPktInvalid();
      return;
    }

  hdr_ip* iph = hdr_ip::access(p);
  if (iph->saddr() != dstAddr_)
    {
      drop(p,1,FC_DROP_REASON_WRONG_SADDR);
      incrPktInvalid();
      return;
    }
  if (iph->sport() != dstPort_)
    {
      drop(p,1,FC_DROP_REASON_WRONG_SPORT);
      incrPktInvalid();
      return;
    }
  
  if (fch->block_id != curr_block) 
  {
     drop(p,1,FC_DROP_REASON_WRONG_BLOCK_NUMBER); // per nn far scrivere nulla nel trace
     //Packet::free(p);
     incrPktWrblock();
     return;
  }
   
  if (debug_>0) printf("\e[34m FCoder(%d)::recv() a pkt, sn %d sender %d blkno %d \e[0m \n",idcurrfc_,fch->sn,fch->sender_id,fch->block_id);
  
  static vector<hdr_fcoder> buffer(0); // buffer

  int esn = hrsn + 1; // expected sn
  hrsn = fch->pktid;
  if (hrsn > esn)
  { // packet losses are observed
         incrPktLost(hrsn - esn );
  }

  if ((isdecoder == true) && (done == false)){ // sn decoder e devo ancora risolvere
     int block = fch->block_id; 
     int sender = fch->sender_id; // we retrieve the sender id
     fcm_peers.insert(sender);
     uint16_t index = fch->pktid;  // we retrieve the array index
     uint8_t* pkt_point = fch->pkt_pointer; // we retrieve the pkt pointer
     int serial = fch-> sn;
     incrDataPktReceived(); // poi aumento gli ack miei
     if (decoding == false) { // non sto già decodificando
        coder->setToDecoder(pkt_point, index); // do il pkt al decoder
        for(hdr_iter i = buffer.begin(); i != buffer.end(); i++) {  // svuoto il buffer
 	    coder->setToDecoder((*i).pkt_pointer , (*i).pktid); //setto i pkts del buffer 
        }
        buffer.clear(); //svuoto il buffer
     }
     else { //i pkts arrivano mentre sto decodificando, devo bufferizzare 
        buffer.push_back(*fch);          
     }
     if (debug_ > 0) 
           printf("  FCoder(%d)::recv() received a DATA pkt: sn %d sender %d index %d pointer %d\n",idcurrfc_,serial,sender,index,pkt_point);             
    }
    if ((isdecoder == true) && (done == true)) { // ho già risolto, scarto il pacchetto dati
         drop(p,1,FC_DROP_REASON_DECODED_DATA);
         //Packet::free(p);
         incrPktWrdata();
         return;
    }
    if ( isdecoder == false ) { // sono sender
         drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
         //Packet::free(p);
         incrPktInvalid();
         return;
    } 

  incrPktRecv();
 
  Packet::free(p);
 
}

void FCoder::tryToDecode() // decodifica e aggiorna min ack o min rank
{
  if (isdecoder == true) { // controllo ridondante
     newblock_done = false;
     if ( curr_ack_stat >= blockSize_ ) { // posso provare a decodificar
         decoding = true;
         curr_rank_stat = coder->decode();
         if (curr_rank_stat == 0) done = true; // se ho decodificato setto il flag done       
         decoding = false;
     } 
     else curr_rank_stat = INF; 
     if (debug_ > 0 ) {
         printf("FCoder(%d)::tryToDecode() residual rank = %d current acked pkts %d\n",idcurrfc_,curr_rank_stat,curr_ack_stat);
         //waitForUser();
     }
  }
  else {
     printf("\e[32mFCoder(%d)::tryToDecode() error, encoder is not supposed to decode!!!\e[0m\n",idcurrfc_);
     exit(1);
  }
}

inline void FCoder::printStats()
{
  printf("\e[1;37;40mFCoder::printStats() blocks failed=%d of %d total blocks | totaldata pkts=%d | total good rounds=%d \e[0m\n",block_failed,totBlock_,total_data_pkt,total_good_rounds);
  
  for (int i = 0; i < fcm_pointers.size() ; i++) {
     PerMap per_stats = fcm_pointers[i]->GetPER();

     for (PerMapIter it = per_stats.begin(); it != per_stats.end(); it++ ) {

         printf("\e[1;37;40mFCoder::printStats() user %d  has PER = %e from sender user %d\e[0m\n",i,(*it).second,(*it).first);
     }
  }
  printf("\e[1;37;40mFCoder::printStats() sender N_tx = %f\e[0m\n",(double)total_data_pkt/(double)totBlock_);
  printf("\e[1;37;40mFCoder::printStats() sender P_fail = %e\e[0m\n",(double)block_failed/(double)totBlock_);
  printf("\e[1;37;40mFCoder::printStats() sender N_rounds = %f (max rounds = %d)\e[0m\n",((double)(max_rounds_*block_failed+total_good_rounds)/(double)totBlock_),max_rounds_);
  printf("\n\n");
}

PerMap FCoder::GetPER() // funziona solo per i pkt DATA x i pkt di controllo sono su canale senza errori
{
  PerMap per_peers;
  per_peers.clear();
  double per;
  for (Setiter it = fcm_peers.begin(); it != fcm_peers.end(); it++) {        
         int total_data = fcm_pointers.at((*it))->total_data_pkt;
         if (total_data != 0) {
             per = (double) (pkts_lost ) / (double)(total_data);
         }
         else per = 0.0;
         per_peers[(*it)] = per;
  }                 
  
  return per_peers;
}

inline int FCoder::getFcId() 
{
  return idcurrfc_;
}

inline int FCoder::getCurrAck() 
{
  return curr_ack_stat;
}

inline int FCoder::getCurrRank() 
{
 return curr_rank_stat;
}

inline bool FCoder::isDecoder()
{
  return isdecoder;
}

inline double getMaxRTT()
{
  return max_RTT;
}
inline void FCoder::incrPktLost(int npkts)
{
  pkts_lost += npkts;
}

inline void FCoder::incrPktRecv()
{
  pkts_recv++;
}

inline void FCoder::incrPktInvalid()
{
  pkts_invalid++;
}

inline void FCoder::incrPktDupl()
{
  pkts_dupl++;
}

inline void FCoder::incrPktWrblock()
{
  pkts_wrblock++;
}

inline void FCoder::incrPktWrdata()
{
  pkts_wrdata++;
}


inline void FCoder::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
}

