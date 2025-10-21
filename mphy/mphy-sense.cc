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
 * 
 */

#include<iostream>
#include<module.h>
#include<clmessage.h>

#include "mphy-sense.h"
#include "phymac-clmsg.h"
#include "clmsg-phy-on-off-switch.h"




MPhyChSense::MPhyChSense()
  : AcquisitionThreshold_dB_(0)
{
  // binding to TCL variables
  bind("TxPower_", &TxPower_);
  bind("NoiseSPD_", &NoiseSPD_);
  bind("AcquisitionThreshold_dB_", &AcquisitionThreshold_dB_);
}

MPhyChSense::~MPhyChSense()
{

}


int MPhyChSense::recvSyncClMsg(ClMessage* m)
{
  if (m->type() == CLMSG_MAC2PHY_GETISIDLE) {
    int status = 1;
    double aux_noise = getNoiseChannel();
    double powerChannel = interference_->getCurrentTotalPower();
    if (aux_noise > 0.0 && powerChannel > 0.0 && interference_) {	
      double snr_dB = 10*log10(powerChannel / aux_noise);
      if(snr_dB >= getAcquisitionThresholdDb()) {
        status = 0;
      }
    }
      ((ClMsgMac2PhyGetIsIdle*)m)->setIsIdle(status);
      return 0;
  }
  return MPhy::recvSyncClMsg(m);
}

// int MPhy::getMacAddr() {
// 	static int mac_addr = -1;
// 	ClMsgPhy2MacAddr msg;
// 	sendSyncClMsg(&msg);
// 	mac_addr = msg.getAddr();
// 	return mac_addr;
// }


double MPhyChSense::getNoiseChannel() 
{
  assert(spectralmask_);
  return (NoiseSPD_ * spectralmask_->getBandwidth());
}
