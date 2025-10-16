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
 * thanks to: Roberto Petroccia for the channel sense extension
 */


/**
 * @file   phymac-clmsg.h
 * @author Baldo Nicola
 * @date   Mon Nov 26 14:05:17 2007
 * 
 * @brief  
 * 
 * 
 */


#ifndef PHYMAC_CLMSG_H
#define PHYMAC_CLMSG_H

#include<packet.h>
#include<clmessage.h>

#define CLMSG_PHY2MAC_VERBOSITY 3
#define CLMSG_MAC2PHY_VERBOSITY 3 // ver

extern ClMessage_t CLMSG_PHY2MAC_ENDTX;
extern ClMessage_t CLMSG_PHY2MAC_STARTRX;
extern ClMessage_t CLMSG_PHY2MAC_CCA;
extern ClMessage_t CLMSG_PHY2MAC_ADDR;
extern ClMessage_t CLMSG_MAC2PHY_GETTXDURATION;
extern ClMessage_t CLMSG_MAC2PHY_GETISIDLE;

class ClSAP;


class ClMsgPhy2MacEndTx : public ClMessage
{
public:
  ClMsgPhy2MacEndTx(const Packet* p);
  ClMessage* copy();	// copy the message
  
  const Packet* pkt;
};


class ClMsgPhy2MacStartRx : public ClMessage 
{
public:
  ClMsgPhy2MacStartRx(const Packet* p);
  ClMessage* copy();	// copy the message

  const Packet* pkt;
};


class ClMsgPhy2MacCCA : public ClMessage 
{
public:
  ClMsgPhy2MacCCA(bool new_cca);
  ClMessage* copy();	// copy the message

  bool CCA;
};

//////////// new ClMsg MAC2PHY

class ClMsgMac2PhyGetTxDuration : public ClMessage
{

public:

  ClMsgMac2PhyGetTxDuration(Packet* p);
  ClMsgMac2PhyGetTxDuration(int moduleId, Packet* p);

  void setDuration(double time);
  double getDuration();

  ClMessage* copy();	// copy the message
  
  Packet* pkt;
	
private:

  double duration;
};

class ClMsgMac2PhyGetIsIdle : public ClMessage {
		
	public:
		
		ClMsgMac2PhyGetIsIdle();
		ClMsgMac2PhyGetIsIdle(int moduleId);
		
		void setIsIdle(int status);
		int getIsIdle();
		
		ClMessage* copy();	// copy the message
		
	private:
		
		int channelStatus;
};		

/**
 * Message to get the mac address for a mac module
 */
class ClMsgPhy2MacAddr : public ClMessage
{
  public:

    ClMsgPhy2MacAddr();
        
    virtual ~ClMsgPhy2MacAddr() { }

    /**
     * Creates a copy of the object
     * @return Pointer to a copy of the object
     */
    ClMessage* copy();

    /**
     * Sets the address for mac layer
     * @param addr Address for mac layer
     */
    void setAddr(int addr);

    /**
     * Get the the address for mac layer
     * @return Address for mac layer
     */
    int getAddr();
    
    
  private:
    /** Address for mac layer */
    int addr_;
  
};



//////////////////////////////////





#endif /* PHYMAC_CLMSG_H */
