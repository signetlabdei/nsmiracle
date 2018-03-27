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
 * Ported to NS-Miracle by Marco Miozzo, 2007-2008
 * 
 */


#ifndef _AODV_RT_TABLE_
#define _AODV_RT_TABLE_

#include <mrcl-address.h>

struct AddrList
{
	char addr[MRCL_ADDRESS_MAX_LEN];
	int id;
	double expire;
	AddrList *next;
	AddrList *prev;
};

#define INFINITY2 0x7fffffff //2147483647
//#define INFINITY2 0xffffffff //-1

#define RTF_DOWN 0
#define RTF_UP 1
#define RTF_IN_REPAIR 2

#define MAX_HISTORY	3
	
class Aodv_rt_entry
{
public:
	Aodv_rt_entry();
	~Aodv_rt_entry();

	void nb_insert(char *addr);
	AddrList *nb_lookup(char *addr);

	void pc_insert(char *addr);
	AddrList *pc_lookup(char *addr);
	void pc_delete(char *addr);
	void pc_delete();
	void pc_show();
	int pc_empty();	
	Aodv_rt_entry *getNext();
	Aodv_rt_entry *getPrev();
	void setNext(Aodv_rt_entry *e);
	void setPrev(Aodv_rt_entry *e);
	void setDst(char *addr);
	char *getDst();

	void setSeqno(int sn);
	void incrSeqno();
	int getSeqno();

	void setHops(int h);
	int getHops();

	void setLastHopCount(int lh);
	int getLastHopCount();

	void setNexthop(char *addr);
	void resetNexthop();
	char *getNexthop();

	void setExpire(double e);
	double getExpire();

	void setFlags(char f);
	char getFlags();

	void setReqLastTtl(int lt);
	int getReqLastTtl();

	void setDiscLatency(int i, double val);
	double getDiscLatency(int i);

	char getHistIndx();
	void setHistIndx(char val);

	double rt_req_timeout;
	char rt_req_cnt;
protected:
	Aodv_rt_entry  *next_;
	Aodv_rt_entry *prev_;

	char rt_dst_[MRCL_ADDRESS_MAX_LEN];
	int rt_seqno_;
	int rt_hops_;
	int rt_last_hop_count_;
	char nexthop_[MRCL_ADDRESS_MAX_LEN];
	AddrList *rt_pclist_;
	double expire_;
	char rt_flags_;
	double rt_disc_latency_[MAX_HISTORY];
	char hist_indx_;
	int rt_req_last_ttl_;
	AddrList *rt_nblist_;
};


class Aodv_rtable
{
public:
	Aodv_rtable();
	Aodv_rt_entry *head();
	Aodv_rt_entry *rt_add(char *addr);
	void rt_delete(char *addr);
	Aodv_rt_entry *rt_lookup(char *addr);
	Aodv_rt_entry *next(Aodv_rt_entry *addr);
protected:
	Aodv_rt_entry *rt_head_;
};

#endif

