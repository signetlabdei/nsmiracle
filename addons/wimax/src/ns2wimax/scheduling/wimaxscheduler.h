/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This software is based on the NIST NS2 WiMAX module, which can be found at
 * http://www.antd.nist.gov/seamlessandsecure/doc.html
 *
 * $Id: wimaxscheduler.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef WIMAX_SCHEDULER_H
#define WIMAX_SCHEDULER_H

#include "packet.h"
#include "mac802_16.h"
#include "framemap.h"
#include "../neighbordb.h"

#define TX_GAP 0.000001 //Time in seconds between sending 2 packets (to avoid collision)

/**
 * Super class for schedulers (BS and MS schedulers)
 */ 
class WimaxScheduler : public TclObject 
{
public:
  /*
   * Create a scheduler
   */
  WimaxScheduler ();

  /*
   * Set the mac
   * @param mac The Mac where it is located
   */
  void setMac (Mac802_16 *mac);

  /**
   * Initializes the scheduler
   */
  virtual void init ();

  /**
   * Return the Mac layer
   */
  inline Mac802_16 *  getMac () { return mac_;}
    
  /**
   * Transfert the packets from the given connection to the given burst
   * @param con The connection
   * @param b The burst
   * @param b_data The amount of data in burst
   * @return the new burst occupation
   */
  int transfer_packets (Connection *c, Burst *b, int b_data);

  /**
   * This function is used to schedule bursts/packets
   */
  virtual void schedule ();

protected:

  /**
   * The Mac layer
   */
  Mac802_16 * mac_;
  
private:



};
#endif //SCHEDULER_H

