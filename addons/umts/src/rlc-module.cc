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

#include"rlc-module.h"
#include<iostream>

class ModuleRlcClass : public TclClass {
public:
	ModuleRlcClass() : TclClass("Module/UMTS/RLC") {}
	TclObject* create(int, const char*const*) {
	return (new RlcModule());
	}
} class_module_rlc;


RlcModule::RlcModule() 
{
  rlc_id_counter++;
  rlc_id_ = rlc_id_counter;
  bind("ipAddr_",&ipAddr_);
  bind("destIpAddr_",&destIpAddr_);
}



RlcModule::~RlcModule()
{


}


int RlcModule::rlc_id_counter = 0;


int RlcModule::command (int argc, const char *const *argv)
{


  Tcl& tcl = Tcl::instance();
  
  if(argc == 2)
    {
      if (strcasecmp(argv[1],"getRlcId")==0)
	{
	  tcl.resultf("%d",rlc_id_);
	  return TCL_OK;	       
	}
      if (strcasecmp(argv[1],"getRlc")==0)
	{
	  if(rlc_)
	    {
	    tcl.result(rlc_->name());
	    return TCL_OK;
	    }
	  else return TCL_ERROR;	    
	}
    }
  else if(argc == 3)
    {
      if(strcasecmp(argv[1], "setRlc")==0)
	{
	  rlc_ = dynamic_cast<RLC*>(TclObject::lookup(argv[2]));
	    
	  if(!rlc_) 
	    return TCL_ERROR;

	  tcl.evalf("%s down-target %s",rlc_->name(), name());	  
	  tcl.evalf("%s up-target %s",rlc_->name(), name());	  
	  
	  return TCL_OK;		  
	}
      if (strcasecmp(argv[1],"setDestRlcId")==0)
	{
	  int id = atoi(argv[2]);
	  assert(id>0);
	  assert(id<=rlc_id_counter);
	  assert(id != rlc_id_);
	  dst_rlc_id_ = id;
	  return TCL_OK;	       
	}
      if (strcasecmp(argv[1],"setMeCodeId")==0)
	{
	  int id = atoi(argv[2]);
	  me_code_id_ = id;
	  return TCL_OK;	       
	}
    }
      
  /* If command is unknown, fallback to parent command intepreter */      
  return  Module::command(argc,argv);
  
}




void RlcModule::recv(Packet* p)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_rlc *rh = HDR_RLC(p);
	hdr_ip *iph = HDR_IP(p);
	if (ch->direction() == hdr_cmn::UP) 
	{
		// discarding packets going UP which are not for this RLC
		if (rh->dst_rlc_id_ != rlc_id_)
		{
		//	  drop(p, 10, "NFM");
			Packet::free(p);
			return;
		}
		// discarding packets going UP which have not been received correctly
		if (ch->error())
			{
			if (debug_) 
				std::cerr << "RlcModule::recv(p,h) packet with errors" << std::endl;      
			
			drop(p, 1, "ERR");
			return;
		}
	}
	if (ch->direction() == hdr_cmn::DOWN)
	{
		// discarding packets going DOWN which are not for the destination
		if ((iph->daddr() != destIpAddr_)
		    &&(ch->next_hop_ != destIpAddr_)
		    &&(iph->daddr() != IP_BROADCAST))
		{
		  //drop(p, 10, "NFM");
		  if (debug_>1)
		    cerr << NOW << "RlcModule::recv()" 
			 << " iph->daddr()=" << iph->daddr()
			 << " ch->next_hop_=" << ch->next_hop_
			 << " destIpAddr_=" << destIpAddr_
			 << endl;
		  Packet::free(p);
		  return;
		}
	}
// 	if (ch->direction() == hdr_cmn::DOWN)
// 		printf("RlcModule::recv(p) DOWN myIp %d daddr %d destIpAddr_ %d\n", ipAddr_, iph->daddr(), destIpAddr_);
// 	else
// 		printf("RlcModule::recv(p) UP myIp %d daddr %d destIpAddr_ %d\n", ipAddr_, iph->daddr(), destIpAddr_);

	if (debug_>1) std::cerr << "RlcModule::recv(p) Received Packet " << std::endl;

	assert(rlc_);

	// this is called for packets going both UP and DOWN
	// since RLC is able to distinguish packet direction
	rlc_->recv(p, (Handler*) 0);
}





/** 
 * This method is called by RLC::sendUp() and Rlc::sendDown()
 * since eurane RLC sees RlcModule as being 
 * as being both its down-target and up-target. RlcModule then
 * forwards the packet to upper and lower modules using Miracle methods
 * Module::sendUp(p) and Module::sendDown(p)
 * 
 * 
 * @param p pointer to the packet 
 * @param callback unused, kept only for compatibility
 */
void RlcModule::recv(Packet* p, Handler* callback)
{
  hdr_cmn *ch = HDR_CMN(p);
  hdr_rlc *rh = HDR_RLC(p);
  hdr_umtsphy *uh = HDR_UMTSPHY(p);

  if(ch->direction() == hdr_cmn::DOWN)
    {

      rh->src_rlc_id_ = rlc_id_;
      assert(dst_rlc_id_>0);  // Fails if RLC is not connected
      rh->dst_rlc_id_ = dst_rlc_id_;

      uh->me_code_id = me_code_id_; // Only useful if we're the BS
      
      if (debug_>1) std::cerr << "RlcModule::recv(p,h) Forwarding packet to lower modules" << std::endl;
      uh->data = TRUE;
      sendDown(p);
    }
  else 
    {  
      assert(ch->direction() == hdr_cmn::UP);
	
      if (debug_>1) std::cerr << "RlcModule::recv(p,h) Forwarding packet to upper modules" << std::endl;
      sendUp(p);
	
    }
}

AM* RlcModule::getAM()
{
  AM* ptr = dynamic_cast<AM*>(rlc_);
  assert(ptr); /* fails if rlc_ is not an AM RLC */
  return ptr;
}

int RlcModule::getTotPDUs()
{
  return (getAM()->getTotPDUs());
}

int RlcModule::getErrPDUs()
{
  return (getAM()->getErrPDUs());
}

int RlcModule::getAckPDUs()
{
  return (getAM()->getAckPDUs());
}

int RlcModule::getAckSDUs()
{
  return (getAM()->getAckSDUs());
}

int RlcModule::getDropPDUs()
{
  return (getAM()->getDropPDUs());
}

int RlcModule::getDropSDUs()
{
  return (getAM()->getDropSDUs());
}


