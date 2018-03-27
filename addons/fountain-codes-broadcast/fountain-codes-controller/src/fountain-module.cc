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
#include <iomanip>
//#include <node-core.h>
#include <ip.h>
#include <stdio.h>
//#include <bitset>
#include <fstream>
#include <sstream>
#include <string>
#include "FCM-Multihop-Controller.h"
#include "fountain-module.h"
#include "forward-module.h"
#include "FCM-CRBCast-Controller.h"


 // aliases for timer actions and for stats_mode
enum
{
  ACTION_DATA = 1, ACTION_DECODE = 2, ACTION_SYNC = 3, ACTION_WAIT_STATS = 4, ACTION_NEWBLOCK = 5,
  ACTION_WAIT_NEWBLOCK = 6, 
  ACK_MODE = 7, RANK_MODE = 8,
  DECODE = 9, DATA = 10, RESEND_STATS = 11, SYNC = 12, SYNC_ACK = 13, STATS = 14, NEW_BLOCK = 16,
  NEW_BLOCK_ACK = 17 , SYNC_COOP = 18, SYNC_COOP_ACK = 19, RESEND_NEWBLOCKACK = 20,
  CTRL = 5000, ALL = 21, NOT_SET = 1111,
  CRB_PHASE_I = 30, CRB_PHASE_II = 31 , CRB_PHASE_I_SGT = 32 , CRB_PHASE_II_SGT = 33, CRB_INACTIVE = 88, 
  MU_NORMAL = 50 , MU_CRB = 51, INCOMPLETE = 101, COMPLETE = 102
};

extern packet_t PT_FCM;

int hdr_fc::offset_;

static class FCPktClass : public PacketHeaderClass {
public:
  FCPktClass() : PacketHeaderClass("PacketHeader/FCM", sizeof(hdr_fc)) {
    this->bind();
    bind_offset(&hdr_fc::offset_);
  }
} class_fc_pkt; 



static class FCModuleClass : public TclClass {
public:
  FCModuleClass() : TclClass("Module/FCM/SimpleMAC") {}
  TclObject* create(int, const char*const*) {
    return (new FCModule());
  }
} class_module_fcm;



void FCMTimer::expire(Event *e)
{
  if ( (module->debug_ > 0) && (module->idcurrfc_ == 42) ) {
    printf("\e[33mFCMTimer(%d)::expire() expired action event %s \e[0m \n",module->idcurrfc_,module->printCommand(action));
    if( action != ACTION_DATA) module->waitForUser();
  }

  switch (action) {
  
     case (ACTION_DATA): {
        module->transmit(); 
     }
     break;

     case (ACTION_WAIT_STATS): {
        module->changePolicy();
     }
     break;

     case (ACTION_WAIT_NEWBLOCK):{
        module->controlNewBlock();
     }
     break;
     
     case (ACTION_SYNC): {
        module->discovery();        
     }
     break;

     case (ACTION_NEWBLOCK): {
        module->sendNewBlock();
     }
     break;

     default:     
        fprintf(stderr,"FCMTimer(%d)::expire() unknown expired event",module->idcurrfc_);
        exit(1);
        break;
  }
}

inline void FCMTimer::setAction(int fcm_action)
{
  action = fcm_action;   
}

inline int FCMTimer::getAction()
{
  return action;
}

vector<FCModule*> FCModule::fcm_pointers; // metto tutti a zero
vector<double> FCModule::dist_vector; // metto tutti a zero
set<int> FCModule::complete_peers;
set<int> FCModule::incomplete_peers;
//set<int> FCModule::current_compl_peers;
list<int> FCModule::fcm_lcomp;
set<int> FCModule::source_phase_I_index;
map< Mu_entry , Mu > FCModule::mu_map;
map< Mu_entry , bool > FCModule::mu_map_done;

int FCModule::uidcnt_;		// unique id of the packet generated
int FCModule::uidfc_ = 0;	// simulation-unique id of the FCModules

FCModule::FCModule() 
  : FCMTmr_(this),
    txsn(1),
    hrsn(0),
    pkts_recv(0),
    pkts_lost(0),
    pkts_wrblock(0),
    pkts_dupl(0),
    pkts_last_reset(0),    
    pkts_tx(0),
    pkts_wrdata(0),
    rftt(-1),
    lrtime(0),
    sthr(0),
    sumrtt(0),
    rttsamples(0),
    sumftt(0),
    fttsamples(0),
    sumbytes(0),
    sumdt(0),
    total_data_pkt(0),
    total_ctrl_pkt(0),
    curr_ack_stat(NOT_SET), // the min current acked pkts among classes
    curr_rank_stat(NOT_SET), // the max current rank among classes
    old_ack_stat(NOT_SET),
    old_rank_stat(NOT_SET),
    curr_round(1),
    curr_class_min(0),
    curr_policy(0),
    curr_block(0),
    total_good_rounds(0),
    block_failed(0),
    isdecoder(true),
    decoding(false),
    do_retx(false),
    is_master(false),
    done(true),
    newblock_done(false),
    finished(false),
    printstats(false),
    mhop(false),
    mu(0),
    data(0),
    encodedPkts(0),
    coder(0),
    debug_(0),
    n_class(0),
    ctrl_module_id(0),
    send_module_id(0),
    N_tx(0.0),
    P_fail(0.0),
    N_rounds(0.0),
    multihop_ctrl(0),
    crbcast_ctrl(0),
    crb_phase(CRB_INACTIVE),
    crb_phase_I_seed(NOT_SET),
    class_dist(0.0),
    crb_phase_I_ack_done(false)
  {
  // binding to TCL variables
  bind("period_", &period_);
  bind("destPort_", (int*)&dstPort_);
  bind("destAddr_", (int*)&dstAddr_);
  bind("packetSize_", &pktSize_);
  bind("alpha_", &alpha_);
  bind("blockSize_", (int*)&blockSize_); // size of the encoding block
  bind("symSize_", (int*)&symSize_);   // size of symbols
  bind("totBlock_", (int*)&totBlock_); // total tx block number
  bind("genType_", (int*)&genType_); // generator type
  bind("seed_",(int*)&seed_); //seed for encoder 
  bind("max_rounds_", (int*)&max_rounds_); // MAX no of tx rounds
  bind("debug_", (int*)&debug_); //degug mode
  bind("propSpeed_", (double*)&propSpeed_); //propagation speed
  bind("n_class", (int*)&n_class); // no of classes
  bind("d_max", (double*)&d_max);
  bind("d_lower", (double*)&d_lower);
  bind("d_upper", (double*)&d_upper);
  bind("XI_", (int*)&XI_);
  bind("TxRadius_", (double*)&TxRadius_);

  idcurrfc_ = uidfc_; // every FCModule Object stores its own ID
  fcm_pointers.push_back(this);	
  acks_stats.clear();
  rank_stats.clear();
  fcm_peers.clear();
  complete_peers.clear();
  incomplete_peers.insert(idcurrfc_);
  fcm_lcomp.clear();
  crb_phase_I_index.clear();
  crb_index_iter = crb_phase_I_index.begin();
  source_phase_I_index.clear();
  mu_map_done.clear();
  mu_map.clear();
  uidfc_++;
}

FCModule::~FCModule()
{
  delete coder;
  map< Mu_entry , Mu >::iterator iter;
  for (iter = mu_map.begin(); iter != mu_map.end(); iter++) {
      Mu curr_mu = (*iter).second;
      Mu_entry entry = (*iter).first;
      deleteMu(curr_mu, entry);
  }
  deleteData();
  deleteEncodedData();
}


void FCModule::initialize()
{
  //ximax = blockSize_ /2; 
  ximax = blockSize_ / 2 ;
  maxPktsToSend = blockSize_ + max_rounds_*ximax;
  bool verbose;
  if (debug_ > 0) verbose = true;
  else verbose = false;
  coder = new Codec( blockSize_, symSize_, genType_, verbose, false);
  setDist();
  if(debug_ > 0) {
    printf("FCModule(%d)::initialize() coder created %p\n", idcurrfc_,coder);
    printf("FCModule(%d)::initialize() coder parameters: blockS %d symS %d gentT %d \n", idcurrfc_,blockSize_,symSize_,genType_);
    //waitForUser();
  }
}

