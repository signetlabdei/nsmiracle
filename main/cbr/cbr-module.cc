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
//    documentation and/or other materials provided with the distribtion.
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

#include <iostream>

#include <node-core.h>
#include <ip.h>
#include<rng.h>

#include "cbr-module.h"


extern packet_t PT_MCBR;


static class CbrModuleClass : public TclClass {
public:
  CbrModuleClass() : TclClass("Module/CBR") {}
  TclObject* create(int, const char*const*) {
    return (new CbrModule());
  }
} class_module_cbr;



void SendTimer::expire(Event *e)
{
  module->transmit();
}


int CbrModule::uidcnt_;		// unique id of the packet generated


CbrModule::CbrModule() 
  : sendTmr_(this),
    txsn(1),
    hrsn(0),
    pkts_recv(0),
    pkts_ooseq(0),
    pkts_lost(0),
    pkts_last_reset(0),
    rftt(-1),
    lrtime(0),
    sthr(0),
    sumrtt(0),
    rttsamples(0),
    sumftt(0),
    fttsamples(0),
    sumbytes(0),
    pob_(0),
    pob_time(0),
    pkts_opt(0),
    sumdt(0),
    MOS(0),
    first_pkt_rftt(-1),
    first_pkt_(-1),
    addfd_(false),
    fd_(0.0)
{ // binding to TCL variables
  bind("period_", &period_);
  bind("destPort_", (int*)&dstPort_);
  bind("destAddr_", (int*)&dstAddr_);
  bind("packetSize_", &pktSize_);
  bind("alpha_", &alpha_);
  bind("PoissonTraffic_", &PoissonTraffic_);
  uidcnt_ = 0;
}

CbrModule::~CbrModule()
{
}

// TCL command interpreter
int CbrModule::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc==2)
    {
      if(strcasecmp(argv[1], "start") == 0)	// TCL command to start the packet generation and transmission
	{
	  start();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "stop") == 0)	// TCL command to stop the packet generation
	{
	  stop();
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
      else if(strcasecmp(argv[1], "getftt_pob") == 0)	
	{
	  tcl.resultf("%f",GetFTTpob());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getper") == 0)	
	{
	  tcl.resultf("%f",GetPER());
	  return TCL_OK;
	}	
      else if(strcasecmp(argv[1], "getper_pb") == 0)	
	{
	  tcl.resultf("%f",GetPER_PB());
	  return TCL_OK;
	}	

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
       if(strcasecmp(argv[1], "getmos") == 0)	
      	{
	  tcl.resultf("%f",GetMOS());
      	  return TCL_OK;
      	}
       if(strcasecmp(argv[1], "getsentpkts") == 0)	
      	{
	  tcl.resultf("%d",txsn-1);
      	  return TCL_OK;
      	}
      if(strcasecmp(argv[1], "getrecvpkts") == 0)	
      	{
	  tcl.resultf("%d",pkts_recv);
      	  return TCL_OK;
      	}
      else if(strcasecmp(argv[1], "sendPkt") == 0)	
	{
	  sendPkt();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "resetStats") == 0)	
	{
	  resetStats();
	  fprintf(stderr,"CbrModule::command() resetStats %s, pkts_last_reset=%d, hrsn=%d, txsn=%d\n", tag_ , pkts_last_reset, hrsn, txsn);
	  return TCL_OK;
	}
    }
  if(argc==3){
        
    if(strcasecmp(argv[1], "use_playout_buffer") == 0)	// TCL command to start the packet generation and transmission
    {
      pob_ = 1;
      pob_time = atof(argv[2]);
      return TCL_OK;
    }
  
if(strcasecmp(argv[1], "set_codec_rate") == 0)	// TCL command to start the packet generation and transmission
    {
      
    cr_ = atof(argv[2]);
      return TCL_OK;
    }

if(strcasecmp(argv[1], "add_fixed_delay") == 0)	// TCL command to start the packet generation and transmission
    {
      addfd_= true;      
      fd_ = atof(argv[2]);
      return TCL_OK;
    }

}
  return Module::command(argc, argv);
}


int CbrModule::crLayCommand(ClMessage* m)
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

void CbrModule::initPkt(Packet* p)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  ch->uid() = uidcnt_++;
  ch->ptype() = PT_MCBR;
  ch->size() = pktSize_;
  
  hdr_ip* iph = hdr_ip::access(p);
  iph->daddr() = dstAddr_;
  iph->dport() = dstPort_;

  hdr_cbr* cbrh = HDR_CBR(p);
  cbrh->sn = txsn;
  txsn++;
  cbrh->ts = Scheduler::instance().clock();
  ch->timestamp() = cbrh->ts;

  if (rftt >= 0)
    {
      cbrh->rftt = rftt;
      cbrh->rftt_valid = true;
    }
  else
    {
      cbrh->rftt_valid = false;
    }

}

