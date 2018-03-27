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


#include<packettracer.h>
#include<sap.h>
#include<packet.h>


#include"mypacket.h"


class MyPktTracer : public Tracer
{
 public:
  MyPktTracer();
 protected:
  void format(Packet *p, SAP* sap);
};



MyPktTracer::MyPktTracer() : Tracer(1) {}


void MyPktTracer::format(Packet *p, SAP *sap)
{

  hdr_cmn* ch = HDR_CMN(p);
  hdr_mypkt* mh = HDR_MYPKT(p);

  packet_t  t = ch->ptype();

  if (t == PT_MYPKT)
    {
      writeTrace(sap, " MYPKT sn: %d ts: %6.9f ", mh->myseqno, mh->mytimestamp);
    }
}


extern "C" int Mypkttracer_Init()
{  
  SAP::addTracer(new MyPktTracer);
  return 0;
}
extern "C" int  Cygmypkttracer_Init()
{
  Mypkttracer_Init();
}

