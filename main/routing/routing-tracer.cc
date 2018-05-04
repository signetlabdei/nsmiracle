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

#include<packet.h>
#include<node-core.h>


#include"routing-tracer.h"
#include "mrcl-address.h"

extern packet_t PT_MRCL_ROUTING;

MrclRoutingTracer::MrclRoutingTracer() :  Tracer(3) {}

void MrclRoutingTracer::format(Packet *p, SAP *sap)
{

//	hdr_cmn* ch = hdr_cmn::access(p);
// 	if (ch->ptype()!=PT_MRCL_ROUTING)
// 		return;

	RoutingHdr* h = HDR_ROUTING(p);
	
	int saddrLen;
	memcpy(&saddrLen, h->saddr(), sizeof(int));
	if (saddrLen==0) return; // routing addresses not yet initialized
	char saddr[MRCL_ADDRESS_MAX_LEN] = "";
	char temp[10];
	for(int i=saddrLen-1; i>=0; i--)
	{
		if (i==0)
			sprintf(temp,"%d", h->saddr()[i+sizeof(int)]);
		else
			sprintf(temp,"%d.", h->saddr()[i+sizeof(int)]);
		strcat(saddr,temp);
	}
	strcat(saddr,"\0");
	char daddr[MRCL_ADDRESS_MAX_LEN] = "";
	int daddrLen;
  memcpy(&daddrLen, h->daddr(), sizeof(int));
  if (daddrLen>0)
	{
		strcpy(temp,"");
		for(int i=daddrLen-1; i>=0; i--)
		{
			if (i==0)
				sprintf(temp,"%d", h->daddr()[i+sizeof(int)]);
			else
				sprintf(temp,"%d.", h->daddr()[i+sizeof(int)]);
			strcat(daddr,temp);
		}
	}
	strcat(daddr,"\0");

	char nexthop[MRCL_ADDRESS_MAX_LEN] = "";
	int nexthopLen;
	memcpy(&nexthopLen, h->nexthop(), sizeof(int));
	strcpy(temp,"");
	if (nexthopLen>0)
	{
		for(int i=nexthopLen-1; i>=0; i--)
		{
			if (i==0)
				sprintf(temp,"%d", h->nexthop()[i+sizeof(int)]);
			else
				sprintf(temp,"%d.", h->nexthop()[i+sizeof(int)]);
			strcat(nexthop,temp);
		}
	}
	strcat(nexthop,"\0");

	
	writeTrace(sap, (char*)" --mROUTING-- s%s d%s n%s", saddr, daddr, nexthop);

}

