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

#include <iostream>

#include"link_channel.h"



class LinkChannelModuleClass : public TclClass {
public:
	LinkChannelModuleClass() : TclClass("Module/Link") {}
	TclObject* create(int, const char*const*) {
	  return (new LinkChannelModule());
	}
} class_module_channel_link;


LinkChannelModule::LinkChannelModule() : linkhead_(0), srcsap(0), dstsap(0) 
{

}

LinkChannelModule::~LinkChannelModule()  
{

}



int LinkChannelModule::command(int argc, const char*const* argv)
{

  Tcl& tcl = Tcl::instance();
  
    if(argc == 3)
    {

      if (strcasecmp(argv[1],"addsap")==0)
	{
	  // intercept this command so we can set srcsap and dstsap
	  // quite a wild hack, it depends on the order in which ChSAPs are added
	  ChSAP *chsap = dynamic_cast<ChSAP*>(TclObject::lookup(argv[2]));

	  if (srcsap == NULL) 
	    {
	      srcsap = chsap;
	    }
	  else
	    {
	      if (debug_)
		{
		  if (dstsap)
		    {
		      cerr << "dstsap:" << dstsap 
			   << "  addsap:" << chsap << endl;
		    }
		}
	      assert (dstsap == NULL); // this will fail if you try to attach more than two ChSAPs 
	      dstsap = chsap;      
	    }
	  // we still need the parent method
	  return ChannelModule::command(argc,argv);

	}
      else if(strcasecmp(argv[1], "setLinkHead")==0)
	{
	  linkhead_ = (NsObject *)TclObject::lookup(argv[2]);
	    
	  if(!linkhead_) 
	    return TCL_ERROR;

	  // link head is not the tail!!! We can't do this!
	  //tcl.evalf("%s target %s",linkhead_->name(), name());	  	  
	  return TCL_OK;		  
	}

    }
      
  /* If command is unknown, fallback to parent command intepreter */      
  return  ChannelModule::command(argc,argv);
  

}




void LinkChannelModule::recv(Packet* p, ChSAP* cs)
{
  assert(linkhead_ != NULL);

  if (cs != srcsap)
    {
      if (debug_) 
	cerr <<  this <<" packet received from the wrong ChSAP - Module/Link is unidirectional!" << endl;      

      // drop(p,5,LINKCH_DROP_REASON_WRONG_DIRECTION);
      Packet::free(p);
      return;
    }

  if (debug_) 
    cerr << this << " received packet from ChSAP" << cs << " (srcsap=" << srcsap << "), sending to  " << linkhead_ << endl;

  linkhead_->recv(p, (Handler *) 0);
  
}


void LinkChannelModule::recv(Packet* p, Handler* h)
{

  hdr_cmn* ch = hdr_cmn::access(p);

  if (ch->error())
    {
      drop(p,1,LINKCH_DROP_REASON_ERROR);
      return;
    }

  
  if (debug_) 
    cerr << this <<" sending packet from " << h << " (srcsap=" << srcsap << ") to dstsap " <<  dstsap << endl;
  
  dstsap->sendUp(p,0);
}
