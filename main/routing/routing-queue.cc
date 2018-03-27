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

#include <assert.h>

#include "routing-queue.h"
#include "routing-module.h"

#define CURRENT_TIME    Scheduler::instance().clock()
// #define QDEBUG

/*
  Packet Queue used by AODV.
*/

RoutingQueue::RoutingQueue(MrclRouting *m) : module_(m)
{
	head_.next = &tail_;
	tail_.prev = &head_;
	len_ = 0;
	limit_ = RTR_RTQ_MAX_LEN;
	timeout_ = RTR_RTQ_TIMEOUT;
}

void RoutingQueue::enque(Packet *p) 
{
	struct hdr_cmn *ch = HDR_CMN(p);
	/*
	* Purge any packets that have timed out.
	*/
	purge();
 
	ch->ts_ = CURRENT_TIME + timeout_;
	if (len_ == limit_) 
	{
		Packet *p0 = remove_head();	// decrements len_
		assert(p0);
		if(HDR_CMN(p0)->ts_ > CURRENT_TIME) 
		{
			module_->drop(p0, DROP_RTR_QFULL_VERBOSITY, DROP_RTR_QFULL);
		}
		else 
		{
			module_->drop(p0, DROP_RTR_QTIMEOUT_VERBOSITY, DROP_RTR_QTIMEOUT);
		}
	}
 	PktQueue *pq = new PktQueue;
	pq->p = p;
	pq->prev = tail_.prev;
	pq->next = &tail_;
	tail_.prev->next = pq;
	tail_.prev = pq;

	len_++;
#ifdef QDEBUG
	verifyQueue();
#endif // QDEBUG
}
                

Packet* RoutingQueue::deque() 
{
	/*
	* Purge any packets that have timed out.
	*/
	purge();
	Packet *p = remove_head();
#ifdef QDEBUG
	verifyQueue();
#endif // QDEBUG
	return p;
}


Packet *RoutingQueue::deque(char *dst) {
	PktQueue *pq;

	/*
	* Purge any packets that have timed out.
	*/
	purge();

	findPacketWithDst(dst, pq);

	if(pq == 0)
		return 0;

	Packet *p = pq->p;
	pq->prev->next = pq->next;
	pq->next->prev = pq->prev;
	delete pq;
	len_--;
#ifdef QDEBUG
	verifyQueue();
#endif // QDEBUG
	return p;

}

Packet *RoutingQueue::deque(MrclAddress *dst) 
{
	return deque(dst->getAddr());
}

char RoutingQueue::find(char *dst) 
{
	PktQueue *pq;  
	
	findPacketWithDst(dst, pq);
	if(pq)
		return 1;
	else
		return 0;
}

char RoutingQueue::find(MrclAddress *dst) 
{
	return find(dst->getAddr());
}
	
	

/*
  Private Routines
*/

Packet* RoutingQueue::remove_head() 
{
	if(len_ == 0)
		return 0;
	
	
	PktQueue *pq = head_.next;
	Packet *p = pq->p;
	pq->prev->next = pq->next;
	pq->next->prev = pq->prev;
	len_--;

	return p;
}

void RoutingQueue::findPacketWithDst(char *dst, PktQueue*& pq) 
{
	for(pq = head_.next; pq != &tail_; pq = pq->next) 
	{
		RoutingHdr *rhdr = HDR_ROUTING(pq->p);
		if(MrclAddress::areEqual(rhdr->daddr(), dst)) 
		{
			return;
		}
	}
	pq = 0;
}


void RoutingQueue::verifyQueue() 
{
	PktQueue *pq;
	int cnt = 0;

	for(pq = head_.next; pq != &tail_; pq = pq->next) 
	{
		cnt++;
	}
	assert(cnt == len_);
}


/*
void
AodvQueue::purge() {
Packet *p;

 while((p = head_) && HDR_CMN(p)->ts_ < CURRENT_TIME) {
   // assert(p == remove_head());     
   p = remove_head();     
   drop(p, DROP_RTR_QTIMEOUT);
 }

}
*/

int RoutingQueue::findAgedPacket(PktQueue *& pq) {
  
	for(pq = head_.next; pq != &tail_; pq = pq->next) 
	{
		if(HDR_CMN(pq->p)->ts_ < CURRENT_TIME) 
		{
			return 1;
		}
	}
	return 0;
}

void RoutingQueue::purge() 
{
	PktQueue *pq;

	while (findAgedPacket(pq)) 
	{
 		if(pq->p == 0) 
			return;
		
		pq->prev->next = pq->next;
		pq->next->prev = pq->prev;
		len_--;
		module_->drop(pq->p, DROP_RTR_QAGED_VERBOSITY, DROP_RTR_QAGED); 
#ifdef QDEBUG
		verifyQueue();
#endif // QDEBUG
	}
}

