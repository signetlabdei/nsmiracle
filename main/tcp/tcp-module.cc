/*
 * Copyright (c) 2006 Regents of the SIGNET lab, University of Padova.
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



#include"tcp-module.h"
#include<assert.h>
#include<unistd.h>
#include<fstream>
#include<iostream>
#include<tcp.h>
#include<ipmodule.h>



class ModuleTcpClass : public TclClass {
public:
	ModuleTcpClass() : TclClass("Module/TCP") {}
	TclObject* create(int, const char*const*) {
	return (new TcpModule());
	}
} class_module_tcp;





TcpModule::TcpModule() 
{


}



TcpModule::~TcpModule()
{


}


/** 
 * This method is called by TcpAgent::send()
 * since standard ns TcpAgent see TcpModule
 * as being their (down)target. TcpModule then
 * forwards the packet to lower layers using SendDown()
 * 
 * 
 * @param p pointer to the packet to be sent
 * @param callback unused by all known TcpAgents, kept only for compatibility
 */
void TcpModule::recv(Packet* p, Handler* callback)
{
  sendDown(p);
}


void TcpModule::recv(Packet *p)
{
  assert(tcpagent_ != NULL);
  tcpagent_->recv(p,NULL);
}


void TcpModule::recv(Packet *p, int idSrc)
{
  recv(p);
}


int TcpModule::command(int argc, const char*const* argv)
{

  Tcl& tcl = Tcl::instance();
  
  if(argc == 2)
    {
      if (strcasecmp(argv[1],"getTcpAgent")==0)
	{
	  if(tcpagent_)
	    {
	    tcl.result(tcpagent_->name());
	    }
	  else
	    {
	      /* 
	       * This block is intended to be executed
	       * only by Module/Transport/TCP init instproc
	       * see tcp-transport-modules.tcl for details
	       */
	      tcl.resultf("%s","NONE");
	    }
	  return TCL_OK;
	}
    }
  else if(argc == 3)
    {
      if(strcasecmp(argv[1], "setTcpAgent")==0)
	{
	  tcpagent_ = (TcpAgent *)TclObject::lookup(argv[2]);
	    
	  if(!tcpagent_) 
	    return TCL_ERROR;

	  tcl.evalf("%s target %s",tcpagent_->name(), name());	  
	  
	  return TCL_OK;		  
	}
     if(strcasecmp(argv[1], "setDestAddr")==0)
	{
	  if(!tcpagent_) 
	    return TCL_ERROR;
	  tcpagent_->daddr() = IPModule::str2addr(argv[2]);	  
	  return TCL_OK;		  
	}
     if(strcasecmp(argv[1], "setDestPort")==0)
	{
	  if(!tcpagent_) 
	    return TCL_ERROR;
	  tcpagent_->dport() = atoi(argv[2]);	  
	  return TCL_OK;		  
	}
    }
      
  /* If command is unknown, fallback to parent command intepreter */      
  return  Module::command(argc,argv);
  
}