void CbrModule::start()
{
  sendTmr_.resched(getTimeBeforeNextPkt());
}

void CbrModule::sendPkt()
{
  double delay = 0;
  Packet* p = Packet::alloc();
  initPkt(p);
  hdr_cmn* ch = hdr_cmn::access(p);
  if (debug_>10)
    printf("CbrModule(%d)::sendPkt, send a pkt (%d)\n",getId(), ch->uid());
  sendDown(p,delay);
}

void CbrModule::transmit()
{
  sendPkt();
  // schedule next trasnmission
  sendTmr_.resched(getTimeBeforeNextPkt());
}

void CbrModule::stop()
{
  sendTmr_.force_cancel();
}


void CbrModule::recv(Packet* p, Handler* h)
{
	hdr_cmn* ch = hdr_cmn::access(p);
  
  recv(p);

}


void CbrModule::recv(Packet* p)
{
  hdr_cmn* ch = hdr_cmn::access(p);
  if (debug_>10)     
    printf("CbrModule(%d)::recv(Packet*p,Handler*) pktId %d\n",getId(), ch->uid());
    
  if (ch->ptype() != PT_MCBR)
    {
      drop(p,1,CBR_DROP_REASON_UNKNOWN_TYPE);
      incrPktInvalid();
      return;
    }

  hdr_ip* iph = hdr_ip::access(p);
  if (iph->saddr() != dstAddr_)
    {
      drop(p,1,CBR_DROP_REASON_WRONG_SADDR);
      incrPktInvalid();
      return;
    }
  if (iph->sport() != dstPort_)
    {
      drop(p,1,CBR_DROP_REASON_WRONG_SPORT);
      incrPktInvalid();
      return;
    }

  hdr_cbr* cbrh = HDR_CBR(p);

  int esn = hrsn + 1; // expected sn
    
  if (cbrh->sn < esn)
    { // packet is out of sequence and is to be discarded
      incrPktOoseq();
      if (debug_ > 1)
	printf("CbrModule::recv() Pkt out of sequence! cbrh->sn=%d\thrsn=%d\tesn=%d\n",cbrh->sn,hrsn,esn);
      drop(p,1,CBR_DROP_REASON_OUT_OF_SEQUENCE);
      return;
    }
  
  rftt = Scheduler::instance().clock() - cbrh->ts;  
 
  if(cbrh->rftt_valid) 
    {
    
      double rtt = rftt + cbrh->rftt;
      updateRTT(rtt);
    }


  if (addfd_ == true){
    
    rftt += fd_;
  
  }


  updateFTT(rftt);
 
  
  if (pob_ )
    {

      if  (first_pkt_ <= 0){
	first_pkt_=1;
	first_pkt_rftt= rftt + pob_time;


      } else{


	if ( rftt  > first_pkt_rftt   ) {

	  //if ( rftt  > GETFtt() + pob_time  ) {

	  incrPktOutBuff();

	  updateFTT(cbrh->rftt);
	  drop(p,1,CBR_DROP_REASON_EXCEEDED_PLAYOUT);   
	  return;}
      }
    }
  
  /* a new packet has been received */
  incrPktRecv();
  hrsn = cbrh->sn;  
  
  if (cbrh->sn > esn)
    { // packet losses are observed
      incrPktLost(cbrh->sn - (esn));     
    }
  
  
  /* throughput calculation */

  double dt = Scheduler::instance().clock() - lrtime;
  updateThroughput(ch->size(), dt);
  
 
  lrtime = Scheduler::instance().clock();
  

  /* Forward and Round Trip Time calculation */
  
   

  updateMOS(GetPER(), GetFTT(), cr_ );


  //drop(p,5,CBR_DROP_REASON_RECEIVED);   
  Packet::free(p);

  if (pkts_lost + pkts_recv + pkts_last_reset != hrsn)     
    fprintf(stderr,"ERROR CbrModule::recv() pkts_lost=%d  pkts_recv=%d  hrsn=%d\n", pkts_lost, pkts_recv, hrsn);

  //assert(pkts_lost + pkts_recv == hrsn);
  
}



double CbrModule::GetRTT() 
{
  //return srtt;
  return (rttsamples>0) ? sumrtt/rttsamples : 0;
}


double CbrModule::GetFTT() 
{
  //  return sftt;
  return (fttsamples>0) ? sumftt/fttsamples : 0;
}


double CbrModule::GetFTTpob() 
{
  //  return sftt;
  if (pob_)
    return first_pkt_rftt;
  else 
    return (fttsamples>0) ? sumftt/fttsamples : 0;
}



double CbrModule::GetRTTstd() 
{
  if (rttsamples>1) 
    {
      double var = (sumrtt2 - (sumrtt*sumrtt / rttsamples) ) / (rttsamples - 1);
      return (sqrt(var));
    }
  else 
    return 0;
}


