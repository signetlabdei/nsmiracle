/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: mll-tracer.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "mll-tracer.h"

MLLTracer::MLLTracer() : Tracer(1) {}

void MLLTracer::format(Packet *p, SAP *sap)
{
	hdr_cmn* ch = HDR_CMN(p);
	packet_t  t = ch->ptype();

	std::stringstream descrstream;

	if(t == PT_ARP)
	{
		hdr_arp	*ah = HDR_ARP(p);

		if(ah->arp_op  == ARPOP_REQUEST)
		{
			descrstream << " ARP Who has "
				    << ah->arp_tpa
				    << " tell "
				    << ah->arp_sha;
		}
		else if(ah->arp_op == ARPOP_REPLY)
		{
			descrstream << " ARP "
				    << ah->arp_spa
				    << " is at "
				    << ah->arp_sha;
		}
	}
	else
	  {
	    hdr_mac *mh = HDR_MAC(p);
	    descrstream << " " << mh->macSA()  << " -> " << mh->macDA() << " ";
	  }

		

	if(descrstream.str().length() > 0)
		writeTrace(sap, (char*)"%s", descrstream.str().c_str());
}

extern "C" int Mlltracer_Init()
{
	SAP::addTracer(new MLLTracer);
	return 0;
}
extern "C" int  Cygmlltracer_Init()
{
	return Mlltracer_Init();
}
