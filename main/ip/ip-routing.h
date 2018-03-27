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

#ifndef IP_ROUTING_H
#define IP_ROUTING_H

#include "ipmodule.h"

#define IP_ROUTING_MAX_ROUTES 100


/**
 * @brief Basic IP routing functionality 
 *
 * This class implements tcl class Module/IP/Routing and is expected
 * to be installed above one or more Module/IP/Interface classes.
 *
 * @note Inheritance from IPModule is actually needed only for the
 * str2add() method et similia. Other functionality from the IPModule
 * is not needed in IPRoutingModule, and all methods are overloaded.
 * In particular, it should be remembered that an IPRoutingModule 
 * does *NOT* have its own IP address.
*/
class IPRoutingModule : public IPModule {
public:
  IPRoutingModule();
  virtual ~IPRoutingModule();
  virtual void recv(Packet *p);
  virtual int recvSyncClMsg(ClMessage* m);

  virtual int command(int argc, const char*const* argv);


  /*
   * returns address of the next hop, 
   * or returns 0 if destination is unreachable
   * 
   */
  virtual nsaddr_t GetNextHop(Packet *p);

  virtual nsaddr_t GetNextHop(nsaddr_t dst);

  virtual void clearRoutes();
  
  virtual void addRoute(nsaddr_t dst, nsaddr_t mask, nsaddr_t next);

protected:
  
  nsaddr_t destination[IP_ROUTING_MAX_ROUTES];
  nsaddr_t subnetmask[IP_ROUTING_MAX_ROUTES];
  nsaddr_t nexthop[IP_ROUTING_MAX_ROUTES];

  int nroutes;
};

#endif  /*  IP_ROUTING_H */
