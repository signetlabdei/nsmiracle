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
 * $Id: umts-queue.h,v 1.11 2004/02/06 16:34:02 simon Exp $
 */

#ifndef ns_umtsQueue_h
#define ns_umtsQueue_h


#include "packet.h"
#include "umts-headers.h"
#include <vector>


class umtsQueue:public TclObject {
public:
   umtsQueue();

   void        enque(Packet * p);
   void        enqueUniqueFront(Packet * p);
   void        orderedEnque(Packet * p);
   Packet     *deque();
   Packet     *dequeTail();
   Packet     *dequeCopy();
   Packet     *dequeTailCopy();
   Packet     *deque(int seqno);
   Packet     *dequeCopy(int seqno);
   Packet     *dequeFirstSendable();
   void        updateLastServedTime();
   int         getPacketSize();
   void        dropTill(int seqno);
   void        printQueue();
   int         size();
   int         size(int position);
   int         sizeInBits();
   int         sizeInBits(int position);
   int         length();
   int         red_size(int bytes);
   void        dump();

   // lastServedTime_ is set to the current simulation time, when the first
   // element is changed. This enables round-robin schedulers to determine
   // which queue to handle.
   double      lastServedTime_;

   // for debugging purposes only...
   int         len_;

   // TODO: change hsdpalink in such a way that this variable isn't needed here
   // anymore.
   int         tx_seq_nr_;      // last TSN

private:
               vector < Packet * >q_;

};

#endif