//const char*const*
// TCL command interpreter
int FCModule::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc==3)
  {
      if(strcasecmp(argv[1], "setDistFile") == 0)
        {
          distFile.clear();
          distFile = argv[2];
          if (debug_ >0) printf("FCModule(%d)::command() setDistFile set to %s\n", idcurrfc_,distFile.c_str());
          return TCL_OK;
        }
      else if(strcasecmp(argv[1], "setOptFile") == 0)
        {
          optPolicyFile.clear();
          optPolicyFile = argv[2];
          if(debug_ >0) printf("FCModule(%d)::command() setOptFile set to %s\n", idcurrfc_,optPolicyFile.c_str());
          return TCL_OK;
        }
      else if(strcasecmp(argv[1], "setCRBOptFile") == 0)
        {
          crb_policy_file.clear();
          crb_policy_file = argv[2];
          if(debug_ >0) printf("FCModule(%d)::command() setCRBOptFile set to %s\n", idcurrfc_,crb_policy_file.c_str());
          return TCL_OK;
        }
      else if(strcasecmp(argv[1], "addForwardModule") == 0)
        {
          ctrl_module_id =((EmptyModule*)TclObject::lookup(argv[2]))->getId(); //get Id of module
          return TCL_OK;
        }
      else if(strcasecmp(argv[1], "addSendModule") == 0)
        {
          send_module_id =((EmptyModule*)TclObject::lookup(argv[2]))->getId(); //get Id of module
          return TCL_OK;
        }     
      else if(strcasecmp(argv[1], "addCtrlMultihop") == 0)
        {
          if(debug_ >0) printf("FCModule(%d)::command() addCtrlMultihop called\n", idcurrfc_);
          multihop_ctrl =(FCMMultihopController*)TclObject::lookup(argv[2]); //get Pointer to Module
          return TCL_OK;
        }     
      else if(strcasecmp(argv[1], "addCrbCastCtrl") == 0)
        {
          if(debug_ >0) printf("FCModule(%d)::command() addCrbCastCtrl called\n", idcurrfc_);
          crbcast_ctrl =(FCMCRBController*)TclObject::lookup(argv[2]); //get Pointer to Module
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
      if(strcasecmp(argv[1], "startMasterEncoder") == 0)	// TCL command to start the packet generation and
	{
	  if(debug_ >0) printf("FCModule(%d)::command() startMasterEncoder called\n", idcurrfc_);
	  startMasterEncoder();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "stop") == 0)	// TCL command to stop the packet generation
	{
	  stop();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "initialize") == 0)	// TCL command to stop the packet generation
	{
	  initialize();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setToCoder") == 0) // TCL command to set the encoding activity
	{
	  setToCoder();
	  return TCL_OK;
	  if(debug_ >0) printf("FCModule(%d)::command() setToCoder called \n", idcurrfc_);
	}
      else if(strcasecmp(argv[1], "setToDecoder") == 0)	// TCL command to set the decoding activity
	{
	  setToDecoder();
          if(debug_ >0) printf("FCModule(%d)::command() setToDecoder called \n", idcurrfc_);
          return TCL_OK;
	}
      if(strcasecmp(argv[1], "printStats") == 0)	// TCL command to start the packet generation and transmission
	{
	  printstats = true;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getrtt") == 0)	
	{  
	  tcl.resultf("%f",GetRTT());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getftt") == 0)	
	{
	  tcl.resultf("%f",GetFTT());
	  return TCL_OK;
	}
      /*else if(strcasecmp(argv[1], "getper") == 0)	
	{
	  tcl.resultf("%f",GetPER());
	  return TCL_OK;
	}*/	
      else if(strcasecmp(argv[1], "getthr") == 0)	
	{
	  tcl.resultf("%f",GetTHR());
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "getrttstd") == 0)	
      	{
	  tcl.resultf("%f",GetRTTstd());
      	  return TCL_OK;
      	}
      if(strcasecmp(argv[1], "getfttstd") == 0)	
      	{
	  tcl.resultf("%f",GetFTTstd());
      	  return TCL_OK;
      	}
      /*else if(strcasecmp(argv[1], "sendPkt") == 0)	
	{
	  sendPkt();
	  return TCL_OK;
	}*/
      else if(strcasecmp(argv[1], "resetStats") == 0)	
	{
	  resetStats();
          resetTxStats();
	  fprintf(stderr,"FCModule::command() resetStats %s, pkts_last_reset=%d, hrsn=%d, txsn=%d\n", tag_ , pkts_last_reset, hrsn, txsn);
	  return TCL_OK;
	}
    }  
  return Module::command(argc, argv);
}


int FCModule::crLayCommand(ClMessage* m)
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

inline int FCModule::xxmax(int curr_round) 
{
  return(blockSize_ + (curr_round*ximax));
}

inline void FCModule::setToCoder()
{
  isdecoder = false;
  coder->setCoder(); 
}

inline void FCModule::setToDecoder()
{
  isdecoder = true;
  coder->setDecoder((maxPktsToSend - blockSize_));

}

void FCModule::createMu(Mu& input_mu, Mu_entry entry) 
{
  if (n_class == 0) {
     printf("FCModule(%d)::createMu() error; call setOptFile command first!!\n",idcurrfc_);
     exit(1);
  }
  else if (input_mu == 0){
     static int i = 0;
     i++;
     if (debug_ >0) cout << "FCModule(" << idcurrfc_ <<")::createMu() creating " << i << "th Mu " << endl;
     
     input_mu = new uint16_t***[n_class];
		
     for (int cl=0; cl < n_class; cl++){
   	   input_mu[cl] = new uint16_t**[max_rounds_+1];
	   for (int i=0; i<=max_rounds_; i++) {
		   input_mu[cl][i] = new uint16_t*[xxmax(max_rounds_)+2];
		   for (int j=0; j<=xxmax(max_rounds_)+1; j++) {
			   input_mu[cl][i][j] = new uint16_t[xxmax(max_rounds_)+2]; // from 0 -> L
		   }
	   }
     }
  }
}

void FCModule::deleteMu(Mu& input_mu, Mu_entry entry) 
{
  mu_map_done[entry] = false;
  if ( input_mu == 0) delete input_mu;
  else {
     for (int cl=0; cl < n_class; cl++) {
	   for (int i=0; i<=max_rounds_; i++) {
	   	for (int j=0; j<=xxmax(max_rounds_)+1; j++) {
		     delete[] input_mu[cl][i][j]; // from 0 -> L
		}
	        delete[] input_mu[cl][i];		
	}
        delete[] input_mu[cl];	
      }
      delete[] input_mu;
  }
}

void FCModule::insertAck(int class_id, int ack_value)
{
   acks_stats[class_id].insert(ack_value);

}

void FCModule::insertRank(int class_id, int rank_value)
{
  rank_stats[class_id].insert(rank_value);
}

Stats FCModule::getMinAck() 
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

Stats FCModule::getMaxRank()
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

inline void FCModule::resetRankStats()
{
  rank_stats.clear();
}

inline void FCModule::resetAckStats(){
  acks_stats.clear();
}

inline void FCModule::setDist()
{
  coder->setDistribution(distFile.c_str());
}

void FCModule::getOptPolicy()
{

  using namespace std;
  
  map<int,Mu >::iterator mu_iter;
  map<int,int>::iterator class_iter;
  stringstream str_out;
  string name_prefix = "optimal-policy-GUERRA_d";
  string path = "/home/fguerra/simulazioni/optpolicies/multihop/";
  string file_name;

  ifstream in;  

  for (class_iter = tot_nds_classes.begin() ; class_iter != tot_nds_classes.end() ; class_iter++) {
 
      int class_no = (*class_iter).first;
      int tot_nds = (*class_iter).second;
      if (tot_nds == 0) continue;
            
      double dist = d_lower + ( class_dist * class_no);

      if (dist > TxRadius_) continue;

      /*if (tot_nds > 10) cout << "FCModule(" << idcurrfc_ <<")::getOptPolicy() class = " << class_no << " tot nds = "
                             << tot_nds  << " dist = " << dist << endl;*/

      dist /= 1000.0;


      str_out << path << name_prefix << fixed << setprecision(2) << dist << "_nodes" << tot_nds;
      file_name = str_out.str();
      str_out.str("");

      if (debug_ > 0) cout << "FCModule(" << idcurrfc_ <<")::getOptPolicy() class = " << class_no << " tot nodes = "
                           << tot_nds << " distance = " << dist << endl;


      if (n_class > 1) in.open(file_name.c_str());
      else in.open(optPolicyFile.c_str());
 
      if (debug_ > 0) cout << "FCModule(" << idcurrfc_ <<")::getOptPolicy() opening policy " 
                           << file_name << endl;

      if (!in) {
         cout << "\e[1;31mFCModule(" << idcurrfc_ <<")::getOptPolicy() empty or wrong optimal policy file: "
              << " you provided \"" << file_name << "\" \e[0m\n";
         exit(1);
      }

      //mu_map[class_no] = 0;  
      Mu_entry entry;
      entry = make_pair(class_no, tot_nds);    
      createMu(mu_map[entry] , entry);
      Mu curr_map = mu_map[entry];
     
      if ( (curr_map != 0) && (mu_map_done[entry] == true) ) {
 
         if (debug_ >0) cout << "FCModule::getOptPolicy() current_map already created , class = " << class_no 
                << " dist = " << dist <<" tot_nds = " << tot_nds << endl;

          in.close();
          continue;
      }
      else mu_map_done[entry] = true;

      int  c, x, m, l, op;
      
      while (!in.eof()) {

	   in >> c;
	   in >> l;
	   in >> x;
	   in >> m; 
	   in >> op; 	// optimal policy value

	  // if(debug_>0) printf("Read: c=%d\tl=%d\tx=%d\tm=%d\top=%d\t\n", c, l, x, m, op);
           
	   if (m == 65535 && op == 65535) // to detect class change
	   {
		curr_map[c-1][1][0][0] = x; // here the optimal policy value is x, not op
		//if (debug_ > 0) printf("mu[%d][%d][%d][%d] given value: %d\n", c-1,1,0,0, curr_map[c-1][1][0][0]);
	   }
	   else
	   {
	      if ( (l <= max_rounds_) && ( x <= xxmax(max_rounds_)+2) && (m <= xxmax(max_rounds_)+1)) {
	         curr_map[c-1][l][x][m] = op;
	         //if (debug_ > 0) printf("mu[%d][%d][%d][%d] given value: %d\n", c-1,l,x,m, curr_map[c-1][l][x][m]);
	      }	
	      //else if (debug_ > 0) printf("\e[1;31mFCModule(%d)::getOptFile() incorrect format of policy \"%s\"...truncating...\e[0m\n",idcurrfc_,PolicyFileName);
	   } 
      }
      in.close();
   }
   if (debug_>0) waitForUser();
}

void FCModule::allocateData()
{
  data = new uint8_t*[blockSize_];
   
  // Initialize data structure
  for(int i=0; i<blockSize_; i++) {
    data[i] = new uint8_t[symSize_]; 
    for(int j=0; j<symSize_; j++) {
       data[i][j] = i;
       //if(debug_>0) printf("FCModule(%d)::allocateData() data[%d][%d] = %d; \n",idcurrfc_,i,j,data[i][j]); 
    }
  }
  //if (debug_ > 0) waitForUser();
}

void FCModule::deleteData()
{
  if (data == 0) delete data;
  else {
     for (int i=0; i<blockSize_; i++) delete[] data[i] ;
     delete[] data;
  }
}

void FCModule::encodeData()
{
  encodedPkts = new uint8_t*[maxPktsToSend];

  for (int i=0; i< maxPktsToSend; i++) {
    encodedPkts[i] = new uint8_t[symSize_];
    //if(debug_>0) printf("FCModule(%d)::encodeData() created encodedPkts[%d])\n",idcurrfc_,i);
  }
  //if (debug_ > 0) waitForUser();
  
}

void FCModule::deleteEncodedData()
{
  if (encodedPkts == 0) delete encodedPkts;
  else {
     for (int i=0; i<blockSize_; i++) delete[] encodedPkts[i] ;
     delete[] encodedPkts;
  }
}

