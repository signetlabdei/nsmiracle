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
 
#include "sap.h"
#include "module.h"
#include "scheduler.h"
#include<iostream>

#include "deprecated.h"

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class SAPClass : public TclClass {
public:
	SAPClass() : TclClass("ConnectorTrace/SAP") {}
	TclObject* create(int, const char*const*) {
		return (new SAP());
	}
} class_sap;

/*
Static attributes and methods for SAP
*/
PktTracer* SAP::pktTr_ = 0;

void SAP::addTracer(Tracer *tr)
{
	if(!pktTr_)
		pktTr_ = new PktTracer();
	//printf("pktTr_=%p size=%i(%i)\n", pktTr_, sizeof(*pktTr_), sizeof(PktTracer));
 	pktTr_->addTracer(tr);
}

/*
Non static methods for SAP
*/

SAP::SAP() : ConnectorTrace(), upModule_(0), downModule_(0)
{
	depthUp_ = DEFAULTDEPTH;
	depthDown_ = DEFAULTDEPTH;
	bind("depthUp_", &depthUp_);
	bind("depthDown_", &depthDown_);
}

SAP::~SAP()
{
	//ConnectorTrace::~ConnectorTrace();
}


int SAP::command(int argc, const char* const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc==3)
	{

		if (strcasecmp(argv[1],"upmodule")==0)
		{
			Module *module = (Module*)TclObject::lookup(argv[2]);
			if (upModule_!=0)
				{
					tcl.resultf("Error SAP cmd = %s: an above module is already installed", argv[1]);
					return (TCL_ERROR);
				}
				upModule_ = module;
				return (TCL_OK);
		}
		if (strcasecmp(argv[1],"downmodule")==0)
		{
			Module *module = (Module*)TclObject::lookup(argv[2]);
			if (downModule_!=0)
				{
					tcl.resultf("Error SAP cmd = %s: a bottom module is already installed", argv[1]);
					return (TCL_ERROR);
				}
				downModule_ = module;
				return (TCL_OK);
		}
	}
	return ConnectorTrace::command(argc, argv);
}

int SAP::depthUp()
{
	return depthUp_;
}

int SAP::depthDown()
{
	return depthDown_;
}

void SAP::sendUp(Packet* p, double delay)
{

	if (upModule_==0)
	{
		fprintf(stderr, "Error, SAP.sendUp: upModule not yet installed\n");
		exit(1);
	}
	if(debug_)
	{
		printf("%f -- SAP::sendUp(%p,%f) --- depthUp_=%i\n", Scheduler::instance().clock(), p, delay, depthUp_);
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
		upModule_->recv(p, getModuleDownId());
	}
}

void SAP::sendDown(Packet* p,  double delay)
{

	if (downModule_==0)
	{
		fprintf(stderr, "Error, SAP.downUp: downModule not yet installed\n");
		exit(1);
	}
	hdr_cmn *ch = HDR_CMN(p);
	ch->direction() = hdr_cmn::DOWN;
// 	printf("SAP %p send to downmodule %d\n",this, downModule_->getId());
	if(delay > 0)
	{
		Scheduler::instance().schedule(this, p, delay);
	}
	else
	{
		if (depthDown_)
			trace(p);
		downModule_->recv(p, getModuleUpId());
	}
}

void SAP::sendUp(ClMessage* m, double delay)
{
	// TEST direction???
	if (upModule_==0)
	{
		fprintf(stderr, "Error, SAP.sendUp(ClMessage): upModule not yet installed\n");
		exit(1);
	}
	m->direction(UP);
	if(delay > 0)
	{
		Scheduler::instance().schedule(this, m, delay);
	}
	else
	{
// 		printf("ClSAP::sendModule depth=%i verb=%i\n",depth_, m->verbosity());
		if (depthUp_ >= m->verbosity())
			trace(m);
		RUN_DEPRECATED_OR_NEW_VIRTUAL_METHOD(upModule_->crLayCommand(m), upModule_->recvAsyncClMsg(m));
	}
}

