/* -*-	Mode:C++ -*- */
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

#ifndef UMTSPHY_H 
#define UMTSPHY_H

#include<mphy.h>



#define CODE_UNKNOWN       0
#define CODE_CONVOLUTIONAL 1
#define CODE_TURBO         2

// Magic number used to scale erfc to get BER
#define ZETA_CONVOLUTIONAL_ONEHALF 2.72


#define UMTS_MODNAME "UMTS"

#define DIRECTION_DOWNLINK 1
#define DIRECTION_UPLINK   2


typedef struct pcEntry
{
  int id;
  bool powerUp_;
  double TxPower_;
  double Pi;
  double sinr;
  double SIRtarget;
  
  struct pcEntry* next_;
} pcEntry;


class UmtsPhy : public MPhy 
{

 public:
  UmtsPhy();
  int command(int argc, const char*const* argv);
  int getModulationType(Packet*);
  double getTxDuration(Packet* p);
  virtual double getDataRate(){assert(0);}

 protected:

  void startTx(Packet* p);
  void endTx(Packet* p);
  void startRx(Packet* p);
  void endRx(Packet* p);


  /* Power Ctrl methods*/
  double getTxPower(Packet* p);
  virtual pcEntry* getPcEntry(int id);
  virtual void addPcEntry(int id);
  virtual void pcUpdateCmd(Packet* p);
  virtual void pcUpdatePi(Packet* p);
  virtual void pcReadCmd(Packet* p);
  
  virtual double getPrAfterDeSpreading(Packet* p) {assert(0);}
  double sinr2ber(double);

  virtual void calculateErrors(Packet* p);
  virtual double getPacketErrorRate(Packet* p, double sinr);
  virtual int getNumBits(Packet* p);

  double chip_rate_ ;         /// Chip Rate in chip/second
  int    spreading_factor_ ;  /// Spreading Factor
  int     bits_per_symbol_;    /// Number of Bits per Symbol
  double coding_rate_;        /// Coding Rate (e.g., 1/2, 1/3...)
  int    coding_type_;          /// Type of code, e.g. CODE_CONVOLUTIONAL
  double slot_duration_;      /// Duration of a slot in seconds

  int code_id_;     /**
		    * id of the code used for CDMA
		    * indicates the UL scrambling code id for the ME
		    * or the DL scramling code for the BS (Nodeb)
		    */

  static int code_id_counter;        /// increments as each new ME/BS
				     /// gets its own code upon creation
  
  static bool mod_initialized; /// used to register the modulation type only once
  static int modid;        /// modulation type id

  MSpectralMask* dl_smask_;   /// downlink spectral mask
  MSpectralMask* ul_smask_;   /// uplink spectral mask
  
  double alpha_;		/// alpha_ is used to compute the signal power used for the data from the whole one
						/// which is comprehensive of data and cotrol, it's value is 49/225 
  
  pcEntry* pcTable_;		/// table in which are stored the next power control command
  							/// for each connection monitored
  double maxTxPower_;		/// maximum value to be used as transmission power [W], typically:
  							/// UE side (uplink):		21 dBm (0.125 W)
  							/// Node B side (downlink):	43 dBm (20 W)
  double minTxPower_;		/// minimum value to be used as transmission power [W]
  double pcStep_;			/// power control step [W], typically 1 dB
  double SIRtarget_;		/// SIRtarget value (fast power control algorithm)
  double PERtarget_;		/// Packet Error Rate value to be used to set the SIRtarget in the power control algorithm 

  int PowerCtrlUpdateCmdAtStartRx_;  /**<
				     * If nonzero, SIR is 
				     * estimated in startRx() using
				     * current RX power and previous
				     * interference power. If zero, SIR
				     * is estimated in endRx() using
				     * current RX and interference
				     * power. The zero setting yields a
				     * longer delay in the PC loop.				    
				     */

  int PowerCtrlReadCmdAtStartRx_; /**<
				   * If nonzero, parse Power Ctrl messages
				   * upon startRx(), otherwise (zero) parse
				   * them upon endRx(). The zero
				   * setting yields a longer delay in
				   * the PC loop. 
				   */



  /// 

};




class UmtsPhyBS : public UmtsPhy 
{

 public:
  UmtsPhyBS();
  virtual double getDataRate();
  double getPrAfterDeSpreading(Packet* p);

 protected:
  void startTx(Packet* p);
  void endRx(Packet* p);
  MSpectralMask* getTxSpectralMask(Packet* p) {return dl_smask_;}
  MSpectralMask* getRxSpectralMask(Packet* p) {return ul_smask_;}

  double iuccorr_;     // inter-user spreading code correlation

};



class UmtsPhyME : public UmtsPhy 
{

 public:
  int command(int argc, const char*const* argv);
  virtual double getDataRate();
  double getPrAfterDeSpreading(Packet* p);

//   virtual double getSNR();    
//   virtual double getSINR();

 protected:
  void startTx(Packet* p);
  void endRx(Packet* p);
  MSpectralMask* getTxSpectralMask(Packet* p) {return ul_smask_;}
  MSpectralMask* getRxSpectralMask(Packet* p) {return dl_smask_;}

//    double SNR;
//    double SINR;


  int bs_code_id_;   /// scrambling code of the BS this ME is associated with

};


#endif /* UMTSPHY_H */



