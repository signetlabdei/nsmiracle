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

#ifndef MPHY_H 
#define MPHY_H

#include<module.h>


#include <minterference.h>
#include <mpropagation.h>
#include <mcorrelation.h>
#include <mantenna.h>
#include <mspectralmask.h>
#include <mphy_pktheader.h>
#include <mphy_timers.h>



#define MPHY_DROP_ERROR_INTERFERENCE "EINTRF" 
#define MPHY_DROP_ERROR_NOISE        "ENOISE"


class MPhy : public Module
{
  friend class MPhyRxTimer;
  friend class MPhyTxTimer;

public:

  MPhy();
  virtual ~MPhy();
  virtual void recv(Packet* p);

  virtual int recvSyncClMsg(ClMessage* m);

  /** 
   * Returns the duration of the transmission for the packet being
   * transmitted. Should account for modulation type, PHY overhead,
   * and so on. This method is not implemented in the MPhy class, it
   * must be implemented by each derived PHY class
   *
   * The MPhy class evaluates this method just prior to starting the
   * transmission of a packet.
   * 
   * @param p the packet being transmitted
   * 
   * @return duration in s
   */
  virtual double getTxDuration(Packet* p) = 0;

  /** 
   * Returns the transmission power for the packet being transmitted.
   *
   * The MPhy class evaluates this method just prior to starting the
   * transmission of a packet.
   * 
   * @param p packet being transmitted
   * 
   * @return the transmission power in W
   */
  virtual double getTxPower(Packet* p) { return TxPower_; }

  /** 
   * Returns the signal power for the packet being received. Signal
   * power is calculated evaluating the TX power of the packet, TX and
   * RX antenna gains, propagation gain and the spectral overlapping
   * of the transmission with the receiver spectral mask.
   *
   * The MPhy class evaluates this method when reception of a packet begins.
   * 
   * @param p  the packet being received
   * 
   * @return the received signal power
   */
  virtual double getRxPower(Packet* p);

  /** 
   * Returns the noise power at the receiver. This noise power should
   * NOT include interference power, which is handled
   * independently. By default, in the parent MPhy class, noise power
   * is calculated with respect to a given noise spectral power
   * density which is multiplied by the spectral mask of the
   * receiver. In derived PHY classes, this method can be overridden
   * and the result may be computed as a function of the packet being
   * received.
   *
   * The MPhy class evaluates this method when reception of a packet begins.
   * 
   * @param p the packet being received.
   * 
   * @return 
   */
  virtual double getNoisePower(Packet* p);

  /** 
   * Returns the antenna to be used for the transmission of a given
   * packet. 
   *
   * The MPhy class evaluates this method just prior to starting the
   * transmission of a packet.
   * 
   * @param p the packet being transmitted
   * 
   * @return a pointer to the antenna instance to be used for transmission.
   */
  virtual MAntenna* getTxAntenna(Packet* p) {return antenna_;}

  /** 
   * Returns the antenna to be used for the reception of a given
   * packet. 
   *
   * The MPhy class evaluates this method when reception of a packet begins.
   * 
   * @param p the packet being received
   * 
   * @return a pointer to the antenna instance to be used for reception
   */

  virtual MAntenna* getRxAntenna(Packet* p) {return antenna_;}

  /** 
   * Returns the spectral mask to be used for the transmission of a
   * given packet.
   * 
   * @param p the packet being transmitted
   * 
   * @return a pointer to the spectral mask to be used for transmission
   */
  virtual MSpectralMask* getTxSpectralMask(Packet* p) {return spectralmask_;}


  /** 
   * Returns the spectral mask to be used for reception
   * 
   * @param p the packet being received
   * 
   * @return a pointer to the spectral mask to be used for reception
   */
  virtual MSpectralMask* getRxSpectralMask(Packet* p) {return spectralmask_;} 



  /** 
   * Method used to register new modulation types. Should be called
   * ONCE for each modulation type implemented by a MPhy-derived
   * class; the obtained modulation id(s) should be shared among all 
   * implementations of such class.
   * 
   * @param name the name of the new modulation type registered
   * 
   * @return the id of the new modulation type registered
   */
  static int registerModulationType(const char* name);


