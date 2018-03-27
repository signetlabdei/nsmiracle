
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
 */


/**
 * @file   clmsg-phy-on-off-switch.cc
 * @author Federico Guerra
 * @date   Mon Nov 26 14:59:38 2007
 * 
 * @brief  
 * 
 * 
 */



#include "clmsg-phy-on-off-switch.h"

#include <iostream>

ClMessage_t CLMSG_PHY_ON_OFF_SWITCH;
ClMessage_t CLMSG_PHY_ON_OFF_SWITCH_STATUS;

ClMsgPhyOnOffSwitch::ClMsgPhyOnOffSwitch()
  : turnOn(true), ClMessage(CLMSG_PHY_ON_OFF_SWITCH_VERBOSITY, CLMSG_PHY_ON_OFF_SWITCH)
{
}

ClMsgPhyOnOffSwitch::ClMsgPhyOnOffSwitch(int destination)
  : turnOn(true), ClMessage(CLMSG_PHY_ON_OFF_SWITCH_VERBOSITY, CLMSG_PHY_ON_OFF_SWITCH, UNICAST, destination)
{
}

ClMessage* ClMsgPhyOnOffSwitch::copy()
{
  // Supporting only synchronous messages!!!
  assert(0);
}


///// new clmsg MAC2PHY
void ClMsgPhyOnOffSwitch::setOn()
{
  turnOn = true;
}

void ClMsgPhyOnOffSwitch::setOff()
{
  turnOn = true;
}

bool ClMsgPhyOnOffSwitch::isOn()
{
  return(turnOn);
}

ClMsgPhyOnOffSwitchStatus::ClMsgPhyOnOffSwitchStatus()
  : isOn_status(false), ClMessage(CLMSG_PHY_ON_OFF_SWITCH_STATUS_VERBOSITY, CLMSG_PHY_ON_OFF_SWITCH_STATUS)
{
}

ClMsgPhyOnOffSwitchStatus::ClMsgPhyOnOffSwitchStatus(int destination)
  : isOn_status(false), ClMessage(CLMSG_PHY_ON_OFF_SWITCH_STATUS_VERBOSITY, CLMSG_PHY_ON_OFF_SWITCH_STATUS, UNICAST, destination)
{
}

ClMessage* ClMsgPhyOnOffSwitchStatus::copy()
{
  // Supporting only synchronous messages!!!
  assert(0);
}


///// new clmsg MAC2PHY
void ClMsgPhyOnOffSwitchStatus::setStatus(bool flag)
{
  isOn_status = flag;
}

bool ClMsgPhyOnOffSwitchStatus::getStatus()
{
  return(isOn_status);
}

 
