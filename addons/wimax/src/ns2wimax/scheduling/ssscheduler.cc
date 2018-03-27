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

#include "ssscheduler.h"
#include "burst.h"
#include "mac802_16SS.h"

/**
 * Tcl hook for creating SS scheduler 
 */
static class SSschedulerClass : public TclClass {
public:
  SSschedulerClass() : TclClass("WimaxScheduler/SS") {}
  TclObject* create(int, const char*const*) {
    return (new SSscheduler());
    
  }
} class_ssscheduler;

/*
 * Create a scheduler
 */
SSscheduler::SSscheduler ()
{
  debug2 ("SSscheduler created\n");
}

/**
 * Initializes the scheduler
 */
void SSscheduler::init ()
{
  WimaxScheduler::init();
}

/**
 * Interface with the TCL script
 * @param argc The number of parameter
 * @param argv The list of parameters
 */
int SSscheduler::command(int argc, const char*const* argv)
{
  //no command. Remove this function if not used.
  return TCL_ERROR;
}

/**
 * Schedule bursts/packets
 */
void SSscheduler::schedule ()
{
  Burst *b;
  FrameMap *map = mac_->getMap();
  PeerNode *peer = mac_->getPeerNode_head(); //this is the BS
  assert (peer!=NULL);

  //We go through the list of UL bursts until we find an allocation for us
  //printf ("SS has %d ul bursts\n", map->getUlSubframe()->getNbPdu ());
  for (int index = 0 ; index < map->getUlSubframe()->getNbPdu (); index++) {
    b = map->getUlSubframe()->getPhyPdu (index)->getBurst (0);

    if (b->getIUC()==UIUC_END_OF_MAP) {
      //consistency check..
      assert (index == map->getUlSubframe()->getNbPdu ()-1);
      break;
    }    
    
    if (b->getIUC()==UIUC_INITIAL_RANGING || b->getIUC()==UIUC_REQ_REGION_FULL)
      continue; //contention slots are handled automatically

    int b_data = 0; 
    //get the packets from the connection with the same CID
    //printf ("\tBurst CID=%d\n", b->getCid());
    Connection *c=mac_->getCManager ()->get_connection (b->getCid(), true);
    //assert (c);
    if (!c)
      continue; //I do not have this CID. Must be for another node
    assert (c->getType()==CONN_BASIC); //allocation are made per SS using their basic CIDs

    //transfer the packets until it reaches burst duration or no more packets
    //Priority: Basic, Primary, Secondary, Data
    if (peer->getBasic(OUT_CONNECTION)!= NULL) 
      b_data = mac_->getScheduler()->transfer_packets (peer->getBasic(OUT_CONNECTION), b, b_data);
    if (peer->getPrimary(OUT_CONNECTION)!= NULL)
      b_data = mac_->getScheduler()->transfer_packets (peer->getPrimary(OUT_CONNECTION), b, b_data);
    if (peer->getSecondary(OUT_CONNECTION)!= NULL)
      b_data = mac_->getScheduler()->transfer_packets (peer->getSecondary(OUT_CONNECTION), b, b_data);
    if (peer->getOutData()!=NULL)
      b_data = mac_->getScheduler()->transfer_packets (peer->getOutData(), b, b_data);
  }

  //compute size of data left to create bandwidth requests
  if (peer->getBasic(OUT_CONNECTION)!= NULL) 
    create_request (peer->getBasic(OUT_CONNECTION));
  if (peer->getPrimary(OUT_CONNECTION)!= NULL)
    create_request (peer->getPrimary(OUT_CONNECTION));
  if (peer->getSecondary(OUT_CONNECTION)!= NULL)
    create_request (peer->getSecondary(OUT_CONNECTION));
  if (peer->getOutData()!=NULL)
    create_request (peer->getOutData());
  
}

/**
 * Create a request for the given connection
 * This function may need to be updated to handle
 * incremental and aggregate requests
 * @param con The connection to check
 */
void SSscheduler::create_request (Connection *con)
{
  if (con->queueLength()==0)
    return; //queue is empty
  else if (mac_->getMap()->getUlSubframe()->getBw_req()->getRequest (con->get_cid())!=NULL) {
    debug2 ("At %f in Mac %d already pending requests for cid=%d\n", NOW, mac_->addr(), con->get_cid());
    return; //there is already a pending request
  }

  Packet *p= mac_->getPacket();
  hdr_cmn* ch = HDR_CMN(p);
  bw_req_header_t *header = (bw_req_header_t *)&(HDR_MAC802_16(p)->header);
  header->ht=1;
  header->ec=1;
  header->type = 1; //aggregate
  header->br = con->queueByteLength();
  header->cid = con->get_cid();

  double txtime = mac_->getPhy()->getTrxTime (ch->size(), mac_->getMap()->getUlSubframe()->getProfile (UIUC_REQ_REGION_FULL)->getEncoding());
  ch->txtime() = txtime;
  mac_->getMap()->getUlSubframe()->getBw_req()->addRequest (p, con->get_cid(), con->queueByteLength());
  debug2 ("SSscheduler enqueued request for cid=%d len=%d (nbPacket=%d)\n", con->get_cid(), con->queueByteLength(), con->queueLength());
}
