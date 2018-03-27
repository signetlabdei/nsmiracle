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

#include"underwater-bpsk.h"
#include"underwater-mpropagation.h"
#include<rng.h>


#include <mac.h>
#include<position-clmsg.h>
#include<iostream>
#include"uwlib.h"

#define MAX(X,Y) ( (X) > (Y) ? (X) : (Y) ) 
#define MIN(X,Y) ( (X) > (Y) ? (Y) : (X) ) 

static class UWMPhyBpskClass : public TclClass {
public:
  UWMPhyBpskClass() : TclClass("Module/MPhy/BPSK/Underwater") {}
  TclObject* create(int, const char*const*) {
    return (new UnderwaterMPhyBpsk);
  }
} class_UWMPhyBpsk;



UnderwaterMPhyBpsk::UnderwaterMPhyBpsk()
{
  bind("MinTxSPL_dB_",&MinTxSPL_dB_);
  bind("MaxTxSPL_dB_",&MaxTxSPL_dB_);
  bind("PER_target_",&PER_target_);
  bind("RxSnrPenalty_dB_",&RxSnrPenalty_dB_);
  bind("TxSPLMargin_dB_",&TxSPLMargin_dB_);
  bind("ConsumedEnergy_",&ConsumedEnergy_);
  bind("MaxTxRange_", &MaxTxRange_);
  bind("SPLOptimization_", &SPLOptimization_);
  bind("CentralFreqOptimization_", &CentralFreqOptimization_);
  bind("BandwidthOptimization_", &BandwidthOptimization_);
  
  txsmask.setPropagationSpeed(1500); // actually the underwater-channel
				   // uses a more accurate model. Just in case... 

}


// double UnderwaterMPhyBpsk::consumedEnergy(Packet* p) {
//   hdr_MPhy* ph = HDR_MPHY(p);
//   return consumedEnergy(ph->Pt, ph->duration); 
// }


double UnderwaterMPhyBpsk::consumedEnergy(double Ptx, double duration) {
  // Parameters originally intended to be for the WHOI modem  
  const double uPa_to_W  =  6.3096e-18; // -172 dB
  const double TX_fixed = 0.0; // fixed TX consumption in Watt
  const double TX_efficiency = 0.25;  
  const double RX_fixed = 2.0; // fixed RX consumption in Watt

  double power = Ptx * uPa_to_W / TX_efficiency + TX_fixed + RX_fixed; // in Watt

  return (power * duration);
}


double UnderwaterMPhyBpsk::getNoisePower(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);

  assert(propagation_);

  // This assumes that the noise SPD is flat within the signal
  // bandwidth. Not really a very accurate model, of course you can do
  // better than this.

  double freq = ph->srcSpectralMask->getFreq();
  double bw = ph->srcSpectralMask->getBandwidth();

//   std::cerr << " txbw " << ph->srcSpectralMask->getBandwidth()
// 	    << " rxbw " << ph->dstSpectralMask->getBandwidth()
// 	    << " txf " << ph->srcSpectralMask->getFreq()
// 	    << " rxf " << ph->dstSpectralMask->getFreq()
// 	    << std::endl;

  UnderwaterMPropagation* uwmp = dynamic_cast<UnderwaterMPropagation*>(propagation_);
  assert(uwmp);
  double noiseSPDdBperHz = uwmp->uw.getNoise(freq/1000.0);
  double noisepow = bw * pow(10.0, noiseSPDdBperHz/10.0);

  if (debug_) std::cout << NOW << "  UnderwaterMPhyBpsk::getNoisePower() Pn=" << noisepow << std::endl;

  return (noisepow);

}

double UnderwaterMPhyBpsk::getTxDistance(Packet* p)
{
  // need cross-layer information here!!!  

  hdr_mac* mach = HDR_MAC(p);

  ClMsgPositionGetDist m( mach->macSA(), mach->macDA() );
  sendSyncClMsg(&m);

  if (debug_) ::std::cout << NOW << "  UnderwaterMPhyBpsk::getTxDistance() mac src " << mach->macSA() 
                     << "; mac rcv " << mach->macDA() << ::std::endl;
                          
                          
  if (m.isValid())
    {
      //std::cerr << __PRETTY_FUNCTION__ << " got valid distance: " << m.getDist() << std::endl;
      return m.getDist();
    }
  else
    {
      //std::cerr << __PRETTY_FUNCTION__ << " got invalid distance! "  << std::endl;
      return -1;  
    }
}


double UnderwaterMPhyBpsk::getTxPower(Packet* p)
{
  if (SPLOptimization_) {
  
    double dist = getTxDistance(p);
    //  cerr << __PRETTY_FUNCTION__ << "distance=" << dist << std::endl;
    if (dist<0)
      {
        // distance of destination is unknown, tx at max power
        return pow(10.0, MaxTxSPL_dB_ / 10.0);
      }
    else
      {
        hdr_cmn* ch = HDR_CMN(p);
        hdr_MPhy* ph = HDR_MPHY(p);
        
        double Ptx; 

        Ptx = uwlib_Ptx_for_PER_2(dist/1000.0, ph->srcSpectralMask->getFreq() / 1000.0, ph->srcSpectralMask->getBandwidth() / 1000.0, PER_target_, 8*ch->size(), RxSnrPenalty_dB_);  
 
        Ptx = Ptx * pow(10.0, TxSPLMargin_dB_ / 10.0);
      //sanity check
//       double per = uwlib_PER(dist/1000.0, 10*log10(Ptx), 8*ch->size(), RxSnrPenalty_dB_);
//         std::cerr << " per= " << per
//                   << " pertarget= " << PER_target_
//                   << " Ptx= " << Ptx <<"; Ptx db = " << (10*log10(Ptx)) 
//                   << std::endl;
                
        Ptx = MIN(Ptx,  pow(10.0, MaxTxSPL_dB_ / 10.0) );
        Ptx = MAX(Ptx,  pow(10.0, MinTxSPL_dB_ / 10.0) );

        if (debug_) std::cout << NOW << "  UnderwaterMPhyBpsk::getTxPower() optimized value = " 
                              <<  (10.0*log10(Ptx))  << "; for dist = " << dist << "; PER = " << PER_target_ 
                              << std::endl;
                
        return Ptx;
      }   
  }
  
  return pow(10.0, MaxTxSPL_dB_ / 10.0);
}

