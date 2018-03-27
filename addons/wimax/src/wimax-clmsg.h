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
 * $Id: wimax-clmsg.h 97 2008-03-26 16:03:11Z erikande $
 */

#ifndef _WIMAX_CLMSG_H_
#define _WIMAX_CLMSG_H_

#include "clmessage.h"
#include "ofdm-packet.h"

#define VERBOSITY 5	// verbosity of this message

extern ClMessage_t WIMAX_CLMSG_SET_PHYMODE;
extern ClMessage_t WIMAX_CLMSG_SET_MODULATION;
extern ClMessage_t WIMAX_CLMSG_SET_FREQ;
extern ClMessage_t WIMAX_CLMSG_GET_SYMBOLTIME;
extern ClMessage_t WIMAX_CLMSG_GET_PS;
extern ClMessage_t WIMAX_CLMSG_GET_TRXTIME;
extern ClMessage_t WIMAX_CLMSG_GET_TRXSYMBOLTIME;
extern ClMessage_t WIMAX_CLMSG_SET_NODE_ON;
extern ClMessage_t WIMAX_CLMSG_SET_NODE_OFF;
extern ClMessage_t WIMAX_CLMSG_GET_SYMBOLPS;
extern ClMessage_t WIMAX_CLMSG_GET_FREQ;
extern ClMessage_t WIMAX_CLMSG_GET_MAX_PKT_SIZE;

/**
 * Message to set modulation on PHY module
 */
class WimaxClMsgSetPHYMode : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgSetPHYMode(Ofdm_phy_state mode);

		/** Copy constructor */
		WimaxClMsgSetPHYMode(WimaxClMsgSetPHYMode *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Get the mode for physical layer
		 * @return mode for phy layer (as Ofdm_phy_state enum)
		 */
		Ofdm_phy_state getMode();
	private:
		/** phy layer modulation */
		Ofdm_phy_state mode_;
	
};

/**
 * Message to set modulation on PHY module
 */
class WimaxClMsgSetPHYModulation : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgSetPHYModulation(Ofdm_mod_rate modulation);

		/** Copy constructor */
		WimaxClMsgSetPHYModulation(WimaxClMsgSetPHYModulation *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Get the modulation for physical layer
		 * @return modulation for phy layer (as Ofdm_mod_rate enum)
		 */
		Ofdm_mod_rate getModulation();
	private:
		/** phy layer modulation */
		Ofdm_mod_rate modulation_;
	
};

/**
 * Message to set frequency on PHY module
 */
class WimaxClMsgSetPHYFreq : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgSetPHYFreq(double freq);

		/** Copy constructor */
		WimaxClMsgSetPHYFreq(WimaxClMsgSetPHYFreq *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Get the frequency for physical layer
		 * @return frequency for phy layer (as double)
		 */
		double getFreq();
	private:
		/** phy layer frequency */
		double freq_;
	
};

/**
 * Message to get symbol time on PHY module
 */
class WimaxClMsgGetPHYSymbolTime : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgGetPHYSymbolTime();

		/** Copy constructor */
		WimaxClMsgGetPHYSymbolTime(WimaxClMsgGetPHYSymbolTime *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Sets the symboltime for physical layer
		 * @param symboltime The symboltime
		 */
		void setSymbolTime(double symboltime);

		/**
		 * Get the symboltime for physical layer
		 * @return symboltime for phy layer (as double)
		 */
		double getSymbolTime();
	private:
		/** phy layer symbol time */
		double symboltime_;
	
};

/**
 * Message to get physical slot from PHY module
 */
class WimaxClMsgGetPHYPS : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgGetPHYPS();

		/** Copy constructor */
		WimaxClMsgGetPHYPS(WimaxClMsgGetPHYPS *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Sets the PS for physical layer
		 * @param ps The physical slot
		 */
		void setPS(double ps);

		/**
		 * Get the PS for physical layer
		 * @return physical slot for phy layer (as double)
		 */
		double getPS();
	private:
		/** phy layer physical slot */
		double ps_;
	
};

/**
 * Message to get transmission time of a packet from PHY module
 */
