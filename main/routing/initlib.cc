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

#include "routing-module.h"
#include "routing-tracer.h"
#include <sap.h>
#include <tclcl.h>

ClMessage_t CL_ROUTEMESSAGE;
extern packet_t PT_MRCL_ROUTING;

int RoutingHdr::offset_ = 0;

static class RoutingHdrClass : public PacketHeaderClass {
public:
	RoutingHdrClass() : PacketHeaderClass("PacketHeader/Routing", sizeof(RoutingHdr)) {
	 	bind_offset(&RoutingHdr::offset_);
	}
} class_routinghdr;

extern EmbeddedTcl MrclRoutingInitTclCode;

extern "C" int Miraclerouting_Init()
{
	RoutingHdrClass* mrh = new RoutingHdrClass;
	mrh->bind();
	PT_MRCL_ROUTING = p_info::addPacket((char*)"MRCL_ROUTING");

	CL_ROUTEMESSAGE = ClMessage::addClMessage();
	SAP::addTracer(new MrclRoutingTracer);
	MrclRoutingInitTclCode.load();
	return 0;
}
extern "C" int  Cygmiraclerouting_Init()
{
  return Miraclerouting_Init();
}


