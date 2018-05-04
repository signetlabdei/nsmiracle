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

#include "mphy.h"
#include<math.h>
#include<node-core.h>

class SinrTracer : public Tracer
{
 public:
  SinrTracer();
 protected:
  void format(Packet *p, SAP* sap);
};



SinrTracer::SinrTracer() : Tracer(1) {}



void SinrTracer::format(Packet *p, SAP *sap)
{

  hdr_MPhy* ph = HDR_MPHY(p);
  hdr_cmn* ch = HDR_CMN(p);

  // tracing only downward packets
  //if (ch->direction() == hdr_cmn::DOWN) return;


  // tracing only worth packets
  if (ph->worth_tracing)
    {
      assert((ph->Pn + ph->Pi)>0);
      double sinr = ph->Pr / (ph->Pn + ph->Pi);
      assert(sinr > 0);
      
      
      // linear version
      writeTrace(sap, (char*)" SINR=%05.2f %s ",
		 sinr,
		 ch->error() ? "ERR" : "OK "
		 );

      // dB version
//       writeTrace(sap, " SINR=%4.2fdB %s ",
// 		 10*log10(sinr),
// 		 ch->error() ? "ERR" : "OK "
// 		 );
    
    }
}

extern "C" int Sinrtracer_Init()
{  
  SAP::addTracer(new SinrTracer);
  return 0;
}
extern "C" int  Cygsinrtracer_Init()
{
  return Sinrtracer_Init();
}


