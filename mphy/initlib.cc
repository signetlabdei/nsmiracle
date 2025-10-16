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
 * OR BUSINESS ON_OFF_SWITCHION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * thanks to: Roberto Petroccia for the channel sense extension
 * 
 */

#include <tclcl.h>
#include <packet.h>
#include "mphy_pktheader.h"
#include "phymac-clmsg.h"
#include "clmsg-phy-on-off-switch.h"

extern EmbeddedTcl MPhyInitTclCode;


static class MPhyHeaderClass : public PacketHeaderClass {
public:
  MPhyHeaderClass()	: PacketHeaderClass("PacketHeader/MPhy",sizeof(hdr_MPhy))  {
    bind_offset(&hdr_MPhy::offset_);
    bind();
  }
} class_hdr_mphy;


int hdr_MPhy::offset_;






extern "C" int Mphy_Init()
{
  /*
   * Put here all the commands which must be execute when the library
   * is loaded (i.e. TCL script execution)  
   * Remember to return 0 if all is OK, otherwise return 1
  */ 
  CLMSG_PHY2MAC_ENDTX = ClMessage::addClMessage();
  CLMSG_PHY2MAC_STARTRX = ClMessage::addClMessage();
  CLMSG_PHY2MAC_CCA = ClMessage::addClMessage();
  CLMSG_MAC2PHY_GETTXDURATION = ClMessage::addClMessage();
  CLMSG_PHY_ON_OFF_SWITCH = ClMessage::addClMessage();
  CLMSG_PHY_ON_OFF_SWITCH_STATUS = ClMessage::addClMessage();
  CLMSG_PHY2MAC_ADDR = ClMessage::addClMessage();
  CLMSG_MAC2PHY_GETISIDLE = ClMessage::addClMessage();
  
  MPhyInitTclCode.load();
  return 0;
}

extern "C" int  Cygmphy_Init()
{
  return Mphy_Init();
}


