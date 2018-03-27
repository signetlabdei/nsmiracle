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

#include<iostream>
#include<module.h>
#include<clmessage.h>

#include "mphy.h"
#include "phymac-clmsg.h"
#include "clmsg-phy-on-off-switch.h"




MPhy::MPhy()
  : txtimer(this), rxtimer(this),
  interference_(0), propagation_(0),
  antenna_(0), spectralmask_(0),
  correlation_(0), isOn(true),
  droppedPktsNoise(0), droppedPktsInterf(0),
  droppedPktsDeaf(0), droppedPktsWrongModId(0),
  errorPktsNoise(0), errorPktsInterf(0)
{
  // binding to TCL variables
  bind("TxPower_", &TxPower_);
  bind("NoiseSPD_", &NoiseSPD_);
}

MPhy::~MPhy()
{

}

int MPhy::command(int argc, const char*const* argv)
{
  //printf("MPhy::command -- %s (%d)\n", argv[1], argc);
  Tcl& tcl = Tcl::instance();

  if(argc == 2)
    {

      if(strcasecmp(argv[1], "getPropagation")==0)
	{
	  if(!propagation_)
	    return TCL_ERROR;
	  tcl.result(propagation_->name());
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "getAntenna")==0)
	{
	  if(!antenna_)
	    return TCL_ERROR;
	  tcl.result(antenna_->name());
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "getSpectralMask")==0)
	{

	  if(!spectralmask_)
	    return TCL_ERROR;
	  tcl.result(spectralmask_->name());
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "getInterference")==0)
	{
	  if(!interference_)
	    return TCL_ERROR;
	  tcl.result(interference_->name());
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "getCorrelation")==0)
	{
	  if(!correlation_)
	    return TCL_ERROR;
	  tcl.result(correlation_->name());
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "getDroppedPktsNoise")==0)
	{
	  tcl.resultf("%d",getDroppedPktsNoise());
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "getDroppedPktsInterf")==0)
	{
	  tcl.resultf("%d",getDroppedPktsInterf());
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "getDroppedPktsDeaf")==0)
	{
	  tcl.resultf("%d",getDroppedPktsDeaf());
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "getDroppedPktsWrongModId")==0)
	{
	  tcl.resultf("%d",getDroppedPktsWrongModId());
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "getErrorPktsNoise")==0)
	{
	  tcl.resultf("%d",getErrorPktsNoise());
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "getErrorPktsInterf")==0)
	{
	  tcl.resultf("%d",getErrorPktsInterf());
	  return TCL_OK;
	}
    }

  if(argc == 3)
    {

      if(strcasecmp(argv[1], "setPropagation")==0)
	{
	  // dynamic cast returns NULL if object is not of the correct type
	  propagation_ = dynamic_cast<MPropagation*> (TclObject::lookup(argv[2]));
	  if(!propagation_)
	    return TCL_ERROR;
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "setAntenna")==0)
	{
	  // dynamic cast returns NULL if object is not of the correct type
	  antenna_ = dynamic_cast<MAntenna*> (TclObject::lookup(argv[2]));
	  if(!antenna_)
	    return TCL_ERROR;
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "setSpectralMask")==0)
	{
	  // dynamic cast returns NULL if object is not of the correct type
	  spectralmask_ = dynamic_cast<MSpectralMask*> (TclObject::lookup(argv[2]));
	  if(!spectralmask_)
	    return TCL_ERROR;
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "setInterference")==0)
	{
	  // dynamic cast returns NULL if object is not of the correct type
	  interference_ = dynamic_cast<MInterference*> (TclObject::lookup(argv[2]));
	  if(!interference_)
	    return TCL_ERROR;
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "setCorrelation")==0)
	{
	  // dynamic cast returns NULL if object is not of the correct type
	  correlation_ = dynamic_cast<MCorrelation*> (TclObject::lookup(argv[2]));
	  if(!correlation_)
	    return TCL_ERROR;
	  return TCL_OK;
	}
    }

  return Module::command(argc, argv);
}



/*
 * Very naive implementation of modulation type registration
 * 
 */

#define MPHY_MAX_MOD_TYPES 200
#define MPHY_MAX_MOD_NAME_LENGTH 10

static int mphy_mod_id_counter = -1;

static char mphy_mod_name[MPHY_MAX_MOD_TYPES][MPHY_MAX_MOD_NAME_LENGTH];

