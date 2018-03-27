/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This software is based on the NIST NS2 WiMAX module, which can be found at
 * http://www.antd.nist.gov/seamlessandsecure/doc.html
 *
 * $Id: ofdm-packet.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef WIMAX_PACKET_H
#define WIMAX_PACKET_H

#include "packet.h"

/** Status of physical layer */
enum Ofdm_phy_state {
 OFDM_IDLE,	/** Module is not doing anything */
 OFDM_SEND,	/** Module is ready to send or sending */
 OFDM_RECV,	/** Module is can receive or is receiving */
 OFDM_RX2TX,	/** Module is transitioning from receiving mode to sending mode */
 OFDM_TX2RX	/** Module is transitioning from sending mode to receiving mode */
};

/** Definition of supported rate */
enum Ofdm_mod_rate {
 OFDM_BPSK_1_2,	/** Efficiency is 1 bps/Hz */
 OFDM_QPSK_1_2,	/** Efficiency is 2 bps/Hz */
 OFDM_QPSK_3_4,	/** Efficiency is 2 bps/Hz */
 OFDM_16QAM_1_2,	/** Efficiency is 4 bps/Hz */
 OFDM_16QAM_3_4,	/** Efficiency is 4 bps/Hz */
 OFDM_64QAM_2_3,	/** Efficiency is 6 bps/Hz */
 OFDM_64QAM_3_4,	/** Efficiency is 6 bps/Hz */
};

/**
 * How to compute the number of information bit per symbol:
 * - Each symbol has 192 data subcarrier (200-8 for pilots)
 * - A modulation has a coding rate (1/2, 2/3, or 3/4)
 * - A modulation has an efficiency (1, 2, 4, or 6)
 * - There is a 0x00 tail byte at the end of each OFDM symbol
 * So for BPSK, 192*1*1/2-8=88
 */
enum Ofdm_bit_per_symbol {
 OFDM_BPSK_1_2_bpsymb = 88,
 OFDM_QPSK_1_2_bpsymb = 184,
 OFDM_QPSK_3_4_bpsymb = 280,
 OFDM_16QAM_1_2_bpsymb = 376,
 OFDM_16QAM_3_4_bpsymb = 578,
 OFDM_64QAM_2_3_bpsymb = 760,
 OFDM_64QAM_3_4_bpsymb = 856,
};

/** Struct for phy-specific header */
typedef struct hdr_ofdmphy {
	double freq_;			///the frequency at which it is sent
	Ofdm_mod_rate modulation_;	///modulation at which the packet was sent
	double g_;			///The cyclic prefix information. Used for synchronization

	static int  offset_;
	inline int &offset() { return offset_; }
	static hdr_ofdmphy* access(const Packet * p) {
		return (hdr_ofdmphy *) p->access(offset_);
	}
} hdr_ofdmphy;

#define HDR_OFDMPHY(p) (hdr_ofdmphy::access(p))

#endif
