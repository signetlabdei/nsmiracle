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

#include "connectionmanager.h"
#include "mac802_16.h"

/**
 * Create the manager for the given mac
 * @param mac The Mac where the manager is located
 */
ConnectionManager::ConnectionManager (Mac802_16 * mac) 
{
  assert (mac!=NULL);
  mac_ = mac;

  //init list
  LIST_INIT (&i_con_list_);
  LIST_INIT (&o_con_list_);
}


/**
 * Add a connection to the list
 * @param con The connection to add
 * @param out true if it is an outgoing connection
 */
void ConnectionManager::add_connection (Connection* con, bool out) {
  assert (con!=NULL);
  assert (!get_connection (con->get_cid(), out)); //check duplicate
  mac_->debug ("At %f in %d adding %s connection %d\n", \
	       NOW, mac_->addr(), out?"out":"in", con->get_cid());
  if (out)
    con->insert_entry (&o_con_list_);
  else
    con->insert_entry (&i_con_list_);

  con->setManager(this);
}

/**
 * Remove a connection
 * @param The connection to remove
 */
void ConnectionManager::remove_connection (Connection* con) {
  assert (con !=NULL);
  mac_->debug ("At %f in %d removing connection %d\n", \
	       NOW, mac_->addr(), con->get_cid());
  con->remove_entry ();
}

/**
 * Remove connection by CID, both directions.
 * @param cid The connection id
 */
void ConnectionManager::remove_connection (int cid)
{
  Connection *con = get_connection (cid, IN_CONNECTION);
  if (con)
    remove_connection (con);
  con = get_connection (cid, OUT_CONNECTION);
  if (con)
    remove_connection (con);
}
  

/**
 * Return the connection that has the given CID
 * @param cid The connection ID
 * @param out The direction
 * @return the connection or NULL
 */
Connection* ConnectionManager::get_connection (int cid, bool out) {
  //search throught the list
  for (Connection *n=out?o_con_list_.lh_first:i_con_list_.lh_first; 
       n; n=n->next_entry()) {
    if (n->get_cid ()==cid)
      return n;
  }
  return NULL;
}

/**
 * Flush the queues. This can be called after switching BS.
 */
void ConnectionManager::flush_queues () {
  mac_->debug ("At %f in %d Flushing queues\n", NOW, mac_->addr());
  for (Connection *n=o_con_list_.lh_first; n; n=n->next_entry()) {
    int i = n->flush_queue();
    mac_->debug ("\tFreed %d packet in queue for connection %d\n", i, n->get_cid());
  }
}

