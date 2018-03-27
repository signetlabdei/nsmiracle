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
 * @file   underwater-shannon.h
 * @author Nicola Baldo
 * @date   Tue Nov 27 16:16:40 2007
 * 
 * @brief  
 * 
 * 
 */


#ifndef UNDERWATER_SHANNON_H 
#define UNDERWATER_SHANNON_H



#include<mphy.h>



#define UWMPHYSHANNON_MODNAME "UWSHAN"



class UnderwaterMPhyShannon : public MPhy  
{
public:
  UnderwaterMPhyShannon();
  virtual ~UnderwaterMPhyShannon() { }
  
  int getModulationType(Packet*);
  double getTxDuration(Packet* p);
  double getNoisePower(Packet* p);
  int command(int argc, const char*const* argv);

//   MSpectralMask* getTxSpectralMask(Packet* p);
//   MSpectralMask* getRxSpectralMask(Packet* p);

 protected:

  void startTx(Packet* p);
  void endTx(Packet* p);
  void startRx(Packet* p);
  void endRx(Packet* p);


  void calculateCapacity(Packet* p);

  Packet* PktRx;    /// Packet the receiver is synchronized to for reception

  double capacity; 

  bool txready; /* true if shannon parameters have been calculated for
		   current transmission, false otherwise */
  
  Position* destPos_; 
 
  static bool initialized; /// used to register the modulation type only once
  static int modid;        /// modulation type id

  /** The parameters used for the most recent call to uw.getCapacity()  */
  double last_fl;
  double last_fr;
  double last_dist;
  double last_txpow;
  
  double last_Pr;
  double last_Pn;

};









#endif /* UNDERWATER_SHANNON_H */

