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

#ifndef NEIGHBORENTRY_H
#define NEIGHBORENTRY_H

#include "scheduling/framemap.h"

/* Data structure used to store Mac state information during scanning */
struct sched_state_info {
  struct state_info *state_info; 
  FrameMap *map;
  WimaxT1Timer  *t1timer;
  WimaxT2Timer  *t2timer;
  WimaxT6Timer  *t6timer;
  WimaxT12Timer *t12timer;
  WimaxT21Timer *t21timer;
  WimaxLostDLMAPTimer *lostDLMAPtimer;
  WimaxLostULMAPTimer *lostULMAPtimer;
  WimaxT44Timer *t44timer;
};

/**
 * Store information about a neighboring BS.
 */
class WimaxNeighborEntry {
 public:
  /** 
   * Constructor
   */
  WimaxNeighborEntry (int id);

  /**
   * Destructor
   */
  ~WimaxNeighborEntry ();

  /**
   * Return the address of the neighbor of this entry
   * @return the address of the neighbor of this entry
   */
  int getID ();

  /**
   * Set the neighbor advertisement message
   * @param frame The advertisment message
   */
  void setNbrAdvMessage (mac802_16_nbr_adv_info *frame);
  
  /**
   * Return the neighbor advertisement message
   * @param frame The advertisment message
   */
  mac802_16_nbr_adv_info * getNbrAdvMessage ();

  /**
   * Set the DCD message received during scanning
   * @param dcd the DCD message received
   */
  void setDCD (mac802_16_dcd_frame *frame);

  /**
   * Get the DCD message received during scanning
   * @return the DCD message received
   */
  mac802_16_dcd_frame *getDCD ();  

  /**
   * Set the UCD message received during scanning
   * @param dcd the DCD message received
   */
  void setUCD (mac802_16_ucd_frame *frame);

  /**
   * Get the DCD message received during scanning
   * @return the DCD message received
   */
  mac802_16_ucd_frame *getUCD ();  

  /**
   * Set the UCD message received during scanning
   * @param dcd the DCD message received
   */
  void setRangingRsp (mac802_16_rng_rsp_frame *frame);

  /**
   * Get the DCD message received during scanning
   * @return the DCD message received
   */
  mac802_16_rng_rsp_frame *getRangingRsp ();  

  /**
   * Mark the neighbor as being detected
   * @param detected indicate if the neighbor has been detected
   */
  void setDetected (bool detected);

  /**
   * Indicates the neighbor as being detected
   * @return indication if the neighbor has been detected
   */
  bool isDetected ();

  /**
   * Set the MAC state associated with this neighbor
   * @param state
   */
  //void setState (sched_state_info *state);

  /**
   * Get the MAC state associated with this neighbor
   * @return the MAC state associated with this neighbor
   */
  sched_state_info * getState ();

 private:
  /**
   * The MAC address of neighbor
   */
  int id_;

  /**
   * The neighbor info adv message
   */
  struct mac802_16_nbr_adv_info *nbr_adv_;

  /**
   * The DCD message received during scanning
   */
  mac802_16_dcd_frame *dcd_;

  /**
   * The DCD message received during scanning
   */
  mac802_16_ucd_frame *ucd_;

  /** 
   * The ranging response received during scanning
   */
  mac802_16_rng_rsp_frame *rng_rsp_;

  /**
   * Save the state
   */
  struct sched_state_info state_;

  /**
   * Indicate if the neighbor has been detected
   */
  bool detected_;
};
  
#endif
