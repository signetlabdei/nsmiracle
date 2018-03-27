/* -*-	Mode:C++; -*- */

/* 
 * Underwater NS2 library
 * Original version: Al Harris, Robin S. 
 * 
 * Modifications Nov 2007 by Nicola Baldo (baldo@dei.unipd.it):
 * turned into a general purpose class, so that it can be used with
 * NS-Miracle, in particular with the MPhy framework
 */

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

#include<cmath>
#include<cstdio>
#include<iostream>
#include<cassert>


#include "underwater.h"


Underwater::Underwater()
: windspeed(0.0),
  shipping(0.5),
  practical_spreading(1.75),
  prop_speed(1500.0)
{
  // fill the table of frequencies... I suppose this could be done
  // offline... also some tunability would be nice... right now
  // we search between 0 and 60 kHz which is reasonable for 
  // acoustic devices. Note that freq[0] > 0
  for(int i = 0; i < NUM_FREQ; i++)
    {
      freq[i] = (i+1) * (60.0/NUM_FREQ);
    }
  
}


double Underwater::getAttenuation(double dist, double freq)
{
  //double k = 1.75;	// practical spreading
  
  // NB: what if dist < 1m??? 
  if (dist > 0)    
    {
      double att =  (practical_spreading * 10.0 * log10(dist) + dist * getThorp(freq) );
      if (att > 1.0) 
	return att;
      else 
	return 1.0;
    }
  else 
    return (1.0);
}



void Underwater::getBand(double dist, double* cfreq, double* lfreq, double* rfreq)
{
  double AN[NUM_FREQ];
  double max_AN = -1e100; /* -infinity */
  int max_index;
  int edge_index;

  // calculate the 1/AN factor vector 
  for(int i = 0; i < NUM_FREQ; i++)
    {
      AN[i] = - (getAttenuation(dist, freq[i]) + getNoise(freq[i]));
      // also, track the best AN factor
      if(AN[i] > max_AN)
	{
          max_index = i;
	  max_AN = AN[max_index];
	}
    }
  
  //  printf("max_index=%d freq[max_index]=%lf dist=%lf\n", max_index, freq[max_index], dist);
  *cfreq = freq[max_index] * 1000.0;
 
  //find the edges of the usable band... use 3dB def for 1/AN

  //right edge
  edge_index = max_index;
  while((edge_index < NUM_FREQ - 1) && (max_AN - AN[edge_index] <= 3.0))
    {
      assert(AN[edge_index] <= max_AN);
      edge_index++;
    }
  *rfreq = freq[edge_index] * 1000.0;

  //left edge
  edge_index = max_index;
  while((edge_index >= 1) && (max_AN - AN[edge_index] <= 3.0))
    {
      assert(AN[edge_index] <= max_AN);
      edge_index--;
    }
  *lfreq = freq[edge_index] * 1000.0;
}

double Underwater::getThorp(double f)
{
  double atten;
  double f2;
  f2 = pow(f,2);
  if(f > 0.4)
    atten = 0.11 * f2 / (1 + f2) +
      44 * (f2 / (4100 + f2)) +
      2.75e-4 * f2 +
      0.003;
  else
    atten = 0.002 +
      0.11 * (f2 / (1 + f2)) +
      0.011 * f2;

  return atten/1000;
}


double Underwater::getNoise(double f)
{
    double s; // must be in [0,1]
    double w; // wind speed in m/s (should be positive)
    double turbulence;
    double ship;
    double wind;
    double thermal;
    double n;

    s = shipping;
    w = windspeed;

    if(s < 0)
    {
        s = 0;
        fprintf(stderr,
                "shipping factor must be positive, it is set to zero\n");
    }
    if(s > 1)
    {
        s = 1;
        fprintf(stderr,
                "shipping factor must be <= 1, it is set to one\n");
    }
    if(w < 0)
    {
        w = 0;
        fprintf(stderr,
                "wind speed must be positive, it is set to zero\n");
    }

    turbulence = 17 - 30 * log10(f);
    turbulence = pow(10.0,(turbulence * 0.1));
    ship = 40 +
            20 * (s - 0.5) +
            26 * log10(f) -
            60 * log10(f + 0.03);
    ship = pow(10.0,(ship * 0.1));
    wind = 50 +
            7.5 * pow(w,0.5) + 20 * log10(f) -
            40 * log10(f + 0.4);
    wind = pow(10.0,(wind * 0.1));
    thermal = -15 + 20 * log10(f);
    thermal = pow(10.0,(thermal * 0.1));
    n = 10 * log10(turbulence + ship + wind + thermal);
    return n;
}


