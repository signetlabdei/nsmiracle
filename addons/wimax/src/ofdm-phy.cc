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
 * $Id: ofdm-phy.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "ofdm-phy.h"

static class OfdmPhyClass : public TclClass {
	public:
		OfdmPhyClass() : TclClass("Module/MPhy/OFDM") {}
		TclObject* create(int, const char*const*) {
			return (new OfdmPhy());
		}
} class_OfdmPhy;

bool OfdmPhy::mod_initialized = false;
int OfdmPhy::modid[7] = {-1, -1, -1, -1, -1, -1, -1};

OfdmPhy::OfdmPhy()
{
	// Bind attributes
	bind("CPThresh_", &CPThresh_);
	bind("CSThresh_", &CSThresh_);
	bind("RXThresh_", &RXThresh_);
	bind ("g_", &g_);
	bind ("freq_", &freq_);

	if (!mod_initialized)
	{
		modid[OFDM_BPSK_1_2] = MPhy::registerModulationType(OFDM_MODNAMES[OFDM_BPSK_1_2]);
		modid[OFDM_QPSK_1_2] = MPhy::registerModulationType(OFDM_MODNAMES[OFDM_QPSK_1_2]);
		modid[OFDM_QPSK_3_4] = MPhy::registerModulationType(OFDM_MODNAMES[OFDM_QPSK_3_4]);
		modid[OFDM_16QAM_1_2] = MPhy::registerModulationType(OFDM_MODNAMES[OFDM_16QAM_1_2]);
		modid[OFDM_16QAM_3_4] = MPhy::registerModulationType(OFDM_MODNAMES[OFDM_16QAM_3_4]);
		modid[OFDM_64QAM_2_3] = MPhy::registerModulationType(OFDM_MODNAMES[OFDM_64QAM_2_3]);
		modid[OFDM_64QAM_3_4] = MPhy::registerModulationType(OFDM_MODNAMES[OFDM_64QAM_3_4]);
		mod_initialized = true;
	}

	modulation_ = OFDM_BPSK_1_2;
	Tcl& tcl = Tcl::instance();
	tcl.evalf("Module/802_16/MAC set fbandwidth_");
	fbandwidth_ = atof (tcl.result());
	state_ = OFDM_IDLE;
	node_on();

	updateFs ();
}

OfdmPhy::~OfdmPhy()
{
}

int OfdmPhy::recvSyncClMsg(ClMessage* m)
{
	if(m->type() == WIMAX_CLMSG_SET_PHYMODE)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_SET_PHYMODE -> set mode to %d\n", ((WimaxClMsgSetPHYMode *)m)->getMode());
		setMode(((WimaxClMsgSetPHYMode *)m)->getMode());
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_SET_MODULATION)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_SET_MODULATION -> set modulation to %d\n", ((WimaxClMsgSetPHYModulation *)m)->getModulation());
		setModulation(((WimaxClMsgSetPHYModulation *)m)->getModulation());
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_SET_FREQ)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_SET_FREQ -> set frequency to %d\n", ((WimaxClMsgSetPHYFreq *)m)->getFreq());
		setFrequency(((WimaxClMsgSetPHYFreq *)m)->getFreq());
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_GET_SYMBOLTIME)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_GET_SYMBOLTIME -> get symboltime\n");
		((WimaxClMsgGetPHYSymbolTime *)m)->setSymbolTime(getSymbolTime());
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_GET_PS)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_GET_PS -> get physical slot\n");
		((WimaxClMsgGetPHYPS *)m)->setPS(getPS());
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_GET_TRXTIME)
	{
		WimaxClMsgGetPHYTrxTime *m_ptr = (WimaxClMsgGetPHYTrxTime*)m;
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_GET_TRXTIME -> get transmission time\n");

		m_ptr->setTrxTime(getTrxTime(m_ptr->getSize(), m_ptr->getModRate()));
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_GET_TRXSYMBOLTIME)
	{
		WimaxClMsgGetPHYTrxSymbolTime *m_ptr = (WimaxClMsgGetPHYTrxSymbolTime*)m;
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_GET_TRXSYMBOLTIME -> get transmission time using OFDM symbol\n");

		m_ptr->setTrxSymbolTime(getTrxSymbolTime(m_ptr->getSize(), m_ptr->getModRate()));
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_SET_NODE_ON)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_SET_NODE_ON -> activate node\n");
		node_on();
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_SET_NODE_OFF)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_SET_NODE_OFF -> deactivate node\n");
		node_off();
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_GET_SYMBOLPS)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_GET_SYMBOLPS -> get number of PS\n");
		((WimaxClMsgGetSymbolPs *)m)->setSymbolPs(getSymbolPS());
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_GET_FREQ)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_GET_FREQ -> get frequency\n");
		((WimaxClMsgGetFreq *)m)->setFreq(getFreq());
		return 1;
	}
	else if(m->type() == WIMAX_CLMSG_GET_MAX_PKT_SIZE)
	{
		if(debug_ > 5)
			printf("OfdmPhy, rx a WIMAX_CLMSG_GET_MAX_PKT_SIZE -> get max packet size\n");
		WimaxClMsgGetMaxPktSize *mtmp = (WimaxClMsgGetMaxPktSize*)m;
		mtmp->setMaxPktSize(getMaxPktSize(mtmp->getNbsymbols(), mtmp->getModulation()));
		return 1;
	}
	else
	{
		if (debug_ > 5)
			printf("OfdmPhy, rx an unknown cl-msg (type = %i)\n",m->type());
	}
	return Module::recvSyncClMsg(m);
}

