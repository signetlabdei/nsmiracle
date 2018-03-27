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

#include"tcp-tracer.h"

#include<packet.h>
#include<tcp.h>

TcpTracer::TcpTracer() : Tracer(4) {}


void TcpTracer::format(Packet *p, SAP *sap)
{

  hdr_cmn* ch = hdr_cmn::access(p);

  if ((ch->ptype() != PT_TCP)&&(ch->ptype() != PT_ACK))
    return;

  hdr_tcp* tcph = hdr_tcp::access(p);
  //  hdr_ip* iph = hdr_ip::access(p);


  writeTrace(sap, " id=%d %s sn=%d chts=%f ts=%f ts_echo=%f RTT=%f sz=%d",
	     ch->uid_,
	     ch->ptype() == PT_TCP ? "TCP" : "ACK",
	     tcph->seqno_,
	     ch->ts_,
	     tcph->ts(),
	     tcph->ts_echo(),
	     NOW - tcph->ts_echo(),
	     ch->size()
             );
}


extern "C" int Tcptracer_Init()
{
  SAP::addTracer(new TcpTracer);  
  return 0;
}
extern "C" int  Cygtcptracer_Init()
{
  Tcptracer_Init();
}


