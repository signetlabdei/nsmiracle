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

/**
 * @file   mmac.cc
 * @author Baldo Nicola, Federico Guerra
 * @date   Mon Nov 26 15:02:34 2007
 * 
 * @brief  
 * 
 * 
 */

#include <phymac-clmsg.h>
#include "mmac.h"
#include "mmac-clmsg.h"
#include "clmsg-phy-on-off-switch.h"

static int mmac_addr_counter = 0;


MMac::MMac()
  : mac2phy_delay_(1e-9),
   addr(mmac_addr_counter++),
   up_data_pkts_rx(0),
   data_pkts_tx(0),
   data_pkts_rx(0),
   ack_pkts_tx(0),
   ack_pkts_rx(0),
   xdata_pkts_rx(0),
   xack_pkts_rx(0),
   ctrl_pkts_tx(0),
   ctrl_pkts_rx(0),
   xctrl_pkts_rx(0),
   backoff_times_no(0),
   dropped_pkts_tx(0),
   error_pkts_rx(0),
   discarded_data_pkts(0),
   sum_wait_time(0),
   sum_backoff_time(0),
   queue_wait_time()
{

}

MMac::~MMac()
{
}


int MMac::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if (argc == 2)
    {
      if (strcasecmp(argv[1], "addr") == 0) 
	{
	  tcl.resultf("%d", addr);
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getDataPktsTx") == 0)	
	{
	  tcl.resultf("%d",getDataPktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getDataPktsRx") == 0)	
	{
	  tcl.resultf("%d",getDataPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getXDataPktsRx") == 0)	
	{
	  tcl.resultf("%d",getXDataPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getAckPktsTx") == 0)	
	{
	  tcl.resultf("%d",getAckPktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getAckPktsRx") == 0)	
	{
	  tcl.resultf("%d",getAckPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getCtrlPktsTx") == 0)	
	{
	  tcl.resultf("%d",getCtrlPktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getCtrlPktsRx") == 0)	
	{
	  tcl.resultf("%d",getCtrlPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getXCtrlPktsRx") == 0)	
	{
	  tcl.resultf("%d",getXCtrlPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getDroppedPktsTx") == 0)	
	{
	  tcl.resultf("%d",getDroppedPktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getErrorPktsRx") == 0)	
	{
	  tcl.resultf("%d",getErrorPktsRx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getDiscardedPktsTx") == 0)	
	{
	  tcl.resultf("%d",getDiscardedPktsTx());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getEfficiency") == 0)	
	{
	  tcl.resultf("%f",getEfficiency());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanWaitTime") == 0)	
	{
	  tcl.resultf("%f",getMeanWaitTime());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanBackoffTime") == 0)	
	{
	  tcl.resultf("%f",getMeanBackoffTime());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getTotalBackoffTimes") == 0)	
	{
	  tcl.resultf("%d",getTotalBackoffTimes());
      	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "resetStats") == 0)	
	{
	  resetStats();
      	  return TCL_OK;
	}
    } 
  return Module::command(argc, argv);
}


int MMac::recvSyncClMsg(ClMessage* m)
{
  if (m->type() == CLMSG_PHY2MAC_ENDTX)
    {
      Phy2MacEndTx(((ClMsgPhy2MacEndTx*)m)->pkt);
      return 0;
    }
  else if (m->type() == CLMSG_PHY2MAC_STARTRX)
    {
      Phy2MacStartRx(((ClMsgPhy2MacEndTx*)m)->pkt);
      return 0;
    }
  else if (m->type() == CLMSG_PHY2MAC_CCA)
    {
      Phy2MacCCA(((ClMsgPhy2MacCCA*)m)->CCA);
      return 0;
    }
  else if (m->type() == MAC_CLMSG_GET_ADDR)
    {
      ((MacClMsgGetAddr *)m)->setAddr(addr);
      return 0;
    }
  else if (m->type() == CLMSG_PHY2MAC_ADDR)
    {
      ((ClMsgPhy2MacAddr *)m)->setAddr(addr);
      return 0;
    }
  else return Module::recvSyncClMsg(m);
}


void MMac::recv(Packet* p)
{
  hdr_cmn *ch = HDR_CMN(p);
  if(ch->direction() == hdr_cmn::UP)
    {
      Phy2MacEndRx(p);
    }
  else
    {
      //direction DOWN: packet is coming from upper layers
      recvFromUpperLayers(p);
    }
}


void MMac::Mac2PhyStartTx(Packet* p)
{
  sendDown(p, mac2phy_delay_);
}

void MMac::Mac2PhyStartTx(int moduleId, Packet* p)
{
  sendDown(moduleId, p, mac2phy_delay_);
}


void MMac::Phy2MacEndTx(const Packet* p)
{
}

void MMac::Phy2MacStartRx(const Packet* p)
{
}

void MMac::Phy2MacEndRx(Packet* p)
{
}

void MMac::recvFromUpperLayers(Packet* p)
{
  incrUpperDataRx();

}

void MMac::Phy2MacCCA(bool cca)
{
}

double MMac::Mac2PhyTxDuration(Packet* pkt)
{
  ClMsgMac2PhyGetTxDuration m(pkt);
  sendSyncClMsgDown(&m);
  return(m.getDuration());
}

double MMac::Mac2PhyTxDuration(int moduleId, Packet* pkt)
{
  ClMsgMac2PhyGetTxDuration m(moduleId, pkt);
  sendSyncClMsgDown(&m);
  return(m.getDuration());
}

void MMac::Mac2PhyTurnOn()
{
  ClMsgPhyOnOffSwitch m;
  m.setOn();
  sendSyncClMsgDown(&m);
}

void MMac::Mac2PhyTurnOn(int moduleId)
{
  ClMsgPhyOnOffSwitch m(moduleId);
  m.setOn();
  sendSyncClMsgDown(&m);
}

void MMac::Mac2PhyTurnOff()
{
  ClMsgPhyOnOffSwitch m;
  m.setOff();
  sendSyncClMsgDown(&m);
}  

void MMac::Mac2PhyTurnOff(int moduleId)
{
  ClMsgPhyOnOffSwitch m(moduleId);
  m.setOff();
  sendSyncClMsgDown(&m);
}

bool MMac::Mac2PhyOnOffSwitchStatus()
{
  ClMsgPhyOnOffSwitchStatus m;
  sendSyncClMsgDown(&m);
  return(m.getStatus());
}

bool MMac::Mac2PhyOnOffSwitchStatus(int moduleId)
{
  ClMsgPhyOnOffSwitchStatus m(moduleId);
  sendSyncClMsgDown(&m);
  return(m.getStatus());
}

int MMac::getRemainingPkts() 
{
 return 0;
}

// void MMac::computeQueueWaitTime() 
// {
//   sum_wait_time += ( NOW - queue_wait_time.front() );
// }

void MMac::waitEndTime(bool flag, Packet* p)
{
  if (flag == true) sum_wait_time += ( NOW - queue_wait_time.front() );
  queue_wait_time.pop(); 
}