void FCModule::loadEncodedPkt() 
{
   for (int j = 0; j < blockSize_; j++) {
       coder->setToCoder(data[j]);
       //if(debug_>0) printf("FCModule(%d)::loadEncodedPkt() loaded data[%d]\n",idcurrfc_,j);
   }
   //if (debug_ > 0) waitForUser();
}

void FCModule::computeClass() 
{
  class_dist = (d_upper-d_lower)/(n_class-2);
  tot_nds_classes.clear();
  Position* p_sender = getPosition();
  double dist;
  double max_dist = 0.0;

  for (int i = 0; i < fcm_pointers.size() ; i++) {
       Position* p_receiver = fcm_pointers.at(i)->getPosition();
       dist = p_sender->getDist(p_receiver);
       if (dist > max_dist) max_dist = dist;
       if (debug_ > 0) printf("FCModule(%d)::computeClass() user(%d) has distance: %f \n",idcurrfc_,i,dist);
       if (n_class == 1) {
	   class_id.push_back(0);
           if (complete_peers.count(i) == 0) tot_nds_classes[0]++;
       }
       else if (n_class == 2) {
          if (dist > d_max) {
	     printf("\e[1;31mFCModule(%d)::computeClass() ERROR!! user(%d) has distance %f > d_max %f! \e[0m\n",idcurrfc_,i,dist,d_max);
             exit(1);
          }
          else if (dist <= d_lower) {
                  class_id.push_back(0); // caso d< d_lower
                  if (complete_peers.count(i) == 0) tot_nds_classes[0]++;
          }
          else {
             class_id.push_back(1);
             if (complete_peers.count(i) == 0) tot_nds_classes[1]++;
          }
       }
       else{ 
         if (dist <= d_lower) {
             class_id.push_back(0); // caso d< d_lower
             if (complete_peers.count(i) == 0) tot_nds_classes[0]++;
             /*cout << " class = " << 0 << " tot_nds_classes[" << 0 << "] = " 
                  << tot_nds_classes[0] << endl;*/
         }
         else if ((dist > d_lower) && (dist <= d_upper)) { // caso d_lower < d < d_upper
      	    //class_dist = (d_upper-d_lower)/(n_class-2);
            int class_user = int(ceil((dist-d_lower)/class_dist));
            class_id.push_back(class_user);
            if (complete_peers.count(i) == 0) tot_nds_classes[class_user]++;
           /* cout << "class = " << class_user << " tot_nds_classes[" << class_user << "] = " 
                 << tot_nds_classes[class_user] << endl;*/
         }
         else if (( dist > d_upper) && (dist <= d_max)) { // caso d_upper <= d <= d_max
	    class_id.push_back(n_class-1);
            if (complete_peers.count(i) == 0) tot_nds_classes[n_class-1]++;
            /*cout << "class = " << n_class-1 << " tot_nds_classes[" << n_class-1 << "] = " 
                 << tot_nds_classes[n_class-1] << endl;*/
              
         }
         else { // caso d > d_max
	     printf("\e[1;31mFCModule(%d)::computeClass() ERROR!! user(%d) has distance %f > d_max %f! \e[0m\n",idcurrfc_,i,dist,d_max);
             exit(1);
	 }       
      }
  if (debug_ > 0) printf("FCModule(%d)::computeclass() user(%d) has class: %d\n",idcurrfc_,i,class_id.at(i));
  }
  max_RTT = 2*(max_dist/propSpeed_);
  if (debug_ > 0) printf("FCModule(%d)::computeclass() max_RTT is: %f\n",idcurrfc_,max_RTT);
  if (debug_ > 0) printf("FCModule(%d)::computeclass() tot_nds_classes.size() is: %d\n",idcurrfc_,tot_nds_classes.size());
  //if (debug_ > 0) waitForUser();
}

inline void FCModule::getTxPolicy(int class_input, int curr_stats, int max_res_rank)
{

  //cout << "curr_round = " << curr_round << " crb_phase = " << crb_phase << endl;

  if( (curr_round == 1)  && (crb_phase != CRB_INACTIVE) && (crb_phase != CRB_PHASE_II_SGT) ) {
     if ( crb_phase == CRB_PHASE_I ) {
         int crb_size = crb_phase_I_index.size();
         if (debug_ >0 ) {
            cout << "FCModule(" << idcurrfc_ << ")::getTxPolicy() CRB_PHASE_I index size is " << crb_size << endl;
            //waitForUser();
         }
         if (crb_size > 0) {
            set<int>::reverse_iterator rit = crb_phase_I_index.rend();
            curr_policy = (*rit);
            if (debug_ > 0 ) {
                 cout << "FCModule(" << idcurrfc_ << ")::getTxPolicy() CRB_PHASE_I index size is " << crb_size 
                      << " the current policy is " << curr_policy << endl;
                      waitForUser();
            }
            return; 
         }
         else {
            curr_policy = blockSize_ + XI_;
         }
    }
    else if ( crb_phase == CRB_PHASE_I_SGT ) {
            curr_policy = blockSize_ + XI_;
            return;
    }
    else if ( (crb_phase == CRB_PHASE_II) ) {
        if ((curr_stats == NOT_SET) || (curr_stats == 0) ) {
            curr_policy = blockSize_ + XI_;
            return;
        }
        else {
           int max_policy = blockSize_ + XI_;
           if (curr_stats > max_policy) {
              //int normalized = curr_stats % max_policy;
              int normalized = blockSize_ - max_res_rank;
              int diff = abs(max_policy - normalized);
              if ( (diff < max_policy ) && (diff >= 1)) curr_policy = diff;
              else curr_policy = 1;
           }
           else {
              int diff = abs(max_policy - curr_stats);
              if ( (diff < max_policy ) && (diff >= 1)) curr_policy = diff;
              else curr_policy = 1;
           }
        }
     }
     else {
        curr_policy = 0;
        int id;
        int class_no;
        for (Setiter i = fcm_peers.begin(); i != fcm_peers.end() ; i++) {
            id = (*i); 
            class_no = class_id.at(id); 
            int nodes = tot_nds_classes[class_no];
            Mu_entry entry;
            entry = make_pair(class_no,nodes);
            Mu current_mu = mu_map.at(entry);
            if ( current_mu[0][1][0][0] > curr_policy) curr_policy = current_mu[0][1][0][0];
        }
     } 
  }
  else {
     if ((curr_stats == 0) || (curr_stats == NOT_SET)) {
        curr_policy = 0;
        int id;
        int class_no;
        for (Setiter i = fcm_peers.begin(); i != fcm_peers.end() ; i++) {
            id = (*i); 
            class_no = class_id.at(id); 
            int nodes = tot_nds_classes[class_no];
            Mu_entry entry;
            entry = make_pair(class_no,nodes);
            Mu current_mu = mu_map.at(entry);
            if ( current_mu[0][1][0][0] > curr_policy) curr_policy = current_mu[0][1][0][0];
        }
     /*cout <<"sender = " <<idcurrfc_ << " curr_stats = " << curr_stats << " curr_round = "
          << curr_round << " curr_policy = "  << curr_policy << endl;*/
     return;
     }

     if (curr_stats == NOT_SET) curr_stats = 0;
 
     //int curr_pkts;
     if (curr_round <= 1) {
         if ( crb_phase_I_index.size() > blockSize_ - 1) {
            pkts_tx = crb_phase_I_index.size();
            curr_policy = crb_phase_I_index.size();
         }
         else {
            int id;
            int class_no;
           for (Setiter i = fcm_peers.begin(); i != fcm_peers.end() ; i++) {
              id = (*i); 
              class_no = class_id.at(id); 
              int nodes = tot_nds_classes[class_no];
              Mu_entry entry;
              entry = make_pair(class_no,nodes);
              Mu current_mu = mu_map.at(entry);
              if ( current_mu[0][1][0][0] > curr_policy) curr_policy = current_mu[0][1][0][0];
           }
           pkts_tx = curr_policy;
         }
     }
     //else curr_pkts = pkts_tx;

     //cout << " pkts_tx = " << pkts_tx << "   " ;

     int max_policy = 0;
     int round; 
     if (curr_round < max_rounds_) round = curr_round + 1;
     else round = curr_round;

     if (curr_stats > curr_policy) {
         /*cout << "sender = " << idcurrfc_ << " curr_round = " << curr_round 
              << " curr_stats = " << curr_stats << " > curr_policy = " << curr_policy << endl;*/
         curr_stats = (blockSize_ - max_res_rank);
         //cout << " stats_normalized = " << curr_stats << endl;
         //round = curr_round;
     }


     for (Setiter i = fcm_peers.begin(); i != fcm_peers.end() ; i++) {
         int id = (*i); 
         int class_no = class_id.at(id); 
         int nodes = tot_nds_classes[class_no];
         Mu_entry entry;
         entry = make_pair(class_no,nodes);
         Mu current_mu = mu_map.at(entry);
         int policy = curr_policy + current_mu[0][round][pkts_tx][curr_stats];
         if (policy > max_policy) max_policy = policy;

         if (debug_ > 0) cout << "FCModule("<< idcurrfc_ <<")::getTxPolicy() class = " << class_no 
                              << " policy = " << policy << " max policy = " << max_policy << endl;
     }
     
     curr_policy = max_policy + 1;
     //cout << "curr_policy = " << curr_policy << " curr_round = " << curr_round << endl;
     
  }
  if(debug_>0) {
     printf("FCModule(%d)::getTxPolicy(): the new policy is %d \n",idcurrfc_,curr_policy);
     //waitForUser();
  }
}

void FCModule::incrDataPktReceived()
{
   curr_ack_stat++;
   if (debug_ > 0) printf("\e[35m FCModule(%d)::incrDataPktReceived(): added a DATA pkt. current DATA total is %d\e[0m\n",idcurrfc_,curr_ack_stat);
}

void FCModule::resetTxStats()
{
   pkts_tx = 0;
   curr_ack_stat = 0;
   curr_rank_stat = 0;
   if ( (crb_phase == CRB_PHASE_II) || (crb_phase == CRB_PHASE_II_SGT) ) curr_round = 0;
   else curr_round = 1;
   curr_policy = 0;

   acks_stats.clear();
   rank_stats.clear();
}

