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

#ifndef SCANNINGSTATION_H
#define SCANNINGSTATION_H

#include "packet.h"

class ScanningStation;
LIST_HEAD (scanningStation, ScanningStation);

/**
 * Contains information about a scanning station and 
 * helps to determine if it is in a scanning or interleaving
 * time.
 */
class ScanningStation
{	
 public:
  /**
   * Create an data object with the given attributes
   * @param nodeid The node 
   * @param duration The scanning duration
   * @param start The frame at which the scanning start
   * @param interleaving The interleaving interval
   * @param iteration The number of iterations
   */
  ScanningStation (int nodeid, int duration, int start, int interleaving, int iteration);

  /**
   * Determines if the node is currently scanning
   * @param frame The current frame
   */
  bool isScanning (int frame);

  /**
   * Return the node id 
   */
  int getNodeId () { return nodeid_; }

  // Chain element to the list
  inline void insert_entry_head(struct scanningStation *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Chain element to the list
  inline void insert_entry(ScanningStation *elem) {
    LIST_INSERT_AFTER(elem, this, link);
  }

  // Return next element in the chained list
  ScanningStation* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }

 protected:

  /**
   * Pointer to next in the list
   */
  LIST_ENTRY(ScanningStation) link;
  //LIST_ENTRY(ScanningStation); //for magic draw


 private:
  /**
   * Duration of scanning allocation in frames
   */
  int duration_;
  
  /**
   * Start frame (absolute)
   */
  int start_frame_;

  /**
   * interleaving in frames
   */
  int interleaving_;

  /**
   * number of iterations
   */
  int iteration_;

  /**
   * The node that is scanning
   */
  int nodeid_;
};



#endif
