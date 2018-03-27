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

#include"umts-interference_miv.h"
#include"mphy.h"
#include <interference_miv.h>
#include"umts-headers.h"

#include<iostream>
#include <iomanip>


#define POWER_PRECISION_THRESHOLD (- 1e-14)

static class UmtsMInterferenceMIVClass : public TclClass {
public:
  UmtsMInterferenceMIVClass() : TclClass("MInterference/MIV/UMTS") {}
  TclObject* create(int, const char*const*) {
    return (new UmtsMInterferenceMIV);
  }
} class_umts_minterference_miv;


UmtsMInterferenceMIV::UmtsMInterferenceMIV()
  : MInterferenceMIV()
{
  bind("alpha_",&alpha_);
}

UmtsMInterferenceMIV::~UmtsMInterferenceMIV()
{

}





void UmtsMInterferenceMIV::addToInterference(Packet* p)
{
  hdr_MPhy*    ph = HDR_MPHY(p);
  hdr_umtsphy* uh = HDR_UMTSPHY(p);
  double interfpower = 0;
  
  if (uh->data==TRUE)
  {
    interfpower = ph->Pr;
      if (debug_ > 2)
	printf("UmtsMInterferenceMIV::addToInterference, DATA pkt -> add %e\n",interfpower);
    MInterferenceMIV::addToInterference(interfpower, NOW);

  }
  else
  {
    // packets without data interfer only a fraction of the whole power
    interfpower = ph->Pr*(alpha_/(1+alpha_));
    if (debug_ > 2)
      printf("UmtsMInterferenceMIV::addToInterference, CNTRL pkt -> add %e instead %e\n", interfpower, ph->Pr);
    MInterferenceMIV::addToInterference(interfpower, NOW);
	
  }

  PowerEvent* pe = new PowerEvent(interfpower);
  Scheduler::instance().schedule(&endinterftimer, pe, ph->duration);
  if (debug_)
    dump(std::string("UmtsMInterferenceMIV::addToInterference"));
}




double UmtsMInterferenceMIV::getInterferencePower(Packet* p)
{
  hdr_MPhy *ph = HDR_MPHY(p);
  hdr_umtsphy* uh = HDR_UMTSPHY(p);
  if (uh->data==TRUE) 
    return (MInterferenceMIV::getInterferencePower(ph->Pr, ph->rxtime, ph->duration));
  else
    return (MInterferenceMIV::getInterferencePower(0.0, ph->rxtime, ph->duration));
}




// double UmtsMInterferenceMIV::getInterferencePower(double power, double starttime, double duration)
// {
// 
//   Function::reverse_iterator rit; 
//   
//   double integral = 0;
//   double lasttime = NOW;
// 
//   assert(starttime<= NOW);
//   assert(duration > 0);
//   assert(maxinterval_ > duration);
// 
//   for (rit = pp.rbegin(); rit != pp.rend(); ++rit )
//     {
//       if (starttime < rit->time)
// 	{
// 	  integral += rit->value * (lasttime - rit->time);
// 	  lasttime = rit->time;	  
// 	}
//       else
// 	{
// 	  integral += rit->value * (lasttime - starttime);
// 	  break;
// 	}
//     }
// 
//   double interference = (integral/duration) - power;
// 
//   // Check for cancellation errors
//   // which can arise when interference is subtracted
//   if (interference < 0)
//     {
//       assert(interference > POWER_PRECISION_THRESHOLD); // should be a cancellation error
//       interference = 0;
//     }
// 
// 
//   if (debug_) {
//     dump("MInterferenceMIV::getInterferencePower");
//     std::cerr << "transmission from " << starttime 
// 	      << " to " << starttime + duration 
// 	      << " power " << power
// 	      << " gets interference " << interference
// 	      << std::endl;
//   }
// 
//   return interference;
// }
// 
// 
