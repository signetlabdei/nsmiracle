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
 * $Id: wimax-clmsg.cc 97 2008-03-26 16:03:11Z erikande $
 */

#include "wimax-clmsg.h"
#include "clsap.h"

/**
 * Message to set mode on PHY module
 */
WimaxClMsgSetPHYMode::WimaxClMsgSetPHYMode(Ofdm_phy_state mode) : ClMessage(VERBOSITY, WIMAX_CLMSG_SET_PHYMODE), mode_(mode) {}
WimaxClMsgSetPHYMode::WimaxClMsgSetPHYMode(WimaxClMsgSetPHYMode *m) : ClMessage(m), mode_(m->getMode()) { }

Ofdm_phy_state WimaxClMsgSetPHYMode::getMode()
{
	return(mode_);
}

ClMessage *WimaxClMsgSetPHYMode::copy()
{
	return (new WimaxClMsgSetPHYMode(this));
}

/**
 * Message to set modulation on PHY module
 */
WimaxClMsgSetPHYModulation::WimaxClMsgSetPHYModulation(Ofdm_mod_rate modulation) : ClMessage(VERBOSITY, WIMAX_CLMSG_SET_MODULATION), modulation_(modulation) {}
WimaxClMsgSetPHYModulation::WimaxClMsgSetPHYModulation(WimaxClMsgSetPHYModulation *m) : ClMessage(m), modulation_(m->getModulation()) { }

Ofdm_mod_rate WimaxClMsgSetPHYModulation::getModulation()
{
	return(modulation_);
}

ClMessage *WimaxClMsgSetPHYModulation::copy()
{
	return (new WimaxClMsgSetPHYModulation(this));
}

/**
 * Message to set frequency on PHY module
 */
WimaxClMsgSetPHYFreq::WimaxClMsgSetPHYFreq(double freq) : ClMessage(VERBOSITY, WIMAX_CLMSG_SET_FREQ), freq_(freq) {}
WimaxClMsgSetPHYFreq::WimaxClMsgSetPHYFreq(WimaxClMsgSetPHYFreq *m) : ClMessage(m), freq_(m->getFreq()) { }

double WimaxClMsgSetPHYFreq::getFreq()
{
	return(freq_);
}

ClMessage *WimaxClMsgSetPHYFreq::copy()
{
	return (new WimaxClMsgSetPHYFreq(this));
}

/**
 * Message to get symbol time on PHY module
 */
WimaxClMsgGetPHYSymbolTime::WimaxClMsgGetPHYSymbolTime() : ClMessage(VERBOSITY, WIMAX_CLMSG_GET_SYMBOLTIME), symboltime_(0) {}
WimaxClMsgGetPHYSymbolTime::WimaxClMsgGetPHYSymbolTime(WimaxClMsgGetPHYSymbolTime *m) : ClMessage(m), symboltime_(m->getSymbolTime()) { }

void WimaxClMsgGetPHYSymbolTime::setSymbolTime(double symboltime)
{
	symboltime_ = symboltime;
}

double WimaxClMsgGetPHYSymbolTime::getSymbolTime()
{
	return(symboltime_);
}

ClMessage *WimaxClMsgGetPHYSymbolTime::copy()
{
	return (new WimaxClMsgGetPHYSymbolTime(this));
}

/**
 * Message to get physical slot from PHY module
 */
WimaxClMsgGetPHYPS::WimaxClMsgGetPHYPS() : ClMessage(VERBOSITY, WIMAX_CLMSG_GET_PS), ps_(0) {}
WimaxClMsgGetPHYPS::WimaxClMsgGetPHYPS(WimaxClMsgGetPHYPS *m) : ClMessage(m), ps_(m->getPS()) { }

void WimaxClMsgGetPHYPS::setPS(double ps)
{
	ps_ = ps;
}

double WimaxClMsgGetPHYPS::getPS()
{
	return(ps_);
}

ClMessage *WimaxClMsgGetPHYPS::copy()
{
	return (new WimaxClMsgGetPHYPS(this));
}

/**
 * Message to get the time required to transmit a packet with certain parameters from PHY module
 */
WimaxClMsgGetPHYTrxTime::WimaxClMsgGetPHYTrxTime(int sdusize, Ofdm_mod_rate mod) : ClMessage(VERBOSITY, WIMAX_CLMSG_GET_TRXTIME), sdusize_(sdusize), mod_(mod) {}
WimaxClMsgGetPHYTrxTime::WimaxClMsgGetPHYTrxTime(WimaxClMsgGetPHYTrxTime *m) : ClMessage(m), sdusize_(m->getSize()), mod_(m->getModRate()), trxtime_(m->getTrxTime()) {}

int WimaxClMsgGetPHYTrxTime::getSize()
{
	return sdusize_;
}

Ofdm_mod_rate WimaxClMsgGetPHYTrxTime::getModRate()
{
	return mod_;
}

double WimaxClMsgGetPHYTrxTime::getTrxTime()
{
	return trxtime_;
}

void WimaxClMsgGetPHYTrxTime::setTrxTime(double trxtime)
{
	trxtime_ = trxtime;
}

ClMessage *WimaxClMsgGetPHYTrxTime::WimaxClMsgGetPHYTrxTime::copy()
{
	return (new WimaxClMsgGetPHYTrxTime(this));
}

/**
 * Message to get the time required to transmit a packet with certain parameters from PHY module
 */
