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

#ifndef ULSUBFRAME_H
#define ULSUBFRAME_H

#include "mac802_16.h"
#include "phypdu.h"

class UlSubFrame;
/**
 * This timer is used to handle the transmission of 
 * a UlSubframe.
 */
class UlSubFrameTimer: public TimerHandler
{
 public:
  /**
   * Creates a timer to handle the subframe transmission
   * @param subframe The UlSubframe
   */
  UlSubFrameTimer (UlSubFrame *subframe);

  /**
   * When it expires, the timer will handle the next packet to send
   * @param e not used
   */
  void expire( Event* e );

  /**
   * Reset the timer
   */
  void reset ();

 private:
  /**
   * The subframe
   */
  UlSubFrame *subframe_;

  /**
   * The current PhyPdu being handled
   */
  PhyPdu *pdu_;

  /**
   * Tag to know if we are changing PhyPdu
   */
  bool newphy_;

  /** 
   * Store local variables for faster access
   */
  Mac802_16 *mac_;

};

#endif
