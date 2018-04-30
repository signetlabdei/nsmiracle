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

#include "802_11-tracer.h"
#include <sap.h>
#include <address.h>
#include <mac.h>
#include <mac-802_11.h>

Mac_802_11_Tracer::Mac_802_11_Tracer() : Tracer(2) {}

void Mac_802_11_Tracer::format(Packet *p, SAP *sap)
{
	struct hdr_mac802_11 *mh = HDR_MAC802_11(p);
	hdr_cmn *ch = hdr_cmn::access(p);
	
	writeTrace(sap, (char*)" --MAC-- [%d %.10f %d %d %x]",
			mh->dh_duration,
			ch->txtime(),
			ETHER_ADDR(mh->dh_ra),
                        ETHER_ADDR(mh->dh_ta),
			GET_ETHER_TYPE(mh->dh_body)
			);
}


extern "C" int Mac_Init()
{
  /*
   * Put here all the commands which must be execute when the library
   * is loaded (i.e. TCL script execution)  
   * Remember to return 0 if all is OK, otherwise return 1
  */
  	
	SAP::addTracer(new Mac_802_11_Tracer);
	return 0;
}
extern "C" int  Cygmac_Init()
{
  return Mac_Init();
}


