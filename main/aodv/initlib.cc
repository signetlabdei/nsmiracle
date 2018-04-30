/*
 * Copyright (c) 2008 Regents of the SIGNET lab, University of Padova.
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

#include <tclcl.h>
#include <sap.h>
#include "aodv-pkt.h"
#include "aodv-tracer.h"


extern packet_t PT_MRCL_AODV;

int hdr_mrcl_aodv::offset_;

static class MrclAODVHeaderClass : public PacketHeaderClass {
public:
        MrclAODVHeaderClass() : PacketHeaderClass("PacketHeader/MRCL_AODV",
                                              sizeof(hdr_all_mrcl_aodv)) {
	  bind_offset(&hdr_mrcl_aodv::offset_);
	} 
} class_mrcl_AODV_hdr;


extern EmbeddedTcl AodvInitTclCode;

extern "C" int Miracleaodv_Init()
{
	MrclAODVHeaderClass* mah = new MrclAODVHeaderClass;
	mah->bind();
	PT_MRCL_AODV = p_info::addPacket((char*)"MRCL_AODV");
	SAP::addTracer(new MrclAodvTracer);
	AodvInitTclCode.load();
	return 0;
}
