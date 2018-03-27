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

#ifndef PEERNODE_H
#define PEERNODE_H

#include "connection.h"
#include "mac-stats.h"

class PeerNode;
LIST_HEAD (peerNode, PeerNode);
/**
 * Class PeerNode
 * Supports list
 */ 
class PeerNode {

public:

  /**
   * Constructor
   * @param index The Mac address of the peer node
   */
  PeerNode (int index);

  /**
   * Return the address of the peer node
   * @return The address of the peer node
   */
  int getAddr () { return peerIndex_; }

  /**
   * Set the connection for delay-intolerant management messages
   * @param i_con The connection used as basic for incoming
   * @param o_con The connection used as basic for outgoing
   */
  void  setBasic (Connection* i_con, Connection* o_con);
  
  /**
   * Return the connection used for delay-intolerant messages
   */
  inline Connection*  getBasic (bool out) { return out?basic_out_:basic_in_; }
  
  /**
   * Set the connection for delay-tolerant management messages
   * @param i_con The connection used as primary for incoming
   * @param o_con The connection used as primary for outgoing
   */
  void  setPrimary (Connection* i_con, Connection* o_con);
  
  /**
   * Return the connection used for delay-tolerant messages
   */
  inline Connection*  getPrimary (bool out) { return out?primary_out_:primary_in_; }
  
  /**
   * Set the channel used for standard-based messages
   * @param i_con The connection used as secondary for incoming
   * @param o_con The connection used as secondary for outgoing
   */
  void  setSecondary (Connection* i_con, Connection* o_con);
  
  /**
   * Return the connection used for standard-based messages
   */
  inline Connection*  getSecondary (bool out) { return out?secondary_out_:secondary_in_; }

  /**
   * Set the channel used for data messages
   * @param connection 
   */
  void  setInData (Connection * connection);
  
  /**
   * Set the channel used for data messages
   * @param connection 
   */
  void  setOutData (Connection * connection);
  
  /**
   * Return the connection used for data messages
   */
  Connection*  getOutData () { return outdata_; }

  /**
   * Return the connection used for data messages
   */
  Connection*  getInData () { return indata_; }

  /**
   * Set the time the last packet was received
   * @param time The time the last packet was received
   */
  void setRxTime (double time);

  /**
   * Get the time the last packet was received
   * @return The time the last packet was received
   */
  double getRxTime ();  

  /**
   * Return the stat watch
   * @return The stat watch
   */
  StatWatch * getStatWatch();

  /**
   * Return true if the peer is going down
   * @return true if the peer is going down
   */
  inline bool isGoingDown () { return going_down_; }

  /**
   * Set the status of going down
   * @param status The link going down status
   */
  inline void setGoingDown (bool status) { going_down_ = status; }

  /** 
   * Return the requested bandwidth for this node
   * @return The requested bandwidth for this node
   */
  int getReqBw ();

  /**
   * Return the amount of data queued for this node
   * @return The queued data for this node
   */
  int getQueueLength ();

  /**
   * Set the requested downlink profile
   * @param diuc The downlink profile
   */
  void setDIUC (int diuc);

  /**
   * Get the requested downlink profile
   * @return The downlink profile
   */
  int getDIUC ();


  // Chain element to the list
  inline void insert_entry(struct peerNode *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Return next element in the chained list
  PeerNode* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }  
protected:

  /*
   * Pointer to next in the list
   */
  LIST_ENTRY(PeerNode) link;
  //LIST_ENTRY(PeerNode); //for magic draw

private:
  /**
   * Mac address of peer node
   */
  int peerIndex_;

  /**
   * Used to receive delay intolerant management messages
   */
   Connection* basic_in_;

  /**
   * Used to send delay intolerant management messages
   */
   Connection* basic_out_;

  /**
   * Used to receive delay tolerant mac messages
   */
   Connection* primary_in_;

  /**
   * Used to send delay tolerant mac messages
   */
   Connection* primary_out_;

  /**
   * Used to receive standard-based protocol (DHCP...)
   */
   Connection* secondary_in_;

  /**
   * Used to send standard-based protocol (DHCP...)
   */
   Connection* secondary_out_;

  /**
   * Incoming data connection to this client
   */
   Connection* indata_;  
   
  /**
   * Outgoing data connection to this client
   */
   Connection* outdata_;  
   
   /**
    * Time last packet was received for this peer
    */
   double rxtime_;

   /**
    * Received signal strength stats
    */
   StatWatch rxp_watch_;

   /**
    * Inidicate the link going down status of the peer node
    */
   bool going_down_;

   /**
    * For MS, it indicates the requested downlink profile
    */
   int diuc_;

};
#endif //PEERNODE_H

