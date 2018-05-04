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

#include"rect_spectral_mask.h"

#include<stdlib.h>
#include <assert.h>
#include<algorithm>

static class RectSpectralMaskClass : public TclClass {
public:
  RectSpectralMaskClass() : TclClass("MSpectralMask/Rect") {}
  TclObject* create(int, const char*const*) { return (new RectSpectralMask);  }
} class_Rect_Spectral_Mask;



RectSpectralMask::RectSpectralMask()
  : freq_(2.437e3),
    bandwidth_(22e6),
    c_(3e8)
{

}


double RectSpectralMask::getFreq() 
{
  return freq_;
}

void RectSpectralMask::setFreq(double f)
{
  freq_ = f;
}

double RectSpectralMask::getPropagationSpeed()
{
  return c_;
}

void RectSpectralMask::setPropagationSpeed(double s)
{
  c_ = s;
}

double RectSpectralMask::getLambda() 
{
  return (c_ / freq_);
}

double RectSpectralMask::getBandwidth() 
{
  return bandwidth_;
}

void RectSpectralMask::setBandwidth(double b)
{
  bandwidth_ = b;
}



int RectSpectralMask::command(int argc, const char*const* argv)
{
  //printf("MPhy::command -- %s (%d)\n", argv[1], argc);
  Tcl& tcl = Tcl::instance();

  if(argc == 2) {
    if(strcasecmp(argv[1], "getFreq")==0)
    {
      tcl.resultf("%f",getFreq());
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "getPropagationSpeed")==0)
    {
      tcl.resultf("%f",getPropagationSpeed());
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "getLambda")==0)
    {
      tcl.resultf("%f",getLambda());
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "getBandwidth")==0)
    {
      tcl.resultf("%f",getBandwidth());
      return TCL_OK;
    }

  }
  else if(argc == 3) {
    if(strcasecmp(argv[1], "setFreq")==0)
    {
      setFreq(atof(argv[2]));
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "setPropagationSpeed")==0)
    {
      setPropagationSpeed(atof(argv[2]));
      return TCL_OK;
    }
    else if(strcasecmp(argv[1], "setBandwidth")==0)
    {
      setBandwidth(atof(argv[2]));
      return TCL_OK;
    }
  }
  return MSpectralMask::command(argc, argv);
}


double RectSpectralMask::getOverlap(MSpectralMask* msm, Packet*)
{
  // we perform a dynamic cast so we can check if the object is of the correct type
  RectSpectralMask* tbm = dynamic_cast<RectSpectralMask *>(msm);
  assert(tbm);

  double hbw = bandwidth_/2;
  double hbw2 = (tbm->getBandwidth())/2;

  double overlap = std::min(tbm->getFreq() + hbw2, freq_ + hbw) - std::max(tbm->getFreq() - hbw2, freq_ - hbw);
  
  if (overlap > 0)
    return (overlap/bandwidth_);
  else
    return 0;

}