double CbrModule::GetFTTstd() 
{
  if (fttsamples>1) 
    {
      double var;
      var = (sumftt2 - (sumftt*sumftt / fttsamples) ) / (fttsamples - 1);
      if (var>0)
	return (sqrt(var));
      else return 0;
    }
  else 
    return 0;
}


double CbrModule::GetPER() 
{
  if ((pkts_recv + pkts_lost)> 0)
    return ((double) pkts_lost / (double)(pkts_recv + pkts_lost));
  else 
    return 0;
}


double CbrModule::GetPER_PB() 
{
  if ((pkts_recv  )> 0)
    return ((double) pkts_opt / (double)(pkts_recv+ pkts_lost));
  else 
    return 0;
}

double CbrModule::GetTHR() 
{
  //  return sthr;
  return ((sumdt != 0) ? sumbytes*8/sumdt : 0);
}

double CbrModule::GetMOS() 
{
  //  return sthr;
  return MOS;
}


void CbrModule::updateRTT(double rtt)
{
  srtt = alpha_ * srtt + (1-alpha_) * rtt;
  sumrtt += rtt;
  sumrtt2 += rtt*rtt;
  rttsamples++;
}

void CbrModule::updateFTT(double ftt)
{
  sftt = alpha_ * sftt + (1-alpha_) * ftt;
  
  sumftt += ftt;
  sumftt2 += ftt*ftt;
  fttsamples++;


}


void CbrModule::updateMOS(double Pdrop, double delay, double rsource){
  
  double mos_= getMOS(Pdrop, delay, rsource);

  MOS = alpha_ * MOS + (1-alpha_) * mos_;
  
}



double CbrModule::getMOS(double Pdrop, double delay, double rsource){


  
  double Iel;
  Iel = 34.3*log(1+12.8*Pdrop);
  
  
  double rl;
  rl = rsource; //suppongo che rsouce si a gia quantizzata ai avalori possibili
  
  
  double Iec_v [6];
  
 
  Iec_v[0]=(94.2);
  Iec_v[1]=(94.2-71);
  Iec_v[2]=(94.2-79);
  Iec_v[3]=(94.2-84);
  Iec_v[4]=(94.2-92);
  Iec_v[5]=(0);
  
  double  Iec_r [6];
  
 
  Iec_r[0]=(0);
  Iec_r[1]=(5.6);
  Iec_r[2]=(6.3);
  Iec_r[3]=(8);
  Iec_r[4]=(40);
  Iec_r[5]=(64);
  
  double Iec;
  
  if (rl>=Iec_r[5]){
    Iec= Iec_v[5];
  }else{
    
    for (int i=0;i<4; i++)
      {
	if (rl>=Iec_r[i] && rl<=Iec_r[i+1]) ;
	Iec= Iec_v[i] + (Iec_v[i+1] - Iec_v[i+1]) / (Iec_r[i+1] - Iec_r[i]) * rsource;
      }
  }
  
  
  
  double Id;
  Id = 24*delay +110*(delay-177.3e-3)*(delay>177.3e-3);
  double R;
  R = 94.2 - Id-Iec-Iel;
  double MOS_;
  


  if (R<100 && R > 0)
    MOS_ = max(1+0.035*R+7e-6*R*(R-60)*(100-R),1.0);
  if (R>=100)
    MOS_=4.5;

  if (R<=0)
    MOS_=1;
  // cerr << "MOS_  " <<  MOS <<endl;
  return MOS_;
}








void CbrModule::updateThroughput(int bytes, double dt)
{
  //sthr = alpha_ * sthr + (1-alpha_) * thr;

  sumbytes += bytes;
  sumdt += dt;

  if (debug_) 
    cerr << "bytes=" << bytes << "  dt=" << dt << endl;
}

void CbrModule::incrPktLost(int npkts)
{
  pkts_lost += npkts;
}

void CbrModule::incrPktRecv()
{
  pkts_recv++;
}

void CbrModule::incrPktOoseq()
{
  pkts_ooseq++;
}

void CbrModule::incrPktInvalid()
{
  pkts_invalid++;
}

void CbrModule::incrPktOutBuff(){
  pkts_opt++; 
}


void CbrModule::resetStats()
{
  pkts_last_reset += pkts_lost + pkts_recv;
  pkts_recv = 0;
  pkts_ooseq = 0;
  pkts_lost = 0;
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
  pob_time =0;		
  MOS=0;
}


double CbrModule::getTimeBeforeNextPkt()
{
  if (period_<0)
    {
      fprintf(stderr,"%s : Error : period <= 0", __PRETTY_FUNCTION__);
      exit(1);
    }
  if (PoissonTraffic_)
    {
      double u = RNG::defaultrng()->uniform_double();
      double lambda = 1/period_;
      return (-log(u) / lambda);
    }
  else
    {
      // CBR
      return period_;
    }
  
  
}
