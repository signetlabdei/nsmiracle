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

#include "arp-tracer.h"
#include <sap.h>
#include <address.h>
#include <mac.h>
#include <mac-802_11.h>



ARP_Tracer::ARP_Tracer() : Tracer(5) {}

void ARP_Tracer::format(Packet *p, SAP *sap)
{
	hdr_cmn *ch = hdr_cmn::access(p);
	if(ch->ptype() != PT_ARP)
		return;
	
	struct hdr_arp *ah = HDR_ARP(p);
	
	/*
	writeTrace(sap, " --ARP-- [%s %d/%d %d/%d]",
			ah->arp_op == ARPOP_REQUEST ?  "REQUEST" : "REPLY",
			ah->arp_sha,
			ah->arp_spa,
			ah->arp_tha,
			ah->arp_tpa
			);
	*/
	switch (ah->arp_op) 
	  {
	  case ARPOP_REQUEST:
	    writeTrace(sap, " -- ARP -- [Who has %d.%d.%d.%d? Tell %d.%d.%d.%d at %02x:%02x:%02x:%02x]",
		       (ah->arp_tpa & 0xff000000)>>24,
		       (ah->arp_tpa & 0x00ff0000)>>16,
		       (ah->arp_tpa & 0x0000ff00)>>8,
		       (ah->arp_tpa & 0x000000ff),
		       (ah->arp_spa & 0xff000000)>>24,
		       (ah->arp_spa & 0x00ff0000)>>16,
		       (ah->arp_spa & 0x0000ff00)>>8,
		       (ah->arp_spa & 0x000000ff),
		       (ah->arp_sha & 0xff000000)>>24,
		       (ah->arp_sha & 0x00ff0000)>>16,
		       (ah->arp_sha & 0x0000ff00)>>8,
		       (ah->arp_sha & 0x000000ff)
		       );
	    break;

	  case ARPOP_REPLY: 
	    writeTrace(sap, " -- ARP -- [%d.%d.%d.%d is at %02x:%02x:%02x:%02x]",
		       (ah->arp_spa & 0xff000000)>>24,
		       (ah->arp_spa & 0x00ff0000)>>16,
		       (ah->arp_spa & 0x0000ff00)>>8,
		       (ah->arp_spa & 0x000000ff),
		       (ah->arp_sha & 0xff000000)>>24,
		       (ah->arp_sha & 0x00ff0000)>>16,
		       (ah->arp_sha & 0x0000ff00)>>8,
		       (ah->arp_sha & 0x000000ff)
		       );
	    break;
	    
	  case ARPOP_REVREQUEST:
	    writeTrace(sap, " -- ARP -- [REVREQUEST]");
	    break;

	  case ARPOP_REVREPLY:
	    writeTrace(sap, " -- ARP -- [REVREPLY]");
	    break;
	    
	  case ARPOP_INVREQUEST:
	    writeTrace(sap, " -- ARP -- [INVREQUEST]");
	    break;
	    

	  case ARPOP_INVREPLY:
	    writeTrace(sap, " -- ARP -- [INVREPLY]");
	    break;
	  }
	    
}

              
extern "C" int Arptracer_Init()
{
  /*
   * Put here all the commands which must be execute when the library
   * is loaded (i.e. TCL script execution)  
   * Remember to return 0 if all is OK, otherwise return 1
  */
  	
	SAP::addTracer(new ARP_Tracer);
	return 0;
}
extern "C" int  Cygarptracer_Init()
{
  Arptracer_Init();
}


