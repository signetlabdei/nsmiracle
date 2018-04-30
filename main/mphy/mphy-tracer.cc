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

class MPhyTracer : public Tracer
{
 public:
  MPhyTracer();
 protected:
  void format(Packet *p, SAP* sap);
  double WtodBm(double W);
};



MPhyTracer::MPhyTracer() : Tracer(1) {}


double MPhyTracer::WtodBm(double W)
{
  return ( W > 0 ? (10*log10(W)+30) : -HUGE_VAL);
}



void MPhyTracer::format(Packet *p, SAP *sap)
{

  hdr_MPhy* ph = HDR_MPHY(p);

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


      // dBm version
      //   writeTrace(sap, " %s Pt=%.2fdBm Pr=%.2fdBm Pn=%.2fdBm Pi=%.2fdBm tau=%.2es dur=%.5fs dist=%.1fm ",
      // 	     MPhy::getModulationName(ph->modulationType),
      // 	     WtodBm(ph->Pt),
      // 	     WtodBm(ph->Pr),
      // 	     WtodBm(ph->Pn),
      // 	     WtodBm(ph->Pi),
      // 	     ph->rxtime - ph->txtime,
      // 	     ph->duration,
      // 	     (ph->srcPosition && ph->dstPosition) ? ph->srcPosition->getDist(ph->dstPosition) : 0.0
      // 	     );

      // Watt (or whatever else... anyway, linear) version
      writeTrace(sap, (char*)" %s Pt=%.2e Pr=%.2e Pn=%.2e Pi=%.2e tau=%.2es dur=%.5fs dist=%.1fm ",
		 MPhy::getModulationName(ph->modulationType),
		 ph->Pt,
		 ph->Pr,
		 ph->Pn,
		 ph->Pi,
		 ph->rxtime - ph->txtime,
		 ph->duration,
		 dist
		 );
      //ph->worth_tracing = false; // no more phy tracing for this pkt
    }
}

extern "C" int Mphytracer_Init()
{  
  SAP::addTracer(new MPhyTracer);
  return 0;
}
extern "C" int  Cygmphytracer_Init()
{
  return Mphytracer_Init();
}


