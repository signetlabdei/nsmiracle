/*
 * Copyright (c) 2007 Regents of the SIGNET lab, University of Padova.
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

#include"mac-module.h"
#include"umts-phy.h"
#include"umts-headers.h"
#include"umts-packet.h"
#include<iostream>

class ModuleUmtsMacClass : public TclClass {
public:
	ModuleUmtsMacClass() : TclClass("Module/UMTS/MAC") {}
	TclObject* create(int, const char*const*) {
	return (new UmtsMacModule());
	}
} class_module_umts_mac;


UmtsMacModule::UmtsMacModule() 
  : tx_timer(this),
    initslotoffset_(-1)
{
  bind("src_rlc_id_",&src_rlc_id_);
  bind("dst_rlc_id_",&dst_rlc_id_);
  bind("me_code_id_",&me_code_id_);
  bind("slot_duration_",&slot_duration_);
  bind("TTI_PDUs_",&TTI_PDUs_);
  bind("TTI_",&TTI_);
  bind("interleaving_",&interleaving_);
  bind("initslotoffset_",&initslotoffset_);

}



UmtsMacModule::~UmtsMacModule()
{


}


int UmtsMacModule::command (int argc, const char *const *argv)
{


  Tcl& tcl = Tcl::instance();
  
  if(argc == 2)
    {
      if (strcasecmp(argv[1],"start")==0)
	{	  
	  assert(initslotoffset_>0);
	  Scheduler::instance().schedule(&tx_timer, &tx_event, initslotoffset_ * slot_duration_);	 	    
	  return TCL_OK;	       
	}

    }
  else if(argc == 3)
    {
      
  //     if (strcasecmp(argv[1],"setDestRlcId")==0)
// 	{
// 	  int id = atoi(argv[2]);
// 	  assert(id>0);
// 	  dst_rlc_id_ = id;
// 	  return TCL_OK;	       
// 	}
//       if (strcasecmp(argv[1],"setSrcRlcId")==0)
// 	{
// 	  int id = atoi(argv[2]);
// 	  assert(id>0);
// 	  src_rlc_id_ = id;
// 	  return TCL_OK;	       
// 	}
    }
      
  /* If command is unknown, fallback to parent command intepreter */      
  return  Module::command(argc,argv);
  
}




void UmtsMacModule::recv(Packet* p)
{
  hdr_cmn *ch = HDR_CMN(p);
  hdr_rlc *rh = HDR_RLC(p);
  hdr_umtsphy* uh = HDR_UMTSPHY(p);


  if (ch->direction() == hdr_cmn::UP) 
    {
      assert(src_rlc_id_>0);  // Fails if MAC not initialized
      // discarding packets going UP which are not for the RCL connected to this MAC
      if (rh->dst_rlc_id_ != src_rlc_id_)
	{
	  //	  drop(p, 10, "NFM");
	  Packet::free(p);
	  return;
	}

	if (uh->data==FALSE)
	{
		//discard packet without data, used only to exchange control signalling (i.e., power control commands)
		Packet::free(p);
		return;
	}
      // discarding packets going UP which have not been received correctly
      if (ch->error())
	{
	  if (debug_) 
	    std::cerr << "UmtsMacModule::recv(p) packet with errors" << std::endl;      
	  
	  drop(p, 1, "ERR");
	  return;
	}
      sendUp(p);
    }
  else
    {
      assert(ch->direction() == hdr_cmn::DOWN); 
      assert(dst_rlc_id_>0);  // Fails if MAC not initialized
      // discarding packets going DOWN which are not for the peer RLC
      // connected to the RCL connected above this MAC
      if (rh->dst_rlc_id_ != dst_rlc_id_)
	{
	  //	  drop(p, 10, "NFM");
	  Packet::free(p);
	  return;
	}
     
      schedule(p);
    }

}



void UmtsMacModule::schedule(Packet* p)
{
  /// This MAC implementation supposes the RLC layer sends no more
  /// than the amount of data which the MAC layer can transmit
  assert(Q.size() < 2*TTI_PDUs_);
  Q.push(p);
}



void UmtsMacModule::transmit()
{
	Packet* p;
	bool data = false;
	if (! Q.empty())
	{
		// has a PDU to trasmit
		p = Q.front();
		hdr_MPhy*    ph = HDR_MPHY(p);
		hdr_umtsphy* uh = HDR_UMTSPHY(p);
		uh->data = true;
		uh->powerUp = true;
		Q.pop();
		sendDown(p, interleaving_);
	}else{
		// does not have any SDU to transmit -> tx an empty packet to 
		// simulate control channel parameters transmission
		p = Packet::alloc();
		hdr_cmn* ch = hdr_cmn::access(p);
		ch->ptype() = PT_UMTSCTRL;
		ch->size() = CNTRL_PKT_SIZE;		// non influential, i.e., the phy send the packet within the whole frame anyway
		hdr_rlc *rh = HDR_RLC(p);
		rh->dst_rlc_id_ = dst_rlc_id_;
		rh->src_rlc_id_ = src_rlc_id_;
		hdr_umtsphy* uh = HDR_UMTSPHY(p);
		uh->me_code_id = me_code_id_;
		hdr_MPhy*    ph = HDR_MPHY(p);
		uh->data = false;
		uh->powerUp = true;
		sendDown(p, interleaving_);
	}
	
	// Schedule next transmission slot
	Scheduler::instance().schedule(&tx_timer, &tx_event, slot_duration_);
}






void UmtsMacTimer::handle(Event*)
{
  mac->transmit(); 
}