int OfdmPhy::command(int argc, const char*const* argv)
{
	return MPhy::command(argc, argv);
}

void OfdmPhy::startTx(Packet* p)
{
	if (state_ != OFDM_SEND) {
		if(debug_ > 5)
			printf ("OfdmPhy, Warning: OFDM not in sending state. Drop packet.\n");
		Packet::free (p);
		return;
	}

	hdr_ofdmphy* oh = HDR_OFDMPHY(p);

	oh->freq_ = getFreq();
	oh->modulation_ = getModulation();
	oh->g_ = getG();

	sendDown(p);
}

void OfdmPhy::endTx(Packet* p)
{
}

void OfdmPhy::startRx(Packet* p)
{

}

void OfdmPhy::endRx(Packet* p)
{
	if (!activated_)
		return;

	hdr_ofdmphy* oh = HDR_OFDMPHY(p);
	if (freq_ != oh->freq_) {
		if(debug_ > 5)
			printf ("OfdmPhy, drop packet because frequency is different (%f, %f)\n", freq_,oh->freq_);
		Packet::free(p);
		return;
	}

	/* Check phy status */
	if (state_ != OFDM_RECV) {
		if(debug_ > 5)
			printf ("OfdmPhy, Warning: OFDM phy not in receiving state. Drop packet.\n");
		Packet::free(p);
		return;
	}

	/* Begin Threshhold checks from WirelessPhy */
	hdr_MPhy *ph = HDR_MPHY(p);

	if (ph->Pr < CSThresh_) {
		if(debug_ > 5)
			printf ("OfdmPhy, Recieve power < CSThresh_. Drop packet.\n");
		Packet::free(p);
		return;
	}
	if (ph->Pr < RXThresh_) {
			/*
		* We can detect, but not successfully receive
		* this packet.
			*/
		hdr_cmn *hdr = HDR_CMN(p);
		hdr->error() = 1;

		if(debug_ > 5)
			printf ("OfdmPhy, Recieve power < RXThresh_. Setting packet in error state.\n");
	}

	/* WILD HACK: From wireless-phy.cc This info is needed on the 802.16 mac layer */
	p->txinfo_.RxPr = ph->Pr;
	p->txinfo_.CPThresh = CPThresh_;

	/* End WirelessPhy implementation fixes */

	if(debug_ > 5)
		printf ("OfdmPhy, receiving packet with mod=%d and G=%f\n", oh->modulation_,oh->g_);

	sendUp(p);
}

double OfdmPhy::getPS()
{
	return (4/getFs());
}

int OfdmPhy::getSymbolPS()
{
	return (int) (ceil (getSymbolTime() / getPS()));
}

double OfdmPhy::getTxDuration(Packet*)
{
	/* WILD HACK: No duration, we take care of txtime in the MAC-layer */
	return 0.000000001;
}

int OfdmPhy::getModulationType(Packet* p)
{
	assert(mod_initialized);
	hdr_ofdmphy* oh = HDR_OFDMPHY(p);
	return modid[oh->modulation_];
}

void OfdmPhy::updateFs ()
{
	double n;

	if (((int) (fbandwidth_ / 1.75)) * 1.75 == fbandwidth_) {
		n = 8.0/7;
	} else if (((int) (fbandwidth_ / 1.5)) * 1.5 == fbandwidth_) {
		n = 86.0/75;
	} else if (((int) (fbandwidth_ / 1.25)) * 1.25 == fbandwidth_) {
		n = 144.0/125;
	} else if (((int) (fbandwidth_ / 2.75)) * 2.75 == fbandwidth_) {
		n = 316.0/275;
	} else if (((int) (fbandwidth_ / 2.0)) * 2.0 == fbandwidth_) {
		n = 57.0/50;
	} else {
		n = 8.0/7;
	}

	fs_ = floor (n*fbandwidth_/8000) * 8000;

	if(debug_ > 5)
		printf ("Fs updated. Bw=%f, n=%f, new value is %e\n", fbandwidth_, n, fs_);
}

