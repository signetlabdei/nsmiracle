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

/* -*-	Mode:C++ -*- */

#ifndef MAC_MODULE_H
#define MAC_MODULE_H

#include<module.h>
#include<packet.h>
#include<rlc.h>
#include<scheduler.h>

#include<queue>

#define CNTRL_PKT_SIZE 1

class UmtsMacModule;

class UmtsMacTimer : public Handler {

public:
  UmtsMacTimer(UmtsMacModule* ptr) : mac(ptr), active(false) {}
  void handle(Event *e);
  bool active;

protected:
  UmtsMacModule* mac;
};


class UmtsMacModule : public Module
{
  
public: 
  UmtsMacModule();
  ~UmtsMacModule();
  void recv (Packet *p);
  int command (int argc, const char *const *argv);
  virtual void schedule(Packet* p);
  virtual void transmit();

protected:

  int src_rlc_id_;      /// id of the RLC connected above this MAC
			/// (if this MAC is at a BS, the upper RLC
			/// entity might be in the RNC and connected
			/// to this MAC by means of the Iub interface)

  
  int dst_rlc_id_;      /// the id of the other RLC instance connected
			/// to the RLC above this MAC
  int me_code_id_;		/// the code identifier of the PHY
			/// corresponding to dst_rlc_id. Note that it
			/// is used only in the core network to
			/// determine which is the destination. It is
			/// not de facto used at Mobile Equipments
			/// since all base station attempt to receive
			/// all incoming transmissions anyway.

  double slot_duration_; /// duration of a slot within a TTI
  int    TTI_PDUs_;      /// number of slots in a TTI_
  double TTI_;           /// TTI duration
  double interleaving_;  /// interleaving duration. In practice, delay
			 /// between MAC and PHY


  std::queue<Packet*> Q;   /// MAC queue used for packet scheduling 

  UmtsMacTimer tx_timer; /// Timer used to schedule transmissions

  Event        tx_event; /// Almost unused, it is only required by the
			 /// interface... maybe using a NULL pointer
			 /// instead? 

  double initslotoffset_;       /// Offset which is given to slot boundaries
		  	 /// at the beginning of a simulation. Useful
			 /// to change synchronization between UL and
			 /// DL transmissions (e.g., power control
			 /// works better if they're not in phase but
			 /// rather alternating)

};





#endif /* MAC_MODULE_H */
