/*
 * Copyright (c) 2008 Regents of the SIGNET lab, University of Padova.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Padova (SIGNET lab) nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MRCL_ROUTINGQUEUE_
#define _MRCL_ROUTINGQUEUE_

#include <packet.h>
#include "mrcl-address.h"

/*
 * The maximum number of packets that we allow a routing protocol to buffer.
 */
#define RTR_RTQ_MAX_LEN     64      // packets

/*
 *  The maximum period of time that a routing protocol is allowed to buffer
 *  a packet for.
 */
#define RTR_RTQ_TIMEOUT     30	// seconds

#define DROP_RTR_QFULL_VERBOSITY 5
#define DROP_RTR_QFULL "QFULL"
#define DROP_RTR_QTIMEOUT "QTOUT"
#define DROP_RTR_QTIMEOUT_VERBOSITY 5
#define DROP_RTR_QAGED_VERBOSITY 5
#define DROP_RTR_QAGED "AGED"

struct PktQueue
{
	Packet *p;
	PktQueue *next;
	PktQueue *prev;
};

class MrclRouting;

class RoutingQueue : public TclObject {
 public:
        RoutingQueue(MrclRouting *m);

        virtual void enque(Packet *p);
        /*
         *  Returns a packet from the head of the queue.
         */
        virtual Packet* deque();

        /*
         * Returns a packet for destination "D".
         */
        virtual Packet* deque(MrclAddress *dst);
        virtual Packet* deque(char *dst);
	/*
	* Finds whether a packet with destination dst exists in the queue
	*/
        virtual char find(MrclAddress *dst);
        virtual char find(char *dst);

 private:
        virtual Packet* remove_head();
        virtual void purge();
	virtual void findPacketWithDst(char *dst, PktQueue *& pq);
	virtual int findAgedPacket(PktQueue *& pq); 
	virtual void verifyQueue();

        PktQueue head_;
        PktQueue tail_;

        int len_;

        int limit_;
        double timeout_;
	MrclRouting *module_;
};

#endif /* _MRCL_ROUTINGQUEUE_ */
