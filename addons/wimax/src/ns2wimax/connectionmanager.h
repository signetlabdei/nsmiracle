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

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "connection.h"

class Mac802_16;

/**
 * Class ConnectionManager
 * The class handles the list of connections for a Mac 802.16
 */ 
class ConnectionManager {
  friend class Connection;
 public:
  /**
   * 
   * @param mac The mac where the manager belongs
   */
  ConnectionManager (Mac802_16 * mac);
  
  /**
   * Add a connection
   * @param con The connection to add
   * @param out true if it is an outgoing connection
   */
  void add_connection (Connection* con, bool out);
  
  /**
   * Remove the given connection
   * @param connection Remove the given connection
   */
  void remove_connection (Connection* connection);

  /**
   * Remove connection by CID, both directions.
   * @param cid The connection id
   */
  void remove_connection (int cid);
  
  /**
   * Return the connection with the given cid and direction
   * @param cid The connection id
   * @param out specifies the direction of the connection
   */
  Connection*  get_connection (int cid, bool out);
  
  /**
   * Return the head of the incoming connection list
   */
  Connection* get_in_connection () { return i_con_list_.lh_first; }

  /**
   * Return the head of the outgoing connection list
   */
  Connection* get_out_connection () { return o_con_list_.lh_first; }


  /**
   * Flush the queues. This can be called after switching BS.
   */
  void  flush_queues ();
  
 protected:
  /**
   * Get the value of mac_
   * The Mac where this object is located
   * @return the value of mac_
   */
  inline Mac802_16 * getMac ( ) { return mac_; }
  
 private:
  /**
   * The list of available connections
   */
  Connection* connections_;
  
  /**
   * The Mac where this object is located
   */
  Mac802_16 * mac_;
  
  /**
   * The list of incoming connections
   */
  struct connection i_con_list_;

  /**
   * The list of outgoing connections
   */
  struct connection o_con_list_;

  
};
#endif //CONNECTIONMANAGER_H

