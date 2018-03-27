/*
Copyright (c) 1997, 1998 Carnegie Mellon University.  All Rights
Reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The AODV code developed by the CMU/MONARCH group was optimized and tuned by Samir Das and Mahesh Marina, University of Cincinnati. The work was partially done in Sun Microsystems.
*/

/**
 * Ported to NS-Miracle by Marco Miozzo, 2008
 * 
 */



#include "aodv-rt-table.h"

/*---------------------------------------
 *					|
 * 	Aodv_rt_entry			|
 *					|
 ---------------------------------------*/

Aodv_rt_entry::Aodv_rt_entry()
{
	rt_req_timeout = 0.0;
	rt_req_cnt = 0;

	memset(rt_dst_, 0, MRCL_ADDRESS_MAX_LEN);
	rt_seqno_ = 0;
	rt_hops_ = rt_last_hop_count_ = INFINITY2;
	memset(nexthop_, 0, MRCL_ADDRESS_MAX_LEN);
	rt_pclist_ = 0;
	expire_ = 0.0;
	rt_flags_ = RTF_DOWN;

	for (int i=0; i < MAX_HISTORY; i++) {
		rt_disc_latency_[i] = 0.0;
	}
	hist_indx_ = 0;
	rt_req_last_ttl_ = 0;

	rt_nblist_ = 0;;
	next_ = 0;
	prev_ = 0;
}

Aodv_rt_entry::~Aodv_rt_entry()
{
	AddrList *cur = rt_nblist_;
	while(cur)
	{
		AddrList *tmp = cur;
		cur = cur->next;
		delete tmp;
	}
	cur = rt_pclist_;
	while(cur)
	{
		AddrList *tmp = cur;
		cur = cur->next;
		delete tmp;
	}
}

void Aodv_rt_entry::nb_insert(char *addr)
{
	AddrList *a = new AddrList;
	MrclAddress::storeAddr(a->addr, addr);
	a->expire = 0;
	a->next = rt_nblist_;
	a->prev = 0;
	if(rt_nblist_)
		rt_nblist_->prev = a;
	rt_nblist_ = a;
}

AddrList *Aodv_rt_entry::nb_lookup(char *addr)
{
	for(AddrList *cur = rt_nblist_; cur; cur = cur->next)
	{
		if(MrclAddress::areEqual(addr, cur->addr))
			return cur;
	}
	return 0;
}

