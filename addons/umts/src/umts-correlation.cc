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

#include"umts-correlation.h"
#include"umts-phy.h"
#include"umts-headers.h"


#include<iostream>

static class UmtsCorrelationClass : public TclClass {
public:
  UmtsCorrelationClass() : TclClass("Correlation/UMTS") {}
  TclObject* create(int, const char*const*) {
    return (new Umts_Correlation);
  }
} class_Umts_Correlation;




Umts_Correlation::Umts_Correlation()
{
  bind("dl_orthogonality_",&dl_orthogonality_);
  //  bind("dl_interbs_",&dl_interbs_);
  //  bind("ul_interuser_",&ul_interuser_); 
  bind("code_id_", &code_id_);
  bind("debug_", &debug_);
}




double Umts_Correlation::getGain(Packet* p)
{

  hdr_umtsphy* uh = HDR_UMTSPHY(p);

  
  if (uh->direction == DIRECTION_DOWNLINK)
    {
      assert(code_id_>0); // Fails if not initialized

      // if (uh->bs_code_id_ != bs_code_id_)
      //  return dl_interbs_;
      // else 
      if (uh->me_code_id != code_id_)
	{
	  if (debug_) std::cerr << "Umts_Correlation::getGain(p) Packet is NOT for me ("
				<< code_id_ << ")" << std::endl;
	  return dl_orthogonality_;      
	}
      else 
	{
	  if (debug_) std::cerr << "Umts_Correlation::getGain(p) Packet is for me ("
				<< code_id_ << ")" << std::endl;
	  return 1; // Packet is for me 
	}
    }
  else
    { // uplink. Interuser interference is handled by MphyUmtsBS::endRx    
      return 1;  
    }
}




int Umts_Correlation::command(int argc, const char*const* argv) 
{



  return MCorrelation::command(argc, argv); 
}
