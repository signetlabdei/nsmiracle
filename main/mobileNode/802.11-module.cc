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

#include "802.11-module.h"

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class MacInterfaceClass : public TclClass {
public:
	MacInterfaceClass() : TclClass("MacInterface") {}
	TclObject* create(int, const char*const*) {
	return (new MacInterface());

}
} class_macinterface;

MacInterface::MacInterface() : NsObject()
{
}

MacInterface::~MacInterface()
{
}

void MacInterface::setModule(MacModule802_11 *m)
{
	module_ = m;
}

void MacInterface::recv(Packet *p, const char *why)
{
	module_->drop(p, 5, why);
}

void MacInterface::recv(Packet *p)
{
	module_->drop(p, 5);
}

void MacInterface::recv(Packet *p, Handler *h)
{
	module_->drop(p, 5);
}

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class LogInterfaceClass : public TclClass {
public:
	LogInterfaceClass() : TclClass("LogInterface") {}
	TclObject* create(int, const char*const*) {
	return (new LogInterface());

}
} class_loginterface;

LogInterface::LogInterface() : NsObject()
{
}

LogInterface::~LogInterface()
{
}

void LogInterface::setModule(MacModule802_11 *m)
{
	module_ = m;
}

void LogInterface::recv(Packet *p, const char *why)
{
	module_->drop(p, 10, why);
}

void LogInterface::recv(Packet *p)
{
	module_->drop(p, 10);
}

void LogInterface::recv(Packet *p, Handler *h)
{
	module_->drop(p, 10);
}

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class LLInterfaceClass : public TclClass {
public:
	LLInterfaceClass() : TclClass("LLInterface") {}
	TclObject* create(int, const char*const*) {
	return (new LLInterface());

}
} class_llinterface;

LLInterface::LLInterface() : NsObject()
{
}

LLInterface::~LLInterface()
{
}

void LLInterface::setModule(MacModule802_11 *m)
{
	module_ = m;
}

void LLInterface::recv(Packet *p)
{
	module_->sendUp(p);
}

void LLInterface::recv(Packet *p, Handler *h)
{
	module_->sendUp(p);
}

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class Module802_11Class : public TclClass {
public:
	Module802_11Class() : TclClass("Module/802_11") {}
	TclObject* create(int, const char*const*) {
	return (new MacModule802_11());

}
} class_module802_11;

MacModule802_11::MacModule802_11() : Module(), mac_(0), queue_(0), logif_(0), macif_(0), llif_(0)
{
}

MacModule802_11::~MacModule802_11()
{
}

int MacModule802_11::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 2)
	{
		if(strcasecmp(argv[1], "getMac")==0)
		{
			if(mac_)
				tcl.result(mac_->name());
			else
				return TCL_ERROR;
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getQueue")==0)
		{
			if(queue_)
				tcl.result(queue_->name());
			else
				return TCL_ERROR;
			return TCL_OK;
		}
	}
	else if(argc == 3)
	{
		if(strcasecmp(argv[1], "setMac")==0)
		{
			mac_ = (Mac *)TclObject::lookup(argv[2]);
			if(!mac_)
				return TCL_ERROR;
			tcl.evalf("%s down-target %s",mac_->name(), name());
			if(macif_)
			{
				tcl.evalf("%s drop-target %s",mac_->name(), macif_->name());
			}
			if(logif_)
			{
				tcl.evalf("%s log-target %s",mac_->name(), logif_->name());
			}
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setIfq")==0)
		{
			queue_ = (Queue *)TclObject::lookup(argv[2]);
			if(!queue_)
				return TCL_ERROR;
			if(macif_)
				tcl.evalf("%s drop-target %s",queue_->name(), macif_->name());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setLL")==0)
		{
			ll_ = (LL *)TclObject::lookup(argv[2]);
			if(!ll_)
				return TCL_ERROR;
			tcl.evalf("%s up-target %s",ll_->name(), name());
			if(macif_)
				tcl.evalf("%s drop-target %s",ll_->name(), macif_->name());
			if(llif_)
				tcl.evalf("%s up-target %s",ll_->name(), llif_->name());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setMacInterface")==0)
		{
			macif_ = (MacInterface *)TclObject::lookup(argv[2]);
			if(!macif_)
				return TCL_ERROR;
			if(mac_)
			{
				tcl.evalf("%s drop-target %s",mac_->name(), macif_->name());
			}
			if(queue_)
				tcl.evalf("%s drop-target %s",queue_->name(), macif_->name());
			if(ll_)
				tcl.evalf("%s drop-target %s",ll_->name(), macif_->name());
			macif_->setModule(this);
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setLogInterface")==0)
		{
			logif_ = (LogInterface *)TclObject::lookup(argv[2]);
			if(!logif_)
				return TCL_ERROR;
			if(mac_)
			{
				tcl.evalf("%s log-target %s",mac_->name(), logif_->name());
			}
			logif_->setModule(this);
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setLLInterface")==0)
		{
			llif_ = (LLInterface *)TclObject::lookup(argv[2]);
			if(!llif_)
				return TCL_ERROR;
			if(ll_)
				tcl.evalf("%s up-target %s",ll_->name(), llif_->name());
			llif_->setModule(this);
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}

void MacModule802_11::recv(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	if(ch->direction() == hdr_cmn::DOWN)
	{
		if(ll_)
			ll_->recv(p, 0);
		else
		{
			fprintf(stderr, "MacModule802_11: there is no ll connected!!");
			exit(1);
		}
	}
	else
	{
		if(mac_)
			mac_->recv(p, 0);
		else
		{
			fprintf(stderr, "MacModule802_11: there is no mac connected!!");
			exit(1);
		}
	}
	
}

void MacModule802_11::recv(Packet *p, Handler *callback)
{
	hdr_cmn *ch = HDR_CMN(p);
	if(ch->direction() == hdr_cmn::DOWN)
	{
		sendDown(p);
	}
	else
	{
		sendUp(p);
	}
}