double Underwater::getPropagationDelay(double tX, double tY, double tZ, double rX, double rY, double rZ)
{
  // I think this may be overkill... actually as long as the paths are not
  // straight along the z-axis, just using 1500 m/s is probably
  // sufficient... on the other hand, this doesn't really add to much time
  // to the simulation compared to the bandwidth calculation


  double T; 
  // temp in C
  // from 1000m to 4500 m, the temp is in [2,4]
  // from 750m to 1000m [4,8]
  // from 250 to 750 [8,22]
  // 0 - 250 it is 22C
  
//   rZ = ::std::abs(rZ);
//   tZ = ::std::abs(tZ);
  
  double t, z, S; // temp comp, depth, salinity
  double lZ, hZ;
  double prop_delay = 0;
  double dist;
  double totalZ;

  double total_dist = sqrt((rX - tX) * (rX - tX) 
                  + (rY - tY) * (rY - tY) 
                  + (rZ - tZ) * (rZ - tZ));
         
  prop_delay = total_dist / 1500.0;
  
	S = 35; // for the ocean, it is in [32,37] with average at 35

	lZ = tZ > rZ?tZ:rZ;
	hZ = tZ < rZ?tZ:rZ; 

	totalZ = lZ - hZ;
	if(totalZ <= 0)
	{
			z = lZ;
			//put a temp
			T = z>4500?2.0:lZ>1000?((4500 - z) * 0.00057142) + 2.0:
					z>750?((1000 - z) * 0.016) + 4.0:
					z>250?((750 - z) * 0.028) + 8:22.0;
			t = T/10.0;
			prop_delay += (total_dist) / 
					(1449.05 + 45.7 * t - 5.21 * pow(t,2) +
					0.23 * pow(t,3) +
					(1.333 - 0.126 * t + 0.009 * pow(t,2)) * (S - 35) +
					16.3 * (z / 1000.0) + 0.18 * pow((z / 1000),2));

	}
	else
	{
		if(lZ > 4500 && lZ > hZ)
		{
				dist = lZ - (hZ > 4500?4500:hZ);
				z = lZ - dist / 2;
				lZ -= dist;
				T = 2.0;
				t = T/10.0;
				prop_delay += (total_dist * dist / totalZ) / 
						(1449.05 + 45.7 * t - 5.21 * pow(t,2) +
						0.23 * pow(t,3) +
						(1.333 - 0.126 * t + 0.009 * pow(t,2)) * (S - 35) +
						16.3 * (z / 1000.0) + 0.18 * pow((z / 1000),2));
		}
		if(lZ > 1000 && lZ > hZ)
		{
			dist = lZ - (hZ > 1000?1000:hZ);
			z = lZ - dist / 2;
			lZ -= dist;
			T = ((4500 - z) * 0.00057142) + 2.0; 
			t = T/10.0;
			prop_delay += (total_dist * dist / totalZ) / 
					(1449.05 + 45.7 * t - 5.21 * pow(t,2) +
					0.23 * pow(t,3) +
					(1.333 - 0.126 * t + 0.009 * pow(t,2)) * (S - 35) +
					16.3 * (z / 1000.0) + 0.18 * pow((z / 1000),2));
		}	
		else if(lZ > 750)
		{
			dist = lZ - (hZ > 750?750:hZ);
			z = lZ - dist / 2;
			lZ -= dist;
			T = ((1000 - z) * 0.016) + 4.0;
			t = T/10.0;
			prop_delay += (total_dist * dist / totalZ) / 
					(1449.05 + 45.7 * t - 5.21 * pow(t,2) +
					0.23 * pow(t,3) +
					(1.333 - 0.126 * t + 0.009 * pow(t,2)) * (S - 35) +
					16.3 * (z / 1000.0) + 0.18 * pow((z / 1000),2));
		}
		else if(lZ > 250)
		{
			dist = lZ - (hZ > 250?250:hZ);
			z = lZ - dist / 2;
			lZ -= dist;
			T = ((750 - z) * 0.028) + 8;
			t = T/10.0;
			prop_delay += (total_dist * dist / totalZ) / 
					(1449.05 + 45.7 * t - 5.21 * pow(t,2) +
					0.23 * pow(t,3) +
					(1.333 - 0.126 * t + 0.009 * pow(t,2)) * (S - 35) +
					16.3 * (z / 1000.0) + 0.18 * pow((z / 1000),2));
		}
		else
		{
			dist = lZ - (hZ > 750?750:hZ);
			z = lZ - dist / 2;
			lZ -= dist;
			t = 22/10.0;
			prop_delay += (total_dist * dist / totalZ) / 
					(1449.05 + 45.7 * t - 5.21 * pow(t,2) +
					0.23 * pow(t,3) +
					(1.333 - 0.126 * t + 0.009 * pow(t,2)) * (S - 35) +
					16.3 * (z / 1000.0) + 0.18 * pow((z / 1000),2));
		}
	}

//   printf("Total_dist: %f, prop_delay: %f, def_prop_delay: %f, speed: %f \n",total_dist, prop_delay, 
//          total_dist/1500.0, total_dist / prop_delay );

//   ::std::cout << " speed = " << prop_speed << ::std::endl;

  if (prop_speed > 0.0)
  {
    prop_delay = total_dist/prop_speed;
  }

//   ::std::cout << "Tx = " << tX << " Ty = " << tY << " Tz = " << tZ << " Rx = " << rX 
//               << " Ry = " << rY << " Rz = "<< rZ << " dist = " << total_dist 
//               << " delay = " << prop_delay << " speed = " << (total_dist / prop_delay) 
//               << " delay std = " << (total_dist/1500.0) << " delay l = " << (total_dist / 3.0e8) << ::std::endl;
              
 	return prop_delay;
}



double Underwater::getCapacity(double txpow, double dist, double fl, double fr, double df)
{
   double C = 0;
   double bw = fr -fl;   
   assert(bw>0);
   //  double TxPowPerHz = txpow/bw;

   for (double f = fl; f < fr; f = f + df)
     {
       double attdB = getAttenuation(dist, f/1000.0);
       double S = txpow / (pow(10, attdB/10.0)) ; // per Hz
       double noiseSPDdBperHz = getNoise(f/1000.0);  
       double N =  pow(10, noiseSPDdBperHz/10.0);      // per Hz

       C += log2(1+S/N) * df; 
       
     }
  
   return C;
  
}