/* getRxSpectralMask removed by Filippo Campagnaro

MSpectralMask* UnderwaterMPhyBpsk::getRxSpectralMask(Packet* p)
{  
  if (PktRx == 0)
    {
      // this is the packet MPhyBpsk will synchronize onto.
      // we assume that the receiver uses a perfect RX filter
      hdr_MPhy *ph = HDR_MPHY(p);
      return ph->srcSpectralMask;
    }
  else
    {
      // for all interfering packets, we use the RX filter of the
      // packet we synchronized onto. 
      hdr_MPhy *ph = HDR_MPHY(PktRx);
      return ph->srcSpectralMask;
    }
  
}

*/

MSpectralMask* UnderwaterMPhyBpsk::getTxSpectralMask(Packet* p)
{  
  if ( CentralFreqOptimization_ || BandwidthOptimization_ ) {
  
    double dist = getTxDistance(p);
    if (dist<0) {
      // distance of destination is unknown, be conservative
      dist = MaxTxRange_;
    }

//     // This uses Albert's code
//     double fc,fl,fr,f2,flr;
//     assert(propagation_);
//     UnderwaterMPropagation* uwmp = dynamic_cast<UnderwaterMPropagation*>(propagation_);
//     assert(uwmp);
//     uwmp->uw.getBand(dist, &fc, &fl, &fr);
//     f2 = (fr+fl)/2;
//     flr = fr-fl;

//     This uses Paolo's code
    double f0,bw;
    if ( CentralFreqOptimization_ ) f0 = uwlib_f0(dist/1000.0)*1000.0;
    else f0 = spectralmask_->getFreq();
    if ( BandwidthOptimization_ ) bw = uwlib_BW(dist/1000.0)*1000.0;
    else bw = spectralmask_->getBandwidth();
//     std::cerr << NOW 
//               << " " << __PRETTY_FUNCTION__ 
//               << " dist= " << dist
//               << " fc= " << fc
//               << " f0= " << f0
//               << " f2= " << f2
//               << " flr= " << flr
//               << " bw= " <<  bw
//               << std::endl;

    hdr_mac* mach = HDR_MAC(p);

    if (debug_) std::cout << NOW << "  UnderwaterMPhyBpsk::getTxSpectralMask() optimized f0 = " << f0 
                          << "; optimized bandwith = " << bw << "; for mac src = " << mach->macSA() 
                          << "; mac rcv = " << mach->macDA() << "; dist = " << dist << std::endl;

    txsmask.setFreq(f0);
    txsmask.setBandwidth(bw);
    return &txsmask;
  }
  
  return spectralmask_;
}

double UnderwaterMPhyBpsk::getNoiseChannel()
{
  assert(propagation_);

  // This assumes that the noise SPD is flat within the signal
  // bandwidth. Not really a very accurate model, of course you can do
  // better than this.

  double freq = spectralmask_->getFreq();
  double bw = spectralmask_->getBandwidth();

  if (debug_) std::cout << NOW << "  UnderwaterMPhyBpsk::getNoiseChannel() freq=" 
                        << freq << " [Hz]; bandwith=" << bw << " [Hz]" << std::endl;

  UnderwaterMPropagation* uwmp = dynamic_cast<UnderwaterMPropagation*>(propagation_);
  assert(uwmp);
  
  double noiseSPDdBperHz = uwmp->uw.getNoise(freq/1000.0);
  double noisepow = bw * pow(10.0, noiseSPDdBperHz/10.0);

  if (debug_) std::cout << NOW << "  UnderwaterMPhyBpsk::getNoiseChannel() Pn=" << noisepow << std::endl;

  return (noisepow);  
}

double UnderwaterMPhyBpsk::getPER(double snr, int nbits)
{
  // the formula is the same for ordinary BPSK, except that we account
  // for a penalty on SNR to account for non-idealities at the
  // receiver

  double snr_with_penalty = snr * pow(10.0, RxSnrPenalty_dB_/10.0);
 
//   cout << "snr_with_penalty = " << snr_with_penalty << "; RxSnrPenalty_dB_ = " << RxSnrPenalty_dB_ << endl;
//   waitForUser(); 
  return MPhy_Bpsk::getPER(snr_with_penalty, nbits);

}

double UnderwaterMPhyBpsk::consumedEnergyTx(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);
  return consumedEnergyTx(ph->Pt, ph->duration);
}

double UnderwaterMPhyBpsk::consumedEnergyTx(double Ptx, double duration)
{
  // Parameters originally intended to be for the WHOI modem  
  const double uPa_to_W  =  6.3096e-18; // -172 dB
  const double TX_fixed = 0.0; // fixed TX consumption in Watt
  const double TX_efficiency = 0.25;  
  const double RX_fixed = 2.0; // fixed RX consumption in Watt

  double power = Ptx * uPa_to_W / TX_efficiency + TX_fixed + RX_fixed; // in Watt

  return (power * duration);
}


void UnderwaterMPhyBpsk::endTx(Packet* p)
{
  ConsumedEnergy_ += consumedEnergyTx(p);
  MPhy_Bpsk::endTx(p);
}


