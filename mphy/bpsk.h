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
 * 
 * thanks to: Roberto Petroccia for the channel sense extension
 * 
 */



#ifndef BPSK_H 
#define BPSK_H

#include <mphy-sense.h>
#include <map>

#define BPSK_MODNAME "BPSK"

class MPhy_Bpsk : public MPhyChSense 
{

 public:
  MPhy_Bpsk();
  virtual int getModulationType(Packet*);
  virtual double getTxDuration(Packet* p);
  virtual double getPER(double snr, int nbits);
  virtual int getDroppedPktsTxPending() { return droppedPktsTxPending; }

  virtual int command(int argc, const char* const* argv);

 protected:

  virtual void dropPacket(Packet* p);
  virtual void checkDropReason(Packet* p);

  virtual void startTx(Packet* p);
  virtual void endTx(Packet* p);
  virtual void startRx(Packet* p);
  virtual void endRx(Packet* p);

  virtual double getAcquisitionThreshold() { return AcquisitionThreshold_dB_; }

void incrDroppedPktsTxPending() { droppedPktsTxPending++; }
 
  void waitForUser();

  Packet* PktRx;  /// Packet the receiver is synchronized to for reception
  
  

  static bool initialized; /// used to register the modulation type only once
  static int modid;        /// modulation type id


  double BitRate_; /// user defined bitrate. if <= 0 ===> bitrate is calculated with standard formula
  
  bool txPending;

  map<double, map<double,int> > drp_rsn_map;
  
//   int countFailBER;

  int droppedPktsTxPending; /// Total number of dropped pkts due to tx pending

};


#endif /* BPSK_H */
