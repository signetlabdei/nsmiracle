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


/**
 * @file   underwater-bpsk.h
 * @author Nicola Baldo
 * @date   Tue Nov 27 16:18:04 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef UNDERWATER_BPSK_H 
#define UNDERWATER_BPSK_H

#include<mphy.h>
#include<node-core.h>
#include<rect_spectral_mask.h>
#include<bpsk.h>

#define UWMPHYBPSK_MODNAME "UWBPSK"

class UnderwaterMPhyBpsk :  public MPhy_Bpsk
{
public:
  UnderwaterMPhyBpsk();
  virtual ~UnderwaterMPhyBpsk() { }
 
//   static double consumedEnergy(Packet* p);
  static double consumedEnergy(double Ptx, double duration);
 
  protected:
  virtual double getNoisePower(Packet* p);
  virtual double getTxPower(Packet* p);
  virtual MSpectralMask* getTxSpectralMask(Packet* p);

  // getRxSpectralMask removed by Filippo Campagnaro
  //virtual MSpectralMask* getRxSpectralMask(Packet* p);

  virtual double getTxDistance(Packet* p);
  virtual double getPER(double snr, int nbits);
  virtual void endTx(Packet* p);

  virtual double consumedEnergyTx(Packet* p);
  virtual double consumedEnergyTx(double Ptx, double duration);
  
  virtual double getNoiseChannel();
  
  double MaxTxSPL_dB_;
  double MinTxSPL_dB_;
  double PER_target_;
  double RxSnrPenalty_dB_;
  double TxSPLMargin_dB_;
  double ConsumedEnergy_;
  double MaxTxRange_;
  double CentralFreqOptimization_;
  double BandwidthOptimization_;
  double SPLOptimization_;

  RectSpectralMask txsmask;
  MSpectralMask*   rxsmask;
};



#endif /* UNDERWATER_BPSK_H */



