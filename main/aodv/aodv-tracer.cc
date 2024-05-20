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


#include"aodv-tracer.h"
#include"aodv-pkt.h"

extern packet_t PT_MRCL_AODV;

MrclAodvTracer::MrclAodvTracer() :  Tracer(3) {}

void MrclAodvTracer::format(Packet *p, SAP *sap)
{
	hdr_cmn* ch = hdr_cmn::access(p);
	
	if (ch->ptype()!=PT_AODV)
		return;
	
	char pktinfo[606] = "";
	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);
	int saddrLen;
	char saddr[MRCL_ADDRESS_MAX_LEN] = "";
	char temp[10];
	char daddr[MRCL_ADDRESS_MAX_LEN] = "";
	int daddrLen;
	if (ah->ah_type == AODVTYPE_RREQ)
	{
		hdr_mrcl_aodv_request *rq = HDR_MRCL_AODV_REQUEST(p);
		memcpy(&saddrLen, rq->rq_src, sizeof(int));
		if (saddrLen==0) return; // routing addresses not yet initialized
		strcpy(saddr, "\0");
		for(int i=saddrLen-1; i>=0; i--)
		{
			if (i==0)
				snprintf(temp, sizeof(temp), "%d", rq->rq_src[i+sizeof(int)]);
			else
				snprintf(temp, sizeof(temp), "%d.", rq->rq_src[i+sizeof(int)]);
			strncat(saddr, temp, sizeof(saddr)-1);
		}
		strcat(saddr,"\0");

		memcpy(&daddrLen, rq->rq_dst, sizeof(int));
		if (daddrLen>0)
		{
			strcpy(temp,"");
			for(int i=daddrLen-1; i>=0; i--)
			{
				if (i==0)
					snprintf(temp, sizeof(temp), "%d", rq->rq_dst[i+sizeof(int)]);
				else
					snprintf(temp, sizeof(temp), "%d.", rq->rq_dst[i+sizeof(int)]);
				strncat(daddr, temp, sizeof(daddr)-1);
			}
		}
		strcat(daddr,"\0");
		snprintf(pktinfo, sizeof(pktinfo), "RREQ s%s d%s id%d hc%d ", saddr, daddr, rq->rq_bcast_id, rq->rq_hop_count);
		
	}else if (ah->ah_type == AODVTYPE_RREP)
	{
		hdr_mrcl_aodv_reply *rp = HDR_MRCL_AODV_REPLY(p);
		memcpy(&saddrLen, rp->rp_src, sizeof(int));
		if (saddrLen==0) return; // routing addresses not yet initialized
		for(int i=saddrLen-1; i>=0; i--)
		{
			if (i==0)
				snprintf(temp, sizeof(temp), "%d", rp->rp_src[i+sizeof(int)]);
			else
				snprintf(temp, sizeof(temp), "%d.", rp->rp_src[i+sizeof(int)]);
			strncat(saddr,temp, sizeof(saddr)-1);
		}
		strcat(saddr,"\0");

		memcpy(&daddrLen, rp->rp_dst, sizeof(int));
		if (daddrLen>0)
		{
			strcpy(temp,"");
			for(int i=daddrLen-1; i>=0; i--)
			{
				if (i==0)
					snprintf(temp, sizeof(temp), "%d", rp->rp_dst[i+sizeof(int)]);
				else
					snprintf(temp, sizeof(temp), "%d.", rp->rp_dst[i+sizeof(int)]);
				strncat(daddr,temp, sizeof(daddr)-1);
			}
		}
		strcat(daddr,"\0");
		snprintf(pktinfo, sizeof(pktinfo), "RREP s%s d%s sno%d hc%d", saddr, daddr, rp->rp_dst_seqno, rp->rp_hop_count);
	}else if (ah->ah_type == AODVTYPE_RERR)
	{
		snprintf(pktinfo, sizeof(pktinfo), "%s", "RERR");
	}
	else if (ah->ah_type == AODVTYPE_HELLO)
	{
		hdr_mrcl_aodv_reply *rp = HDR_MRCL_AODV_REPLY(p);
		snprintf(pktinfo, sizeof(pktinfo), "HELLO sno%d", rp->rp_dst_seqno);
	}
	else snprintf(pktinfo, sizeof(pktinfo), "%s", "UNKNW");

	writeTrace(sap, (char*)" --mAODV-- %s", pktinfo);

}
