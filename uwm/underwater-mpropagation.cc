/* -*-	Mode:C++ -*- */

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


#include<node-core.h>
#include<mpropagation.h>

#include<iostream>

#include"underwater-mpropagation.h"
#include"uwlib.h"

static class UnderwaterMPropagationClass : public TclClass {
public:
  UnderwaterMPropagationClass() : TclClass("MPropagation/Underwater") {}
  TclObject* create(int, const char*const*) {
    return (new UnderwaterMPropagation);
  }
} class_UnderwaterMPropagation;


UnderwaterMPropagation::UnderwaterMPropagation()
{
  bind("windspeed_", &uw.windspeed);
  bind("shipping_", &uw.shipping);
  bind("practicalSpreading_", &uw.practical_spreading);
  bind("debug_", &debug_);
}


int UnderwaterMPropagation::command(int argc, const char*const* argv)
{

  return MPropagation::command(argc, argv);
}


double UnderwaterMPropagation::getGain(Packet* p)
{
   hdr_MPhy *ph = HDR_MPHY(p);

   Position* sp = ph->srcPosition;
   Position* rp = ph->dstPosition;

   MSpectralMask* sm = ph->srcSpectralMask;

   assert(sp);
   assert(rp);
   assert(sm);

   double freq = ph->srcSpectralMask->getFreq();
   double dist = sp->getDist(rp);

   //double AHgaindB = - uw.getAttenuation(dist, freq/1000.0);
   //double AHgain = pow(10.0, (AHgaindB / 10.0));
   //double k = 1.75; // practical spreading
   double PCgain = uwlib_AInv(dist/1000.0, uw.practical_spreading, freq/1000.0);

   if (debug_)
     cerr << NOW 
	  << " UnderwaterMPropagation::getGain()" 
	  << " dist=" << dist
	  << " freq=" << freq
       //<< " AHgain=" << AHgain 
	  << " gain=" << PCgain 
	  << endl;

   return (PCgain);

}
