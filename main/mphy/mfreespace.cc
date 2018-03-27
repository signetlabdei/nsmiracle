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

/* -*-	Mode:C++ -*- */

#include<node-core.h>
#include"mfreespace.h"
#include"mphy.h"


static class MFreeSpaceClass : public TclClass {
public:
  MFreeSpaceClass() : TclClass("MPropagation/MFreeSpace") {}
  TclObject* create(int, const char*const*) {
    return (new MFreeSpace);
  }
} class_MFreeSpace;



int MFreeSpace::command(int argc, const char*const* argv)
{

  return MPropagation::command(argc, argv);
}


double MFreeSpace::getGain(Packet* p)
{
   hdr_MPhy *ph = HDR_MPHY(p);

   Position* sp = ph->srcPosition;
   Position* rp = ph->dstPosition;

   MSpectralMask* sm = ph->srcSpectralMask;

   assert(sp);
   assert(rp);
   assert(sm);

   double lambda = ph->srcSpectralMask->getLambda();
   double d = sp->getDist(rp);
   if (d>0) 
     {
       double g = lambda / (4 * M_PI * d);
       if (g<1)
	 return  g*g;
       else 
	 return 1;
     }
   else 
     return 1;

}