void SAP::sendDown(ClMessage* m, double delay)
{
	// TEST direction???
	if (downModule_==0)
	{
		fprintf(stderr, "Error, SAP.sendDown(ClMessage): downModule not yet installed\n");
		exit(1);
	}
	m->direction(DOWN);
	if(delay > 0)
	{
		Scheduler::instance().schedule(this, m, delay);
	}
	else
	{
		if (depthDown_ >= m->verbosity())
			trace(m);
		RUN_DEPRECATED_OR_NEW_VIRTUAL_METHOD(downModule_->crLayCommand(m), downModule_->recvAsyncClMsg(m));
	}
}

void SAP::sendSynchronousUp(ClMessage* m)
{
	// TEST direction???
	if (upModule_==0)
	{
		fprintf(stderr, "Error, SAP.sendSynchronousUp: upModule not yet installed\n");
		exit(1);
	}
	m->direction(UP);
	if (depthUp_ >= m->verbosity())
		traceSync(m);
	RUN_DEPRECATED_OR_NEW_VIRTUAL_METHOD(upModule_->crLaySynchronousCommand(m), upModule_->recvSyncClMsg(m));
	m->direction(DOWN);
	if (depthDown_ >= m->verbosity())
		traceSync(m);
}

void SAP::sendSynchronousDown(ClMessage* m)
{
	// TEST direction???
	if (downModule_==0)
	{
		fprintf(stderr, "Error, SAP.sendSynchronousDown: downModule not yet installed\n");
		exit(1);
	}
	m->direction(DOWN);
	if (depthDown_ >= m->verbosity())
		traceSync(m);
	RUN_DEPRECATED_OR_NEW_VIRTUAL_METHOD(downModule_->crLaySynchronousCommand(m), downModule_->recvSyncClMsg(m));
	m->direction(UP);
	if (depthUp_ >= m->verbosity())
		traceSync(m);
}

// get the module id of the module of the above layer
int SAP::getModuleUpId()
{
	return (upModule_->getId());
}

// get the module id of the module of the bottom layer
int SAP::getModuleDownId()
{
	return (downModule_->getId());
}


/* FIXED ONLY BY TCL ???
void SAP::depthTrace(int depth)
{
	depthTrace_ = depth;
}
*/

void SAP::trace(Packet *p)
{
	if(debug_>5)
		printf("%f --- SAP::trace(%p) --- pktTr_=%p -- [%c %.9f %s]\n", Scheduler::instance().clock(), p, pktTr_,dirDown_, Scheduler::instance().clock(), preambleDown_);
	hdr_cmn *ch = HDR_CMN(p);
	if(ch->direction() == hdr_cmn::DOWN)
		writeTrace((char*)"%c %.9f %s",dirDown_, Scheduler::instance().clock(), preambleDown_);
	else
		writeTrace((char*)"%c %.9f %s",dirUp_, Scheduler::instance().clock(), preambleUp_);
	if(debug_>10)
		printf("SAP::trace(%p) --- pktTr_=%p\n", p, pktTr_);
	if(pktTr_)
		pktTr_->trace(p, this);
	dump();
}

void SAP::trace(ClMessage *m)
{
	ConnectorTrace::trace(m);
}

void SAP::handle(Event* e)
{
	if(debug_)
	{
		printf("SAP::handle(%p) --- downModule_=%p\n", e, downModule_);
		fflush(stdout);
	}
	
	hdr_cmn *ch = HDR_CMN((Packet *)e);
	if(ch->direction() == hdr_cmn::DOWN)
	{
		if (depthDown_)
			trace((Packet *)e);
		downModule_->recv((Packet*)e, getModuleUpId());
	}
	else
	{
		if (depthUp_)
			trace((Packet *)e);
		upModule_->recv((Packet*)e, getModuleDownId());
	}
}

