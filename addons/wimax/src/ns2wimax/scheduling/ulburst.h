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

#ifndef ULBURST_H
#define ULBURST_H

#include "burst.h"

/* Defines the OFDM UIUC values */
enum uiuc_t {
  //0 is reserved
  UIUC_INITIAL_RANGING=1,
  UIUC_REQ_REGION_FULL,
  UIUC_REQ_REGION_FOCUSED,
  UIUC_FOCUSED_CONTENTION_IE,
  //5-12 reserved for burst profiles
  UIUC_PROFILE_1,
  UIUC_PROFILE_2,
  UIUC_PROFILE_3,
  UIUC_PROFILE_4,
  UIUC_PROFILE_5,
  UIUC_PROFILE_6,
  UIUC_PROFILE_7,
  UIUC_PROFILE_8,
  UIUC_SUBCH_NET_ENTRY=13,
  UIUC_END_OF_MAP,
  UIUC_EXT_UIUC
};

/* Defines Midamble repetition interval */
enum midamble_t {
  PREAMBLE_ONLY,
  MIDAMBLE_8_SYMBOLS,  //every 8 data symbols
  MIDAMBLE_16_SYMBOLS, //every 16 data symbols
  MIDAMBLE_32_SYMBOLS  //every 32 data symbols
};

/* List of Extended UIUC */
enum extuiuc_t {
  UIUC_FAST_POWER_CTRL=0,
  UIUC_PHY_MODIFIER,
  UIUC_AAS,
  UIUC_FAST_RANGING,
  UIUC_FAST_TRACKING
};


/**
 * Uplink Burst description
 */
class UlBurst : public Burst
{
public:
  /**
   * Default contructor
   * @param phypdu The PhyPdu where it is located
   */
  UlBurst (PhyPdu *phypdu);

  /**
   * Return the midamble used in the burst
   */
  inline int getMidamble( ) { return midamble_; }
  /**
   * Set the midamble used in the burst
   */
  inline void setMidamble( int midamble ) { midamble_ = midamble; }
  
  /**
   * Return extended uiuc
   */
  inline int getExtendedUIUC () { return extuiuc_; }

  /**
   * Set the burst to Fast Ranging
   * @param macAddr address of MN
   * @param uiuc Encoding to use
   */
  void setFastRangingParam (int macAddr, int uiuc);
  
  /**
   * Return the mac address for fast ranging
   * @return The mac address for fast ranging
   */
  int getFastRangingMacAddr ();

  /**
   * Return the UIUC encoding to use
   * @return The UIUC encoding to use
   */
  int getFastRangingUIUC ();


 private:
  /**
   * The type of midamble included in the burst
   */
  int midamble_;

  /**
   * The extended UIUC
   */
  int extuiuc_;

  /**
   * For Fast Ranging
   */
  int uiuc2_;
  int macAddr_; 
};

#endif