  /** 
   * Returns the name of the corresponding modulation id. Useful
   * e.g. for tracing purposes.
   * 
   * @param modtype modulation type id
   * 
   * @return the name
   */
  static const char* getModulationName(int modtype);


  /** 
   * Get the modulation type to be used for the packet being
   * transmitted. This method should be implemented by all derived classes

   * 
   * @param p the packet being transmitted
   * 
   * @return 
   */
  virtual int getModulationType(Packet* p) = 0;


  /** 
   * TCL command parser
   * 
   * @param argc 
   * @param argv 
   * 
   * @return TCL_OK or TCL_ERROR
   */
  virtual int command(int argc, const char* const* argv);


  /** 
   * Returns total dropped pkts due to noise
   *
   * @return the value
   */
  int getDroppedPktsNoise() { return(droppedPktsNoise); }

  /** 
   * Returns total dropped pkts due to interference
   *
   * @return the value
   */
  int getDroppedPktsInterf() { return(droppedPktsInterf); }

  /** 
   * Returns total dropped pkts due to deafness conditions 
   *
   * @return the value
   */
  int getDroppedPktsDeaf() { return(droppedPktsDeaf); }

  /** 
   * Returns total dropped pkts due to wrong modulation id
   *
   * @return the value
   */
  int getDroppedPktsWrongModId() { return(droppedPktsWrongModId); }


  /** 
   * Returns total wrong pkts due to noise
   *
   * @return the value
   */
  int getErrorPktsNoise() { return(errorPktsNoise); }

  /** 
   * Returns total wrong pkts due to interference
   *
   * @return the value
   */
  int getErrorPktsInterf() { return(errorPktsInterf); }

protected:

  /** 
   * Method invoked at the beginning of the transmission of a given
   * packet. This method must be implemented by each class inheriting
   * from the MPhy class. It should be used to set
   * specific header fields of the particular PHY technology
   * being implemented, such as the ModulationType in the MPhy packet
   * header. Furthermore, each implementation of this 
   * method must take care of calling sendDown(p) 
   *
   * 
   * @param p the given packet
   */
  virtual void startTx(Packet* p) = 0;

  /** 
   * Method invoked at the end of the transmission of a given
   * packet. This method must be implemented by each class inheriting
   * from the MPhy class. No idea what this method might be useful
   * for, however... probably most implementations will be fine with
   * an empty body for it.
   *
   * 
   * @param p the given packet
   */
  virtual void endTx(Packet* p)   = 0;

  /** 
   * Method invoked at the beginning of the reception of a given
   * packet. This method must be implemented by each class inheriting
   * from the MPhy class. It should be used to check synchronization
   * on the transmission. The ModulationType is probably useful also
   * for this purpose.
   * 
   *
   * 
   * @param p the given packet
   */
  virtual void startRx(Packet* p) = 0;


  /** 
   * Method invoked at the end of the reception of a given
   * packet. This method must be implemented by each class inheriting
   * from the MPhy class. 
   *
   * Please note that this method is called for ALL incoming
   * transmissions, that is even if the receiver was actually not
   * synchronized on packet p. As a consequence, each implementation
   * of this method should check if the receiver was actually
   * synchronized on packet p. 
   * 
   * Furthermore, reception errors should be evaluated within this
   * method, and the appropriate action should be taken. It is left to
   * the implementer to decide whether the packet should be dropped in
   * case of errors, or whether some error flag (e.g., ch->error)
   * should be set instead to let some upper layer (e.g., MAC) drop
   * the packet by itself. The ModulationType is probably to be
   * checked for, since even if SNIR is high it is not likely we can
   * decode a modulation other the ones each Phy implementation was
   * designed to decode.
   *
   * Finally, each implementation of this method should take care to
   * call sendUp(p) in case the reception of the packet was successful.
   * 
   * @param p the given packet
   */
  virtual void endRx(Packet* p)   = 0;

  /** 
   * Turns the PHY on
   */
  void turnOn() {isOn = true; }


  /** 
   * Turns the PHY off
   */
  void turnOff() {isOn = false; }


  /** 
   * Notify the MAC that transmission of a packet has ended
   * 
   * @note this method is never called in the base MPhy class, it
   * should be called whenever appropriate within a class inheriting
   * from MPhy 
   * 
   * @param p pointer to the packet whose transmission has ended
   */
  void Phy2MacEndTx(const Packet* p);

