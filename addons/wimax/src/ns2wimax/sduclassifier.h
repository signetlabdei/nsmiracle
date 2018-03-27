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

#ifndef SDUCLASSIFIER_H
#define SDUCLASSIFIER_H

#include "packet.h"

class Mac802_16;

class SDUClassifier;
LIST_HEAD (sduClassifier, SDUClassifier);
/*
 * Abstract class for classifiers that will map a packet to a CID
 */
class SDUClassifier : public TclObject
{
  friend class Mac802_16;

 public:
  /*
   * Create a classifier in the given mac
   */
  SDUClassifier ();

  /*
   * Interface with the TCL script
   * @param argc The number of parameter
   * @param argv The list of parameters
   */
  int command(int argc, const char*const* argv);

  /* 
   * Return the classifier's priority
   */
  inline int getPriority () { return priority_; }

  /* 
   * Set the classifier's priority
   * @param prio The new priority
   */
  inline void setPriority (int prio) { priority_ = prio; }

  /**
   * Classify a packet and return the CID to use (or -1 if unknown)
   * @param p The packet to classify
   * @return The CID or -1
   */
  virtual int classify (Packet * p);

  // Chain element to the list
  inline void insert_entry_head(struct sduClassifier *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Chain element to the list
  inline void insert_entry(struct SDUClassifier *elem) {
    LIST_INSERT_AFTER(elem, this, link);
  }
  
  // Return next element in the chained list
  SDUClassifier* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }

 protected:
  /* 
   * The max where the classifier is located
   */
  Mac802_16 *mac_;

  /*
   * The priority
   */
  int priority_;

  /*
   * Pointer to next in the list
   */
  LIST_ENTRY(SDUClassifier) link;
  //LIST_ENTRY(SDUClassifier); //for magic draw

  /* 
   * Register the Mac
   */
  inline void setMac (Mac802_16 *mac) { assert (mac); mac_ = mac; }
};

#endif
