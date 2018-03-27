/* This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * <BR>
 * We would appreciate acknowledgement if the software is used.
 * <BR>
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * </PRE></P>
 * @author  rouil
 */

#ifndef SUBFRAME_H
#define SUBFRAME_H

#include "profile.h"
//#include "contentionslot.h"
#include "phypdu.h"
//#include "ulsubframetimer.h"
//#include "dlsubframetimer.h"


class FrameMap;
class DlSubFrameTimer;
class UlSubFrameTimer;

/**
 * Abstract class for subframe
 */
class SubFrame 
{
 public:
  /**
   * Constructor
   * @param map The frame 
   */
  SubFrame (FrameMap *map);

  /**
   * Destructor
   */
  ~SubFrame ();

  /**
   * Add a downlink profile
   * @param f The frequency of the profile
   * @param enc The encoding used in the profile
   * @return a new profile with the given caracteristics
   */
  Profile * addProfile (int f, Ofdm_mod_rate enc);
  
  /**
   * Return the DL profile with the given UID
   * @return the DL profile with the given UID
   */
  Profile * getProfile (int iuc); 

  /**
   * Remove a downlink profile
   * @param p The profile to remove
   */
  void removeProfile (Profile *p);

  /**
   * Remove all downlink profiles
   */
  void removeProfiles ();

  /**
   * Return the number of profiles for this subframe
   */
  int getNbProfile ();

  /**
   * Return the head of the profile list
   */
  Profile * getFirstProfile ();

  /**
   * Return the Configuration Change count
   */
  int getCCC ();

  /**
   * Increment the configuration change count
   */
  void incrCCC ();

 protected:
  /**
   * The frame where it is located
   */
  FrameMap *map_;

  /**
   * List of burst profiles for downlink
   */
  struct profile profile_list_;
  
  /**
   * Number of element in the PhyPDU list
   */
  int nbProfile_;

  /**
   * Configuration change count
   */
  int ccc_;
};



/**
 * This class defines a downlink subframe
 */
class DlSubFrame : public SubFrame
{
  friend class DlSubFrameTimer;
 public:
  /**
   * Constructor
   * @param map The frame 
   */
  DlSubFrame (FrameMap *map);

  /**
   * Destructor
   */
  ~DlSubFrame ();

  /**
   * Return the DL phypdu
   * @return the phypdu
   */
  inline PhyPdu * getPdu () { return phypdu_; }

  /**
   * Set the channel ID
   * @param id The channel id
   */
  inline void setChannelID (int id) { channel_ = id; }

  /**
   * Return the channel ID
   * @return The channel ID
   */
  inline int getChannelID () { return channel_; }

  /**
   * Return the timer handling the subframe
   */
  inline DlSubFrameTimer *getTimer () { return timer_; }

 private:

  /**
   * List of uplink PhyPDU composing the uplink subframe
   */
  PhyPdu * phypdu_;

  /**
   * The downlink channel id
   */
  int channel_;

  /**
   * The timer for handling bursts
   */
  DlSubFrameTimer *timer_;
};

class BwContentionSlot;
class RngContentionSlot;
/**
 * This class defines a downlink subframe
 */
class UlSubFrame : public SubFrame
{
  friend class UlSubFrameTimer;
 public:
  /**
   * Constructor
   * @param map The frame 
   */
  UlSubFrame (FrameMap *map);

  /**
   * Destructor
   */
  ~UlSubFrame ();

  /**
   * Return the DL phypdu
   * @return the phypdu
   */
  inline PhyPdu * getFirstPdu () { return phypdu_list_.lh_first; }
  
  /**
   * Add an uplink phy pdu
   * @param pos The position of the PhyPdu in the subframe
   * @param preamble The size of the preamble in units of XX
   * @return newly created PhyPdu
   */
  PhyPdu * addPhyPdu (int pos, int preamble);

  /**
   * Remove a Phy PDU
   * @param pdu The Phy PDU to remove
   */
  void removePhyPdu (PhyPdu *p);

  /**
   * Return the number of PhyPdu
   */
  inline int getNbPdu () { return nbPhyPdu_; }

  /**
   * Return the burst located at the given index
   * @param pos The position of the burst
   */
  PhyPdu* getPhyPdu(int pos);

  /*
   * Return the contention slot for BW requests
   * @return the pointer to the contention slot for BW requests
   */
  inline BwContentionSlot* getBw_req( ) { return bw_req_; }

  /*
   * Return the contention slot for BW requests
   * @return the pointer to the contention slot for BW requests
   */
  inline RngContentionSlot* getRanging( ) { return ranging_; }

  /**
   * Set the start time in units of PS
   * @param time The subframe start time
   */
  inline void setStarttime (int time) { starttime_ = time;}

  /**
   * Get the start time in units of PS
   * @return The subframe start time
   */
  inline int getStarttime () { return starttime_;}

  /**
   * Set the channel ID
   * @param id The channel id
   */
  inline void setChannelID (int id) { channel_ = id; }

  /**
   * Return the channel ID
   * @return The channel ID
   */
  inline int getChannelID () { return channel_; }

  /**
   * Return the timer handling the subframe
   */
  inline UlSubFrameTimer *getTimer () { return timer_; }

 protected:

  /**
   * Information about the bandwidth request contention
   */
  BwContentionSlot *bw_req_;
  
  /**
   * Information about the ranging contention
   */
  RngContentionSlot *ranging_;

 private:

  /**
   * List of uplink PhyPDU composing the uplink subframe
   */
  struct phyPdu phypdu_list_;

  /**
   * Number of phypdu
   */
  int nbPhyPdu_;
  
  /**
   * Start time of the subframe in unit of PS
   */
  int starttime_;

  /**
   * The downlink channel id
   */
  int channel_;

  /**
   * The timer for handling bursts
   */
  UlSubFrameTimer *timer_;

};

#endif