void Aodv_rt_entry::pc_insert(char *addr)
{
	AddrList *a = new AddrList;
	MrclAddress::storeAddr(a->addr, addr);
	a->expire = 0;
	a->next = rt_pclist_;
	a->prev = 0;
	if(rt_pclist_)
		rt_pclist_->prev = a;
	rt_pclist_ = a;
	char add[20];
	int ip;
	memcpy(&ip, a->addr + sizeof(int), sizeof(int));
	sprintf(add,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
}

AddrList *Aodv_rt_entry::pc_lookup(char *addr)
{
	for(AddrList *cur = rt_pclist_; cur; cur = cur->next)
	{
		if(MrclAddress::areEqual(addr, cur->addr))
			return cur;
	}
	return 0;
}

void Aodv_rt_entry::pc_delete(char *addr)
{
	AddrList *a = pc_lookup(addr);
	if(!a)
		return;
	if(a == rt_pclist_)
	{
		rt_pclist_ = a->next;
		if(rt_pclist_)
			rt_pclist_->prev = 0;
	}
	else
	{
		a->prev->next = a->next;
		if(a->next)
			a->next->prev = a->prev;
	}
	delete a;
}

void Aodv_rt_entry::pc_show()
{
	AddrList *cur = rt_pclist_;
	char addr[20];
	int ip;
	while(cur)
	{
		memcpy(&ip, cur->addr + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("\t%s id %d will expire at %f next %p prev %p this %p\n", addr, cur->id, cur->expire, cur->next, cur->prev, cur);
		cur = cur->next;
	}
}


void Aodv_rt_entry::pc_delete()
{
	AddrList *cur = rt_pclist_;
	while(cur)
	{
		AddrList *tmp = cur;
		cur = cur->next;
		delete tmp;
	}
	rt_pclist_ = 0;
}

int Aodv_rt_entry::pc_empty()
{
	return (rt_pclist_ == 0);
}

Aodv_rt_entry *Aodv_rt_entry::getNext()
{
	return next_;
}

Aodv_rt_entry *Aodv_rt_entry::getPrev()
{
	return prev_;
}

void Aodv_rt_entry::setNext(Aodv_rt_entry *e)
{
	next_ = e;
}

void Aodv_rt_entry::setPrev(Aodv_rt_entry *e)
{
	prev_ = e;
}

void Aodv_rt_entry::setDst(char *addr)
{
	MrclAddress::storeAddr(rt_dst_, addr);
}

char *Aodv_rt_entry::getDst()
{
	return rt_dst_;
}

void Aodv_rt_entry::setSeqno(int sn)
{
	rt_seqno_ = sn;
}

void Aodv_rt_entry::incrSeqno()
{
	rt_seqno_++;
}


int Aodv_rt_entry::getSeqno()
{
	return rt_seqno_;
}

void Aodv_rt_entry::setHops(int h)
{
	rt_hops_ = h;
}

int Aodv_rt_entry::getHops()
{
	return rt_hops_;
}

void Aodv_rt_entry::setLastHopCount(int lh)
{
	rt_last_hop_count_ = lh;
}

int Aodv_rt_entry::getLastHopCount()
{
	return rt_last_hop_count_;
}

void Aodv_rt_entry::setNexthop(char *addr)
{
	MrclAddress::storeAddr(nexthop_, addr);
}

void Aodv_rt_entry::resetNexthop()
{
	memset(nexthop_, 0, MRCL_ADDRESS_MAX_LEN);
}

char *Aodv_rt_entry::getNexthop()
{
	return nexthop_;
}

void Aodv_rt_entry::setExpire(double e)
{
	expire_ = e;
}

double Aodv_rt_entry::getExpire()
{
	return expire_;
}

void Aodv_rt_entry::setFlags(char f)
{
	rt_flags_ = f;
}

char Aodv_rt_entry::getFlags()
{
	return rt_flags_;
}

void Aodv_rt_entry::setReqLastTtl(int lt)
{
	rt_req_last_ttl_ = lt;
}

int Aodv_rt_entry::getReqLastTtl()
{
	return rt_req_last_ttl_;
}

void Aodv_rt_entry::setDiscLatency(int i, double val)
{
	if(i < 0 || i >= MAX_HISTORY)
		return;
	rt_disc_latency_[i] = val;
}

double Aodv_rt_entry::getDiscLatency(int i)
{
	if(i < 0 || i >= MAX_HISTORY)
		return 0.0;
	return rt_disc_latency_[i];
}

void Aodv_rt_entry::setHistIndx(char val)
{
	hist_indx_ = val;
}

char Aodv_rt_entry::getHistIndx()
{
	return hist_indx_;
}


/*---------------------------------------
 *					|
 * 	Aodv_rtable			|
 *					|
 ---------------------------------------*/

Aodv_rtable::Aodv_rtable() : rt_head_(0)
{
}

Aodv_rt_entry *Aodv_rtable::head()
{
	return rt_head_;
}

Aodv_rt_entry *Aodv_rtable::rt_add(char *addr)
{
	if(rt_lookup(addr) != 0)
	{
		fprintf(stderr, "Aodv_rtable::rt_add error\n");
		exit(1);
	}
	Aodv_rt_entry *e = new Aodv_rt_entry;
	e->setDst(addr);
	e->setNext(rt_head_);
	if(rt_head_)
		rt_head_->setPrev(e);
	rt_head_ = e;
	return(e);
}

void Aodv_rtable::rt_delete(char *addr)
{
	Aodv_rt_entry *e = rt_lookup(addr);
	if(!e)
		return;
	if(e == rt_head_)
	{
		rt_head_ = e->getNext();
		if(rt_head_)
			rt_head_->setPrev(0);
	}
	else
	{
		e->getPrev()->setNext(e->getNext());
		if(e->getNext())
			e->getNext()->setPrev(e->getPrev());
	}
}

Aodv_rt_entry *Aodv_rtable::rt_lookup(char *addr)
{
	for(Aodv_rt_entry *cur = rt_head_; cur; cur = cur->getNext())
	{
		if(MrclAddress::areEqual(addr, cur->getDst()))
			return cur;
	}
	return 0;
}

Aodv_rt_entry *Aodv_rtable::next(Aodv_rt_entry *addr)
{
	for(Aodv_rt_entry *cur = rt_head_; cur; cur = cur->getNext())
	{
		if(cur==addr)
			return (cur->getNext());
	}
	return 0;
}

