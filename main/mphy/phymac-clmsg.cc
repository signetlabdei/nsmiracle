
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
 *  thanks to: Roberto Petroccia for the channel sense extension
 */


/**
 * @file   phymac-clmsg.cc
 * @author Baldo Nicola
 * @date   Mon Nov 26 14:59:38 2007
 * 
 * @brief  
 * 
 * 
 */



#include "phymac-clmsg.h"


ClMessage_t CLMSG_PHY2MAC_ENDTX;
ClMessage_t CLMSG_PHY2MAC_STARTRX;
ClMessage_t CLMSG_PHY2MAC_CCA;
ClMessage_t CLMSG_MAC2PHY_GETTXDURATION;
ClMessage_t CLMSG_PHY2MAC_ADDR;
ClMessage_t CLMSG_MAC2PHY_GETISIDLE;

ClMsgPhy2MacEndTx::ClMsgPhy2MacEndTx(const Packet* p)
  : pkt(p),
    ClMessage(CLMSG_PHY2MAC_VERBOSITY, CLMSG_PHY2MAC_ENDTX)
{
}

ClMessage* ClMsgPhy2MacEndTx::copy()
{
  // Supporting only synchronous messages!!!
  assert(0);
}




ClMsgPhy2MacStartRx::ClMsgPhy2MacStartRx(const Packet* p)
  : pkt(p),
    ClMessage(CLMSG_PHY2MAC_VERBOSITY, CLMSG_PHY2MAC_STARTRX)
{
}

ClMessage* ClMsgPhy2MacStartRx::copy()
{
  // Supporting only synchronous messages!!!
  assert(0);
}



ClMsgPhy2MacCCA::ClMsgPhy2MacCCA(bool new_cca)
  : CCA(new_cca),
    ClMessage(CLMSG_PHY2MAC_VERBOSITY, CLMSG_PHY2MAC_CCA)
{
}

ClMessage* ClMsgPhy2MacCCA::copy()
{
  // Supporting only synchronous messages!!!
  assert(0);
}

// messaggi cross-layer mac2phy

//pkt tx duration 
ClMsgMac2PhyGetTxDuration::ClMsgMac2PhyGetTxDuration(Packet* p)
  : pkt(p),
    ClMessage(CLMSG_MAC2PHY_VERBOSITY, CLMSG_MAC2PHY_GETTXDURATION)
{
}

ClMsgMac2PhyGetTxDuration::ClMsgMac2PhyGetTxDuration(int moduleId, Packet*p)
  : pkt(p), ClMessage(CLMSG_MAC2PHY_VERBOSITY, CLMSG_MAC2PHY_GETTXDURATION, UNICAST, moduleId)
{
}

ClMessage* ClMsgMac2PhyGetTxDuration::copy()
{
  // Supporting only synchronous messages!!!
  assert(0);
}


///// new clmsg MAC2PHY
void ClMsgMac2PhyGetTxDuration::setDuration(double time)
{
  duration = time;
}

double ClMsgMac2PhyGetTxDuration::getDuration()
{
  return(duration);
}


//channel status request
ClMsgMac2PhyGetIsIdle::ClMsgMac2PhyGetIsIdle()
: ClMessage(CLMSG_MAC2PHY_VERBOSITY, CLMSG_MAC2PHY_GETISIDLE)
{
}

ClMsgMac2PhyGetIsIdle::ClMsgMac2PhyGetIsIdle(int moduleId)
: ClMessage(CLMSG_MAC2PHY_VERBOSITY, CLMSG_MAC2PHY_GETISIDLE, UNICAST, moduleId)
{
}

ClMessage* ClMsgMac2PhyGetIsIdle::copy()
{
	// Supporting only synchronous messages!!!
	assert(0);
}


///// new clmsg MAC2PHY
void ClMsgMac2PhyGetIsIdle::setIsIdle(int status)
{
//	printf("ClMsgMac2PhyGetIsIdle::setIsIdle status %d\n", status);
	channelStatus = status;
}
int ClMsgMac2PhyGetIsIdle::getIsIdle()
{
//	printf("ClMsgMac2PhyGetIsIdle::getIsIdle status %d\n", channelStatus);
	return(channelStatus);
}


/**
 * Message to get the address of the mac module
 */
ClMsgPhy2MacAddr::ClMsgPhy2MacAddr() : ClMessage(CLMSG_PHY2MAC_VERBOSITY, CLMSG_PHY2MAC_ADDR), addr_(0) {}


void ClMsgPhy2MacAddr::setAddr(int addr)
{
  addr_ = addr;
}

int ClMsgPhy2MacAddr::getAddr()
{
  return(addr_);
}

ClMessage* ClMsgPhy2MacAddr::copy()
{
  assert(0);
}


////////////////