inline void FCModule::setInitialReceivers(const set<int>* receivers)
{
  //initialize();
  //old_ack_stat = curr_ack_stat;
  //old_rank_stat = curr_rank_stat;

  if (crb_phase != CRB_INACTIVE) maxPktsToSend = (blockSize_ + XI_) * 3 + ximax;

  setToCoder();
  resetStats();
  resetTxStats();
  curr_block = 0;
  fcm_peers.clear();
  newblock_peers.clear();
  stats_peers.clear();
  finished = false;

  
  for (Setiter it = receivers->begin(); it != receivers->end() ; it++ ) {
      int id =  (*it);
      
      if ( (id != idcurrfc_) && ( complete_peers.count(id) == 0 ) ) { //  OCCHIOOO
         fcm_peers.insert(id);
         if ((debug_ > 0) || ((crbcast_ctrl->debug_)>0)) cout << "FCModule("<<idcurrfc_<<")::setInitialReceivers()" 
                                                              << " sender " << idcurrfc_ 
                                                              << " has " << id << " as a receiver " << endl;
         //fcm_pointers[id]->initialize();
         fcm_pointers[id]->finished=false;
         if (crb_phase != CRB_INACTIVE) fcm_pointers[id]->maxPktsToSend = (blockSize_ + XI_) * 3 + ximax;
         //cout << "maxPktsToSend of user " << id << " is " << fcm_pointers[id]->maxPktsToSend << endl;
         if ( fcm_pointers[id]->isdecoder == false) {
               fcm_pointers[id]->setToDecoder();
               fcm_pointers[id]->curr_ack_stat = 0;
         }
         else {
            int res_rank = (fcm_pointers[id]->coder)->decode();
            //cout << "node id " << id <<", res rank " << res_rank << endl;
            //waitForUser();
         }
         int res_rank = (fcm_pointers[id]->coder)->decode();
         //cout << "node id " << id <<", res rank " << (fcm_pointers[id]->coder)->decode() << endl;
         fcm_pointers[id]->curr_block = 0;
         //if ((crb_phase != CRB_PHASE_II) && (crb_phase != CRB_PHASE_II_SGT)) fcm_pointers[id]->curr_ack_stat = 0;
         fcm_pointers[id]->fcm_peers.clear();
         fcm_pointers[id]->fcm_peers.insert(idcurrfc_);
      }
      else if (id != idcurrfc_) {
         fcm_peers.erase(id);
         /*if ((crbcast_ctrl->debug_) > 0) cout << "FCModule("<<idcurrfc_<<")::setInitialReceivers()"<< " sender " 
                                              << idcurrfc_ << " has " << id << " as a complete receiver. " 
                                              << "Erasing it from sender's fcm_peers." << endl;*/
      }
  }
  if ( fcm_peers.empty() ) {
     /*if ((debug_ > 0) || ((crbcast_ctrl->debug_)>0)) cout << "FCModule("<<idcurrfc_<<")::setInitialReceivers() "
                                                         << "has no incomplete child nodes. "
                                                         << "No FC session needed." << endl;*/
     if (multihop_ctrl) multihop_ctrl->setDone(idcurrfc_);
     else if (crbcast_ctrl) {
        //cout << "errore non dovevo trasmettere " << " nella fase " << crbcast_ctrl->curr_phase << endl;
        if ( (crb_phase == CRB_PHASE_I) || (crb_phase == CRB_PHASE_I_SGT) ) {
            crbcast_ctrl->phase_I_wasted_tx++; 
            crbcast_ctrl->phase_I_rounds--;
            crbcast_ctrl->global_rounds--;
            //crbcast_ctrl->setDone(idcurrfc_);
        }
        else if ((crb_phase == CRB_PHASE_II) || (crb_phase == CRB_PHASE_II_SGT)) crbcast_ctrl->phase_II_wasted_adv++;
        crbcast_ctrl->setDone(idcurrfc_);
     }
  }
  else startMasterEncoder();
  //if ( (debug_ >0) ) waitForUser();
}

void FCModule::doNewBlock() 
{
  done = false;
  newblock_done = true;
  curr_block++;
  if ((crb_phase != CRB_PHASE_I) || ((crb_phase == CRB_PHASE_I) && (crb_phase_I_seed == NOT_SET)) ) 
     coder->startNewBlock(seed_++); // era seed_++
  else if ( (crb_phase == CRB_PHASE_I) && ( crb_phase_I_seed != NOT_SET) ) 
      coder->startNewBlock(crb_phase_I_seed);  
  if (isdecoder == false) {
     loadEncodedPkt();
  }
  resetTxStats(); // se sn encoder resetto i miei stats
  if (debug_>0) {
     printf("\e[32m FCModule(%d)::doNewBlock() started a new block %d of total %d with seed: %d \e[0m\n",idcurrfc_,curr_block,totBlock_,seed_);
     //waitForUser();
  }
}

inline void FCModule::startDiscovery()
{
  FCMTmr_.setAction(ACTION_SYNC);
  FCMTmr_.resched(period_);   
}

void FCModule::startMasterEncoder()
{
  if (isdecoder == false) {
    if ( period_<0) {
      fprintf(stderr,"\e[32mFCModule(%d)::startMasterEncoder(): ERROR!!! period_ <= 0\e[0m\n",idcurrfc_);
      exit(1);
    } 
    else if ( ( ctrl_module_id == 0 ) || (send_module_id == 0) ) { // DA MIGLIORARE
      fprintf(stderr,"\e[32mFCModule(%d)::startMasterEncoder(): ERROR!!! forward-module(s) not connected \e[0m\n",idcurrfc_);
      exit(1);
    }
    if (debug_ > 0) {
      printf("FCModule(%d)::startMasterEncoder() ximax = %d, maxPktsToSend = %d \n",idcurrfc_,ximax,maxPktsToSend);
      //waitForUser();
    }
    start_time = time(NULL);
    is_master = true; // sn il master sender
    done = true; // il master ha sempre finito
    finished = false; 

    //if (crb_phase != CRB_PHASE_I) complete_peers.insert(idcurrfc_);
    if ((crb_phase == CRB_PHASE_I) || (crb_phase == CRB_PHASE_I_SGT)) max_rounds_ = 1;
    crb_index_iter = crb_phase_I_index.begin();
    
    computeClass();
  
    if (crb_phase == CRB_INACTIVE) getOptPolicy();
    
    //setDist();
    allocateData();  // we allocate the data
    encodeData();
    if ( (crb_phase == CRB_PHASE_II) || crb_phase == CRB_PHASE_II_SGT ) startCRBPhaseTwo();
    else startDiscovery();

    }
  else {
    fprintf(stderr,"\e[32mFCModule(%d)::startMasterEncoder(): ERROR!!! Module has to be set to ENCODER\e[0m\n",idcurrfc_);
    exit(1);	  
  }
}

void FCModule::startCRBPhaseTwo()
{
    if (debug_ > 0) {
      printf("FCModule(%d)::startCRBPhaseTwo() called\n",idcurrfc_);
      //waitForUser();
    }
    
    if (crb_phase == CRB_PHASE_II_SGT) {
       max_rounds_ = 5;
       computeClass();
       getOptPolicy();
    }
    FCMTmr_.setAction(ACTION_NEWBLOCK);
    FCMTmr_.resched(period_);
    
}


inline void FCModule::stop()
{
  if (debug_>0) {

     int sorgente = 0;
     if (crbcast_ctrl) sorgente = crbcast_ctrl->fcm_source_id;
     else if (multihop_ctrl) sorgente = multihop_ctrl->fcm_source_id;

     cout << setw(9) << "sorgente " << setw(12) << " seed = " << setw(5) << fcm_pointers[0]->coder->returnSeed() 
          << setw(8) << " pkt_id = " ;
     for (Setiter it = source_phase_I_index.begin(); it != source_phase_I_index.end(); it++) {
          int index = *it;
          cout << setw(3) << index ;
     }
     cout << endl;

     int space = 1;
     cout << setw(9) << "sender " << setw(3) << idcurrfc_ << setw(9) << " seed = " << setw(5)
          << crb_phase_I_seed << setw(8) << " pkt_id = " ;
     for (Setiter it = (fcm_pointers[idcurrfc_]->crb_phase_I_index).begin(); it != (fcm_pointers[idcurrfc_]->crb_phase_I_index).end(); it++) {
          int index = *it;
          if (space == index) cout << setw(3) << index ;
          int diff = abs(index - space);
          if (diff > 0) {
             space += (diff + 1);
             for (int i = 0; i <= diff ; i++) {
                  cout << setw(3) << " " ;
             }
          }
          else space++;
     }
     cout << endl;
  }

  stop_time = time(NULL);
  if (debug_ > 0) printf("FCModule(%d)::stop() all block completed!!!\n",idcurrfc_);
  int receiver;
  for (Setiter it = fcm_peers.begin(); it != fcm_peers.end(); it++) 
  {
      receiver = (*it);
      int curr_rank = fcm_pointers[receiver]->curr_rank_stat;

      /*if (debug_ > 0) {
         cerr << "FCModule(%d)::stop() node " << receiver << " has curr_rank_stat " 
              << curr_rank << endl;
      }*/
      if (debug_>0) {
         int space = 1;
         cout << setw(9) << "receiver " << setw(3) << receiver << setw(9) << " seed = " << setw(5)
              << crb_phase_I_seed << setw(8) << " pkt_id = " ;
         for (Setiter it = (fcm_pointers[receiver]->crb_phase_I_index).begin(); it != (fcm_pointers[receiver]->crb_phase_I_index).end(); it++) {
              int index = *it;
              if (space == index) cout << setw(3) << index ;
              int diff = abs(index - space);
              if (diff > 0) {
                 space += (diff + 1);
                 for (int i = 0; i <= diff ; i++) {
                      cout << setw(3) << " " ;
                 }
              }
              else space++;
         }
         cout << endl;
      }
      
      if ( (receiver != idcurrfc_) && (curr_rank == 0) ) {
         fcm_pointers[receiver]->finished = true; 
	 complete_peers.insert(receiver);
         incomplete_peers.erase(receiver);
         if (crb_phase != CRB_INACTIVE) fcm_lcomp.push_back(receiver);
      }
      else if ( receiver != idcurrfc_ ) {
         fcm_pointers[receiver]->finished = false;
         complete_peers.erase(receiver);
         incomplete_peers.insert(receiver);
      }

      /*if (debug_ > 0) {
         cerr << "FCModule(%d)::stop() node " << receiver << " has curr_rank_stat " 
              << curr_rank << " and bool finished " << (bool)finished << endl;
      }*/

  }
  if ( (crb_phase == CRB_PHASE_I) || (crb_phase == CRB_PHASE_I_SGT) ) newblock_done = false;
  finished = true;
  is_master = false;

  printStats();

  // destroy data
  if ( mhop == true) {
     /*map<int, Mu >::iterator iter;
     for (iter = mu_map.begin(); iter != mu_map.end(); iter++) {
         Mu curr_mu = (*iter).second;
         Mu_entry entry = (*iter).first;
         deleteMu(curr_mu, entry);
     }
     mu_map.clear();*/
     deleteData();
     deleteEncodedData();
  }
  tot_nds_classes.clear();
  //curr_ack_stat = old_ack_stat;
  //curr_rank_stat = old_rank_stat;

  if (multihop_ctrl != 0) multihop_ctrl->setDone(idcurrfc_);
  else if (crbcast_ctrl != 0) crbcast_ctrl->setDone(idcurrfc_);

}

