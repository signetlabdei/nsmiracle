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

#include "neighbordb.h"
#include "wimaxneighborentry.h"

/** 
 * Constructor
 */
NeighborDB::NeighborDB ()
{
  nbentry_ = 0;
  nbs_ = (WimaxNeighborEntry **) malloc (DEFAULT_DB_SIZE*sizeof (WimaxNeighborEntry *));
}

/**
 * Destructor
 */
NeighborDB::~NeighborDB ()
{
  for (int i=0 ; i < nbentry_ ; i++) {
    delete (nbs_[i]);
  }
}

/**
 * Add an entry in the database
 * @param nb The neighbor to add
 */
void NeighborDB::addNeighbor (WimaxNeighborEntry *nb)
{
  if (nbentry_ == DEFAULT_DB_SIZE) {
    printf ("Default size for neighbor database is too small. Update DEFAULT_DB_SIZE attribute\n");
    exit (0);
  }
  nbs_[nbentry_++] = nb;
}

/**
 * Remove the entry associated with the given node
 * @param nbid The neighbor id
 */
void NeighborDB::removeNeighbor (int nbid)
{
  assert (getNeighbor (nbid)==NULL);

  for (int i = 0 ; i < nbentry_ ; i++) {
    if (nbs_[i]->getID() == nbid) {
      delete (nbs_[i]);
      for (int j = i+1 ; j < nbentry_ ; j++, i++)
	nbs_[i]=nbs_[j];
      nbentry_--;
      break;
    }
  }
}

/**
 * Return the number of neighbor in the list
 * @return the number of neighbor in the list
 */
int NeighborDB::getNbNeighbor ()
{
  return nbentry_;
}

/**
 * Return the entry associated with the given node
 * @param nbid The neighbor id
 * @return the entry for the given node or NULL
 */
WimaxNeighborEntry * NeighborDB::getNeighbor (int nbid)
{
  for (int i = 0 ; i < nbentry_ ; i++) {
    if (nbs_[i]->getID() == nbid) {
      return (nbs_[i]);
    }
  }
  return NULL;
}

/**
 * Return a pointer to the list of all neighbors
 * @return a pointer to the list of all neighbors
 */
WimaxNeighborEntry ** NeighborDB::getNeighbors ()
{
  return nbs_;
}
