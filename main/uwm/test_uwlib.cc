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

#include"underwater.h"
#include"uwlib.h"
#include<iostream>
#include<math.h>

int main (int argc, char**argv)
{
  
  Underwater uw;
  double d;


  if (0)
    {

      // "AH" stands for Albert Harris code, "PC" stands for Paolo Casari code"
      std::cout << "distance(m) AH_f0(Hz) AH_BW(Hz) PC_f0(Hz) PC_BW(Hz)" << std::endl;

      for(d=100; d<10000; d=d+100)
	{
	  double fc,fl,fr;
	  uw.getBand(d, &fc, &fl, &fr);
	
	  std::cout << d
		    << " " << fc
		    << " " << fr-fl
		    << " " << uwlib_f0(d/1000.0)*1000
		    << " " << uwlib_BW(d/1000.0)*1000
		    << std::endl;
	}
    }
  
  if(0)
    {
      std::cout <<  10*log10(uwlib_Ptx_for_PER(5, 0.25, 1000, 0)) << std::endl;
    }

  if(1)
    {
      int ldata = 1000;
      double rxmargindb = 16;
      for(d=1000; d<10000; d=d+1000)
	{	  
	  for(double per=0.01; per<1; per=per+0.1)
	    {	      
	      double Ptx = uwlib_Ptx_for_PER(d/1000.0, per, ldata, rxmargindb);
	      double per2 =  uwlib_PER(d/1000.0, 10*log10(Ptx), ldata, rxmargindb);
	      std::cout << per << " " << per2 << std::endl;
	    }
	}
    }
 
    if(0)
    {      
      for(double f=1; f<60; f++)
	{
	  std::cout << f*1000
		    << " " << pow(10, 0.1 * uw.getNoise(f))
		    << " " << uwlib_Noise(uw.windspeed, uw.shipping, f)
		    << std::endl;
	}
	
    }


    if(0)
    {      
      for(double f=1; f<60; f++)
	{
	  std::cout << " " << f*1000 	       
		    << " " << uw.getThorp(f) // dB / m
		    << " " << uwlib_Thorp(f) // dB / km
		    << std::endl;
	}
      
    }


    if(0)
    {      
      double k=1.5;
      for(double d=1; d<10; d=d+1)
	for(double f=10; f<60; f=f+10)
	  {
	    std::cout << " " << f*1000 
		      << " " << d*1000
		      << " " << uw.getAttenuation(d*1000,f)
		      << " " << 10*log10(1/uwlib_AInv(d,k,f))
		      << std::endl;
	  }
      
    }

 
}
