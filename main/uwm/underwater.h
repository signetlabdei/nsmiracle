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


#ifndef __underwaterprop_h__
#define __underwaterprop_h__


#define NUM_FREQ 600



class Underwater  {

  
  public:

    
  Underwater();
  
  virtual ~Underwater() { }


  /** 
   * Attenuation of acoustic signal in underwater channel
   * 
   * @param dist distance in m
   * @param freq freq in kHz
   * 
   * @return Attenuation in dB
   */
  virtual double getAttenuation(double dist, double freq);


  /** 
   * calculate Thorp's approximation for absorption at a given frequency 
   * 
   * @param freq the given frequency in kHz
   * 
   * @return the absorption factor in dB/m (dB re uPa)
   */
  virtual double getThorp(double freq);



  /** 
   * 
   * 
   * @param freq frequency in kHz
   * 
   * @return noise power in dB re uPa / Hz
   */
  virtual double getNoise(double freq);


  /** 
   * Calculate the effective frequency band with respect to the
   * given distance. The central frequency is the frequency
   * with the best Attenuation/Noise factor; the frequency band is
   * the 3dB band with respect to the central frequency. 
   * 
   * @param dist distance in m
   * @param cfreq the returned central frequency
   * @param lfreq the left edge of the band
   * @param hfreq the right edge of the band
   * 
   * @return 
   */
  virtual void getBand(double dist, double* cfreq, double* lfreq, double* rfreq);


  /** 
   * Calculate propagation delay
   * 
   * @param tX X coordinate of the transmitter
   * @param tY Y coordinate of the transmitter
   * @param tZ Z coordinate of the transmitter
   * @param rX X coordinate of the receiver 
   * @param rY Y coordinate of the receiver 
   * @param rZ Z coordinate of the receiver 
   * 
   * @return the propagation delay in seconds
   */
  virtual double getPropagationDelay(double tX, double tY, double tZ, double rX, double rY, double rZ);


  /** 
   * Calculate Capacity using Shannon's formula and the underwater
   * propagation model  implemented by this class
   * 
   * @param txpow txpower in uPa
   * @param dist  destination distance in m
   * @param fl left edge of the bandwidth used
   * @param fr right edge of the bandwidth used
   * @param df frequency resolution in Hz used for the calculations
   * 
   * @return the capacity in bit/s
   */
  virtual double getCapacity(double txpow, double dist, double fl, double fr, double df);

  double shipping; /**< Shipping factor in [0,1] */
  double windspeed; /**< Wind speed */
  double practical_spreading; /**< practical spreading */
  double prop_speed;
  
protected:

  double freq[NUM_FREQ]; /**< array of considered frequencies */

};


#endif /* __underwaterprop_h__ */
