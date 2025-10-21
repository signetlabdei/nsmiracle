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


#include<iostream>
#include "clsap.h"
#include "module.h"
#include "node-core.h"

#include "deprecated.h" 

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class ClSAPClass : public TclClass {
public:
	ClSAPClass() : TclClass("ConnectorTrace/ClSAP") {}
	TclObject* create(int, const char*const*) {
		return (new ClSAP());
	}
} class_clsap;

/*
Non static methods for CLSAP
*/
ClSAP::ClSAP() : ConnectorTrace(), pluginPtr_(0), nodeCorePtr_(0)
{
	depthNC_ = DEFAULTDEPTH;
	bind("depthToNC_", &depthNC_);
	depthP_ = DEFAULTDEPTH;
	bind("depthToP_", &depthP_);
}

ClSAP::~ClSAP()
{
	//printf("Distruttore CLSAP\n");
	//ConnectorTrace::~ConnectorTrace();
}

int ClSAP::command(int argc, const char* const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc==3)
	{
		// delay settabile solo da c++ o command tcl ??? chi lo setta ?
		// ha senso un settaggio fisso da tcl: ho il problema che �unico per differenti comunicazioni
		
		if (strcasecmp(argv[1],"module")==0)
		{
			Module *module = (Module*)TclObject::lookup(argv[2]);
			if (pluginPtr_!=0)
				{
					tcl.resultf("Error ClSAP cmd = %s: a module is already installed", argv[1]);
					return (TCL_ERROR);
				}
				pluginPtr_ = module;
				return (TCL_OK);
		}
		if (strcasecmp(argv[1],"nodecore")==0)
		{
			NodeCore *nodecore = (NodeCore*)TclObject::lookup(argv[2]);
			if (nodeCorePtr_!=0)
				{
					tcl.resultf("Error ClSAP cmd = %s: a nodecore is already installed", argv[1]);
					return (TCL_ERROR);
				}
				nodeCorePtr_ = nodecore;
				return (TCL_OK);
		}
	}
	return ConnectorTrace::command(argc, argv);
}


void ClSAP::sendModule(ClMessage* m, double delay)
{
	// TEST direction???
	if (pluginPtr_==0)
	{
		fprintf(stderr, "Error, ClSAP.sendModule: module not yet installed\n");
		exit(1);
	}
	m->direction(TOMODULE);
	if(delay > 0)
	{
		Scheduler::instance().schedule(this, m, delay);
	}
	else
	{
// 		printf("ClSAP::sendModule depth=%i verb=%i\n",depth_, m->verbosity());
		if (depthP_ >= m->verbosity())
			trace(m);
		RUN_DEPRECATED_OR_NEW_VIRTUAL_METHOD(pluginPtr_->crLayCommand(m), pluginPtr_->recvAsyncClMsg(m));		
	}
}

void ClSAP::sendSynchronousModule(ClMessage* m)
{
	// TEST direction???
	if (pluginPtr_==0)
	{
		fprintf(stderr, "Error, ClSAP.sendModule: module not yet installed\n");
		exit(1);
	}
	m->direction(TOMODULE);
	if (depthP_ >= m->verbosity())
		traceSync(m);
	RUN_DEPRECATED_OR_NEW_VIRTUAL_METHOD(pluginPtr_->crLaySynchronousCommand(m), pluginPtr_->recvSyncClMsg(m));
	m->direction(TONODECORE);
	if (depthNC_ >= m->verbosity())
		traceSync(m);
}

void ClSAP::sendClLayer(ClMessage* m, double delay)
{
	// TEST direction???
	if (nodeCorePtr_==0)
	{
		fprintf(stderr, "Error, ClSAP.sendClModule: nodeCorePtr not yet installed\n");
		exit(1);
	}
	m->direction(TONODECORE);
	if (depthNC_ >= m->verbosity())
		trace(m);
	if(delay > 0)
	{
		Scheduler::instance().schedule(this, m, delay);
	}
	else
	{
		nodeCorePtr_->crLayCommand(m);
	}
}

void ClSAP::sendSynchronousClLayer(ClMessage* m)
{
	// TEST direction???
  //  fprintf(stderr,  "%s received message type %d\n", __PRETTY_FUNCTION__, m->type());
	if (nodeCorePtr_==0)
	{
		fprintf(stderr, "Error, ClSAP.sendClModule: nodeCorePtr not yet installed\n");
		exit(1);
	}
	m->direction(TONODECORE);
	if (depthNC_ >= m->verbosity())
		traceSync(m);
	nodeCorePtr_->synchronousCrLayCommand(m);
	m->direction(TOMODULE);
	if (depthP_ >= m->verbosity())
		traceSync(m);
}

int ClSAP::getPluginId()
{
	return pluginPtr_?pluginPtr_->getId():NO_MODULE;
}

void ClSAP::handle(Event *e)
{
	ClMessage *m = (ClMessage *)e;
	if(m->direction() == TONODECORE)
		nodeCorePtr_->crLayCommand(m);
	else
	{
		if (depthP_ >= m->verbosity())
			trace(m);
		RUN_DEPRECATED_OR_NEW_VIRTUAL_METHOD(pluginPtr_->crLayCommand(m), pluginPtr_->recvAsyncClMsg(m));
	}
}

Position* ClSAP::getPosition()
{
	return nodeCorePtr_->getPosition();
}


