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

#ifndef INTERFERENCE_MIV_UMTS
#define INTERFERENCE_MIV_UMTS

#include <interference_miv.h>

/**
 * Calculate interference using Mean Integral Value in UMTS Phy,
 * Mphy version sligtly modified to manage signalling packets (i.e.,
 * they interfere only as a fraction of the transmission power)
 * 
 */
class UmtsMInterferenceMIV : public MInterferenceMIV
{
  friend class EndInterferenceMIVTimer;
 public:
  UmtsMInterferenceMIV();
  ~UmtsMInterferenceMIV();
  void addToInterference(Packet* p);
  //void addToInterference(double power, double starttime); 
  double getInterferencePower(Packet* p);
  //double getInterferencePower(double power, double starttime, double duration);

private:
  double alpha_;		/// alpha_ is used to compute the signal power used for the data from the whole one
						/// which is comprehensive of data and cotrol, it's value is 49/225
};


#endif 	// INTERFERENCE_MIV_UMTS
