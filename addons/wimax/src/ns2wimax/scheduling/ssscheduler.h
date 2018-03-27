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

#ifndef SSSCHEDULER_H
#define SSSCHEDULER_H

#include "wimaxscheduler.h"

/**
 * Class SSscheduler
 * Scheduler for SSs
 */ 
class SSscheduler : public WimaxScheduler {
  friend class Mac802_16SS;
 public:
  /**
   * Create a scheduler
   */
  SSscheduler ();

  /**
   * Interface with the TCL script
   * @param argc The number of parameter
   * @param argv The list of parameters
   */
  int command(int argc, const char*const* argv);


  /**
   * Initializes the scheduler
   */
  virtual void init ();

  /**
   * This function is used to schedule bursts/packets
   */
  virtual void schedule ();

 protected:

  /**
   * Create a request for the given connection
   * @param con The connection to check
   */
  void create_request (Connection *con);

 private:
  
};

#endif //SSSCHEDULER_H

