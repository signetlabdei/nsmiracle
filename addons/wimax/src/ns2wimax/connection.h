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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "serviceflow.h"
#include "packet.h"
#include "queue.h"
#include "mac802_16pkt.h"

/* CONSTANTS */
#define INITIAL_RANGING_CID 0x0000
#define BASIC_CID_START     0x0001
#define BASIC_CID_STOP      0x2000
#define PRIMARY_CID_START   0x2001
#define PRIMARY_CID_STOP    0x4000
#define TRANSPORT_SEC_CID_START 0x4001
#define TRANSPORT_SEC_CID_STOP 0xFEFE
#define AAS_INIT_RANGIN_CID 0xFEFF
#define MULTICAST_CID_START 0xFF00
#define MULTICAST_CID_STOP  0xFFFD
#define PADDING_CID         0xFFFE
#define BROADCAST_CID       0xFFFF

#define IN_CONNECTION  false
#define OUT_CONNECTION true

/**
 * Define the type of the connection
 */
enum ConnectionType_t {
  CONN_INIT_RANGING,
  CONN_AAS_INIT_RANGING,
  CONN_MULTICAST_POLLING,
  CONN_PADDING,
  CONN_BROADCAST,
  CONN_BASIC,
  CONN_PRIMARY,
  CONN_SECONDARY,
  CONN_DATA
};

class PeerNode;
class ConnectionManager;
class Connection;
LIST_HEAD (connection, Connection);

/** 
 * Class Connection
 * The class supports LIST.
 */ 
class Connection {
 public:
  /** constructor */
  Connection (ConnectionType_t);

  /** constructor */
  Connection (ConnectionType_t, int cid);    

  /** destructor */
  ~Connection ();

  /**
   * Set the connection manager
   * @param manager The Connection manager 
   */
  void setManager (ConnectionManager *manager);

  /**
   * Enqueue the given packet
   * @param p The packet to enqueue
   */
  void  enqueue (Packet * p);
  
  /**
   * Set the service flow for this connection
   * @param sflow The service flow for this connection
   */
  void  setServiceFlow (ServiceFlow * sflow);
  
  /**
   * Return the service flow for this connection
   */
  ServiceFlow *  getServiceFlow ();

  /**
   * Get the value of cid
   * The connection id
   * @return the value of cid
   */
  inline int get_cid ( ) { return cid_; }

  /**
   * Get the value of category_
   * The connection id
   * @return the value of category_
   */
  inline ConnectionType_t get_category ( ) { return category_; }
      
  /**
   * Set the value of category_
   * The connection id
   * @return the value of category_
   */
  inline void set_category (ConnectionType_t value ) { category_ = value; }
    
  /**
   * Get the value of serviceflow_
   * The service flow associated with the connection
   * @return the value of serviceflow_
   */
  inline ServiceFlow * get_serviceflow ( ) { return serviceflow_; }
  
  /**
   * Set the value of serviceflow_
   * The service flow associated with the connection
   * @return the value of serviceflow_
   */
  inline void set_serviceflow (ServiceFlow * value ) { serviceflow_ = value; }
  
  /**
   * return the connection type
   * @return The connection type
   */
  inline ConnectionType_t getType () { return type_; }

  /**
   * Get the value of queue_
   * The queue for this connection
   * @return the value of queue_
   */
  inline PacketQueue * get_queue ( ) { return queue_; }
    
  /**
   * Dequeue a packet from the queue
   * @param p The packet to enqueue
   */
  Packet * dequeue ();

  /**
   * Return queue size in bytes
   * @return The queue size in bytes
   */
  int queueByteLength ();

  /**
   * Return queue size in number of packets
   * @return The number of packet in the queue
   */
  int queueLength ();

  /**
   * Flush the queue
   */
  int flush_queue ();

  /**
   * Enable/Disable fragmentation
   */
  void enable_fragmentation (bool enable) { frag_enable_ = enable; }

  /**
   * Indicates if the connection supports fragmentation
   */
  bool isFragEnable () { return frag_enable_; }

  // Chain element to the list
  inline void insert_entry(struct connection *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Return next element in the chained list
  Connection* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }

  /**
   * Return the peer node for this connection
   * @return the peer node for this connection
   */
  inline PeerNode * getPeerNode () { return peer_; }

  /**
   * Set the peer node for this connection
   * @param the peer node for this connection
   */
  inline void setPeerNode (PeerNode *peer) { peer_=peer; }

  /** 
   * Update the fragmentation information
   * @param status The new fragmentation status
   * @param index The new fragmentation index
   * @param bytes The number of bytes 
   */
  void updateFragmentation (fragment_status status, int index, int bytes);

  fragment_status getFragmentationStatus () { return frag_status_; }

  int getFragmentNumber () { return frag_nb_; }

  int getFragmentBytes () { return frag_byte_proc_; }


  /**
   * Set the bandwidth requested 
   * @param bw The bandwidth requested in bytes
   */
  void setBw (int bw);

  /**
   * Set the bandwidth requested
   * @param bw The bandwidth requested in bytes
   */
  int getBw ();


 protected:

  /**
   * Pointer to next in the list
   */
  LIST_ENTRY(Connection) link;
  //LIST_ENTRY(Connection); //for magic draw


 private:
  /**
   * The connection manager
   */
  ConnectionManager* manager_;

  /**
   * The connection id
   */
  int cid_;

  /**
   * The category
   */
  ConnectionType_t category_;

  /**
   * The service flow associated with the connection
   */
  ServiceFlow * serviceflow_;

  /**
   * The queue for this connection
   */
  PacketQueue * queue_;

  /** 
   * The connection type
   */
  ConnectionType_t type_;
  
  /**
   * Pointer to the peer node data
   */
  PeerNode *peer_;

  /**
   * Fragmentation status 
   */
  fragment_status frag_status_;

  /**
   * Fragmentation number
   */
  int frag_nb_;
  
  /**
   * Bytes already processed (i.e sent or received)
   */
  int frag_byte_proc_;

  /**
   * Indicates if the connection can use fragmentation
   */
  bool frag_enable_;

  /** 
   * Indicates the number of bytes requested
   */
  int requested_bw_;

};
#endif //CONNECTION_H

