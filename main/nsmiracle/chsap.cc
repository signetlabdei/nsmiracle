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

#include "chsap.h"
#include "channel-module.h"

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class ChSAPClass : public TclClass {
public:
	ChSAPClass() : TclClass("ConnectorTrace/ChSAP") {}
	TclObject* create(int, const char*const*) {
		return (new ChSAP());
	}
} class_chsap;


ChSAP::ChSAP() : channel_(0)
{
}

ChSAP::~ChSAP()
{
}

// TCL command interpreter
int ChSAP::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc==3)
	{
		// install the module
		if (strcasecmp(argv[1],"module")==0)
		{
			 Module *module = (Module*)(TclObject::lookup(argv[2]));
			if (upModule_!=0)
			{
				tcl.resultf("Error SAP cmd = %s: an above module is already installed", argv[1]);
				return (TCL_ERROR);
			}
			upModule_ = module;
			return (TCL_OK);
		}
		else if (strcasecmp(argv[1],"channel")==0)
		{
			ChannelModule *module = (ChannelModule*)TclObject::lookup(argv[2]);
			if (channel_!=0)
				{
					tcl.resultf("Error SAP cmd = %s: a channel module is already installed", argv[1]);
					return (TCL_ERROR);
				}
				channel_ = module;
				downModule_ = module;
				return (TCL_OK);
		}
		else if (strcasecmp(argv[1],"nodeCore")==0)
		{
			nodeCorePtr_ = (NodeCore*)TclObject::lookup(argv[2]);
			if (!nodeCorePtr_)
			{
				tcl.resultf("Error CHSAP cmd = %s: no node core instance found", argv[2]);
				return (TCL_ERROR);
			}
			return (TCL_OK);
		}
	}
	return SAP::command(argc, argv);
}

void ChSAP::sendDown(Packet* p, double delay)
{
	if (channel_==0)
	{
		fprintf(stderr, "Error, ChSAP. downUp: downModule not yet installed\n");
		exit(1);
	}
	if(debug_)
		printf("ChSAP::sendDown(%p, %f) ---- depth_=%i\n", p, delay, depth_);
		
	hdr_cmn *ch = HDR_CMN(p);
	ch->direction() = hdr_cmn::DOWN;
	if(delay > 0)
	{
		Scheduler::instance().schedule(this, p, delay);
	}
	else
	{
		if (depthDown_)
			trace(p);
		channel_->recv(p, this);
	}
}

void ChSAP::sendUp(Packet* p, double delay)
{

	if (upModule_==0)
	{
		fprintf(stderr, "Error, SAP.sendUp: upModule not yet installed\n");
		exit(1);
	}
	if(debug_)
	{
		printf("%f -- SAP::sendUp(%p,%f) --- depth_=%i\n", Scheduler::instance().clock(), p, delay, depth_);
		fflush(stdout);
	}
	
	hdr_cmn *ch = HDR_CMN(p);
	ch->direction() = hdr_cmn::UP;
	
	if(debug_)
	{
		printf("\ttrace done\n");
	}
	if(delay > 0)
	{
		Scheduler::instance().schedule(this, p, delay);
	}
	else
	{
		if (depthUp_)
			trace(p);
		upModule_->recv(p, 0);
	}
}

void ChSAP::handle(Event* e)
{
	if(debug_)
	{
		printf("ChSAP::handle(%p) --- channel_=%p\n", e, channel_);
		fflush(stdout);
	}
	hdr_cmn *ch = HDR_CMN((Packet *)e);
	if(ch->direction() == hdr_cmn::DOWN)
	{
		if (depthDown_)
			trace((Packet *)e);
		channel_->recv((Packet*)e, this);
	}
	else
	{
		if (depthUp_)
			trace((Packet *)e);
		upModule_->recv((Packet*)e);
	}
}

Position* ChSAP::getPosition()
{
	return nodeCorePtr_->getPosition();
}