  /** 
   * Notify the MAC that reception of a packet has begun, e.g. when
   * the PHY preamble of an incoming transmission has been detected
   * 
   * @note this method is never called in the base MPhy class, it
   * should be called whenever appropriate within a class inheriting
   * from MPhy 
   * 
   * @param p pointer to the packet whose reception has begun
   */
  void Phy2MacStartRx(const Packet* p);


  /** 
   * Notify the MAC that the value of the Clear Channel Assessment
   * variable has changed
   * 
   * @note this method is never called in the base MPhy class, it
   * should be called whenever appropriate within a class inheriting
   * from MPhy 
   * 
   * @param cca the new value for the CCA variable (true = channel
   * clear, false = channel busy)
   */
  void Phy2MacCCA(bool cca); 


  /** 
   * Increases the counter of dropped pkts due to noise
   */
  void incrDroppedPktsNoise() { droppedPktsNoise++; }

  /** 
   * Increases the counter of dropped pkts due to interference
   */
  void incrDroppedPktsInterf() { droppedPktsInterf++; }

  /** 
   * Increases the counter of dropped pkts due to deafness conditions
   */
  void incrDroppedPktsDeaf() { droppedPktsDeaf++; }

  /** 
   * Increases the counter of dropped pkts due to wrong modulation id
   */
  void incrDroppedPktsWrongModId() { droppedPktsWrongModId++; }

  /** 
   * Increases the counter of wrong pkts due to noise
   */
  void incrErrorPktsNoise() { errorPktsNoise++; }

  /** 
   * Increases the counter of wrong pkts due to interference
   */
  void incrErrorPktsInterf() { errorPktsInterf++; }


  MPhyTxTimer txtimer; ///< timer handling end of transmissions
  MPhyRxTimer rxtimer; ///< timer handling end of receptions

  MInterference* interference_;  ///< pointer to the interference model
				 /// plugged via tcl

  MPropagation*  propagation_;   ///< pointer to the propagation model
				 /// plugged via tcl

  MAntenna*      antenna_;       ///< pointer to the antenna model
				 /// plugged via tcl. Note that the
				 /// base MPhy class uses a single
				 /// antenna both for TX and RX. This
				 /// can be changed in derived
				 /// implementations by overriding the
				 /// getTxAntenna() and getRxAntenna()
				 /// methods. 

  MSpectralMask* spectralmask_;  ///< pointer to the receiver spectral
				 /// mask plugged via tcl. Note that
				 /// the base MPhy class uses a single
				 /// spectral mask for Tx and Rx.
				 /// Smarter implementations derived
				 /// from MPhy (e.g., cognitive
				 /// radio implementations) might be
				 /// able to use different spectral
				 /// mask both for TX and RX. This can
				 /// be achieved by overriding the
				 /// getTxSpectralMask() and
				 /// getRxSpectralMask() methods.

  MCorrelation* correlation_;    /**<
				  * Pointer to the correlation module
				  * plugged via tcl. 
				  * 
				  */



  double TxPower_;    /// Tx Power in W. Simple adaptations of the
		      /// transmission power might be carried out just
		      /// by re-setting this value. More complex
		      /// strategies (e.g., packet-specific
		      /// transmission power) can be implemented by
		      /// overriding the getTxPower() method.

  double NoiseSPD_;   /// Noise Spectral Power Density in W/Hz. If you
		      /// need to make this time- or packet-dependent 
                      ///  (e.g. as required when simulating underwater
                      ///  acoustic communications) you 
		      /// can either modify the value of this variable
		      /// or re-implement the getNoisePower() method.

  bool isOn; /// Flag to turn phy on/off 


  int droppedPktsNoise;  /// Total number of dropped pkts due to noise

  int droppedPktsInterf; /// Total number of dropped pkts due to interference

  int droppedPktsDeaf; /// Total number of dropped pkts due to deafness conditions

  int droppedPktsWrongModId; /// Total number of dropped pkts due to wrong modulation id

  int errorPktsNoise; /// Total number of wrong pkts due to noise
  
  int errorPktsInterf; /// Total number of wrong pkts due to interference
};






#endif /* MPHY_H */