int MPhy::registerModulationType(const char* name)
{

  mphy_mod_id_counter++;
  assert( (mphy_mod_id_counter >=0) && (mphy_mod_id_counter < MPHY_MAX_MOD_TYPES));
  strncpy(&mphy_mod_name[mphy_mod_id_counter][0], name, MPHY_MAX_MOD_NAME_LENGTH-1);

  // null-terminate to segfaults in case string was truncated
  mphy_mod_name[mphy_mod_id_counter][ MPHY_MAX_MOD_NAME_LENGTH-1] = '\0';

  return mphy_mod_id_counter;
}

const char* MPhy::getModulationName(int id)
{
  //  std::cerr << "getModulationName(" << id <<")" << std::endl;
  if((id>=0)&&(id<=mphy_mod_id_counter)&&(id<MPHY_MAX_MOD_TYPES))
    return (&mphy_mod_name[id][0]);
  else
    return "UNKNOWN";
}





void MPhy::recv(Packet* p)
{

  hdr_cmn *ch = HDR_CMN(p);
  hdr_MPhy *ph = HDR_MPHY(p);

  if(ch->direction() == hdr_cmn::UP)
    {
      ph->dstSpectralMask = getRxSpectralMask(p); 
      ph->dstPosition = getPosition();
      ph->dstAntenna = getRxAntenna(p);

      assert(ph->dstSpectralMask);
      assert(ph->dstPosition);   

      ph->Pr = getRxPower(p); 

      // The following is to avoid some floating point exceptions when using
      // uninitialized values which happen to be NAN or INF. This is
      // due to the following facts:
      // 1) txinfo.RxPr and txinfo.CPThresh are manually copied in Module::copy(Packet*) to
      //    support some possible (though unlikely) case in which you
      //    need to preserve these values throughout a packet copy
      // 2) neither RxPr nor CPThresh are initialized upon packet
      //    creation, they are just assigned by old ns2 wireless-phy, so
      //    if you're not using wireless-phy they are almost random 
      // To fix this issue, we're setting the values to 0 since we're
      // not using them. 
      p->txinfo_.RxPr = 0;
      p->txinfo_.CPThresh = 0;

      /* To reduce complexity, if the received power for this
       * transmission is zero, we completely neglect this packet.
       */
      if (ph->Pr > 0)  
	{	  
	  ph->Pn = getNoisePower(p);

	  if (interference_) 
	    {	
	      interference_->addToInterference(p);
	    }

	  ph->rxtime = NOW;
	  ph->worth_tracing = true;
 
          // new control: interrupt must be on and no tx must be pending
          if (isOn == true) {
	     PacketEvent* pe = new PacketEvent(p);
	     Scheduler::instance().schedule(&rxtimer, pe, ph->duration);
	  
	     startRx(p);
          }
          else {
             // phy is OFF, we must free the packet
             Packet::free(p);
           }
	}
      else
	{	 
	  Packet::free(p);
	}


    }    
  else // Direction DOWN
    {
 
      // new control: phy must be ON.
      assert(isOn);
      
      // we would like to copy the TxSpectralMask and TxAntenna so
      // that we are sure that their parameters are not changed when
      // the packet is received. Unfortunately, we cannot do that:
      // 1) the object would not get deleted when p is deleted
      // 2) the object would not get copied when p->copy() is called,
      //    so we would have just 1 copy per packet transmitted
      // 3) having just a single copy, we cannot detect at the receiver
      //    when the last packet gets deleted so to also delete the
      //    spectral mask/antenna
      // So we have to stick to pointers to instances which the PHY will keep
      // allocated for all the necessary time.

      ph->Pr = 0;
      ph->Pn = 0;
      ph->Pi = 0;
      ph->txtime = NOW;
      ph->rxtime = ph->txtime; // so that incomplete tracing can occur

      ph->worth_tracing = false;

      ph->srcSpectralMask = getTxSpectralMask(p);
      ph->srcAntenna = getTxAntenna(p);
      ph->srcPosition = getPosition();
      ph->dstSpectralMask = 0;
      ph->dstPosition = 0;      
      ph->dstAntenna = 0;
      ph->modulationType = getModulationType(p);
      ph->duration = getTxDuration(p);

      ph->Pt = getTxPower(p);

      assert(ph->srcSpectralMask);
      assert(ph->srcPosition);      
      assert(ph->duration > 0);
      assert(ph->Pt > 0);

      // we copy the packet so that we are sure that the reference
      // passed to the Packet Event is valid 
      PacketEvent* pe = new PacketEvent(p->copy());
      Scheduler::instance().schedule(&txtimer, pe, ph->duration);

      startTx(p);

    }

}

