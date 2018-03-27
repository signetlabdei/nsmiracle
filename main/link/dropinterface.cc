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

#include "dropinterface.h"

class DropInterfaceClass : public TclClass {
public:
	DropInterfaceClass() : TclClass("DropInterface") {}
	TclObject* create(int, const char*const*) {
	  return (new DropInterface());
	}
} class_drop_interface;

DropInterface::DropInterface()
{
}

DropInterface::~DropInterface()
{
}

int DropInterface::command(int argc, const char*const* argv)
{
	if(argc == 3)
	{
		if (strcasecmp(argv[1],"setChannel")==0)
		{
			ch_ = (LinkChannelModule *)(TclObject::lookup(argv[2]));
			if (ch_ == 0) 
				return TCL_ERROR;
			return TCL_OK;
		}
	}
	// we still need the parent method
	return NsObject::command(argc,argv);
}

void DropInterface::recv(Packet *p, Handler *h)
{
	if(ch_)
		ch_->drop(p, DROPINTERFACE_TRACE_DEPTH, DROPINTERFACE_TRACE_REASON);
	else
		Packet::free(p);
}
