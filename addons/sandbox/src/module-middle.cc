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


#include "module-middle.h"
#include "sandbox-clmsg.h"

#include<iostream>


/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class ModuleMiddleClass : public TclClass {
public:
	ModuleMiddleClass() : TclClass("Module/MiddleModule") {}
	TclObject* create(int, const char*const*) {
		return (new ModuleMiddle());
	}
}class_modulemiddle;

ModuleMiddle::ModuleMiddle() 
{
}

ModuleMiddle::~ModuleMiddle()
{
}


	// TCL command interpreter
int ModuleMiddle::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();

  if(argc==4)
    {

      if(strcasecmp(argv[1], "discover") == 0)	
	{
	  cerr << "---------------------------------------------" << endl;
	  cerr <<  __PRETTY_FUNCTION__
	       << " " << argv[1]  
	       << " " << argv[2] 
	       << " " << argv[3]  << endl;

	  if (strcasecmp(argv[2], "sync") == 0)
	    {
	      ClMsgDiscSync m;
	      if (strcasecmp(argv[3], "up") == 0)
		sendSyncClMsgUp(&m);
	      else if (strcasecmp(argv[3], "down") == 0)
		sendSyncClMsgDown(&m);
	      else if (strcasecmp(argv[3], "all") == 0)
		sendSyncClMsg(&m);
	      else
		{
		  cerr << __PRETTY_FUNCTION__   << " " << argv[1]  
		       << "unknown argv[2]: " << argv[2] << endl;
		  return TCL_ERROR;		  
		}
	      m.printDiscoveredModules();
	    }	  
	  else if (strcasecmp(argv[2], "async") == 0)
	    {
	      ClMsgDiscAsync* m = new ClMsgDiscAsync;
	      if (strcasecmp(argv[3], "up") == 0)
		sendAsyncClMsgUp(m);
	      else if (strcasecmp(argv[3], "down") == 0)
		sendAsyncClMsgDown(m);
	      else if (strcasecmp(argv[3], "all") == 0)
		sendAsyncClMsg(m);
	      else
		{
		  cerr << __PRETTY_FUNCTION__   << " " << argv[1]  
		       << "unknown argv[2]: " << argv[2] << endl;
		  return TCL_ERROR;		  
		}
	    }
	  else 
	    {
	      cerr << __PRETTY_FUNCTION__   << " " << argv[1]  
		   << "unknown argv[2]: " << argv[2] << endl;
	      return TCL_ERROR;
	    }	  
	  return TCL_OK;
	}
    }

  return Module::command(argc, argv);	
}

int ModuleMiddle::recvSyncClMsg(ClMessage* m)
{	
  if (m->type()==CLMSG_DISC_SYNC)
    {
      ClMsgDiscSync* dsm = (ClMsgDiscSync*) m;
      dsm->addModule(getId(), "ModuleMiddle");
    }  

  // unknown ClMsg type
  Module::recvSyncClMsg(m);
  
}


int ModuleMiddle::recvAsyncClMsg(ClMessage* m)
{
  if (m->type()==CLMSG_DISC_ASYNC)
    {
      cerr << "ID " << getId() << " received async discovery from ID " << m->getSource() << endl;
      // Reply to async discovery message
      ClMessage *c = new ClMsgDiscReply( m->getSource(), "ModuleMiddle");
      sendAsyncClMsg(c);
      delete m;  
      return 0;
    }

  if (m->type()==CLMSG_DISC_REPLY)
    {
      ClMsgDiscReply* dr = (ClMsgDiscReply*) m;
      cerr << "Got reply from  ID: " << dr->getSource() << " Name: " << dr->getModName() << endl;
      delete m;  
      return 0;
    }


  // Normally you would call Parent::recvAsyncClMsg() to allow
  // the parent class  to process the ClMsg. However, in this
  // case we have an exception: our Parent class is Module, so
  // we must NOT call  Module::recvAsyncClMsg()
  // Instead, we just free the memory by ourselves.
  // See the documentation for recvAsyncClMsg() in plugin.h for
  // reference. 

  delete m;      
}



void ModuleMiddle::recv(Packet* p)
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