inline void FCModule::finish()
{
  if ( (is_master == true) || (curr_rank_stat == 0) ) {
      finished = true;

  }
}

void FCModule::discovery() // da migliorare, se sono su canale nn perfetto nn funziona
{
  static int try_counter = 0;
  if (debug_ >0) printf("FCModule(%d)::discovery() starting discovery\n",idcurrfc_,try_counter +1);
  if (fcm_peers.size() == 0) {
     if (try_counter < 5) { // provo 5 volte se non basta
        sendPkt(SYNC);
        sendPkt(SYNC,period_);
        sendPkt(SYNC,2*period_);
        FCMTmr_.setAction(ACTION_SYNC);
        FCMTmr_.resched(max_RTT+0.5);
        try_counter++;
     }
    else {
      printf("aborted\n");
      printf("\e[32mFCModule(%d)::discovery() no SYNC_ACK receiver for %d rounds!!!\e[0m\n",idcurrfc_,try_counter +1);
      exit(1);
    }
  }
  else { // ho visto gente e comincio a tx. cosa succede se il canale di ritorno nn è ideale?
     FCMTmr_.setAction(ACTION_NEWBLOCK);
     FCMTmr_.resched(period_);
  }
}

void FCModule::sendNewBlock()
{
  if (( isdecoder == false) && (is_master == true) ) { // controllo ridondante ma voglio essere sicuro
     if (curr_block < totBlock_) { // se nn ho ancora trasmesso tutti i blocchi
        if (debug_ > 0) printf("FCModule(%d)::sendNewBlock() trying to start NewBlock\n",idcurrfc_);
        sendPkt(NEW_BLOCK);
        sendPkt(NEW_BLOCK,period_);
        FCMTmr_.setAction(ACTION_WAIT_NEWBLOCK); 
        FCMTmr_.resched(max((GetRTT() + period_),(max_RTT + period_))); 
     }
    else { //altrimenti mi fermo
        stop();
    }
  }
}

void FCModule::controlNewBlock() 
{
  if ( newblock_peers.size() < fcm_peers.size() ) { // ho perso qualche pacchetto newblockack
     if (debug_ > 0) printf("FCModule(%d)::controlNewBlock() controlling NewBlockACK\n",idcurrfc_);
     Setiter nbp_iter = newblock_peers.begin();
     int receiver;
     int timer = 0;
     for (Setiter it = fcm_peers.begin(); it != fcm_peers.end(); it++) {
 
         if (nbp_iter == newblock_peers.end()) break;
            
         if ( (*it) != (*nbp_iter) ) { // trovo il primo
            receiver = (*it);
            sendPkt(RESEND_NEWBLOCKACK,timer*period_,receiver);
            timer++;
            sendPkt(RESEND_NEWBLOCKACK,timer*period_,receiver);         
            timer++;
         }         
      }
   if (debug_ > 0) printf("FCModule(%d)::controlNewBlock() module doesn't have all NBACK\n",idcurrfc_);
   FCMTmr_.setAction(ACTION_WAIT_NEWBLOCK);
   FCMTmr_.resched(max((GetRTT() + period_),(max_RTT + period_)));
  }
  else {
     if (debug_ > 0) printf("FCModule(%d)::controlNewBlock() control ok, starting block no %d\n",idcurrfc_,curr_block +1);
     doNewBlock();
     if ((crb_phase == CRB_PHASE_II) || (crb_phase == CRB_PHASE_II_SGT) ) curr_policy = 0;
     else getTxPolicy();
     FCMTmr_.setAction(ACTION_DATA);
     FCMTmr_.resched(period_);
  }
}

void FCModule::transmit()
{
  if(pkts_tx < curr_policy) { // devo tx finchè nn raggiungo la policy
     if (debug_ > 0) printf("FCModule(%d)::transmit() pkt no %d su curr policy %d\n",idcurrfc_,pkts_tx,curr_policy);
     sendPkt(DATA);
     FCMTmr_.resched(period_);
     return;
  } 
  if (pkts_tx >= curr_policy) { // nn devo piu tx
     if (debug_ > 0) {
       printf("FCModule(%d)::transmit() pkts_tx %d = curr policy %d ==> change policy\n",idcurrfc_,pkts_tx,curr_policy);
       //waitForUser();
     }
     //FCMTmr_.cancel(); //fermo timer
     resetAckStats();
     resetRankStats();
     sendPkt(DECODE);
     sendPkt(DECODE,period_);
     FCMTmr_.setAction(ACTION_WAIT_STATS); //setto timer per decoder
     FCMTmr_.resched(max((GetRTT() + period_),(max_RTT + period_))); //schedulo evento decoder
  } 
}

