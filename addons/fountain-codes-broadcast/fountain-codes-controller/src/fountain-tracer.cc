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


#include "fountain-module.h"
enum
 {
   DECODE = 9, DATA = 10, RESEND_STATS =11, SYNC = 12, SYNC_ACK = 13, STATS = 14 ,CTRL = 0, NEW_BLOCK = 16,
   NEW_BLOCK_ACK = 17 , SYNC_COOP = 18, SYNC_COOP_ACK = 19, RESEND_NEWBLOCKACK = 20
 };


class FCMTracer : public Tracer
{
 
 public:
  FCMTracer();
 protected:

  void format(Packet* p, SAP* sap);
  char* printCommand(int command);
};



FCMTracer::FCMTracer() : Tracer(4) {}



void FCMTracer::format(Packet *p, SAP *sap)
{
  hdr_cmn* ch = hdr_cmn::access(p);
   
  if (ch->ptype() != PT_FCM)
    return;

  hdr_fc* fcmh = HDR_FC(p);

  if (fcmh->rftt_valid)
    {      
      writeTrace(sap, " SN=%d TS=%f RFTT=%f SNDID=%d CMD=%s PKTID=%d BLKID=%d",
		 fcmh->sn,
                 fcmh->ts,
		 fcmh->rftt,
                 fcmh->sender_id,
                 printCommand(fcmh->command),
                 fcmh->pktid,
                 fcmh->block_id);
    }
  else
    {      
      writeTrace(sap, " SN=%d TS=%f SNDID=%d CMD=%s PKTID=%d BLKID=%d",
		 fcmh->sn,
                 fcmh->ts,
                 fcmh->sender_id,
                 printCommand(fcmh->command),
                 fcmh->pktid,
                 fcmh->block_id);
    }
}

char* FCMTracer::printCommand(int command)
{

  switch (command) {

     case(DATA): {
        return ("DATA");
     }
     break;

     case(SYNC): {
        return ("SYNC");
     }
     break;

     case(SYNC_ACK): {
        return ("SYNC_ACK");
     }
     break;

     case(SYNC_COOP):{
        return ("SYNC_COOP");
     }
     break;

     case(SYNC_COOP_ACK):{
        return ("SYNC_COOP_ACK");
     }
     break;
    
     case(STATS):{
        return ("STATS");
     }
     break;

     case(RESEND_STATS):{
        return ("RESEND_STATS");
     }
     break;

     case(RESEND_NEWBLOCKACK):{
         return ("RESEND_NEWBLOCKACK");
     }
     break;

     case(NEW_BLOCK):{
         return ("NEW_BLOCK");
     }
     break;

     case(NEW_BLOCK_ACK):{
          return ("NEW_BLOCK_ACK");
     }
     break;

     case(DECODE):{
         return ("DECODE");
     }
     break;
  }

}

extern "C" int Fcsimmactracer_Init()
{  
  SAP::addTracer(new FCMTracer);
  return 0;
}
extern "C" int  Cygfcsimmactracer_Init()
{
  Fcsimmactracer_Init();
}


