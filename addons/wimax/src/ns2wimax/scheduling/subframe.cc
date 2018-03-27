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

#include "subframe.h"
#include "contentionslot.h"
#include "ulsubframetimer.h"
#include "dlsubframetimer.h"

/*** Functions of super class ***/

/**
 * Constructor
 * @param map The frame 
 */
SubFrame::SubFrame (FrameMap *map)
{
  assert (map);
  map_ = map;
  ccc_ = 0;
  nbProfile_ = 0;
  LIST_INIT (&profile_list_);
}

/**
 * Destructor
 */
SubFrame::~SubFrame ()
{
  removeProfiles();
}

/**
 * Add a profile
 * @param f The frequency of the profile
 * @param enc The encoding used in the profile
 * @return a new profile with the given caracteristics
 */
Profile * SubFrame::addProfile (int f, Ofdm_mod_rate enc)
{
  Profile *p = new Profile (this, f, enc);
  p->insert_entry (&profile_list_);
  nbProfile_++;
  incrCCC();
  return p;
}

/**
 * Remove a profile
 * @param p The profile to remove
 * @return a new profile with the given caracteristics
 */
void SubFrame::removeProfile (Profile *p)
{
  p->remove_entry ();
  nbProfile_--;
  incrCCC();
}

/**
 * Remove all profiles
 */
void SubFrame::removeProfiles ()
{
  for (Profile *p = profile_list_.lh_first; p ; p=profile_list_.lh_first) {
    removeProfile (p);
    delete (p);
  }
}

/**
 * Return the profile with the given IUC
 * @return the profile with the given IUC
 */
Profile * SubFrame::getProfile (int iuc)
{
  Profile *p;
  for (p = profile_list_.lh_first; p ; p=p->next_entry()) {
    if (p->getIUC()==iuc)
      return p;
  }
    return NULL;
}

/**
 * Return the number of profiles for this subframe
 */
int SubFrame::getNbProfile () 
{ 
  return nbProfile_; 
}

/**
 * Return the head of the profile list
 */
Profile * SubFrame::getFirstProfile () 
{ 
  return profile_list_.lh_first; 
}

/**
 * Return the Configuration Change count
 */
int SubFrame::getCCC () 
{ 
  return ccc_; 
}

/**
 * Increment the configuration change count
 * The CCC is modulo 256.
 */
void SubFrame::incrCCC ()
{
  ccc_ = (ccc_+1)%256;
}

/*** end of super class ***/

/**
 * Constructor
 * @param map The frame 
 */
DlSubFrame::DlSubFrame (FrameMap *map): SubFrame(map)
{
  phypdu_ = new DlPhyPdu (map , 0); //no preamble by default
  timer_ = new DlSubFrameTimer (this);
}

/**
 * Destructor
 */
DlSubFrame::~DlSubFrame ()
{
  delete (phypdu_);
  delete (timer_);
}

/*** class UlSubFrame ***/

/**
 * Constructor
 * @param map The frame 
 */
UlSubFrame::UlSubFrame (FrameMap *map): SubFrame(map)
{
  LIST_INIT (&phypdu_list_);
  nbPhyPdu_ =0;
  timer_ = new UlSubFrameTimer (this);
  bw_req_ = new BwContentionSlot (map);
  ranging_ = new RngContentionSlot (map);
}

/**
 * Destructor
 */
UlSubFrame::~UlSubFrame ()
{
  for (PhyPdu *p = phypdu_list_.lh_first; p ; p=phypdu_list_.lh_first) {
    removePhyPdu (p);
    delete (p);
  }
  delete (timer_);
  delete (bw_req_);
  delete (ranging_);
}

/**
 * Add an uplink phy pdu
 * @param pos The position of the PhyPdu in the subframe
 * @param preamble The size of the preamble in units of XX
 * @return newly created PhyPdu
 */
PhyPdu * UlSubFrame::addPhyPdu (int pos, int preamble)
{
  assert (pos >= 0 && pos <= nbPhyPdu_ );
  UlPhyPdu *p = new UlPhyPdu (map_, preamble);
  if (pos==0)
    p->insert_entry_head (&phypdu_list_);
  else {
    PhyPdu *prev = phypdu_list_.lh_first;
    PhyPdu *p2 = prev->next_entry();
    int index = 1;
    while (index < pos) {
      prev=p2;
      p2=p2->next_entry();
      index++;
    }
    p->insert_entry (prev);
  }
  nbPhyPdu_++;
  return p;
}

/**
 * Remove a Phy PDU
 * @param pdu The Phy PDU to remove
 */
void UlSubFrame::removePhyPdu (PhyPdu *p)
{
  p->remove_entry();
  nbPhyPdu_--;
}

/**
 * Return the burst located at the given index
 * @param pos The position of the burst
 */
PhyPdu* UlSubFrame::getPhyPdu(int pos)
{
  assert (pos >= 0 && pos < nbPhyPdu_ );
  PhyPdu *p = phypdu_list_.lh_first ;
  for (int i = 0 ; i < pos ; i++) {
    p=p->next_entry();
  }
  return p;
}
