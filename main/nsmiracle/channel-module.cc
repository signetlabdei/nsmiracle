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

#include "channel-module.h"


ChannelModule::ChannelModule() 
{
}

ChannelModule::~ChannelModule() 
{
}


// // TCL command intepreter
int ChannelModule::command(int argc, const char* const* argv)
{
	if (argc==3)
	{
		// install a SAP to an above module
		if (strcasecmp(argv[1],"addsap")==0)
		{
			ChSAP *chsap = dynamic_cast<ChSAP*>(TclObject::lookup(argv[2]));
			if (debug_>10)
				printf("ChannelModule::command %s\n", argv[1]);
			addUpSAP(chsap);
			return (TCL_OK);
		}
		
	}
	return Module::command(argc, argv);
}


int ChannelModule::getChSAPnum()
{
	return Module::getUpLaySAPnum();
}

ChSAP* ChannelModule::getChSAP(int i)
{
	return((ChSAP *)Module::getUpLaySAP(i));
}


void ChannelModule::swapChSAP(int i, int j)
{
	Module::swapUpLaySAP(i,j);
}


void ChannelModule::recv(Packet* p, Handler *h)
{
	recv(p, dynamic_cast<ChSAP*>(h));
}

void ChannelModule::recv(Packet* p)
{
	recv(p,(ChSAP*)0);
}

void ChannelModule::recv(Packet *p, int idSrc)
{
	recv(p,(ChSAP*)0);
}
