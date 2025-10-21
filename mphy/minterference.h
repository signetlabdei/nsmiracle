/* -*- Mode:C++ -*- */
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
 * thanks to: Roberto Petroccia for the PowerChunk interference model.
 * 
 */

#ifndef MINTERFERENCE_H 
#define MINTERFERENCE_H

#include <tclcl.h>
#include <stdlib.h>
#include <utility>
#include <list>

/// Power, time difference
typedef std::pair< double, double > PowerChunk;
typedef std::list< PowerChunk > PowerChunkList;

class Packet;

/** 
 * Class handling interference calculation. 
 *
 * It needs to be designed to handle all transmissions through
 * addToInterference(), and to be able to separate interferers from
 * signal in getInterferencePower()
 * 
 */
class MInterference : public TclObject
{  
public:

  MInterference() { }

  /** 
   * Add the transmission of this packet to the iterference perceived
   * by the PHY layer. 
   * 
   * @param p this packet
   */
  virtual void addToInterference(Packet* p) = 0;

 /** 
   * Computes the interference power that a given packet is subject
   * to. 
   * 
   * @param p the given packet
   * 
   * @return interference power in W
   */
   virtual double getInterferencePower(Packet* p) = 0;

   /** 
   * Computes the interference power chunks that a given packet is subject
   * to. 
   * 
   * @param p the given packet
   * 
   * @return interference power chunk list
   */	 
   virtual const PowerChunkList& getInterferencePowerChunkList(Packet* p) = 0;

   /** 
   * 
   * Returns the total power on the channel at time NOW
   * 
   * @return total power in W
   */
  virtual double getCurrentTotalPower() = 0;
};







#endif /* MINTERFERENCE_H */
