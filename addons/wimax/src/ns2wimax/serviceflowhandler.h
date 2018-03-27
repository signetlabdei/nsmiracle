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

#ifndef SERVICEFLOWHANDLER_H
#define SERVICEFLOWHANDLER_H

#include "serviceflowqos.h"
#include "serviceflow.h"
#include "packet.h"

class Mac802_16;

/**
 * Handler for service flows
 */
class ServiceFlowHandler {

 public:

  /* 
   * Create a service flow
   * @param mac The Mac where it is located
   */
  ServiceFlowHandler ();

  /*
   * Set the mac it is located in 
   * @param mac The mac it is located in 
   */
  void setMac (Mac802_16 *mac);
  
  /**
   * Process the given packet. Only service related packets must be sent here.
   * @param p The packet to process
   */
  void  process (Packet * p);
  
  /**
   * Add a flow
   * @param qos The qos for the new connection
   */
  ServiceFlow* addFlow (ServiceFlowQoS * qos);
      
  /**
   * Remove a flow
   * @param id The flow ID
   */
  void  removeFlow (int id);
    
  /**
   * Send a flow request to the given node
   * @param index The node address
   * @param incoming The flow direction
   */
  void sendFlowRequest (int index, bool incoming);

 protected:

  /**
   * process a flow request
   * @param p The received request
   */
  void processDSA_req (Packet *p);

  /**
   * process a flow response
   * @param p The received response
   */
  void processDSA_rsp (Packet *p);

  /**
   * process a flow request
   * @param p The received response
   */
  void processDSA_ack (Packet *p);
  
 private:

  /**
   * The Mac where this handler is located
   */
   Mac802_16 * mac_;

  /**
   * The list of current flows
   */
   struct serviceflow flow_head_;

   /**
    * List of pending flows
    */
   struct serviceflow pendingflow_head_;
};

#endif //SERVICEFLOWHANDLER_H

