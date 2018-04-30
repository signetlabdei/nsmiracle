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

#include "bin.h"
/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class BinClass : public TclClass {
public:
	BinClass() : TclClass("ConnectorTrace/Bin") {}
	TclObject* create(int, const char*const*) {
		return (new Bin());
	}
} class_bin;


/*
Non static methods for Bin
*/

Bin::Bin() : SAP(), ClSAP()
{
	preamble_[0] = 0;
	bind("depth", &depth_);
}

Bin::~Bin()
{
}

// TCL command interpreter
int Bin::command(int argc, const char*const* argv)
{
	if (strcmp(argv[1], "preamble") == 0)
		{
			sprintf(preamble_, "%s", argv[2]);
			return (TCL_OK);
		}
	return ConnectorTrace::command(argc, argv);
}

// write down the trace for a dropping of a packet event
void Bin::drop(Packet*p, int depth,const char *reason)
{
	if(debug_)
		printf("Bin::drop(%p,%i,%s) --- preamble_='%s'\n", p, depth, reason, preamble_);
	if(depth <= depth_)
	{
		writeTrace((char*)"D %.9f %s %s", Scheduler::instance().clock(), preamble_, reason);
		if(pktTr_)
			pktTr_->trace(p,this);
		dump();
	}
	Packet::free(p);
}

// write down the trace for a dropping of a message event
void Bin::drop(ClMessage* m, const char* reason)
{
	writeTrace((char*)"D %.9f %s %s", Scheduler::instance().clock(), preamble_, reason);
	if(clTr_)
		clTr_->trace(m,this);
	dump();
	delete m;
}

void Bin::handle(Event *e)
{
}

