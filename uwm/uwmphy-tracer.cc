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

#include <mphy.h>
#include<math.h>
#include<node-core.h>

class UWMPhyTracer : public Tracer
{
 public:
  UWMPhyTracer();
  virtual ~UWMPhyTracer() { }
  
 protected:
  void format(Packet *p, SAP* sap);
  double WtodBm(double W);
};



UWMPhyTracer::UWMPhyTracer() : Tracer(1) {}


double UWMPhyTracer::WtodBm(double W)
{
  return ( W > 0 ? (10*log10(W)+30) : -HUGE_VAL);
}



void UWMPhyTracer::format(Packet *p, SAP *sap)
{

  hdr_MPhy* ph = HDR_MPHY(p);
  hdr_cmn* ch = HDR_CMN(p);

  // tracing only downward packets
  //if (ch->direction() == hdr_cmn::DOWN) return;


  // tracing only worth packets
  if (ph->worth_tracing)
    {

      Position* sp = dynamic_cast<Position*>(ph->srcPosition);
      Position* dp = dynamic_cast<Position*>(ph->dstPosition);

      double dist;
      if (sp && dp)
	dist = sp->getDist(dp);
      else
	dist = 0.0;

      assert(ph->srcSpectralMask);

      writeTrace(sap, " d: %7.1f  fc: %5.0f  bw: %5.0f  Pt: %.2e  Pr: %.2e  Pn: %.2e  SNR: %4.1f  tau: %.2e  dur= %.5f",
		 dist,
		 ph->srcSpectralMask->getFreq(),
		 ph->srcSpectralMask->getBandwidth(),
		 ph->Pt,
		 ph->Pr,
		 ph->Pn,
		 //ph->Pi,
		 10*log10(ph->Pr / ph->Pn),
		 ph->rxtime - ph->txtime,
		 ph->duration		
		 );
    }
}

extern "C" int Uwmphytracer_Init()
{  
  SAP::addTracer(new UWMPhyTracer);
  return 0;
}
extern "C" int  Cyguwmphytracer_Init()
{
  Uwmphytracer_Init();
}


