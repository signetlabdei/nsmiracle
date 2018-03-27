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


#include "umts-wirelessch.h"
#include "umts-headers.h"
#include "umts-phy.h"

static class UmtsWirelessChModuleClass : public TclClass {
public:
	UmtsWirelessChModuleClass() : TclClass("Module/UmtsWirelessCh") {}
	TclObject* create(int, const char*const*) {
	return (new UmtsWirelessChModule());

}
} class_umtswirelesschmodule;

UmtsWirelessChModule::UmtsWirelessChModule() : 
	ChannelModule(),
	BSchsap_(NULL)
{

}

UmtsWirelessChModule::~UmtsWirelessChModule()
{
}



int UmtsWirelessChModule::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc==3)
	{
		if (strcasecmp(argv[1],"BSphymoduleId")==0)
		{
			int id = atoi(argv[2]);
			if (id<0)
				return TCL_ERROR;
			
			return TCL_OK;
			ChSAP *c;
			BSchsap_ = NULL;
			for (int i=0; i < getChSAPnum(); i++) 
			{
				c = (ChSAP*)getChSAP(i);
				if (c->getModuleUpId()==id)
				{
					BSchsap_ = c;
					break;
				}
			}
			if (BSchsap_==NULL)
			{
				fprintf(stderr,"Error UmtsWirelessChModule::command(%s), does not find the BS phy module, is yet attached?\n", argv[2]);
				return TCL_ERROR;
			}
		}
	}

// 	if (argc==3)
// 	{
// 		// Over-ride of a channel-module tcl command:
// 		// 1) and set up the array of the nodes used for reduce complexity in packet trasmission
// 		// 2) install a SAP to an above module (call channel-module tcl command)
// 		if (strcasecmp(argv[1],"addsap")==0)
// 		{
// 			ChSAP *chsap = dynamic_cast<ChSAP*>(TclObject::lookup(argv[2]));
// 			if (!chsap)
// 				return TCL_ERROR;
// 			addNode(chsap);
// 			Module::command(argc, argv);
// 		}
// 		
// 	}
	return ChannelModule::command(argc, argv);
}




double UmtsWirelessChModule::getPropDelay(Position *source, Position* dest)
{
  return	(sqrt( ((source->getX()-dest->getX())*(source->getX()-dest->getX())) + 
		       ((source->getY()-dest->getY())*(source->getY()-dest->getY())) ) 
		 / SPEED_OF_LIGHT);
}

void UmtsWirelessChModule::sendUpPhy(Packet *p,ChSAP *chsap)
{

	
  Scheduler &s = Scheduler::instance();
  struct hdr_cmn *hdr = HDR_CMN(p);
  hdr_umtsphy* uh = HDR_UMTSPHY(p);
	
  hdr->direction() = hdr_cmn::UP;
  Position *sourcePos = chsap->getPosition();
  // check if it's an UPLINK transmission -> use the optimized mode
  if ((BSchsap_!=NULL)&&(uh->direction == DIRECTION_UPLINK))
  {
  	// use the optimized mode -> transmit only to BS (transmission to the other MEs are useless)
  	s.schedule(BSchsap_ ,p->copy(), getPropDelay(sourcePos, BSchsap_->getPosition()));
  }
  else
  {
		// standard mode -> sent to all the ChSAP installed
		//double Pt = p->txinfo_.getTxPr();
	
		ChSAP *dest;
		
		
		for (int i=0; i < getChSAPnum(); i++) {
			
		dest = (ChSAP*)getChSAP(i);
		if (chsap == dest) // it's the source node -> skip it
			continue;
			
		s.schedule(dest,
				p->copy(), 
				getPropDelay(sourcePos, dest->getPosition()));
		}
	}
  Packet::free(p);
}

void UmtsWirelessChModule::recv(Packet *p, ChSAP* chsap)
{
	sendUpPhy(p, chsap);
}
