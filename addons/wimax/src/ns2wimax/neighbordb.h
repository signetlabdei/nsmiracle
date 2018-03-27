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

#ifndef NEIGHBORDB_H
#define NEIGHBORDB_H

//#include "wimaxneighborentry.h"
#include "mac802_16pkt.h"

/* This is the size of database allocated. Needs to be modified if 
 * a node could have more entries
 */
#define DEFAULT_DB_SIZE 10

class WimaxNeighborEntry;
/**
 * The class is used to store and manipulate the list 
 * of neighbors in a given node
 */
class NeighborDB {
 public:
  /** 
   * Constructor
   */
  NeighborDB ();

  /**
   * Destructor
   */
  ~NeighborDB ();

  /**
   * Add an entry in the database
   * @param nb The neighbor to add
   */
  void addNeighbor (WimaxNeighborEntry *nb);

  /**
   * Remove the entry associated with the given node
   * @param nbid The neighbor id
   */
  void removeNeighbor (int nbid);

  /**
   * Return the number of neighbor in the list
   * @return the number of neighbor in the list
   */
  int getNbNeighbor ();

  /**
   * Return the entry associated with the given node
   * @param nbid The neighbor id
   * @return the entry for the given node or NULL
   */
  WimaxNeighborEntry * getNeighbor (int nbid);

  /**
   * Return a pointer to the list of all neighbors
   * @return a pointer to the list of all neighbors
   */
  WimaxNeighborEntry ** getNeighbors ();

 protected:

 private:
  /**
   * Current number of neighbor
   */
  int nbentry_;

  /**
   * Array of neighbors
   */
  WimaxNeighborEntry **nbs_;
  
};


#endif
