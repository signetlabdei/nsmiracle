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

#include"umts-packet.h"
#include"rlc.h"
#include<iostream>

// UMTS - used by hdr_cmn class for tracing purpose
packet_t 	 PT_UM;
packet_t         PT_AM;
packet_t         PT_UMTSCTRL;
// packet_t         PT_AMDA;
// packet_t         PT_AMPA;
// packet_t        PT_AMPBPA;
// packet_t        PT_AMBA;
// packet_t        PT_AMPBBA;

// // Used for tracking HARQ transmissions (MAC-hs PDUs)
// packet_t 	PT_AMDA_H1;
// packet_t 	PT_AMDA_H2;
// packet_t 	PT_AMDA_H3;

// packet_t 	PT_COOT;



int hdr_rlc::offset_;



static class RLCHeaderClass:public PacketHeaderClass {
public:
  RLCHeaderClass():PacketHeaderClass("PacketHeader/RLC", sizeof(hdr_rlc)) {
    bind_offset(&hdr_rlc::offset_);
    bind();
  }
}class_rlc_hdr;



int hdr_umtsphy::offset_;

static class UMTSPHYHeaderClass:public PacketHeaderClass {
public:
  UMTSPHYHeaderClass():PacketHeaderClass("PacketHeader/UMTSPHY", sizeof(hdr_umtsphy)) {
    bind_offset(&hdr_umtsphy::offset_);
    bind();
  }
}class_umtsphy_hdr;
