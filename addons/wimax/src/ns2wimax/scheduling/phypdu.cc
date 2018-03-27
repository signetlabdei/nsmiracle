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

#include "phypdu.h"

/**
 * Create a phy pdu with the given preamble size
 * @param preamble The preamble size in OFDM symbols
 */
PhyPdu::PhyPdu (FrameMap *map, int preamble)
{
  assert (preamble >=0 && map);
  preamble_ = preamble;
  map_ = map;
  nb_burst_=0;
  LIST_INIT(&burst_list_);
}

/*
 * Delete the object
 */
PhyPdu::~PhyPdu ()
{
  for (Burst *b = burst_list_.lh_first; b ; b=burst_list_.lh_first) {
    b->remove_entry ();
    delete (b);
  }
}


/**
 * Set the preamble size for the PDU in unit of OFDM symbols
 * @param preamble the preamble size for the PDU
 */
void PhyPdu::setPreamble( int preamble )
{
  assert (preamble>=0);
  preamble_ = preamble;
}

/**
 * Return the preamble size for the PDU in unit of OFDM symbols
 * @return the preamble size for the PDU
 */
int PhyPdu::getPreamble( )
{
  return preamble_;
}

/**
 * Add a burst in the PDU
 * @param pos The position of the burst 
 */
Burst* PhyPdu::addBurst(int pos)
{
  assert (pos >= 0 && pos <= nb_burst_ );
  Burst *b = new Burst (this);
  if (pos==0)
    b->insert_entry_head (&burst_list_);
  else {
    Burst *prev = burst_list_.lh_first ;
    Burst *b2 = prev->next_entry();
    int index = 1;
    while (index < pos) {
      prev=b2;
      b2=b2->next_entry();
      index++;
    }
    b->insert_entry (prev);
  }
  nb_burst_++;
  return b;
}

/**
 * Remove a burst in the PDU
 * @param burst The burst to remove
 */
void PhyPdu::removeBurst(Burst *b)
{
  b->remove_entry();
  nb_burst_--;
}

/** 
 * Clear all the bursts in the list
 */
void PhyPdu::removeAllBursts ()
{
  Burst *b;
  while (nb_burst_>0) {
    b = getBurst(0);
    removeBurst (b);
    delete (b);
  }
  assert (nb_burst_==0);
}

/**
 * Return the burst located at the given index
 * @param pos The position of the burst
 */
Burst* PhyPdu::getBurst(int pos)
{
  assert (pos >= 0 && pos < nb_burst_ );
  Burst *b = burst_list_.lh_first ;
  for (int i = 0 ; i < pos ; i++) {
    b=b->next_entry();
  }
  return b;
}

/** Methods for class DlPhyPdu **/

/**
 * Create a phy pdu with the given preamble size
 * @param preamble The preamble size in OFDM symbols
 */
DlPhyPdu::DlPhyPdu (FrameMap *map, int preamble) : PhyPdu(map, preamble)
{

}

/**
 * Add a burst in the PDU
 * @param pos The position of the burst 
 */
Burst* DlPhyPdu::addBurst(int pos)
{
  assert (pos >= 0 && pos <= nb_burst_ );
  DlBurst *b = new DlBurst (this);
  if (pos==0 || nb_burst_==0)
    b->insert_entry_head (&burst_list_);
  else {
    Burst *prev = burst_list_.lh_first ;
    Burst *b2 = prev->next_entry();
    int index = 1;
    while (b2 && index < pos) {
      prev=b2;
      b2=b2->next_entry();
      index++;
    }
    b->insert_entry (prev);
  }
  nb_burst_++;
  return b;
}


/** Methods for class UlPhyPdu **/

/**
 * Create a phy pdu with the given preamble size
 * @param preamble The preamble size in OFDM symbols
 */
UlPhyPdu::UlPhyPdu (FrameMap *map, int preamble) : PhyPdu(map, preamble)
{

}

/**
 * Add a burst in the PDU
 * @param pos The position of the burst 
 */
Burst* UlPhyPdu::addBurst(int pos)
{
  //UlPhyPdu only have one burst
  assert (pos == 0 && nb_burst_==0 );
  UlBurst *b = new UlBurst (this);
  b->insert_entry_head (&burst_list_);
  nb_burst_++;
  return b;
}
