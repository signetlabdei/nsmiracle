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

#ifndef DLBURST_H
#define DLBURST_H

#include "burst.h"

/* Defines the OFDM DIUC values */
enum diuc_t {
  DIUC_STC_ZONE=0,
  //1-11 reserved for burst profiles
  DIUC_PROFILE_1,
  DIUC_PROFILE_2,
  DIUC_PROFILE_3,
  DIUC_PROFILE_4,
  DIUC_PROFILE_5,
  DIUC_PROFILE_6,
  DIUC_PROFILE_7,
  DIUC_PROFILE_8,
  DIUC_PROFILE_9,
  DIUC_PROFILE_10,
  DIUC_PROFILE_11,
  //12 is reserved
  DIUC_GAP=13,
  DIUC_END_OF_MAP,
  DIUC_EXT_DIUC
};

/**
 * Downlink Burst Description
 */
class DlBurst : public Burst
{
 public:

  /**
   * Default contructor
   * @param phypdu The PhyPdu where it is located
   */
  DlBurst (PhyPdu *phypdu);

  /**
   * Return true if preamble is present in the burst
   * @return true if preamble is present in the burst
   */
  inline bool isPreamble( ) { return preamble_; }

  /**
   * Set the preamble flag for the burst
   * @param preamble The flag
   */
  inline void setPreamble( bool preamble ) { preamble_ = preamble; }
  
 private:
  /**
   * Indicate if a preamble is included in the burst of not
   */
  bool preamble_;
	
};

#endif