/////////// method used to receive a sync clmessage from MAC
 
int MPhy::recvSyncClMsg(ClMessage* m)
{

  if (m->type() == CLMSG_MAC2PHY_GETTXDURATION)
    {

      Packet* pkt = ((ClMsgMac2PhyGetTxDuration*)m)->pkt;

      hdr_MPhy* ph = HDR_MPHY(pkt);

      ph->srcSpectralMask = getTxSpectralMask(pkt);

      double duration = getTxDuration(pkt);

      ((ClMsgMac2PhyGetTxDuration*)m)->setDuration(duration);
      return 0;
    }
  else if (m->type() == CLMSG_PHY_ON_OFF_SWITCH) {
      if ( ((ClMsgPhyOnOffSwitch*)m)->isOn() == true) turnOn();
      else turnOff();
      return 0;
  }
  else if (m->type() == CLMSG_PHY_ON_OFF_SWITCH_STATUS) {
      ((ClMsgPhyOnOffSwitchStatus*)m)->setStatus(isOn);
      return 0;
  }
  else return Module::recvSyncClMsg(m);
}

double MPhy::getRxPower(Packet* p)
{
  hdr_MPhy *ph = HDR_MPHY(p);

  double tag=1; // TX Antenna gain
  double rag=1; // RX Antenna gain
  double pg=1;  // Propagation Gain
  double smg=1; // Spectral Mask Gain
  double cg=1;  // Correlation Gain


  assert(ph->srcSpectralMask && ph->dstSpectralMask);

  smg = ph->srcSpectralMask->getOverlap(ph->dstSpectralMask, p);    
  if (smg == 0) return 0;


  if (ph->srcAntenna)     
      tag = ph->srcAntenna->getGain(p);

  if (tag == 0) return 0;


  if (ph->dstAntenna)
    rag = ph->dstAntenna->getGain(p);

  if (rag == 0) return 0;

  if (propagation_) 
      pg = propagation_->getGain(p);

  if (pg == 0) return 0;

  if (correlation_)
    cg = correlation_->getGain(p);

  double Pr =  (ph->Pt * smg * tag * rag * pg * cg);

  if (debug_)    std::cerr << NOW 
		           << " Pt=" << ph->Pt
		           << " SMG=" << smg
			   << " TAG=" << tag
			   << " RAG=" << rag
			   << " PG=" << pg
			   << " CG=" << cg
			   << " Pr=" << Pr
			   << " duration=" << ph->duration
			   << std::endl;
  
  return Pr;
}



double MPhy::getNoisePower(Packet* p) 
{
  MSpectralMask* sm= getRxSpectralMask(p);
  assert(sm);
  double noise = NoiseSPD_ * sm->getBandwidth();

  if (debug_) std::cerr << NOW << " Pn=" << noise << std::endl;

  return (noise);
}



void MPhyRxTimer::handle(Event* e)
{
  PacketEvent* pe = (PacketEvent*) e;
  assert(pe);
  Packet* p = pe->pkt;
  assert(p);
  delete e;

  hdr_MPhy *ph = HDR_MPHY(p);

  if (phy->interference_) 
    {	
      ph->Pi =  phy->interference_->getInterferencePower(p);
    }
  else
    {
      ph->Pi = 0;
    }

  phy->endRx(p);

}




void MPhyTxTimer::handle(Event* e)
{
  PacketEvent* pe = (PacketEvent*) e;
  assert(pe);
  Packet* p = pe->pkt;
  assert(p);
  delete e;

  phy->endTx(p);

  // we can now delete the copy we made explicitly for this purpose
  Packet::free(p);
}



void MPhy::Phy2MacEndTx(const Packet* p)
{
  ClMsgPhy2MacEndTx m(p);
  sendSyncClMsgUp(&m);
}


void MPhy::Phy2MacStartRx(const Packet* p)
{
  ClMsgPhy2MacStartRx m(p);
  sendSyncClMsgUp(&m);
}



void MPhy::Phy2MacCCA(bool cca)
{
  ClMsgPhy2MacCCA m(cca);
  sendSyncClMsgUp(&m);
}

