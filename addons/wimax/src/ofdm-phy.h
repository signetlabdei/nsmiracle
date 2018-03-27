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
 * $Id: ofdm-phy.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef OFDMPHY_H 
#define OFDMPHY_H

#include <mphy.h>
#include "ofdm-packet.h"
#include "wimax-clmsg.h"

/* Define subcarrier information */
const int NFFT = 256;
const int NUSED = 200;

const char OFDM_MODNAMES[][10] = {"BPSK_1_2", "QPSK_1_2", "QPSK_3_4", "16QAM_1_2", "16QAM_3_4", "64QAM_2_3", "64QAM_3_4"};

/** Implements OFDM PHY layer for 802.16 */
class OfdmPhy : public MPhy
{
	public:
		/** Constructor */
		OfdmPhy();

		/** Destructor */
		virtual ~OfdmPhy();

		/**
		 * Cross-Layer messages synchronous interpreter.
		 * @param m an instance of <i>ClMessage</i> that represent the message received and used for the answer
		 * @return >0 or 0 whether the command has been dispatched succesfully or no
		 * @see NodeCore, ClMessage, ClSAP, ClTracer
		 **/
//		virtual int crLaySynchronousCommand(ClMessage* m);			// DEPRECATED method
		virtual int recvSyncClMsg(ClMessage* m);
		

		virtual int command(int argc, const char*const* argv);

	protected:
		/** 
		 * Method invoked at the beginning of the transmission of a given
		 * packet.
		 * @param p the given packet
		 */
		virtual void startTx(Packet* p);

		/**
		 * Method invoked at the end of the transmission of a given
		 * packet.
		 * @param p the given packet
		 */
		virtual void endTx(Packet* p);

		/**
		 * Method invoked at the beginning of the reception of a given
		 * packet.
		 * @param p the given packet
		 */
		virtual void startRx(Packet* p);

		/**
		 * Method invoked at the end of the reception of a given
		 * packet.
		 * @param p the given packet
		 */
		virtual void endRx(Packet* p);

	private:
		/**
		 * Return the duration of a PS (physical slot), unit for allocation time.
		 * Use Frame duration / PS to find the number of available slot per frame
		 */
		virtual double getPS();
		
		/**
		 * Return the number of PS used by an OFDM symbol
		 */
		virtual int getSymbolPS();
		
		/** 
		 * Returns the duration of the transmission for the packet being
		 * transmitted.
		 * @param p the packet being transmitted
		 * @return duration in s
		 */
		virtual double getTxDuration(Packet* p);

		/** 
		 * Get the modulation type to be used for the packet being
		 * transmitted.
		 * @param p the packet being transmitted
		 * @return the modulation type
		 */
		virtual int getModulationType(Packet* p);

		/**
		  * Return the OFDM symbol duration time
		  */
		virtual double getSymbolTime();

		/**
		 * Get frequency at which the phy is operating
		 * @return frequency as a double
		 */
		virtual double getFreq();

		/**
		 * Return the current modulation
		 */
		virtual Ofdm_mod_rate getModulation();
		
		/**
		 * Compute the transmission time for a packet of size sdusize and
		 * using the given modulation rate
		 * @param sdusize Size in bytes of the data to send
		 * @param mod The modulation to use
		 */
		virtual double getTrxTime(int sdusize, Ofdm_mod_rate mod);

		/**
		 * Compute the transmission time using OFDM symbol as
		 * minimum allocation for a packet of size sdusize and
		 * using the given modulation rate
		 * @param sdusize Size in bytes of the data to send
		 * @param mod The modulation to use
		 */
		virtual double getTrxSymbolTime(int sdusize, Ofdm_mod_rate mod);

		/**
		 * The ratio of CP-time over useful time
		 * @return ratio of CP-time over useful time
		 */
		virtual double getG();

		/**
		 * The sampling frequency
		 * @return the sampling frequency
		 */
		virtual double getFs();

		/**
		 * Return the maximum size in bytes that can be sent for the given
		 * nb of symbols and modulation
		 */
		int getMaxPktSize(double nbsymbols, Ofdm_mod_rate);

		/**
		 * Change the frequency at which the phy is operating
		 * @param freq The new frequency
		 */
		virtual void setFrequency(double freq);

		/**
		 * Set the new modulation for the physical layer
		 * @param modulation The new physical modulation
		 */
		virtual void setModulation(Ofdm_mod_rate modulation);

		/** 
		 * Set the mode for physical layer
		 */
		virtual void setMode(Ofdm_phy_state mode);

		/**
		 * Update the sampling frequency. Called after changing frequency BW
		 */
		virtual void updateFs();

		/**
		 * Activate node
		 */
		virtual void node_on();
		
		/**
		 * Deactivate node
		 */
		virtual void node_off();

		/** The current modulation */
		Ofdm_mod_rate modulation_;

		/** Ratio of CP time over useful time */
		double g_;
		
		/** Frequency */
		double freq_;

		/** The state of the OFDM */
		Ofdm_phy_state state_;

		/** The sampling frequency */
		double fs_;

		/** The frequency bandwidth (Hz) */
		double fbandwidth_;
		
		/** Indicates if the node is activated */
		bool activated_;

		/** used to register the modulation type only once */
		static bool mod_initialized;

		/** modulation type id */
		static int modid[7];

		/** receive power threshold (W) */
		double RXThresh_;

		/** carrier sense threshold (W) */
		double CSThresh_;

		/** capture threshold (db) */
		double CPThresh_;
};

#endif /* OFDMPHY_H */
