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


#include "forward-module.h"

#include<iostream>


/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class EmptyModuleClass : public TclClass {
public:
	EmptyModuleClass() : TclClass("Module/FCM/SimpleMAC/CtrlModule") {}
	TclObject* create(int, const char*const*) {
		return (new EmptyModule());
	}
}class_emptymodule;

EmptyModule::EmptyModule() 
{
}

EmptyModule::~EmptyModule()
{
}


	// TCL command interpreter
int EmptyModule::command(int argc, const char*const* argv)
{
//   Tcl& tcl = Tcl::instance();

// 	if(argc==4)
// 		if(strcasecmp(argv[1], "discover") == 0)	

  return Module::command(argc, argv);	
}

int EmptyModule::recvSyncClMsg(ClMessage* m)
{
	// unknown ClMsg type
	Module::recvSyncClMsg(m);
}


int EmptyModule::recvAsyncClMsg(ClMessage* m)
{
  delete m;
}



void EmptyModule::recv(Packet* p)
{
	hdr_cmn *ch = HDR_CMN(p);
	if (ch->direction()==hdr_cmn::UP)
	  {
		sendUp(p, 0);
	}
	else
	{	

		sendDown(p, 0);
	}
	
}
