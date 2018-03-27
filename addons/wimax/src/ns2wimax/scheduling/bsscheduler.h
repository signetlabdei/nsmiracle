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

#ifndef BSSCHEDULER_H
#define BSSCHEDULER_H

#include "wimaxscheduler.h"
#include "scanningstation.h"

#define INIT_DL_DURATION 20 //enough for DL_MAP, UL_MAP, DCD, UCD and some RNG-RSP
#define MIN_CONTENTION_SIZE 5 //minimum number of opportunity for allocation

#define DEFAULT_DL_RATIO 0.3 //default ratio for downlink subframe

class Mac802_16BS;
class WimaxCtrlAgent;
/**
 * Class BSScheduler
 * Implement the packet scheduler on the BS side
 */ 
class BSScheduler : public WimaxScheduler {
  //friend class SendTimer;
 public:
  /*
   * Create a scheduler
   */
  BSScheduler ();

  /*
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
   * Default modulation 
   */
  Ofdm_mod_rate default_mod_;

  /**
   * Number of transmission opportunity for initial ranging
   * and bw request (i.e contention slots)
   */
  int contention_size_; 

  /**
   * Compute and return the bandwidth request opportunity size
   * @return The bandwidth request opportunity size
   */
  int getBWopportunity ();

  /**
   * Compute and return the initial ranging opportunity size
   * @return The initial ranging opportunity size
   */
  int getInitRangingopportunity ();  

  /**
   * Add a downlink burst with the given information
   * @param burstid The burst number
   * @param c The connection to add
   * @param iuc The profile to use
   * @param dlduration current allocation status
   * @param the new allocation status
   */
  int addDlBurst (int burstid, Connection *c, int iuc, int dlduration, int maxdlduration);

 private:

  /**
   * Return the MAC casted to BSScheduler
   * @return The MAC casted to BSScheduler
   */
  Mac802_16BS* getMac();
   
  /**
   * The ratio for downlink subframe
   */
  double dlratio_;

  /**
   * The address of the next node for DL allocation
   */
  int nextDL_;
  
  /**
   * The address of the next node for UL allocation
   */
  int nextUL_;


};

#endif //BSSCHEDULER_H