void OfdmPhy::setFrequency(double freq)
{
	freq_ = freq;
	Tcl& tcl = Tcl::instance();
	MSpectralMask* spectralmask = dynamic_cast<MSpectralMask*>(spectralmask_);
	if(spectralmask)
		tcl.evalf("%s setFreq %lf",spectralmask->name(), freq_);
}

double OfdmPhy::getFreq()
{
	return freq_;
}

Ofdm_mod_rate OfdmPhy::getModulation()
{
	return modulation_;
}

void OfdmPhy::setModulation(Ofdm_mod_rate modulation)
{
	modulation_ = modulation;
}

void OfdmPhy::setMode(Ofdm_phy_state mode)
{
	state_ = mode;
}

double OfdmPhy::getSymbolTime()
{
	return ((1+getG())*((double)NFFT))/getFs();
}

double OfdmPhy::getTrxTime(int sdusize, Ofdm_mod_rate mod)
{
	//we compute the number of symbols required and the bits per symbol
	int nb_symbols, bpsymb;

	switch(mod){
		case OFDM_BPSK_1_2:
			bpsymb = OFDM_BPSK_1_2_bpsymb; break;
		case OFDM_QPSK_1_2:
			bpsymb = OFDM_QPSK_1_2_bpsymb; break;
		case OFDM_QPSK_3_4:
			bpsymb = OFDM_QPSK_3_4_bpsymb; break;
		case OFDM_16QAM_1_2:
			bpsymb = OFDM_16QAM_1_2_bpsymb; break;
		case OFDM_16QAM_3_4:
			bpsymb = OFDM_16QAM_3_4_bpsymb; break;
		case OFDM_64QAM_2_3:
			bpsymb = OFDM_64QAM_2_3_bpsymb; break;
		case OFDM_64QAM_3_4:
			bpsymb = OFDM_64QAM_3_4_bpsymb; break;
		default:
			printf ("Error: unknown modulation: method getTrxTime in file ofdmphy.cc\n");
			int *t=NULL;
			printf ("%d",*t);
			exit (1);
	}
	return sdusize*8*getSymbolTime()/bpsymb;
}

double OfdmPhy::getTrxSymbolTime (int sdusize, Ofdm_mod_rate mod) {
	//we compute the number of symbols required and the bits per symbol
	int nb_symbols, bpsymb;

	switch (mod) {
		case OFDM_BPSK_1_2:
			bpsymb = OFDM_BPSK_1_2_bpsymb; break;
		case OFDM_QPSK_1_2:
			bpsymb = OFDM_QPSK_1_2_bpsymb; break;
		case OFDM_QPSK_3_4:
			bpsymb = OFDM_QPSK_3_4_bpsymb; break;
		case OFDM_16QAM_1_2:
			bpsymb = OFDM_16QAM_1_2_bpsymb; break;
		case OFDM_16QAM_3_4:
			bpsymb = OFDM_16QAM_3_4_bpsymb; break;
		case OFDM_64QAM_2_3:
			bpsymb = OFDM_64QAM_2_3_bpsymb; break;
		case OFDM_64QAM_3_4:
			bpsymb = OFDM_64QAM_3_4_bpsymb; break;
		default:
			printf ("Error: unknown modulation: method getTrxTime in file ofdmphy.cc\n");
			int *t=NULL;
			printf ("%d",*t);
			exit (1);
	}

	nb_symbols = (int) (ceil(((double)sdusize*8)/bpsymb));
	return (nb_symbols*getSymbolTime ());
}

double OfdmPhy::getG()
{
	return g_;
}

double OfdmPhy::getFs()
{
	return fs_;
}

int OfdmPhy::getMaxPktSize(double nbsymbols, Ofdm_mod_rate mod)
{
	int bpsymb;

	switch (mod) {
		case OFDM_BPSK_1_2:
			bpsymb = OFDM_BPSK_1_2_bpsymb;
			break;
		case OFDM_QPSK_1_2:
			bpsymb = OFDM_QPSK_1_2_bpsymb;
			break;
		case OFDM_QPSK_3_4:
			bpsymb = OFDM_QPSK_3_4_bpsymb;
			break;
		case OFDM_16QAM_1_2:
			bpsymb = OFDM_16QAM_1_2_bpsymb;
			break;
		case OFDM_16QAM_3_4:
			bpsymb = OFDM_16QAM_3_4_bpsymb;
			break;
		case OFDM_64QAM_2_3:
			bpsymb = OFDM_64QAM_2_3_bpsymb;
			break;
		case OFDM_64QAM_3_4:
			bpsymb = OFDM_64QAM_3_4_bpsymb;
			break;
		default:
			printf ("Error: unknown modulation: method getTrxTime in file ofdm-phy.cc\n");
			exit (1);
	}

	return (int)(nbsymbols*bpsymb)/8;
}

void OfdmPhy::node_on()
{
	activated_ = true;
}

void OfdmPhy::node_off()
{
	activated_ = false;
}
