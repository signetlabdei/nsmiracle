/*
 * Copyright (c) 2008 Regents of the SIGNET lab, University of Padova.
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

#ifndef _SANDBOX_RM1_
#define _SANDBOX_RM1_


#include<../routing-module.h>
#include "sandbox-address.h"

#define STATICROUTE_NO_ROUTE "NRTE"
#define STATICROUTE_NO_ROUTE_VERBOSITY 3
struct RouteEntry
{
	SandboxAddress net;
	SandboxAddress mask;
	SandboxAddress next_hop;
	int module;
	RouteEntry *next;
};

class SandboxMetric;

class SandboxRM1 : public MrclRouting
{
public:
	SandboxRM1();
	~SandboxRM1();
	
	virtual int command(int argc, const char*const* argv);
	virtual char *getNextHop(Packet *p);
	virtual int canIReach(char *a, Metric ***m);
	virtual void forward(Packet *p);
	virtual int controlPacket(Packet *p);
	virtual void resolve(Packet* p);
	
protected:
	RouteEntry *routes_;
	RouteEntry *tail_;
	nsaddr_t ipAddr_;
	SandboxMetric** myMetric_;
};


#endif
