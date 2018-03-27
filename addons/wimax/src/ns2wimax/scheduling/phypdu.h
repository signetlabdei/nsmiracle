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

#ifndef PHYPDU_H
#define PHYPDU_H

#include "burst.h"
#include "dlburst.h"
#include "ulburst.h"

class FrameMap;
class PhyPdu;
LIST_HEAD (phyPdu, PhyPdu);
/**
 * This class describte the content of a Phy PDU
 */
class PhyPdu
{	
 public:
  /**
   * Create a phy pdu with the given preamble size
   * @param preamble The preamble size in OFDM symbols
   */
  PhyPdu (FrameMap *map, int preamble);

  /**
   * Delete the object
   */
  virtual ~PhyPdu ();

  /**
   * Return the preamble size for the PDU in unit of OFDM symbols
   * @return the preamble size for the PDU
   */
  int getPreamble( );

  /**
   * Set the preamble size for the PDU in unit of OFDM symbols
   * @param preamble the preamble size for the PDU
   */
  void setPreamble( int preamble );
  
  /**
   * Create and return a burst in the PDU
   * @param pos The position of the burst
   * @return The burst created
   */
  virtual Burst * addBurst(int pos);

  /**
   * Remove a burst in the PDU
   * @param burst The burst to remove
   */
  void removeBurst(Burst *b);
  
  /**
   * Clear all the bursts in the list
   */
  void removeAllBursts ();

  /**
   * Return the burst located at the given index
   * @param pos The position of the burst
   */
  Burst* getBurst(int pos);

  /**
   * Return the number of burst in the PhyPDU
   */
  inline int getNbBurst () { return nb_burst_; }

  /**
   * Return the FrameMap 
   */
  inline FrameMap * getMap() { return map_; }

  // Chain element to the list
  inline void insert_entry_head(struct phyPdu *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Chain element to the list
  inline void insert_entry(PhyPdu *elem) {
    LIST_INSERT_AFTER(elem, this, link);
  }
  
  // Return next element in the chained list
  PhyPdu* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }
 protected:
  /*
   * Pointer to next in the list
   */
  LIST_ENTRY(PhyPdu) link;
  //LIST_ENTRY(PhyPdu); //for magic draw

  /**
   * Curent number of bursts
   */
  int nb_burst_;

  /**
   * The list of burst contained in this PDU.
   * For uplink Phy PDU, only one burst is allowed
   */
  struct burst burst_list_;

 private:
  /**
   * Size of the preamble in units of OFDM symbols
   */
  int preamble_;
  
  /**
   * The frame map
   */
  FrameMap *map_;
	
};

/**
 * Define subclass for downlink phy pdu
 */
class DlPhyPdu: public PhyPdu
{	
 public:
  /**
   * Create a phy pdu with the given preamble size
   * @param preamble The preamble size in OFDM symbols
   */
  DlPhyPdu (FrameMap *map, int preamble);

  /**
   * Create and return a burst in the PDU
   * @param pos The position of the burst
   * @return The burst created
   */
  virtual Burst * addBurst(int pos);
};

/**
 * Define subclass for uplink phy pdu
 */
class UlPhyPdu: public PhyPdu
{	
 public:
  /**
   * Create a phy pdu with the given preamble size
   * @param preamble The preamble size in OFDM symbols
   */
  UlPhyPdu (FrameMap *map, int preamble);

  /**
   * Create and return a burst in the PDU
   * @param pos The position of the burst
   * @return The burst created
   */
  virtual Burst * addBurst(int pos);
};

#endif
