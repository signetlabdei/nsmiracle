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


#include "sandbox-plugin1.h"
#include "sandbox-clmsg.h"
#include "node-core.h"
#include<iostream>


/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class SandboxPlugIn1Class : public TclClass {
public:
	SandboxPlugIn1Class() : TclClass("SandboxPlugIn1") {}
	TclObject* create(int, const char*const*) {
		return (new SandboxPlugIn1());
	}
} class_sandboxplugin1;


SandboxPlugIn1::SandboxPlugIn1()
{
}

SandboxPlugIn1::~SandboxPlugIn1()
{
}

	// TCL command interpreter
int SandboxPlugIn1::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	
	if (argc==2)
	{
		if(strcasecmp(argv[1], "discover") == 0)
		{
			// have to send a cl-message to discover the send-module
			ClMessage *c = new ClMsgDiscAsync;
 			printf("PLUG-IN1: send a discovery message\n");
			// send cross-layer message broadcast to query the modules
			sendAsyncClMsg(c, 0);
			return TCL_OK;
		}
	} 

	return PlugIn::command(argc, argv);
}


int SandboxPlugIn1::recvAsyncClMsg(ClMessage* m)
{

  if (m->type()==CLMSG_DISC_ASYNC)
    {
      cerr << "ID " << getId() << " received async discovery from ID " << m->getSource() << endl;
      // Reply to async discovery message
      ClMessage *c = new ClMsgDiscReply(m->getSource(), "SandboxPlugin1");
      sendAsyncClMsg(c);
      delete m;  
      return 0;
    }

  if (m->type()==CLMSG_DISC_REPLY)
    {
      ClMsgDiscReply* dr = (ClMsgDiscReply*) m;
      cerr << "Got reply from  ID: " << dr->getSource() << " Name: " << dr->getModName() << endl;
      
      if (strcmp(dr->getModName(),"ModuleSend") == 0)
	{
	  cerr << "Discovered a ModuleSend at ID " << dr->getSource() 
	       << ", Sending CLMSG_SET_SIZE " << endl;      
	}
      ClMsgSet *c = new ClMsgSet(m->getSource());
      c->setSize(777); // the new message simulates a packet size change command
      sendAsyncClMsg(c, 0);
	  
      delete m;  
      return 0;
    }

	
  // Normally you would call Parent::recvAsyncClMsg() to allow
  // the parent class  to process the ClMsg. However, in this
  // case we have an exception: our Parent class is Plugin, so
  // we must NOT call  Plugin::recvAsyncClMsg()
  // Instead, we just free the memory by ourselves.
  // See the documentation for recvAsyncClMsg() in plugin.h for
  // reference. 

  delete m;
  return 0;
}


void SandboxPlugIn1::setClMsgType(ClMessage_t type)
{
	clMsgType_ = type;
}

ClMessage_t SandboxPlugIn1::getClMsgType()
{
	return(clMsgType_);
}

extern "C" int SandboxF_Init()
{
	/*
	Put here all the commands which must be execute when the library is loaded (i.e. TCL script execution)
	Remember to ruturn 0 if all is OK, otherwise return 1
	*/
	return 0;
}
extern "C" int  CygsandboxF_Init()
{
  SandboxF_Init();
}