class WimaxClMsgGetPHYTrxTime : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgGetPHYTrxTime(int sdusize, Ofdm_mod_rate mod);

		/** Copy constructor */
		WimaxClMsgGetPHYTrxTime(WimaxClMsgGetPHYTrxTime *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Get size in bytes of the data to send
		 * @return The size in bytes of the data to send
		 */
		int getSize();

		/**
		 * Get the modulation rate
		 * @return The modulation rate
		 */
		Ofdm_mod_rate getModRate();

		/**
		 * Get the transmission time
		 * @return The transmission time
		 */
		double getTrxTime();

		/**
		 * Set the transmission time
		 * @param trxtime The new value of the transmission time
		 */
		void setTrxTime(double trxtime);

	private:
		/** The size (in bytes) of the data to send */
		int sdusize_;

		/** The modulation to use */
		Ofdm_mod_rate mod_;
 
		/** Transmission time for a packet with the given size and modulation */
		double trxtime_;
};

/**
 * Message to get transmission time of a packet from PHY module using OFDM symbol
 */
class WimaxClMsgGetPHYTrxSymbolTime : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgGetPHYTrxSymbolTime(int sdusize, Ofdm_mod_rate mod);

		/** Copy constructor */
		WimaxClMsgGetPHYTrxSymbolTime(WimaxClMsgGetPHYTrxSymbolTime *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Get size in bytes of the data to send
		 * @return The size in bytes of the data to send
		 */
		int getSize();

		/**
		 * Get the modulation rate
		 * @return The modulation rate
		 */
		Ofdm_mod_rate getModRate();

		/**
		 * Get the transmission time
		 * @return The transmission time
		 */
		double getTrxSymbolTime();

		/**
		 * Set the transmission time
		 * @param trxtime The new value of the transmission time
		 */
		void setTrxSymbolTime(double trxtime);

	private:
		/** The size (in bytes) of the data to send */
		int sdusize_;

		/** The modulation to use */
		Ofdm_mod_rate mod_;
 
		/** Transmission time for a packet with the given size and modulation using OFDM symbol */
		double trxtime_;
};

/**
 * Message to activate the node (physical layer)
 */
class WimaxClMsgSetNodeOn : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgSetNodeOn();

		/** Copy constructor */
		WimaxClMsgSetNodeOn(WimaxClMsgSetNodeOn *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();
	private:
	
};

/**
 * Message to deactivate the node (physical layer)
 */
class WimaxClMsgSetNodeOff : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgSetNodeOff();

		/** Copy constructor */
		WimaxClMsgSetNodeOff(WimaxClMsgSetNodeOff *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();
	private:
	
};

/**
 * Message to get the number of PS used by an OFDM symbol
 */
class WimaxClMsgGetSymbolPs : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgGetSymbolPs();

		/** Copy constructor */
		WimaxClMsgGetSymbolPs(WimaxClMsgGetSymbolPs *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Sets the symbol PS for physical layer
		 * @param symbolps Number of PS used by an OFDM symbol
		 */
		void setSymbolPs(int symbolps);

		/**
		 * Get the number of PS used by an OFDM symbol
		 * @return Number of PS used by an OFDM symbol
		 */
		int getSymbolPs();
	private:
		/** Number of PS used by an OFDM symbol */
		int symbolps_;
	
};

/**
 * Message to get the frequency for the physical channel
 */
class WimaxClMsgGetFreq : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgGetFreq();

		/** Copy constructor */
		WimaxClMsgGetFreq(WimaxClMsgGetFreq *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Sets the frequency for physical layer
		 * @param freq Frequency at which phy operates
		 */
		void setFreq(double freq);

		/**
		 * Get the frequency for physical layer
		 * @return Frequency at which phy operates
		 */
		double getFreq();
	private:
		/** Frequency used by phy layer */
		double freq_;
	
};

/**
 * Message to get the the maximun packet size
 */
class WimaxClMsgGetMaxPktSize : public ClMessage
{
	public:
		/** Constructor */
		WimaxClMsgGetMaxPktSize(double nbsymbols, Ofdm_mod_rate mod);

		/** Copy constructor */
		WimaxClMsgGetMaxPktSize(WimaxClMsgGetMaxPktSize *m);

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Sets the maximal packet size
		 * @param maxPktSize maximal packet size
		 */
		void setMaxPktSize(int maxPktSize);

		/**
		 * Get the maximal packet size
		 * @return maximal packet size
		 */
		int getMaxPktSize();

		/**
		 * Get the number of symbols
		 * @return number of symbols
		 */
		double getNbsymbols();

		/**
		 * Get the modulation rate
		 * @return modulation rate
		 */
		Ofdm_mod_rate getModulation();
	private:
		/** Maximum packet size */
		int maxPktSize_;

		/** Number of symbols */
		double nbsymbols_;

		/** Modulation */
		Ofdm_mod_rate mod_;
};

#endif
