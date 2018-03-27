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
 * $Id: wimax-tracer.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "wimax-tracer.h"

WimaxTracer::WimaxTracer() : Tracer(1) {}



void WimaxTracer::format(Packet *p, SAP *sap)
{
	hdr_cmn* ch = HDR_CMN(p);
	hdr_ofdmphy* oh = HDR_OFDMPHY(p);
	packet_t  t = ch->ptype();

	string descr;
	bool dataPkt = true;

	if(t == PT_WIMAXBS)
	{
		dataPkt = false;
		descr.append("BS CTRL");
	}
	else if(t == PT_MAC)
	{
		descr.append("MAC");
		// NOTE: invalid cast, but ok because all frametypes start with a type
		if (HDR_MAC802_16(p)->header.ht != 0)
			descr.append(" BWREQ");
		else
		{
			switch (((mac802_16_dl_map_frame*) p->accessdata())->type) {
				case MAC_DL_MAP:
					descr.append(" DL MAP");
					break;
				case MAC_DCD:
					descr.append(" DCD");
					break;
				case MAC_UL_MAP:
					descr.append(" UL MAP");
					break;
				case MAC_UCD:
					descr.append(" UCD");
					break;
				case MAC_RNG_RSP:
					descr.append(" RNG RSP");
					break;
				case MAC_REG_RSP:
					descr.append(" REG RSP");
					break;
				case MAC_MOB_SCN_RSP:
					descr.append(" MOB SCN RSP");
					break;
				case MAC_MOB_BSHO_RSP:
					descr.append(" MOB BSHO RSP");
					break;
				case MAC_MOB_NBR_ADV:
					descr.append(" MOB NBR ADV");
					break;
				case MAC_DSA_REQ:
					descr.append(" DSA REQ");
					break;
				case MAC_DSA_RSP:
					descr.append(" DSA RSP");
					break;
				case MAC_DSA_ACK:
					descr.append(" DSA ACK");
					break;
				default:
					descr.append(" UNKN");
			}
		}
		dataPkt = false;
	}
	else
		return;

	string modulation;
	switch(oh->modulation_)
	{
		case OFDM_BPSK_1_2:
			modulation.append("BPSK 1/2");
			break;
		case OFDM_QPSK_1_2:
			modulation.append("QPSK 1/2");
			break;
		case OFDM_QPSK_3_4:
			modulation.append("QPSK 3/4");
			break;
		case OFDM_16QAM_1_2:
			modulation.append("16QAM 1/2");
			break;
		case OFDM_16QAM_3_4:
			modulation.append("16QAM 3/4");
			break;
		case OFDM_64QAM_2_3:
			modulation.append("64QAM 2/3");
			break;
		case OFDM_64QAM_3_4:
			modulation.append("64QAM 3/4");
			break;
	}

	if(oh->freq_ != 0)
	{
		writeTrace(sap, " WiMAX %s %s PHY %lf %s",
			(dataPkt) ? "D" : "C",
			descr.c_str(),
			oh->freq_/1000000,
			modulation.c_str());
	}
	else
	{
		writeTrace(sap, " WiMAX %s %s",
			(dataPkt) ? "D" : "C",
			descr.c_str());
	}
}

extern "C" int Wimaxtracer_Init()
{
	SAP::addTracer(new WimaxTracer);
	return 0;
}
extern "C" int  Cygwimaxtracer_Init()
{
	Wimaxtracer_Init();
}
