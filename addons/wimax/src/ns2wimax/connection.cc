/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This software is based on the NIST NS2 WiMAX module, which can be found at
 * http://www.antd.nist.gov/seamlessandsecure/doc.html
 *
 * $Id: connection.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "connection.h"
#include "connectionmanager.h"
#include "mac802_16.h"

static int basicIndex = BASIC_CID_START;
static int primaryIndex = PRIMARY_CID_START;
static int transportIndex = TRANSPORT_SEC_CID_START;
static int multicastIndex = MULTICAST_CID_START;

/**
 * Constructor used by BS to automatically assign CIDs
 * @param type The connection type. 
 */
Connection::Connection (ConnectionType_t type) : peer_(0), 
						 frag_status_(FRAG_NOFRAG), 
						 frag_nb_(0), 
						 frag_byte_proc_(0),
						 frag_enable_(true),
						 requested_bw_(0)
{
   switch (type) {
   case CONN_INIT_RANGING:
     cid_ = INITIAL_RANGING_CID;
     break;
   case CONN_AAS_INIT_RANGING:
     cid_ = AAS_INIT_RANGIN_CID;
     break;
   case CONN_PADDING:
     cid_ = PADDING_CID;
     break;
   case CONN_BROADCAST:
     cid_ = BROADCAST_CID;
     break;
   case CONN_MULTICAST_POLLING:
     cid_ = multicastIndex++;
     assert (multicastIndex <= MULTICAST_CID_STOP);
     break;
   case CONN_BASIC:
     cid_ = basicIndex++;
     assert (basicIndex <= BASIC_CID_STOP);
     break;
   case CONN_PRIMARY:
     cid_ = primaryIndex++;
     assert (primaryIndex <= PRIMARY_CID_STOP);
       break;
   case CONN_SECONDARY:
   case CONN_DATA:
     cid_ = transportIndex++;
     assert (transportIndex <= TRANSPORT_SEC_CID_STOP);
     break;
   default:
     fprintf (stderr, "Unsupported connection type\n");
     exit (1);
   }
   type_ = type;
   queue_ = new PacketQueue();

}

/**
 * Constructor used by SSs when the CID is already known
 * @param type The connection type
 * @param cid The connection cid
 */
Connection::Connection (ConnectionType_t type, int cid) : peer_(0), 
							  frag_status_(FRAG_NOFRAG), 
							  frag_nb_(0), 
							  frag_byte_proc_(0),
							  frag_enable_(true),
							  requested_bw_(0)
{
  cid_ = cid;
  type_ = type;
  queue_ = new PacketQueue();
}

/**
 * Destructor
 */
Connection::~Connection ()
{
  flush_queue ();
}

/**
 * Set the connection manager
 * @param manager The Connection manager 
 */
void Connection::setManager (ConnectionManager *manager)
{
  manager_ = manager;
}

/**
 * Enqueue the given packet
 * @param p The packet to enqueue
 */
void Connection::enqueue (Packet * p) 
{
  //Mark the timestamp for queueing delay
  HDR_CMN(p)->timestamp() = NOW;
  queue_->enque (p);
}

/**
 * Dequeue a packet from the queue
 * @param p The packet to enqueue
 */
Packet * Connection::dequeue () 
{
  Packet *p = queue_->deque ();
  return p;
}

/**
 * Flush the queue and return the number of packets freed
 * @return The number of packets flushed
 */
int Connection::flush_queue()
{
  int i=0;
  Packet *p;
  while ( (p=queue_->deque()) ) {
    manager_->getMac()->drop(p, 1, "CON");
    i++;
  }
  return i;
}

/**
 * Return queue size in bytes
 */
int Connection::queueByteLength () 
{
  return queue_->byteLength ();
}

/**
 * Return queue size in bytes
 */
int Connection::queueLength () 
{
  return queue_->length ();
}

/** 
 * Update the fragmentation information
 * @param status The new fragmentation status
 * @param index The new fragmentation index
 * @param bytes The number of bytes 
 */
void Connection::updateFragmentation (fragment_status status, int index, int bytes)
{
  frag_status_ = status;
  frag_nb_ = index;
  frag_byte_proc_ = bytes;
}

/**
 * Set the bandwidth requested
 * @param bw The bandwidth requested in bytes
 */
void Connection::setBw (int bw)
{
  //in some cases, the SS may send more data than allocated.
  //assert (bw >=0);
  requested_bw_ = bw<0?0:bw;
  //printf ("Set bw %d for connection %d(%x)\n", requested_bw_,cid_, this);
}

/**
 * Set the bandwidth requested
 * @param bw The bandwidth requested in bytes
 */
int Connection::getBw ()
{
  //printf ("Get %d bw for connection %d(%x)\n", requested_bw_,cid_, this);
  return requested_bw_;
}


