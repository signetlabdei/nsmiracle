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

#ifndef MSPECTRALMASK_H 
#define MSPECTRALMASK_H

#include<tclcl.h>

class Packet;

/**
 * Spectral mask of the transmitted signal
 * Used to calculate interference, propagation, etc.
 * 
 */
class MSpectralMask : public TclObject
{  
  
public: 

  virtual double getFreq() = 0;
  virtual void setFreq(double f) = 0;
  virtual double getPropagationSpeed() = 0;
  virtual void setPropagationSpeed(double s) = 0;
  virtual double getLambda() = 0;
  virtual double getBandwidth() = 0;
  virtual void setBandwidth(double b) = 0;

  /** 
   * TCL command interpreter
   * 
   * @param argc 
   * @param argv 
   * 
   * @return 
   */
  virtual int command(int argc, const char*const* argv) {return TclObject::command(argc, argv); }

  /** 
   * Calculates how much current mask (cm) overlaps with a target mask (tm)
   * 
   * @param tm target mask
   * 
   * @return (integral cm(f) * tm(f) df) / (integral cm(f) df)
   */
  virtual double getOverlap(MSpectralMask* tm, Packet* p) = 0;

};



#endif /* MSPECTRALMASK_H */

