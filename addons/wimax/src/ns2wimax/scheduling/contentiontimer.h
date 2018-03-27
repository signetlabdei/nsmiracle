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

#ifndef CONTENTIONTIMER_H
#define CONTENTIONTIMER_H

#include "timer-handler.h"

class ContentionRequest;

/**
 * Timer for contention window
 */
class ContentionTimer: public TimerHandler
{
 public:
  /**
   * Creates a timer to handle the burst's transmission
   * @param c The contention slot
   */
  ContentionTimer( ContentionRequest* c );
  
  /**
   * When it expires, the timer will handle the next packet to send
   * @param e not used
   */
  void expire( Event* e );

 private:

  /**
   * The contention slot containing the packets to send
   */
  ContentionRequest* c_;
};

#endif
