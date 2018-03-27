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

#include "serviceflowhandler.h"
#include "mac802_16.h"
#include "scheduling/wimaxscheduler.h" 

static int TransactionID = 0; 
/* 
 * Create a service flow
 * @param mac The Mac where it is located
 */
ServiceFlowHandler::ServiceFlowHandler ()
{
  LIST_INIT (&flow_head_);
  LIST_INIT (&pendingflow_head_);
}

/*
 * Set the mac it is located in
 * @param mac The mac it is located in
 */
void ServiceFlowHandler::setMac (Mac802_16 *mac)
{
  assert (mac);

  mac_ = mac;
}

/**
 * Process the given packet. Only service related packets must be sent here.
 * @param p The packet received
 */
void ServiceFlowHandler::process (Packet * p) 
{ 
  hdr_mac802_16 *wimaxHdr = HDR_MAC802_16(p);
  gen_mac_header_t header = wimaxHdr->header;

  //we cast to this frame because all management frame start with
  //a type 
  mac802_16_dl_map_frame *frame = (mac802_16_dl_map_frame*) p->accessdata();

  switch (frame->type) {
  case MAC_DSA_REQ: 
    processDSA_req (p);
    break;
  case MAC_DSA_RSP: 
    processDSA_rsp (p);
    break;
  case MAC_DSA_ACK: 
    processDSA_ack (p);
    break;
  default: 
    printf ("Unknow frame type (%d) in flow handler\n", frame->type);
  }
  //Packet::free (p);
}

/**
 * Add a flow with the given qos
 * @param qos The QoS for the flow
 * @return the created ServiceFlow
 */
ServiceFlow* ServiceFlowHandler::addFlow (ServiceFlowQoS * qos) {
  return NULL;
}

/**
 * Remove the flow given its id
 * @param id The flow id
 */
void ServiceFlowHandler::removeFlow (int id) {
  
}

/**
 * Send a flow request to the given node
 * @param index The node address
 * @param out The flow direction
 */
void ServiceFlowHandler::sendFlowRequest (int index, bool out)
{
  Packet *p;
  struct hdr_cmn *ch;
  hdr_mac802_16 *wimaxHdr;
  mac802_16_dsa_req_frame *dsa_frame;
  PeerNode *peer;

  //create packet for request
  peer = mac_->getPeerNode(index);  
  p = mac_->getPacket ();
  ch = HDR_CMN(p);
  wimaxHdr = HDR_MAC802_16(p);
  p->allocdata (sizeof (struct mac802_16_dsa_req_frame));
  dsa_frame = (mac802_16_dsa_req_frame*) p->accessdata();
  dsa_frame->type = MAC_DSA_REQ;
  dsa_frame->uplink = (out && mac_->getNodeType()==STA_MN) || (!out && mac_->getNodeType()==STA_BS) ;
  dsa_frame->transaction_id = TransactionID++;
  if (mac_->getNodeType()==STA_MN)
    ch->size() += GET_DSA_REQ_SIZE (0);
  else {
    //assign a CID and include it in the message
    Connection *data = new Connection (CONN_DATA);
    mac_->getCManager()->add_connection (data, out);
    if (out)
      peer->setOutData (data);
    else
      peer->setInData (data);
    dsa_frame->cid = data->get_cid();
    ch->size() += GET_DSA_REQ_SIZE (1);
  }

  wimaxHdr->header.cid = peer->getPrimary(OUT_CONNECTION)->get_cid();
  peer->getPrimary(OUT_CONNECTION)->enqueue (p);
}

/**
 * process a flow request
 * @param p The received request
 */
