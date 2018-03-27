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
 * @file   underwater-mphy.cc
 * @author Nicola Baldo
 * @date   Tue Nov 27 16:15:23 2007
 * 
 * @brief  
 * 
 * 
 */



#include"underwater-mphy.h"
#include"underwater-mpropagation.h"



double UnderwaterMPhy::getNoisePower(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);

  assert(propagation_);

  // This assumes that the noise SPD is flat within the signal
  // bandwidth. Not really a very accurate model, of course you can do
  // better than this.

  double freq = ph->srcSpectralMask->getFreq();
  double bw = ph->srcSpectralMask->getBandwidth();

  UnderwaterMPropagation* uwmp = dynamic_cast<UnderwaterMPropagation*>(propagation_);
  assert(uwmp);
  double noiseSPDdBperHz = uwmp->uw.getNoise(freq/1000.0);
  double noiseSPD =  pow(10, noiseSPDdBperHz/10.0);
  
  return (noiseSPD*bw);

}














