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

//#include "mphy.h"
//#include<math.h>
//#include<node-core.h>
#include<packettracer.h>
#include<sap.h>
#include<packet.h>
#include"umts-phy.h"
#include"umts-headers.h"
#include"umts-packet.h"
#include<string>


class UmtsTracer : public Tracer
{
 public:
  UmtsTracer();
 protected:
  void format(Packet *p, SAP* sap);
};



UmtsTracer::UmtsTracer() : Tracer(1) {}


void UmtsTracer::format(Packet *p, SAP *sap)
{


  hdr_cmn* ch = HDR_CMN(p);
  hdr_umtsphy* uh = HDR_UMTSPHY(p);
  hdr_MPhy*    ph = HDR_MPHY(p); 
  hdr_rlc*   rlch = hdr_rlc::access(p);

  packet_t  t = ch->ptype();

  //  if ((t != PT_UM) && (t != PT_AM)) 
  if (t == PT_AM) 
    {
      //fprintf(stderr,"UMTSTRACER:  type RLC/A;\n");   
    }
  else if (t == PT_UMTSCTRL) 
    {
      //fprintf(stderr,"UMTSTRACER:  type UMTS/CTRL\n");
    }
  else 
    {
      // fprintf(stderr,"UMTSTRACER: unknown type\n");
      return;
    }



  char tstr[10];
  //  if ((uh->bs_code_id > 0)||(uh->me_code_id > 0))
  if (uh->me_code_id > 0)
    {
//       if (uh->direction == DIRECTION_DOWNLINK)	
// 	snprintf(tstr,3,"%03d",uh->bs_code_id);
//       else
	snprintf(tstr,9,"%02d",uh->me_code_id);
    }
  else
    strncpy(tstr, "   ", 9);    
  tstr[9] = '\0';  



  string descr;

  if (ch->ptype() == PT_AM) 
    {
      descr.append("AM");
      descr.append((rlch->lltype() & RLC_DATA) ? "DATA" : "STATUS");
      if (rlch->lltype() & RLC_BITMAP) descr.append(" BITMAP");
      if (rlch->lltype() & RLC_MRW) 
	{
	  descr.append(" MRW");
	  char fsn[10];
	  snprintf(fsn,10," %d",rlch->SN_MRW);
	  descr.append(fsn);
	}
      if (rlch->lltype() & RLC_MRWACK) 
	{
	  descr.append(" MRWACK");
	  char fsn[10];
	  snprintf(fsn,10," %d",rlch->SN_MRW_ACK);
	  descr.append(fsn);
	}

      if (rlch->lltype() & RLC_ACK) descr.append(" ACK");
      if (rlch->poll()) descr.append(" POLL");
    }
  else if (ch->ptype() == PT_UMTSCTRL)
    {
      descr.append("CTRL");
    }
  else if (ch->ptype() == PT_UM)
    {
      descr.append("UM");
    }
  else 
    {
      descr.append("UNKOWN");
    }
  



  writeTrace(sap, " UMTS %s %s %s %s %02d %02d %03d %s %s",
	     (uh->direction == DIRECTION_DOWNLINK) ? "DL" : "UL",
	     (uh->data == true) ? "D" : "C",
	     (uh->powerUp == true) ? "PU" : "PD",
	     tstr,
	     rlch->src_rlc_id_, 
	     rlch->dst_rlc_id_,
	     rlch->seqno(),
	     (rlch->eopno() == rlch->seqno()) ? "eop" : "---",
	     descr.c_str());
	     
}

extern "C" int Umtstracer_Init()
{  
  SAP::addTracer(new UmtsTracer);
  return 0;
}
extern "C" int  Cygumtstracer_Init()
{
  Umtstracer_Init();
}


