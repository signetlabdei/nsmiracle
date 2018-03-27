
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


/**
 * @file   phymac-clmsg-tracer.cc
 * @author Baldo Nicola
 * @date   Mon Nov 26 18:23:26 2007
 * 
 * @brief  
 * 
 * 
 */

#include"phymac-clmsg-tracer.h"

#include<clsap.h>



ClMsgPhyMacStartRxTracer::ClMsgPhyMacStartRxTracer()
  : ClMessageTracer(CLMSG_PHY2MAC_STARTRX) 
{
}

void ClMsgPhyMacStartRxTracer::format(ClMessage *m, ConnectorTrace *sap)
{
  writeTrace(sap, "PHY2MAC_STARTRX");
}


ClMsgPhyMacEndTxTracer::ClMsgPhyMacEndTxTracer()
  : ClMessageTracer(CLMSG_PHY2MAC_ENDTX) 
{
}

void ClMsgPhyMacEndTxTracer::format(ClMessage *m, ConnectorTrace *sap)
{
  writeTrace(sap, "PHY2MAC_ENDTX");
}



ClMsgPhyMacCCATracer::ClMsgPhyMacCCATracer()
  : ClMessageTracer(CLMSG_PHY2MAC_CCA) 
{
}

void ClMsgPhyMacCCATracer::format(ClMessage *m, ConnectorTrace *sap)
{
  writeTrace(sap, "PHY2MAC_CCA");
}




extern "C" int Mphymaccltracer_Init()
{
  ClSAP::addTracer(new ClMsgPhyMacStartRxTracer);
  ClSAP::addTracer(new ClMsgPhyMacEndTxTracer);
  ClSAP::addTracer(new ClMsgPhyMacCCATracer);
  return 0;
}


extern "C" int Cygmphymaccltracer_Init()
{
  return Mphymaccltracer_Init();
}
