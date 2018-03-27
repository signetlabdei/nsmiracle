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
 */

#include <cassert>
#include"ip-clmsg.h"

extern ClMessage_t IP_CLMSG_SEND_ADDR;
extern ClMessage_t IP_CLMSG_REQ_ADDR;
extern ClMessage_t IP_CLMSG_UPD_ROUTE;


//IPClMsgReqAddr::IPClMsgReqAddr(int src) : ClMessage(IP_CLMSG_VERBOSITY, IP_CLMSG_SEND_ADDR, BROADCAST, src, 0) {}	// DEPRECATED Method
IPClMsgReqAddr::IPClMsgReqAddr(int src) : ClMessage(IP_CLMSG_VERBOSITY, IP_CLMSG_SEND_ADDR, BROADCAST, 0) {}
IPClMsgReqAddr::IPClMsgReqAddr(IPClMsgReqAddr *m) : ClMessage(m) { }
ClMessage *IPClMsgReqAddr::copy() { return (new IPClMsgReqAddr(this)); }

IPClMsgSendAddr::IPClMsgSendAddr() : ClMessage(IP_CLMSG_VERBOSITY, IP_CLMSG_SEND_ADDR), addr_(0) {}
IPClMsgSendAddr::IPClMsgSendAddr(DestinationType dtype, int value) : 
	ClMessage(IP_CLMSG_VERBOSITY, IP_CLMSG_SEND_ADDR, dtype, value), 
	addr_(0) 
{}
IPClMsgSendAddr::IPClMsgSendAddr(IPClMsgSendAddr *m) : ClMessage(m), addr_(m->getAddr()) { }

void IPClMsgSendAddr::setAddr(nsaddr_t addr)
{
	addr_ = addr;
}

nsaddr_t IPClMsgSendAddr::getAddr()
{
	return(addr_);
}

ClMessage *IPClMsgSendAddr::copy()
{
	return (new IPClMsgSendAddr(this));
}


IpClMsgUpdRoute::IpClMsgUpdRoute( Packet* p )
: ClMessage(IP_CLMSG_UPD_ROUTE_VERBOSITY, IP_CLMSG_UPD_ROUTE),
  packet(p) 
{
}


ClMessage* IpClMsgUpdRoute::copy() {
  // Supporting only synchronous messages!!!
  assert(0);
}
