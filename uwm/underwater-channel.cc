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


#include<iostream>

#include "underwater-channel.h"

static class UnderwaterChannelClass : public TclClass {
public:
	UnderwaterChannelClass() : TclClass("Module/UnderwaterChannel") {}
	TclObject* create(int, const char*const*) {
	return (new UnderwaterChannel());

}
} class_underwaterchannel_module;

UnderwaterChannel::UnderwaterChannel() 
{
    bind("propSpeed_", &uw.prop_speed);
}


int UnderwaterChannel::command(int argc, const char*const* argv)
{
	//Tcl& tcl = Tcl::instance();
	
// 	if (argc==3)
// 	{
// 		// Over-ride of a channel-module tcl command:
// 		// 1) and set up the array of the nodes used for reduce complexity in packet trasmission
// 		// 2) install a SAP to an above module (call channel-module tcl command)
// 		if (strcasecmp(argv[1],"addsap")==0)
// 		{
// 			ChSAP *chsap = dynamic_cast<ChSAP*>(TclObject::lookup(argv[2]));
// 			if (!chsap)
// 				return TCL_ERROR;
// 			addNode(chsap);
// 			Module::command(argc, argv);
// 		}
// 		
// 	}
	return ChannelModule::command(argc, argv);
}




double UnderwaterChannel::getPropDelay(Position *src, Position* dst)
{
  return(uw.getPropagationDelay(src->getX(), 
				src->getY(), 
				src->getZ(),
				dst->getX(), 
				dst->getY(), 
				dst->getZ()));
}

void UnderwaterChannel::sendUpPhy(Packet *p,ChSAP *chsap)
{

	
  Scheduler &s = Scheduler::instance();
  struct hdr_cmn *hdr = HDR_CMN(p);
	
  hdr->direction() = hdr_cmn::UP;
	
  Position *sourcePos = chsap->getPosition();
  ChSAP *dest;

  if (debug_) cerr << "UnderwaterChannel:  sending packet" << endl;
	
  for (int i=0; i < getChSAPnum(); i++) {
		
    dest = (ChSAP*)getChSAP(i);
    if (chsap == dest) // it's the source node -> skip it
      continue;
		
    s.schedule(dest,
	       p->copy(), 
	       getPropDelay(sourcePos, dest->getPosition()));
  }

  Packet::free(p);
}

void UnderwaterChannel::recv(Packet *p, ChSAP* chsap)
{
	sendUpPhy(p, chsap);
}
