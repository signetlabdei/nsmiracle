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

#include <ip.h>

#include "ip-interface.h"

#define NEXT_HOP_UNREACHABLE_DEPHT  5
#define NEXT_HOP_UNREACHABLE_REASON "NHU"

static class IPInterfaceModuleClass : public TclClass {
public:
  IPInterfaceModuleClass() : TclClass("Module/IP/Interface") {}
  TclObject* create(int, const char*const*) {
    return (new IPInterfaceModule);

  }
} class_ipinterface_module;


IPInterfaceModule::IPInterfaceModule()
{
}

IPInterfaceModule::~IPInterfaceModule()
{
}


void IPInterfaceModule::recv(Packet *p)
{
  hdr_ip *iph = HDR_IP(p);
  hdr_cmn *ch = HDR_CMN(p);

  if(ch->direction() == hdr_cmn::UP)
    {

       if (iph->daddr() == ipAddr_ )
      {
        // I am the final destination
        // Unfortunately current IpRouting implementation is not
        // aware of the IP address of each underlying interface, so
        // it can only see if the destination is equal to the next
        // hop. For this reason, the IP Interface module must
        // explicitly perform the following operation to let
        // IpRouting do the right thing(TM).
        ch->next_hop_ = iph->daddr();	   
      }

      if  ( (ch->next_hop_ == ipAddr_ )
          || (iph->daddr() == ipAddr_ ) )
      { // I am either the next hop or the destination,
        // so I'll forward this packet up
        sendUp(p);
      }
      else
	{ // I am NOT the next hop	  
	  drop(p, NOT_FOR_ME_DEPTH, NOT_FOR_ME_REASON);
	}
    }
  else
    { /* direction DOWN */

      // If destination is local, the next_hop_ is the destination
      // NO! This is BAD! Think about it! How can you do ad-hoc routing then?      
      // if((ipAddr_ & subnet_) == (iph->daddr() & subnet_))
      //   ch->next_hop_ = iph->daddr();

      /* Check if next hop can be reached through this interface */
      if ( ! ((ch->next_hop_ & subnet_) == (ipAddr_ & subnet_)))
	{
	  drop(p,NEXT_HOP_UNREACHABLE_DEPHT, NEXT_HOP_UNREACHABLE_REASON);
	}
      else
	{ // next hop is reachable, packet will be sent through this interface	 


	  // Set source IP ONLY IF it was not set before 
	  // (0.0.0.0 is not a valid address)
	  if (iph->saddr() == 0)
	    iph->saddr() = ipAddr_;

	  // Mark this IP interface as the previous hop
	  ch-> prev_hop_ = ipAddr_;
	
	  sendDown(p);
	}
    }
}