void ServiceFlowHandler::processDSA_req (Packet *p)
{
  mac_->debug ("At %f in Mac %d received DSA request from %d\n", NOW, mac_->addr(), HDR_MAC802_16(p)->header.cid);
  
  Packet *rsp;
  struct hdr_cmn *ch;
  hdr_mac802_16 *wimaxHdr_req;
  hdr_mac802_16 *wimaxHdr_rsp;
  mac802_16_dsa_req_frame *dsa_req_frame;
  mac802_16_dsa_rsp_frame *dsa_rsp_frame;
  PeerNode *peer;
  Connection *data;

  //read the request
  wimaxHdr_req = HDR_MAC802_16(p);
  dsa_req_frame = (mac802_16_dsa_req_frame*) p->accessdata();
  peer = mac_->getCManager ()->get_connection (wimaxHdr_req->header.cid, true)->getPeerNode();
  
  //allocate response
  //create packet for request
  rsp = mac_->getPacket ();
  ch = HDR_CMN(rsp);
  wimaxHdr_rsp = HDR_MAC802_16(rsp);
  rsp->allocdata (sizeof (struct mac802_16_dsa_rsp_frame));
  dsa_rsp_frame = (mac802_16_dsa_rsp_frame*) rsp->accessdata();
  dsa_rsp_frame->type = MAC_DSA_RSP;
  dsa_rsp_frame->transaction_id = dsa_req_frame->transaction_id;
  dsa_rsp_frame->uplink = dsa_req_frame->uplink;
  dsa_rsp_frame->confirmation_code = 0; //OK

  if (mac_->getNodeType()==STA_MN) {
    //the message contains the CID for the connection
    data = new Connection (CONN_DATA, dsa_req_frame->cid);
    if (dsa_req_frame->uplink){
      mac_->getCManager()->add_connection (data, OUT_CONNECTION);
      peer->setOutData (data);
    } else {
      mac_->getCManager()->add_connection (data, IN_CONNECTION);
      peer->setInData (data);
    }
    ch->size() += GET_DSA_RSP_SIZE (0);
  } else {
    //allocate new connection
    data = new Connection (CONN_DATA);
    if (dsa_req_frame->uplink) {
      mac_->getCManager()->add_connection (data, IN_CONNECTION);
      peer->setInData (data);
    } else {
      mac_->getCManager()->add_connection (data, OUT_CONNECTION);
      peer->setOutData (data);
    }
    dsa_rsp_frame->cid = data->get_cid();
    ch->size() += GET_DSA_RSP_SIZE (1);
  }

  wimaxHdr_rsp->header.cid = peer->getPrimary(OUT_CONNECTION)->get_cid();
  peer->getPrimary(OUT_CONNECTION)->enqueue (rsp);

}

/**
 * process a flow response
 * @param p The received response
 */
void ServiceFlowHandler::processDSA_rsp (Packet *p)
{
  mac_->debug ("At %f in Mac %d received DSA response\n", NOW, mac_->addr());

  Packet *ack;
  struct hdr_cmn *ch;
  hdr_mac802_16 *wimaxHdr_ack;
  hdr_mac802_16 *wimaxHdr_rsp;
  mac802_16_dsa_ack_frame *dsa_ack_frame;
  mac802_16_dsa_rsp_frame *dsa_rsp_frame;
  Connection *data;
  PeerNode *peer;

  //read the request
  wimaxHdr_rsp = HDR_MAC802_16(p);
  dsa_rsp_frame = (mac802_16_dsa_rsp_frame*) p->accessdata();
  peer = mac_->getCManager ()->get_connection (wimaxHdr_rsp->header.cid, true)->getPeerNode();
  
  //TBD: check if status not OK

  if (mac_->getNodeType()==STA_MN) {
    //the message contains the CID for the connection
    data = new Connection (CONN_DATA, dsa_rsp_frame->cid);
    if (dsa_rsp_frame->uplink) {
      mac_->getCManager()->add_connection (data, OUT_CONNECTION);
      peer->setOutData (data);
    } else {
      mac_->getCManager()->add_connection (data, IN_CONNECTION);
      peer->setInData (data);
    }
  }

  //allocate ack
  //create packet for request
  ack = mac_->getPacket ();
  ch = HDR_CMN(ack);
  wimaxHdr_ack = HDR_MAC802_16(ack);
  ack->allocdata (sizeof (struct mac802_16_dsa_ack_frame));
  dsa_ack_frame = (mac802_16_dsa_ack_frame*) ack->accessdata();
  dsa_ack_frame->type = MAC_DSA_ACK;
  dsa_ack_frame->transaction_id = dsa_rsp_frame->transaction_id;
  dsa_ack_frame->uplink = dsa_rsp_frame->uplink;
  dsa_ack_frame->confirmation_code = 0; //OK
  ch->size() += DSA_ACK_SIZE;

  wimaxHdr_ack->header.cid = peer->getPrimary(OUT_CONNECTION)->get_cid();
  peer->getPrimary(OUT_CONNECTION)->enqueue (ack);

}

/**
 * process a flow request
 * @param p The received response
 */
void ServiceFlowHandler::processDSA_ack (Packet *p)
{
  mac_->debug ("At %f in Mac %d received DSA ack\n", NOW, mac_->addr());
}
