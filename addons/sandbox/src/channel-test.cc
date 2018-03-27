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


#include "channel-test.h"

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class ChannelTestClass : public TclClass {
public:
	ChannelTestClass() : TclClass("ChannelTest") {}
	TclObject* create(int, const char*const*) {
		return (new ChannelTest());
	}
} class_channeltest;

ChannelTest::ChannelTest()
{
  bind("delay_",&delay_);
  printf("channel-test initialized\n");
}

ChannelTest::~ChannelTest()
{
}

// TCL command interpreter
int ChannelTest::command(int argc, const char*const* argv)
{
	return (ChannelModule::command(argc,argv));
}


void ChannelTest::recv(Packet* p, ChSAP *chsap)
{
	// send to all the SAP connected to above layers exept for the one that is connected to the source module
	for(int i = 0; i<getChSAPnum(); i++)
	{
		ChSAP* sap = getChSAP(i);
		if (sap!=chsap)
		{
			Packet *c = p->copy();
			if(debug_ > 10)
				printf("%f -- ChannelTest::recv(%p,%p) --- copy=%p sap[%i]=%p\n", Scheduler::instance().clock(),p,chsap,c,i,sap);
			sap->sendUp(c, delay_);
			if(debug_ > 10)
				printf("\tsendUp done\n");
		}
	}
	Packet::free(p);
}
