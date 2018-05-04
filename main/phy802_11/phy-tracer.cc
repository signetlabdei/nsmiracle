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

#include<math.h>

#include<packet.h>
#include<node-core.h>


#include"phy-tracer.h"
#include"wirelessphy-module.h"

WirelessPhyTracer::WirelessPhyTracer() :  Tracer(1) {}

void WirelessPhyTracer::format(Packet *p, SAP *sap)
{
  hdr_MrclWrlPhy* ph = HDR_MRCLWRLPHY(p);

  if (ph->sourcePos_ && ph->destPos_)
    {
      double dX = ph->sourcePos_->getX() - ph->destPos_->getX();
      double dY = ph->sourcePos_->getY() - ph->destPos_->getY();
      double dist = sqrt(dX*dX + dY*dY);
      
      writeTrace(sap, (char*)" -PHY- TX:%4.1fdBm RX:%4.1fdBm d:%4.1fm ",
		p->txinfo_.getTxPr() > 0 ?  WtodBm(p->txinfo_.getTxPr()) : 999,
		p->txinfo_.RxPr > 0 ?  WtodBm(p->txinfo_.RxPr) : 999,
		 dist
		 );
    }

}


extern "C" int Phytracer_Init()
{
  SAP::addTracer(new WirelessPhyTracer);  
  return 0;
}
extern "C" int  Cygphytracer_Init()
{
  return Phytracer_Init();
}


