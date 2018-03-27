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

#include "headertracer.h"
#include "sap.h"
#include "ip.h"
#include "address.h"
#include "packet.h"
#include "mac.h"
#include "smac.h"

/*-------------------------------------------------------------------------------------------------------
	methods for CommonHeaderTracer class
---------------------------------------------------------------------------------------------------------*/ 

CommonHeaderTracer::CommonHeaderTracer() : Tracer(1) {}

void CommonHeaderTracer::format(Packet *p, SAP *sap)
{

	hdr_cmn *ch = hdr_cmn::access(p);
	struct hdr_mac802_11 *mh = HDR_MAC802_11(p);
	struct hdr_smac *sh = HDR_SMAC(p);

	char dir;
	switch (ch->direction())
	{
		case 1:
			dir = 'u';
			break;
		case -1:
			dir = 'd';
			break;
		case 0:
			dir = 'n';
			break;
	}
	
	writeTrace(sap, " %d %s %d %c",
				ch->uid(),													// unique packet id
				((ch->ptype() == PT_MAC) ? (
					(mh->dh_fc.fc_subtype == MAC_Subtype_RTS) ? "RTS"  :
					(mh->dh_fc.fc_subtype == MAC_Subtype_CTS) ? "CTS"  :
					(mh->dh_fc.fc_subtype == MAC_Subtype_ACK) ? "ACK"  :
					"UNKN") :
					(ch->ptype() == PT_SMAC) ? (
					(sh->type == RTS_PKT) ? "RTS" :
					(sh->type == CTS_PKT) ? "CTS" :
					(sh->type == ACK_PKT) ? "ACK" :
					(sh->type == SYNC_PKT) ? "SYNC" :
					"UNKN") : 
					packet_info.name(ch->ptype())),
				ch->size(),													// size
				dir
				);
	
}

/*-------------------------------------------------------------------------------------------------------
	methods for IpHeaderTracer class
---------------------------------------------------------------------------------------------------------*/ 

IpHeaderTracer::IpHeaderTracer() : Tracer(3) {}

void IpHeaderTracer::format(Packet *p, SAP *sap)
{
	hdr_ip *ih = hdr_ip::access(p);
	hdr_cmn *ch = hdr_cmn::access(p);
	
	int src = Address::instance().get_nodeaddr(ih->saddr());
	int dst = Address::instance().get_nodeaddr(ih->daddr());

	int next_hop = 0;
   if (ch->next_hop_ > 0) next_hop = ch->next_hop_;

	writeTrace(sap, " --IP-- [%d.%d.%d.%d:%d %d.%d.%d.%d:%d %d %d.%d.%d.%d]",
			((src & 0xff000000)>>24), 
         ((src & 0x00ff0000)>>16),
         ((src & 0x0000ff00)>>8), 
         (src & 0x000000ff),      
         ih->sport(),
         ((dst & 0xff000000)>>24), 
         ((dst & 0x00ff0000)>>16),
         ((dst & 0x0000ff00)>>8), 
         (dst & 0x000000ff),  
         ih->dport(),
			ih->ttl_, 
         ((next_hop & 0xff000000)>>24), 
         ((next_hop & 0x00ff0000)>>16),
         ((next_hop & 0x0000ff00)>>8), 
         (next_hop & 0x000000ff) );
}

extern "C" int Trace_Init()
{
	/*
	Put here all the commands which must be execute when the library is loaded (i.e. TCL script execution)
	Remember to ruturn 0 if all is OK, otherwise return 1
	*/
	SAP::addTracer(new CommonHeaderTracer);
	SAP::addTracer(new IpHeaderTracer);
	return 0;
}
extern "C" int  Cygtrace_Init()
{
  Trace_Init();
}


