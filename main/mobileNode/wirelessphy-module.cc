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

#include "wirelessphy-module.h"

static class WirelessPhyModuleClass : public TclClass {
public:
	WirelessPhyModuleClass() : TclClass("Module/Phy") {}
	TclObject* create(int, const char*const*) {
	return (new WirelessPhyModule());

}
} class_wirelessphymodule;

WirelessPhyModule::WirelessPhyModule() : Module(), phy_(0)
{
}

WirelessPhyModule::~WirelessPhyModule()
{
}

int WirelessPhyModule::command(int argc, const char*const* argv)
{
// 	printf("WirelessPhyModule::command -- %s (%d)\n", argv[1], argc);
	Tcl& tcl = Tcl::instance();
	if(argc == 2)
	{
		if(strcasecmp(argv[1], "getPhy")==0)
		{
			if(phy_)
				tcl.result(phy_->name());
			else
				return TCL_ERROR;
			return TCL_OK;
		}
	}
	else if(argc == 3)
	{
		if(strcasecmp(argv[1], "setPhy")==0)
		{
			phy_ = (Phy *)TclObject::lookup(argv[2]);
			if(!phy_)
				return TCL_ERROR;
			tcl.evalf("%s channel %s",phy_->name(), name());
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}

void WirelessPhyModule::recv(Packet *p)
{
	if(phy_)
		phy_->recv(p, 0);
	else
	{
		fprintf(stderr, "WirelessPhyModule: there is no PHY connected!!");
		exit(1);
	}
}

void WirelessPhyModule::recv(Packet *p, Handler *callback)
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

