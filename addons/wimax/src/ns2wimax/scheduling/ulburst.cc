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

#include "ulburst.h"

/**
 * Creates a downlink burst
 * @param phypdu The PhyPdu where it is located
 */
UlBurst::UlBurst (PhyPdu *phypdu) : Burst (phypdu)
{
  midamble_ = PREAMBLE_ONLY;
}

/**
 * Set the burst to Fast Ranging
 * @param macAddr address of MN
 * @param uiuc Encoding to use
 */
void UlBurst::setFastRangingParam (int macAddr, int uiuc)
{
  assert (getIUC() == UIUC_EXT_UIUC);

  extuiuc_ = UIUC_FAST_RANGING;
  uiuc2_ = uiuc;
  macAddr_ = macAddr;
}

  
/**
 * Return the mac address for fast ranging
 * @return The mac address for fast ranging
 */
int UlBurst::getFastRangingMacAddr ()
{
  assert (extuiuc_ == UIUC_FAST_RANGING);
  return macAddr_;
}

/**
 * Return the UIUC encoding to use
 * @return The UIUC encoding to use
 */
int UlBurst::getFastRangingUIUC (){
  assert (extuiuc_ == UIUC_FAST_RANGING);
  return uiuc2_;
}



