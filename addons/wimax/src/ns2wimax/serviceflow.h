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

#ifndef SERVICEFLOW_H
#define SERVICEFLOW_H

#include "serviceflowqos.h"
//#include "connection.h"
#include "packet.h"

#define UNASSIGNED_FLOW_ID -1

/** Defines the supported scheduling mechanism for the flow */
enum SchedulingType_t {
  SERVICE_UGS,
  SERVICE_rtPS,
  SERVICE_nrtPS,
  SERVICE_BE
};

class ServiceFlow;
LIST_HEAD (serviceflow, ServiceFlow);

/**
 * Class ServiceFlow
 * The service flow identifies the service requirement 
 * for the associated connection 
 */ 
class ServiceFlow {
 public:
  /**
   * Constructor
   */
  ServiceFlow (SchedulingType_t, ServiceFlowQoS*);

  /**
   * Return the service flow id
   * @return The service flow id. -1 if not yet assigned
   */
  inline int getID () { return id_; }
  
  /**
   * Assign an ID to the service flow
   * @param id The ID to set
   */
  void setID (int id);

  /**
   * Pick the next available ID. Should be called by a BS to assign a unique ID
   */
  void pickID ();

  /**
   * Set the scheduling mechanism for this flow
   * @param scheduling The scheduling type
   */
  inline void setScheduling (SchedulingType_t scheduling) {scheduling_ = scheduling;}
  
  /**
   * Return the scheduling type for this service flow
   */
  inline SchedulingType_t getScheduling () { return scheduling_; }
  
  /**
   * Set the QoS for this flow
   * @param qos The new QoS for this flow
   */
  inline void setQoS (ServiceFlowQoS* qos) { qos_ = qos; }

  /**
   * Return the QoS for this connection
   */
  inline ServiceFlowQoS * getQoS () { return qos_; }
  
  // Chain element to the list
  inline void insert_entry_head(struct serviceflow *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Chain element to the list
  inline void insert_entry(ServiceFlow *elem) {
    LIST_INSERT_AFTER(elem, this, link);
  }

  // Return next element in the chained list
  ServiceFlow* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }

 protected:

  /**
   * Pointer to next in the list
   */
  LIST_ENTRY(ServiceFlow) link;
  //LIST_ENTRY(ServiceFlow); //for magic draw

 private:
  /**
   * The service flow id
   */
   int id_;

  /**
   * The scheduling type (UGS, rtPS...)
   */
   SchedulingType_t scheduling_;

   /**
    * Flow direction
    */
   int direction_;

   /**
    * The quality of service for this flow
    */
   ServiceFlowQoS * qos_;

};
#endif //SERVICEFLOW_H

