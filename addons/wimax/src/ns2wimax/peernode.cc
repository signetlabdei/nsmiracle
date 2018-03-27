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

#include "peernode.h"

/**
 * Constructor
 * @param index The Mac address of the peer node
 */
PeerNode::PeerNode (int index): basic_in_(0), basic_out_(0),
				primary_in_(0), primary_out_(0),
				secondary_in_(0), secondary_out_(0), 
				indata_(0),
				outdata_(0), rxtime_ (0.0), rxp_watch_()
{
  peerIndex_ = index;
  going_down_ = false;
}
    
/**
 * Set the basic connection
 * @param i_con The connection used as basic for incoming
 * @param o_con The connection used as basic for outgoing
 */
void PeerNode::setBasic (Connection* i_con, Connection* o_con)
{
  assert (i_con != NULL && o_con !=NULL);

  basic_in_ = i_con;
  i_con->set_category (CONN_BASIC);
  i_con->setPeerNode (this);
  basic_out_ = o_con;
  o_con->set_category (CONN_BASIC);
  o_con->setPeerNode (this);
}

/**
 * Set the primary connection
 * @param i_con The connection used as primary for incoming
 * @param o_con The connection used as primary for outgoing
 */
void PeerNode::setPrimary (Connection* i_con, Connection* o_con)
{
  assert (i_con != NULL && o_con !=NULL);

  primary_in_ = i_con;
  i_con->set_category (CONN_PRIMARY);
  i_con->setPeerNode (this);
  primary_out_ = o_con;
  o_con->set_category (CONN_PRIMARY);
  o_con->setPeerNode (this);
}

/**
 * Set the secondary connection
 * @param i_con The connection used as secondary for incoming
 * @param o_con The connection used as secondary for outgoing
 */
void PeerNode::setSecondary (Connection* i_con, Connection* o_con)
{
  assert (i_con != NULL && o_con !=NULL);
  
  secondary_in_ = i_con;
  i_con->set_category (CONN_SECONDARY);
  i_con->setPeerNode (this);
  secondary_out_ = o_con;
  o_con->set_category (CONN_SECONDARY);
  o_con->setPeerNode (this);
}

/**
 * Set the incoming data connection
 * @param connection The connection
 */
void PeerNode::setInData (Connection* connection ) 
{
  assert (connection != NULL);
  
  indata_ = connection;
  connection->set_category (CONN_DATA);
  connection->setPeerNode (this);
}

/**
 * Set the outgoing data connection
 * @param connection The connection
 */
void PeerNode::setOutData (Connection* connection ) 
{
  assert (connection != NULL);
  
  outdata_ = connection;
  connection->set_category (CONN_DATA);
  connection->setPeerNode (this);
}

/**
 * Set the time the last packet was received
 * @param time The time the last packet was received
 */
void PeerNode::setRxTime (double time)
{
  assert (time >=0.0);
  rxtime_ = time;
}

/**
 * Get the time the last packet was received
 * @return The time the last packet was received
   */
double PeerNode::getRxTime ()
{
  return rxtime_;
}

/**
 * Return the stat watch
 * @return The stat watch
 */
StatWatch * PeerNode::getStatWatch()
{
  return &rxp_watch_;
}

/**
 * Return the requested bandwidth for this node
 * @return The requested bandwidth for this node
 */
int PeerNode::getReqBw ()
{
  //printf ("Node req bw\n");
  int bw=0;
  if (getBasic(IN_CONNECTION)!= NULL) {
    bw += getBasic(IN_CONNECTION)->getBw();
    //printf ("\tBasic %d %d\n", getBasic(IN_CONNECTION)->get_cid(), getBasic(IN_CONNECTION)->getBw());
  }
  if (getPrimary(IN_CONNECTION)!= NULL) {
    bw += getPrimary(IN_CONNECTION)->getBw();
    //printf ("\tPrimary %d %d\n", getPrimary(IN_CONNECTION)->get_cid(), getPrimary(IN_CONNECTION)->getBw());
  }
  if (getSecondary(IN_CONNECTION)!= NULL) {
    bw += getSecondary(IN_CONNECTION)->getBw();
    //printf ("\tSecondary %d %d\n", getSecondary(IN_CONNECTION)->get_cid(), getSecondary(IN_CONNECTION)->getBw());
  }
  if (getInData()!=NULL) {
    bw += getInData()->getBw();
    //printf ("\tData %d %d\n", getOutData()->get_cid(), getInData()->getBw());
  }
  return bw;
}

/**
 * Return the amount of data queued for this node
 * @return The queued data for this node
 */
int PeerNode::getQueueLength ()
{
  int bw=0;
  if (getBasic(OUT_CONNECTION)!= NULL)
    bw += getBasic(OUT_CONNECTION)->queueByteLength();
  if (getPrimary(OUT_CONNECTION)!= NULL)
    bw += getPrimary(OUT_CONNECTION)->queueByteLength();
  if (getSecondary(OUT_CONNECTION)!= NULL)
    bw += getSecondary(OUT_CONNECTION)->queueByteLength();
  if (getOutData()!=NULL)
    bw += getOutData()->queueByteLength();
  return bw;
}

/**
 * Set the requested downlink profile
 * @param diuc The downlink profile
 */
void PeerNode::setDIUC (int diuc)
{
  diuc_ = diuc;
}

/**
 * Get the requested downlink profile
 * @return The downlink profile
   */
int PeerNode::getDIUC ()
{
  return diuc_;
}
