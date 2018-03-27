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

#include <packet.h>
#include <ip.h>

#include<packettracer.h>
#include<sap.h>

class RoutingTracer : public Tracer 
{
 public:
  RoutingTracer();
 protected:
  void format(Packet *p, SAP* sap);
};




RoutingTracer::RoutingTracer() : Tracer(3)
{
}

void RoutingTracer::format(Packet *p, SAP *sap)
{
  hdr_cmn *ch = hdr_cmn::access(p);
  hdr_ip *iph = hdr_ip::access(p);
  
  writeTrace(sap, " %d.%d.%d.%d --> %d.%d.%d.%d  SRC %d.%d.%d.%d:%d DST %d.%d.%d.%d:%d",
	     (ch->prev_hop_ & 0xff000000)>>24,
	     (ch->prev_hop_ & 0x00ff0000)>>16,
	     (ch->prev_hop_ & 0x0000ff00)>>8,
	     (ch->prev_hop_ & 0x000000ff),
	     (ch->next_hop_ & 0xff000000)>>24,
	     (ch->next_hop_ & 0x00ff0000)>>16,
	     (ch->next_hop_ & 0x0000ff00)>>8,
	     (ch->next_hop_ & 0x000000ff),
	     (iph->saddr() & 0xff000000)>>24,
	     (iph->saddr() & 0x00ff0000)>>16,
	     (iph->saddr() & 0x0000ff00)>>8,
	     (iph->saddr() & 0x000000ff),
	     iph->sport(),
	     (iph->daddr() & 0xff000000)>>24,
	     (iph->daddr() & 0x00ff0000)>>16,
	     (iph->daddr() & 0x0000ff00)>>8,
	     (iph->daddr() & 0x000000ff),
	     iph->dport());
	     
}

extern "C" int Routingtracer_Init()
{  
  SAP::addTracer(new RoutingTracer);
  return 0;
}
extern "C" int  Cygroutingtracer_Init()
{
  Routingtracer_Init();
}