WimaxClMsgGetPHYTrxSymbolTime::WimaxClMsgGetPHYTrxSymbolTime(int sdusize, Ofdm_mod_rate mod) : ClMessage(VERBOSITY, WIMAX_CLMSG_GET_TRXSYMBOLTIME), sdusize_(sdusize), mod_(mod) {}
WimaxClMsgGetPHYTrxSymbolTime::WimaxClMsgGetPHYTrxSymbolTime(WimaxClMsgGetPHYTrxSymbolTime *m) : ClMessage(m), sdusize_(m->getSize()), mod_(m->getModRate()), trxtime_(m->getTrxSymbolTime()) {}

int WimaxClMsgGetPHYTrxSymbolTime::getSize()
{
	return sdusize_;
}

Ofdm_mod_rate WimaxClMsgGetPHYTrxSymbolTime::getModRate()
{
	return mod_;
}

double WimaxClMsgGetPHYTrxSymbolTime::getTrxSymbolTime()
{
	return trxtime_;
}

void WimaxClMsgGetPHYTrxSymbolTime::setTrxSymbolTime(double trxtime)
{
	trxtime_ = trxtime;
}

ClMessage *WimaxClMsgGetPHYTrxSymbolTime::WimaxClMsgGetPHYTrxSymbolTime::copy()
{
	return (new WimaxClMsgGetPHYTrxSymbolTime(this));
}

/**
 * Message to activate the node (physical layer)
 */
WimaxClMsgSetNodeOn::WimaxClMsgSetNodeOn() : ClMessage(VERBOSITY, WIMAX_CLMSG_SET_NODE_ON) {}
WimaxClMsgSetNodeOn::WimaxClMsgSetNodeOn(WimaxClMsgSetNodeOn *m) : ClMessage(m) {}


ClMessage *WimaxClMsgSetNodeOn::copy()
{
	return (new WimaxClMsgSetNodeOn(this));
}

/**
 * Message to deactivate the node (physical layer)
 */
WimaxClMsgSetNodeOff::WimaxClMsgSetNodeOff() : ClMessage(VERBOSITY, WIMAX_CLMSG_SET_NODE_OFF) {}
WimaxClMsgSetNodeOff::WimaxClMsgSetNodeOff(WimaxClMsgSetNodeOff *m) : ClMessage(m) {}


ClMessage *WimaxClMsgSetNodeOff::copy()
{
	return (new WimaxClMsgSetNodeOff(this));
}

/**
 * Message to get the number of PS used by an OFDM symbol
 */
WimaxClMsgGetSymbolPs::WimaxClMsgGetSymbolPs() : ClMessage(VERBOSITY, WIMAX_CLMSG_GET_SYMBOLPS), symbolps_(0) {}
WimaxClMsgGetSymbolPs::WimaxClMsgGetSymbolPs(WimaxClMsgGetSymbolPs *m) : ClMessage(m), symbolps_(m->getSymbolPs()) {}

void WimaxClMsgGetSymbolPs::setSymbolPs(int symbolps)
{
	symbolps_ = symbolps;
}

int WimaxClMsgGetSymbolPs::getSymbolPs()
{
	return(symbolps_);
}

ClMessage *WimaxClMsgGetSymbolPs::copy()
{
	return (new WimaxClMsgGetSymbolPs(this));
}

/**
 * Message to get the frequency used by the PHY layer
 */
WimaxClMsgGetFreq::WimaxClMsgGetFreq() : ClMessage(VERBOSITY, WIMAX_CLMSG_GET_FREQ), freq_(0) {}
WimaxClMsgGetFreq::WimaxClMsgGetFreq(WimaxClMsgGetFreq *m) : ClMessage(m), freq_(m->getFreq()) {}

void WimaxClMsgGetFreq::setFreq(double freq)
{
	freq_ = freq;
}

double WimaxClMsgGetFreq::getFreq()
{
	return(freq_);
}

ClMessage *WimaxClMsgGetFreq::copy()
{
	return (new WimaxClMsgGetFreq(this));
}

/**
 * Message to get the the maximun packet size
 */
WimaxClMsgGetMaxPktSize::WimaxClMsgGetMaxPktSize(double nbsymbols, Ofdm_mod_rate mod) : ClMessage(VERBOSITY, WIMAX_CLMSG_GET_MAX_PKT_SIZE), maxPktSize_(0), nbsymbols_(nbsymbols), mod_(mod) {}
WimaxClMsgGetMaxPktSize::WimaxClMsgGetMaxPktSize(WimaxClMsgGetMaxPktSize *m) : ClMessage(m), maxPktSize_(m->getMaxPktSize()), nbsymbols_(m->getNbsymbols()), mod_(m->getModulation()) {}

void WimaxClMsgGetMaxPktSize::setMaxPktSize(int maxPktSize)
{
	maxPktSize_ = maxPktSize;
}

int WimaxClMsgGetMaxPktSize::getMaxPktSize()
{
	return maxPktSize_;
}

double WimaxClMsgGetMaxPktSize::getNbsymbols()
{
	return nbsymbols_;
}

Ofdm_mod_rate WimaxClMsgGetMaxPktSize::getModulation()
{
	return mod_;
}

ClMessage *WimaxClMsgGetMaxPktSize::copy()
{
	return (new WimaxClMsgGetMaxPktSize(this));
}