void FCModule::initPkt(Packet* p , int command, int receiver)
{
 
  hdr_cmn* ch = hdr_cmn::access(p);
  ch->uid() = uidcnt_++;
  ch->ptype() = PT_FCM;
  ch->size() = pktSize_;
  
  hdr_ip* iph = hdr_ip::access(p);
  iph->daddr() = dstAddr_;
  iph->dport() = dstPort_;

  hdr_fc* fch = HDR_FC(p);

  switch (command) {

    case (DECODE): { 
       fch->command = DECODE;
       fch->pkt_pointer = 0; 
       fch->rxs_id = &fcm_peers;          
       fch->block_id = curr_block; // curr block
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break; 

    case (DATA): {
       fch->command = DATA;
       fch->rxs_id = &fcm_peers;                   
       fch->block_id = curr_block; // curr block
       if ( (crb_phase == CRB_PHASE_I) && ( crb_index_iter != crb_phase_I_index.end() ) ) {
          fch->pkt_pointer = encodedPkts[pkts_tx];
          fch->seed = crb_phase_I_seed;
          fch->pktid = pkts_tx+1;
          if ( crb_index_iter == crb_phase_I_index.end() ) pkts_tx = curr_policy;
       }
       //else if 
       else {
          //fch->seed = seed_;
          fch->seed = coder->returnSeed();
          fch->pkt_pointer = encodedPkts[pkts_tx]; 
          fch->pktid = pkts_tx+1; // pkt id of curr block

          if ((idcurrfc_ == 0) && (crb_phase==CRB_PHASE_I)) source_phase_I_index.insert(pkts_tx+1);

          pkts_tx++; // aumento il contatore dei pacchetti spediti per il blocco corrente
       }
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break;
  
    case (RESEND_STATS): {
       fch->command = RESEND_STATS;
       fch->pkt_pointer = 0;    
       fch->block_id = curr_block; // curr block
       fch->rx_id = receiver;
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = curr_ack_stat; // no of acked pkts
       fch->res_rank = curr_rank_stat; //residual rank
    }
    break;

    case (RESEND_NEWBLOCKACK): {
       fch->command = RESEND_STATS;
       fch->pkt_pointer = 0;    
       fch->block_id = curr_block; // curr block
       fch->rx_id = receiver;
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break;
  
    case (SYNC): {
       fch->command = SYNC;
       fch->pkt_pointer = 0; 
       fch->rxs_id = &fcm_peers;             
       fch->block_id = curr_block; // curr block
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break;

    case (SYNC_ACK): {
       fch->command = SYNC_ACK;
       fch->rx_id = receiver;   
       fch->pkt_pointer = 0;           
       fch->block_id = curr_block; // curr block
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break;

    case (SYNC_COOP): {
       fch->command = SYNC;
       fch->pkt_pointer = 0; 
       fch->rx_id = ALL;             
       fch->block_id = curr_block; // curr block
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break;

    case (SYNC_COOP_ACK): {
       fch->command = SYNC_ACK;
       fch->rx_id = receiver;   
       fch->pkt_pointer = 0;           
       fch->block_id = curr_block; // curr block
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break;

    case (STATS): {
       fch->command = STATS;
       fch->pkt_pointer = 0;    
       fch->rxs_id = &fcm_peers;          
       fch->block_id = curr_block; // curr block
       fch->pktid = CTRL; // pkt id of curr block
       fch->res_rank = curr_rank_stat;	
       if ((crb_phase == CRB_PHASE_II_SGT) || (crb_phase == CRB_PHASE_II) ) {
         if ( curr_rank_stat < blockSize_) {
           if (crb_phase_I_ack_done == true) fch->total_ack = curr_ack_stat;
           else {
                 curr_ack_stat = crb_phase_I_index.size();
                 fch->total_ack = crb_phase_I_index.size();
                 crb_phase_I_ack_done = true;
           }
         }
         else {
            fch->total_ack = curr_ack_stat;
            crb_phase_I_ack_done = true;
         }
       }
       else fch->total_ack = curr_ack_stat; // no of acked pkts
    }
    break;
    
    case (NEW_BLOCK): {
       fch->command = NEW_BLOCK;
       fch->pkt_pointer = 0;    
       fch->rxs_id = &fcm_peers;          
       fch->block_id = curr_block; // curr block 
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break;

    case (NEW_BLOCK_ACK): {
       fch->command = NEW_BLOCK_ACK;
       fch->pkt_pointer = 0;        
       fch->rx_id = receiver;      
       fch->block_id = curr_block; // curr block + 1
       fch->pktid = CTRL; // pkt id of curr block	
       fch->total_ack = CTRL; // no of acked pkts
       fch->res_rank = CTRL; //residual rank
    }
    break;

  }
  /*comuni a tutti*/
  fch->sn = txsn;  // initialize serial number
  fch->sender_id = idcurrfc_; // initialize sender id
  fch->ts = Scheduler::instance().clock();
  ch->timestamp() = fch->ts;
  
  txsn++; // aumento il serial number
  
  if (debug_ > 0 ) {
    printf("FCModule(%d)::initPkt() sn=%d, pktid=%d, blk_id=%d, TS=%f , seed=%d \n",idcurrfc_,fch->sn,fch->pktid,fch->block_id,fch->ts,fch->seed);  
    //waitForUser();
  }
  if (rftt >= 0)
    {
      fch->rftt = rftt;
      fch->rftt_valid = true;
    }
  else
    {
      fch->rftt_valid = false;
    }

}

void FCModule::sendPkt(int command, double delay,int receiver)
{
  int down_module;
  Packet* p = Packet::alloc();

  switch (command) {

//      case(DATA): {
//         coder->getCodedPck( encodedPkts[pkts_tx], (pkts_tx+1));
//         down_module = send_module_id;
//         total_data_pkt++; //aumento il contatore dei tx pkts totali
//      }
//      break;

     case(DATA): {
       if ( (crb_phase == CRB_PHASE_I) && ( !crb_phase_I_index.empty() ) ) {
          //cout << " SI CI CAPITO " << endl;
          int id = (*crb_index_iter);
          if (crb_index_iter != crb_phase_I_index.end() ) crb_index_iter++;
          id--;
          pkts_tx = id;
          coder->getCodedPck( encodedPkts[id], (id+1));
          down_module = send_module_id;
          total_data_pkt++;
        }
        else {
          coder->getCodedPck( encodedPkts[pkts_tx], (pkts_tx+1));
          down_module = send_module_id;
          total_data_pkt++; //aumento il contatore dei tx pkts totali
        }
     }
     break;

     case(SYNC): {
        down_module = ctrl_module_id;
        total_ctrl_pkt++;
     }
     break;

     case(SYNC_ACK): {
        down_module = ctrl_module_id;
        total_ctrl_pkt++;
     }
     break;

     case(SYNC_COOP):{
        down_module = ctrl_module_id;
        total_ctrl_pkt++;
     }
     break;

     case(SYNC_COOP_ACK):{
        down_module = ctrl_module_id;
        total_ctrl_pkt++;
     }
     break;
    
     case(STATS):{
        down_module = ctrl_module_id;
        total_ctrl_pkt++;
     }
     break;

     case(RESEND_STATS):{
        down_module = ctrl_module_id;
        total_ctrl_pkt++;
     }
     break;

     case(RESEND_NEWBLOCKACK):{
         down_module = ctrl_module_id;
         total_ctrl_pkt++;
     }
     break;

     case(NEW_BLOCK):{
         down_module = ctrl_module_id;
         total_ctrl_pkt++;  
     }
     break;

     case(NEW_BLOCK_ACK):{
          down_module = ctrl_module_id;
         total_ctrl_pkt++;
     }
     break;

     case(DECODE):{
         down_module = ctrl_module_id;
         total_ctrl_pkt++;
     }
     break;
  }

  initPkt(p, command,receiver);
  hdr_cmn* ch = hdr_cmn::access(p);
  hdr_fc* fch = HDR_FC(p);

  if(debug_>0) 
     printf("\e[36m FCModule(%d)::sendPkt(), send a pkt (%d) command = %s block no = %d \e[0m\n",idcurrfc_,ch->uid(),printCommand(command), fch->block_id );
  sendDown(down_module,p,delay);

}

void FCModule::changePolicy() 
{   
   if ( stats_peers.size() < fcm_peers.size() ) { // ho perso qualche pacchetto stats

     //Setiter stats_iter = stats_peers.begin();
     //int receiver;
     int timer = 0;
     for (Setiter it = fcm_peers.begin(); it != fcm_peers.end(); it++) {

         //if ( stats_iter == stats_peers.end() ) break; 
         int index = *it;

         if ( stats_peers.count(index) == 0 ) { // trovo il primo
            sendPkt(RESEND_STATS,timer*period_,index);
            timer++;
            sendPkt(RESEND_STATS,timer*period_,index);         
            timer++;
         }         
      }
   FCMTmr_.setAction(ACTION_WAIT_STATS);
   FCMTmr_.resched(max((GetRTT() + period_),(max_RTT + period_)));
  }
  else { // ho tutti gli stats 
      if(curr_round <= max_rounds_) { // se non ho raggiunto il numero massimo di block tries
         Stats rank_stat = getMaxRank();
         Stats ack_stat = getMinAck();
         if(debug_ > 0) {
         printf("FCModule(%d)::changePolicy():max rank is %d, min ack is %d\n",idcurrfc_,rank_stat.second,ack_stat.second);
         //waitForUser();
         }
         if ( rank_stat.second == 0) { // se il rango massimo è zero ==> new block
             if (debug_ > 0) printf("FCModule(%d)::changePolicy() all peers have resolved, starting new block\n",idcurrfc_);
             total_good_rounds = total_good_rounds + curr_round;
             FCMTmr_.setAction(ACTION_NEWBLOCK);
             FCMTmr_.resched(period_);
             return;
         }
         else { // qualcuno non ha ancora risolto
              int class_no = ack_stat.first;
              int ack_no = ack_stat.second;
              curr_round++;
              if (curr_round <= max_rounds_) {
                 if (debug_ >0) {
                     printf("\e[31mFCModule(%d)::changePolicy() min ack = %d of class_id = %d, a peer hasn't resolved, starting round %d \e[0m  \n",idcurrfc_,ack_no,class_no,curr_round);
                     //waitForUser();
                 }
                 int max_rank = rank_stat.second;
                 getTxPolicy(class_no, ack_no, max_rank );                
                 //waitForUser();
                 if ((crbcast_ctrl) && (curr_round > 1)) crbcast_ctrl->newRound();
                 FCMTmr_.setAction(ACTION_DATA);
                 FCMTmr_.resched(period_);
                 return;
              }
              else {
                    FCMTmr_.setAction(ACTION_NEWBLOCK);
                    FCMTmr_.resched(period_);
                    block_failed++;
                   if (debug_ > 0 )  printf("\e[31mFCModule(%d)::changePolicy() a peer hasn't resolved yet. Aborting current block!\e[0m\n",idcurrfc_);
              }
         }
      }
   }
}

void FCModule::recv(Packet* p, Handler* h)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  recv(p);
}

void FCModule::recv(Packet* p)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  //if (debug_>0) printf("FCModule(%d)::recv(Packet*p,Handler*) pktId %d\n",idcurrfc_, ch->uid());
  hdr_fc* fch = HDR_FC(p);
    
  if (ch->ptype() != PT_FCM)
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
  
  uint16_t sender = fch->sender_id;

  if (finished == true) 
  {
      drop(p,0,FC_DROP_REASON_ALREADY_FINISHED);
      Packet::free(p);
      return; // se nodo ha finito nn riceve piu pacchetti
  }

  if (fch->block_id != curr_block) 
  {
     drop(p,1,FC_DROP_REASON_WRONG_BLOCK_NUMBER); // per nn far scrivere nulla nel trace
     Packet::free(p);
     if (fcm_peers.count(sender) == 0) incrPktInvalid();
     else incrPktWrblock();
     return;
  }
   
  if (debug_>0) printf("\e[34m FCModule(%d)::recv() a pkt cmd %s sn %d sender %d blkno %d \e[0m \n",idcurrfc_,printCommand(fch->command),fch->sn,fch->sender_id,fch->block_id);

  switch (fch->command) {
  
  case (DATA) : {
    static vector<hdr_fc> buffer(0);
    int esn = hrsn + 1; // expected sn
    hrsn = fch->pktid;
    if (hrsn > esn)
    { // packet losses are observed
         incrPktLost(hrsn - esn );
    }
    if ((isdecoder == true) && (done == false)){ // sn decoder e devo ancora risolvere
       uint16_t block = fch->block_id; 
       uint16_t sender = fch->sender_id; // we retrieve the sender id
       uint16_t index = (fch->pktid);  // we retrieve the array index
       uint8_t* pkt_point = fch->pkt_pointer; // we retrieve the pkt pointer
       int serial = fch-> sn;
       int seed = fch->seed;
       if( fcm_peers.count(sender) == 0 ) { // non ho mai visto un pkt da questo sender lo scarto
           drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
           //Packet::free(p);
           incrPktInvalid();
           return;
       }
       incrDataPktReceived(); // poi aumento gli ack miei
       if ( (crb_phase == CRB_PHASE_I) ) {
             if ( crb_phase_I_index.count(index) == 0) {
                  crb_phase_I_index.insert(index);
                  if (debug_ > 0) cout << "FCModule(" << idcurrfc_ << ")::recv() a CRB_PHASE_I pkt,"
                                      << " index is " << index << " curr size is " << crb_phase_I_index.size()
                                      << endl;
                  if (crb_phase_I_seed == NOT_SET) {
                      crb_phase_I_seed = seed;
                      if (debug_ >0) cout << "FCModule(" << idcurrfc_ << ")::recv() set the CRB_PHASE_I seed to "
                                          << crb_phase_I_seed << endl;
                  } 
             }
             else {
                 /*drop(p,1,FC_DROP_REASON_ALREADY_RECEIVED_CRB_PH_I);
                 //Packet::free(p);
                 incrPktWrdata();
                 return;*/
                  //curr_ack_stat--;
             }
       }
       if (decoding == false) { // non sto già decodificando
          coder->setToDecoder(pkt_point,index,seed); // do il pkt al decoder
          for(hdr_iter i = buffer.begin(); i != buffer.end(); i++) {  // svuoto il buffer
 	      coder->setToDecoder((*i).pkt_pointer , (*i).pktid, (*i).seed); //setto i pkts del buffer 
          }
          buffer.clear(); //svuoto il buffer
       }
       else { //i pkts arrivano mentre sto decodificando, devo bufferizzare 
          buffer.push_back(*fch);          
       }
       if (debug_ > 0) 
           printf("  FCModule(%d)::recv() received a DATA pkt: sn %d sender %d index %d pointer %d seed %d\n",idcurrfc_,serial,sender,index,pkt_point,seed);             
           
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
  }
  break;
  

  case (SYNC) : { // se sono trasmettitore nn faccio nulla
    uint16_t sender = fch->sender_id;
    set<int>* rcvrs = fch->rxs_id;
    if ( (rcvrs->size() == 0) || ( ( rcvrs->count(idcurrfc_) ) != 0 ) ) { // è per tutti o è per me
       if (isdecoder == true) {
          if(fcm_peers.count(sender) == 0) { // sn ricevitore e nn ho sync del sender
             fcm_peers.insert(sender); //aggiungo il sender al mio set dei tramsettori 
             sendPkt(SYNC_ACK,0,sender);
             sendPkt(SYNC_ACK,period_,sender);
          }
          else { // sn ricevitore e ho già fatto handshake
            drop(p,1,FC_DROP_REASON_ALREADY_SYNC);
            //Packet::free(p);
            incrPktDupl();
            return;          
          }
       }
       else { // sn trasmettitore lo scarto
         drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
         //Packet::free(p);
         incrPktInvalid();
         return;
       }
     }
  }
  break;
  

  case (SYNC_ACK): { // i ricevitori NON prendono informazioni dai sync_ack 
    uint16_t sender = fch->sender_id;
    uint16_t receiver = fch->rx_id;
    if (receiver != idcurrfc_ ) { // nn sono il destinatario
      drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
      //Packet::free(p);
      incrPktInvalid();
      return;
    }
    if (isdecoder == false) {
       if (fcm_peers.count(sender) == 0) { //sono sender e ricevo un sync_ack da uno mai visto
       fcm_peers.insert(sender);  
       }
       else { // l'ho già visto e lo scarto
         drop(p,1,FC_DROP_REASON_ALREADY_SYNC);
         //Packet::free(p);
         incrPktDupl();
         return;
       }
     }
    else { //sono receiver e ricevo un sync_ack da uno mai visto controllo è ridondante
      printf("\e[32mFCModule(%d)::recv() logic error in case(SYNC_ACK): \e[0m\n",idcurrfc_);
      exit(1);
    }
  }
  break;
  

  case(SYNC_COOP): { // DA FARE

  }  
  break;

   case(SYNC_COOP_ACK): { // DA FARE

  } 
  break;


  case(STATS): {
    uint16_t sender = fch->sender_id; // chi l'ha mandato
    set<int>* receiver = fch->rxs_id; // per chi è
    if ( (receiver->count(idcurrfc_)) == 0) { // nn sono il destinatario lo scarto
      drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
      //Packet::free(p);
      incrPktInvalid();
      return;
    }    
    else if (isdecoder == false) { // è per me prendo le statistiche (controllo ridondante)
     int class_no = class_id[sender]; // prendo la classe del sender e aggiorno mie statistiche
     uint16_t ack_no = fch->total_ack;
     uint16_t rank_no = fch->res_rank;
     if (debug_ > 0) {
         printf("\e[1;34m  FCModule(%d)::recv() STATS pkt: user = %d total ack = %d res rank = %d \e[0m\n",idcurrfc_,sender,ack_no, rank_no);
         // waitForUser();
     }
     ackStats(sender);
     insertAck(class_no, ack_no);
     insertRank(class_no, rank_no);
    }
    else {
      printf("\e[32mFCModule(%d)::recv() logic error in case(STATS): \e[0m\n",idcurrfc_);
      exit(1);
    }
  }
  break;

  case(RESEND_STATS): {
    uint16_t sender = fch->sender_id; // chi l'ha mandato
    uint16_t receiver = fch->rx_id; // per chi è
    if (receiver != idcurrfc_ ) { // nn sono il destinatario
      drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
      //Packet::free(p);
      incrPktInvalid();
      return;
    }
    if (isdecoder == true){ // se sn decoder
       if (done == true) { // ma ho già decodificato 
           sendPkt(STATS,0,sender);
           sendPkt(STATS,period_,sender);    
       }
       else { // ho perso il  pacchetto DECODE PER STRADA decodifico ora
           tryToDecode();
           sendPkt(STATS,0,sender);
           sendPkt(STATS,period_,sender);        
       }
    }
    else {
      printf("\e[32mFCModule(%d)::recv() logic error in case(RESEND_STATS): \e[0m\n",idcurrfc_);
      exit(1);
    }
  }
  break;

  case(RESEND_NEWBLOCKACK): {
    uint16_t sender = fch->sender_id; // chi l'ha mandato
    uint16_t receiver = fch->rx_id; // per chi è
    if (receiver != idcurrfc_ ) { // nn sono il destinatario
      drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
      //Packet::free(p);
      incrPktInvalid();
      return;
    }
    if (isdecoder == true){
       if (newblock_done == true){
           sendPkt(NEW_BLOCK_ACK);
           sendPkt(NEW_BLOCK_ACK, period_);
       }
       else { // DA MODIFICARE
           doNewBlock();
           sendPkt(NEW_BLOCK_ACK,0,sender);
           sendPkt(NEW_BLOCK_ACK,period_,sender);        
       }
    }
    else {
      printf("\e[32mFCModule(%d)::recv() logic error in case(RESEND_NEWBLOCKACK): \e[0m\n",idcurrfc_);
      exit(1);
    }
  }
  break;

  case(NEW_BLOCK): {
    hrsn = 0; // resetto hrsn 
    uint16_t sender = fch->sender_id; // chi l'ha mandato
    set<int>* receiver = fch->rxs_id; // per chi è
    if ( (receiver->count(idcurrfc_)) == 0) { // nn sono il destinatario lo scarto
      drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
      //Packet::free(p);
      incrPktInvalid();
      return;
    }    
    else if ( ((isdecoder == false) && (is_master == false)) || (isdecoder == true) ) { // decoder o coop sender
      if (newblock_done == false){ // è il mio primo pacchetto NEWBLOCK 

         if ((done == false) && (mhop == false)) abortCurrBlock(); // nn ho finito. abort current block

         sendPkt(NEW_BLOCK_ACK,0,sender);
         sendPkt(NEW_BLOCK_ACK,period_,sender);

         if ((done == false) && (mhop == true)) increaseBlock();
         if ((done == true)) doNewBlock();
 
       }
       else {
            drop(p,1,FC_DROP_REASON_ALREADY_NEWBLOCK);
            //Packet::free(p);
            incrPktDupl();
            return;       
       }
     }
     else { // errore logico newblock
        printf("\e[32mFCModule(%d)::recv() logic error in case(NEWBLOCK): \e[0m\n",idcurrfc_);
        exit(1);
     }
    }
  break;
  
  case(NEW_BLOCK_ACK): {
    uint16_t sender = fch->sender_id; // chi l'ha mandato
    uint16_t receiver = fch->rx_id; // per chi è
    if (receiver != idcurrfc_ ) { // nn sono il destinatario
      drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
      //Packet::free(p);
      incrPktInvalid();
      return;
    }
    if ( (isdecoder == true) || ( (isdecoder == false) && (is_master == false) ) ) { // sn decoder o coop sender
      printf("\e[32mFCModule(%d)::recv() logic error in case(NEW_BLOCK_ACK): \e[0m\n",idcurrfc_);
      exit(1);
    }
    if ((isdecoder == false) && (is_master == true)) { // sn encoder master
        newBlockAck(sender);
    }
  }
  break;

  case(DECODE): {
    uint16_t sender = fch->sender_id; // chi l'ha mandato
    set<int>* receiver = fch->rxs_id; // per chi è
    if ( (receiver->count(idcurrfc_)) == 0) { // nn sono il destinatario lo scarto
      drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
      //Packet::free(p);
      incrPktInvalid();
      return;
    } 
    if (isdecoder == true ) { // sono decodificatore: decodifico se non ho già fatto
//        if (done == false) { // è il mio primo pacchetto DECODE
// //           tryToDecode();    
// //           sendPkt(STATS,0,sender);
// //           sendPkt(STATS,period_,sender);
//           }
//        else {
//           drop(p,1,FC_DROP_REASON_DECODED_DATA);
//           //Packet::free(p);
//           incrPktDupl();
//           return;
//        }
          tryToDecode();    
          sendPkt(STATS,0,sender);
          sendPkt(STATS,period_,sender);
    }
    if (isdecoder == false) { // sn encoder (master o coop)  lo scarto
      drop(p,1,FC_DROP_REASON_WRONG_RECEIVER);
      //Packet::free(p);
      incrPktInvalid();
      return;
    }
  }
  break;

  }

  incrPktRecv();
 
 /* throughput calculation */
  double dt = Scheduler::instance().clock() - lrtime;
  updateThroughput(ch->size(), dt);
  lrtime = Scheduler::instance().clock(); 
   
  /* Forward and Round Trip Time calculation */
  rftt = Scheduler::instance().clock() - fch->ts;
  if(fch->rftt_valid) 
    {
      updateFTT(fch->rftt);
      double rtt = rftt + fch->rftt;
      updateRTT(rtt);
    }

  Packet::free(p);

  //if (pkts_lost + pkts_recv + pkts_last_reset != hrsn)     
    //fprintf(stderr,"ERROR FCModule(%d)::recv() pkts_lost=%d  pkts_recv=%d  hrsn=%d\n",idcurrfc_ ,pkts_lost, pkts_recv, hrsn);
}

void FCModule::increaseBlock()
{
  
  if (debug_ > 0) cout << "FCModule::increaseBlock() called " << endl;
  done = false;
  newblock_done = true;
  curr_block++;  
}

inline void FCModule::ackStats(int sender) // ricevo un ack
{
  stats_peers.insert(sender);
}

inline void FCModule::newBlockAck(int sender) 
{
  if (debug_>0) printf("FCModule(%d)::newBlockAck() rcvd a newblock ack from %d \n",idcurrfc_,sender);
  newblock_peers.insert(sender);
}

// void FCModule::tryToDecode() // decodifica e aggiorna min ack o min rank
// {
//   if (isdecoder == true) { // controllo ridondante
//      newblock_done = false;
//      if ( curr_ack_stat >= blockSize_ ) { // posso provare a decodificar
//          decoding = true;
//          curr_rank_stat = coder->decode();
//          if (curr_rank_stat == 0) done = true; // se ho decodificato setto il flag done       
//          decoding = false;
//      } 
//      else curr_rank_stat = INF; 
//      if (debug_ > 0 ) {
//          printf("FCModule(%d)::tryToDecode() residual rank = %d current acked pkts %d\n",idcurrfc_,curr_rank_stat,curr_ack_stat);
//          //waitForUser();
//      }
//   }
//   else {
//      printf("\e[32mFCModule(%d)::tryToDecode() error, encoder is not supposed to decode!!!\e[0m\n",idcurrfc_);
//      exit(1);
//   }
// }


void FCModule::tryToDecode() // decodifica e aggiorna min ack o min rank
{
  if (isdecoder == true) { // controllo ridondante
     newblock_done = false;
     decoding = true;
     curr_rank_stat = coder->decode();
     if (curr_rank_stat == 0) done = true; // se ho decodificato setto il flag done       
     decoding = false;
     //else curr_rank_stat = INF; 
     if (debug_ > 0 ) {
         printf("FCModule(%d)::tryToDecode() residual rank = %d current acked pkts %d\n",idcurrfc_,curr_rank_stat,curr_ack_stat);
         //waitForUser();
     }
  }
  else {
     printf("\e[32mFCModule(%d)::tryToDecode() error, encoder is not supposed to decode!!!\e[0m\n",idcurrfc_);
     exit(1);
  }
}

inline void FCModule::abortCurrBlock() 
{
  done = true;
  newblock_done = false;
  if (debug_ > 0 && (mhop == false) ) printf("\e[1;31mFCModule(%d)::abortCurrBlock() block %d aborted\e[0m\n",idcurrfc_,curr_block);
  //waitForUser();
}


inline void FCModule::printStats()
{
  if (printstats == true) printf("\n");
  double time = difftime(stop_time,start_time);
  int min = (int)time / 60;
  int seconds = (int)(( (double)(time / 60) - (double)min ) * 60 );

  if (printstats == true) {
     printf("\e[1;37;40mFCModule(%d)::printStats() time elapsed: %dm:%ds\e[0m\n",idcurrfc_,min,seconds);
     printf("\e[1;37;40mFCModule(%d)::printStats() blocks failed=%d of %d total blocks | totaldata pkts=%d | totalctrl pkts=%d | total good rounds=%d \e[0m\n",idcurrfc_,block_failed,totBlock_,total_data_pkt,total_ctrl_pkt,total_good_rounds);
  }

  for (Setiter it1 = fcm_peers.begin() ; it1 != fcm_peers.end() ; it1++) {
    int index = *it1; 
    PerMap per_stats = fcm_pointers[index]->GetPER();

     for (PerMapIter it = per_stats.begin(); it != per_stats.end(); it++ ) {

         if (printstats == true) {
             printf("\e[1;37;40mFCModule(%d)::printStats() user %d  has PER = %e from sender user %d\e[0m\n",idcurrfc_,index,(*it).second,(*it).first);
             printf("\e[1;37;40mFCModule(%d)::printStats() user %d  has residual rank = %d\e[0m\n" ,idcurrfc_,index,fcm_pointers[index]->curr_rank_stat);
         } 
    }
  }

  N_tx = (double)total_data_pkt/(double)totBlock_;
  P_fail = (double)block_failed/(double)totBlock_;
  N_rounds = ((double)(max_rounds_*block_failed+total_good_rounds)/(double)totBlock_);

  if (printstats == true) {
     printf("\e[1;37;40mFCModule(%d)::printStats() sender N_tx = %f\e[0m\n",idcurrfc_,N_tx);
     printf("\e[1;37;40mFCModule(%d)::printStats() sender P_fail = %e\e[0m\n",idcurrfc_,P_fail);
     printf("\e[1;37;40mFCModule(%d)::printStats() sender N_rounds = %f (max rounds = %d)\e[0m\n",idcurrfc_,N_rounds,max_rounds_);
     printf("\n\n");
  }
  //if ( idcurrfc_ == 0) waitForUser();
}

char* FCModule::printCommand(int command)
{

  switch (command) {

     case(DATA): {
        return ("DATA");
     }
     break;

     case(SYNC): {
        return ("SYNC");
     }
     break;

     case(SYNC_ACK): {
        return ("SYNC_ACK");
     }
     break;

     case(SYNC_COOP):{
        return ("SYNC_COOP");
     }
     break;

     case(SYNC_COOP_ACK):{
        return ("SYNC_COOP_ACK");
     }
     break;
    
     case(STATS):{
        return ("STATS");
     }
     break;

     case(RESEND_STATS):{
        return ("RESEND_STATS");
     }
     break;

     case(RESEND_NEWBLOCKACK):{
         return ("RESEND_NEWBLOCKACK");
     }
     break;

     case(NEW_BLOCK):{
         return ("NEW_BLOCK");
     }
     break;

     case(NEW_BLOCK_ACK):{
          return ("NEW_BLOCK_ACK");
     }
     break;

     case(DECODE):{
         return ("DECODE");
     }
     break;

     case (ACTION_DATA): {
           return ("ACTION_DATA");
     }
     break;

     case (ACTION_WAIT_STATS): {
           return ("ACTION_WAIT_STATS");
     }
     break;

     case (ACTION_WAIT_NEWBLOCK):{
          return ("ACTION_WAIT_NEWBLOCK");
     }
     break;
     
     case (ACTION_SYNC): {
          return ("ACTION_SYNC");
     }
     break;

     case (ACTION_NEWBLOCK): {
          return ("ACTION_NEWBLOCK");
     }
     break;

  }

}

inline double FCModule::GetRTT() 
{
  //return srtt;
  return (rttsamples>0) ? sumrtt/rttsamples : 0;
}

inline double FCModule::GetFTT() 
{
  //  return sftt;
  return (fttsamples>0) ? sumftt/fttsamples : 0;
}

inline double FCModule::GetRTTstd() 
{
  if (rttsamples>1) 
    {
      double var = (sumrtt2 - (sumrtt*sumrtt / rttsamples) ) / (rttsamples - 1);
      return (sqrt(var));
    }
  else 
    return 0;
}

inline double FCModule::GetFTTstd() 
{
  if (fttsamples>1) 
    {
      double var = (sumftt2 - (sumftt*sumftt / fttsamples) ) / (fttsamples - 1);
      return (sqrt(var));
    }
  else 
    return 0;
}

PerMap FCModule::GetPER() // funziona solo per i pkt DATA x i pkt di controllo sono su canale senza errori
{
  PerMap per_peers;
  per_peers.clear();

  if (isdecoder == true) { 
         
        for (Setiter it = fcm_peers.begin(); it != fcm_peers.end(); it++) { 
            int index = *it;      
            double total_data = (fcm_pointers.at(index))->total_data_pkt;
            double per = 0;
            if (total_data != 0)  per = (double) (pkts_lost ) / total_data;
            else  per = 0;
            per_peers[(*it)] = per;
            /*cout << "totaldata = " << total_data << " pkts_lost = " << pkts_lost << " per = " << per << endl;
            waitForUser(); */
        }                 
  }
  return per_peers;
}

inline double FCModule::GetTHR() 
{
  //  return sthr;
  return ((sumdt != 0) ? sumbytes*8/sumdt : 0);
}

inline int FCModule::GetFCMId()
{
  return idcurrfc_;
}

inline void FCModule::updateRTT(double rtt)
{
  srtt = alpha_ * srtt + (1-alpha_) * rtt;
  sumrtt += rtt;
  sumrtt2 += rtt*rtt;
  rttsamples++;
}

inline void FCModule::updateFTT(double ftt)
{
  sftt = alpha_ * sftt + (1-alpha_) * ftt;
  sumftt += ftt;
  sumftt2 += ftt*ftt;
  fttsamples++;
}

inline void FCModule::updateThroughput(int bytes, double dt)
{
  //sthr = alpha_ * sthr + (1-alpha_) * thr;

  sumbytes += bytes;
  sumdt += dt;

  if (debug_ >0) cerr << "bytes=" << bytes << "  dt=" << dt << endl;
}

inline void FCModule::incrPktLost(int npkts)
{
  pkts_lost += npkts;
}

inline void FCModule::incrPktRecv()
{
  pkts_recv++;
}

inline void FCModule::incrPktInvalid()
{
  pkts_invalid++;
}

inline void FCModule::incrPktDupl()
{
  pkts_dupl++;
}

inline void FCModule::incrPktWrblock()
{
  pkts_wrblock++;
}

inline void FCModule::incrPktWrdata()
{
  pkts_wrdata++;
}

inline void FCModule::resetStats()
{
  pkts_last_reset += pkts_lost + pkts_recv;
  pkts_recv = 0;
  pkts_lost = 0;
  pkts_wrblock = 0;
  pkts_dupl = 0;
  pkts_last_reset = 0;    
  pkts_tx = 0;
  pkts_wrdata = 0;
  srtt = 0;
  sftt = 0;
  sthr = 0;
  //txsn = 1;
  //hrsn=0;
  rftt = -1;
  sumrtt = 0;
  sumrtt2 = 0;
  rttsamples = 0;
  sumftt = 0;
  sumftt2 = 0;
  fttsamples = 0;
  sumbytes = 0;
  sumdt = 0;	
  N_tx = 0;
  P_fail = 0;
  N_rounds = 0;
  total_data_pkt = 0;
  total_ctrl_pkt = 0;
  block_failed = 0;
  total_good_rounds = 0;
  //curr_ack_stat = NOT_SET; // the min current acked pkts among classes
  //curr_rank_stat = NOT_SET; // the max current rank among classes
  curr_round = 1;
  curr_class_min = 0;
  	
		
}

inline bool FCModule::isComplete()
{
  return finished;
}

inline void FCModule::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
}

