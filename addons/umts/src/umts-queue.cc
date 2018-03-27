/*
 * Copyright (c) 2003 Ericsson Telecommunicatie B.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the
 *     distribution.
 * 3. Neither the name of Ericsson Telecommunicatie B.V. may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY ERICSSON TELECOMMUNICATIE B.V. AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ERICSSON TELECOMMUNICATIE B.V., THE AUTHOR OR HIS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Contact for feedback on EURANE: eurane@ti-wmc.nl
 * EURANE = Enhanced UMTS Radio Access Network Extensions
 * website: http://www.ti-wmc.nl/eurane/
 */

/*
 * $Id: umts-queue.cc,v 1.22 2004/02/20 11:31:09 simon Exp $
 */

#include "umts-queue.h"
#include <assert.h>
#include<iostream>

umtsQueue::umtsQueue():TclObject()
{
   lastServedTime_ = 0.0;
   tx_seq_nr_ = 0;
   len_ = 0;
}

// Insert the packet in back of the queue
void umtsQueue::enque(Packet * p)
{
   if (q_.empty()) {
      lastServedTime_ = Scheduler::instance().clock();
   }
   q_.push_back(p);
   len_++;
}

// Insert the packet in front of the queue
void umtsQueue::enqueUniqueFront(Packet * p)
{
   int seqno = hdr_rlc::access(p)->seqno();

   //lastServedTime_ = Scheduler::instance().clock();
   for (unsigned int i = 0; i < q_.size(); i++) {
      int temp_seqno = hdr_rlc::access(q_.at(i))->seqno();

      if (temp_seqno == seqno) {
         return;
      }
   }
   q_.insert(q_.begin(), p);
   len_++;
}

// Insert the packet in front of the queue
void umtsQueue::printQueue()
{
   if (q_.size() > 0) {
      for (unsigned int i = 0; i < q_.size(); i++) {
         printf ("%d ", hdr_rlc::access(q_.at(i))->seqno() );
      }
      printf ("\n");
   }

}

// Insert the packet in sequence in the queue. If all the packets are inserted by
// this method, the queue is ordered.
void umtsQueue::orderedEnque(Packet * p)
{

   if (q_.empty()) {
      lastServedTime_ = Scheduler::instance().clock();

   }

   int seqno = hdr_rlc::access(p)->seqno();

   for (unsigned int i = 0; i < q_.size(); i++) {
      if ((hdr_rlc::access(q_.at(i)))->seqno() > seqno) {
         q_.insert(q_.begin() + i, p);
         len_++;
         return;
      }
   }
   q_.push_back(p);
   len_++;
}

// Return the first packet from the queue, and remove the packet from the queue
Packet     *umtsQueue::deque()
{
   lastServedTime_ = Scheduler::instance().clock();

   if (q_.empty()) {
      return NULL;
   }
   Packet     *temp = q_.front();

   q_.erase(q_.begin());
   len_--;
   return temp;
}

// Return the last packet from the queue, and remove the packet from the queue
Packet     *umtsQueue::dequeTail()
{
   if (q_.size() == 1) {
      lastServedTime_ = Scheduler::instance().clock();

   }
   Packet     *temp = q_.back();

   q_.pop_back();
   len_--;
   return temp;
}

// Return a pointer to a copy of the first packet from the queue
Packet     *umtsQueue::dequeCopy()
{
   if (q_.empty()) {
      return NULL;
   }
   return q_.front()->copy();
}

// Return a pointer to a copy of the last packet from the queue
Packet     *umtsQueue::dequeTailCopy()
{
   if (q_.empty()) {
      return NULL;
   }
   return q_.back()->copy();
}

// Return the packet with a certain seqno from the queue, and remove the packet
// from the queue
Packet     *umtsQueue::deque(int seqno)
{
   for (unsigned int i = 0; i < q_.size(); i++) {
      if ((hdr_rlc::access(q_.at(i)))->seqno() == seqno) {
         Packet     *temp = q_.at(i);

         q_.erase(q_.begin() + i);
         len_--;
         return temp;
      }
   }
   return NULL;
}

// Return a pointer to a copy of the packet with a certain seqno from the queue
Packet     *umtsQueue::dequeCopy(int seqno)
{
   for (unsigned int i = 0; i < q_.size(); i++) {
      if ((hdr_rlc::access(q_.at(i)))->seqno() == seqno) {
         return q_.at(i)->copy();
      }
   }
   return NULL;
}

// Returns the first packet with a seqno >= 0
Packet     *umtsQueue::dequeFirstSendable()
{

   unsigned int pos = 0;

   while (pos < q_.size() && ((hdr_rlc::access(q_.at(pos)))->seqno() < 0)) {
      pos++;
   }
   // pos is now the first packet with seqno >= 0, unless pos == q_.size

   Packet     *temp = NULL;

   if (pos < q_.size()) {
      temp = q_.at(pos);
      q_.erase(q_.begin() + pos);
      len_--;
   }

   return temp;
}

// Removes all packets with a seqno less to a certain seqno from the
// queue and free the packets belonging to the queued pointers.
void umtsQueue::dropTill(int seqno)
{
   for (unsigned int i = 0; i < q_.size(); /* do nothing */ ) {
      // Do not drop sequence numbers lower than 0, because these have not been
      // set yet.
      if (((hdr_rlc::access(q_.at(i)))->seqno() < seqno)
          && ((hdr_rlc::access(q_.at(i)))->seqno() >= 0)) {
         assert(q_.at(i) != NULL);
         Packet::free(q_.at(i));

         q_.erase(q_.begin() + i);
         len_--;
      } else {
         i++;
      }
   }
}

void umtsQueue::updateLastServedTime()
{
   lastServedTime_ = Scheduler::instance().clock();

}

// Returns the total size of all the packets in the queue, in bytes
int umtsQueue::size()
{
   int size = 0;

   for (unsigned int i = 0; i < q_.size(); i++) {
      size = size + hdr_cmn::access(q_.at(i))->size();
   }
   return size;
}

//returns packetsize of the first packet in the queue or at pos
// returns -1 when nothing in the queue
int umtsQueue::getPacketSize() {
   if (q_.empty()) {
      return -1;
   } else {
      return hdr_cmn::access(q_.front())->size();
   }
}

// Returns the size of the packet at a certain position in the queue, in bytes
int umtsQueue::size(int position)
{
   if (q_.size() < (unsigned) position) {
      return 0;
   }
   return hdr_cmn::access(q_.at(position - 1))->size();
}

// Returns the total size of all the packets in the queue, in bits
int umtsQueue::sizeInBits()
{
   return size() * 8;
}

// Returns the size of the packet at a certain position in the queue, in bits
int umtsQueue::sizeInBits(int position)
{
   return size(position) * 8;
}

// TODO: change code in am so this method can be deleted
// Reduces the size of the first packet with a certain amount of bytes. This is
// only done when the packet is large enough
int umtsQueue::red_size(int bytes)
{
   int size = (hdr_cmn::access(q_.at(0)))->size();

   if (q_.empty() || size <= bytes) {
      return -1;
   }
   (hdr_cmn::access(q_.at(0)))->size() = size - bytes;
   return size;
}

// Returns the number of packets in the queue
int umtsQueue::length()
{
   return q_.size();
}


// Dumps to stdout all sequence numbers in the queue:
void umtsQueue::dump()
{
  for (unsigned int i=0; i<q_.size(); i++)
    cout << " "<<  (hdr_rlc::access(q_.at(i)))->seqno() ;
  cout << endl;          
}

