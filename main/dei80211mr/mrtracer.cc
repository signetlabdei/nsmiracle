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

/* For some REALLY WEIRD reason I am forced to include a whole bunch of headers here...*/

#include <delay.h>
#include <connector.h>
#include <packet.h>
#include <random.h>
#include <mobilenode.h>

// // #define DEBUG 99

#include <arp.h>
#include <ll.h>
#include <mac.h>
#include <mac-timers.h>
#include <mac-802_11mr.h>
#include <cmu-trace.h>

// Added by Sushmita to support event tracing
#include <agent.h>
#include <basetrace.h>


#include<packet.h>

#include<mac-802_11mr.h>
#include"mrtracer.h"

MultirateTracer::MultirateTracer() :  Tracer(1) {}


static char* PhyModeStr[NumPhyModes]={"  1Mbps",
				      "  2Mbps",
				      "5.5Mbps",
				      " 11Mbps",
				      "  6Mbps",
				      "  9Mbps",
				      " 12Mbps",
				      " 18Mbps",
				      " 24Mbps",
				      " 36Mbps",
				      " 48Mbps",
				      " 54Mbps"};


void MultirateTracer::format(Packet *p, SAP *sap)
{

  MultiRateHdr* mrh = HDR_MULTIRATE(p);

  if((mrh->mode_ >=0) && (mrh->mode_ < NumPhyModes))
    {
      
      writeTrace(sap, " %s ",PhyModeStr[mrh->mode_]);
  
    }
}


extern "C" int Multiratetracer_Init()
{
  SAP::addTracer(new MultirateTracer);  
  return 0;
}
extern "C" int  Cygmultiratetracer_Init()
{
  Multiratetracer_Init();
}


