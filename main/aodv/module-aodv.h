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


#ifndef _MRCL_AODV_
#define _MRCL_AODV_

#include <routing-module.h>
#include "aodv-rt-table.h"
#include <hop-count-metric.h>
#include <priqueue.h>
//#include "aodv-metrics.h"

#define BCAST_ID_SAVE 6

#define AODV_GOT_MY_REQUEST_VERBOSITY 10
#define AODV_GOT_MY_REQUEST "MyREQ"
#define AODV_DISCARD_REQUEST_VERBOSITY 10
#define AODV_DISCARD_REQUEST "REQ"
#define AODV_RECEIVED_REQUEST_VERBOSITY 10
#define AODV_RECEIVED_REQUEST "RREQ"
#define AODV_RECEIVED_REQUEST_RESOLVED "RQRV"
#define AODV_DROP_RTR_TTL_VERBOSITY 10
#define AODV_DROP_RTR_TTL "TTL"
#define AODV_REPLY_FOR_ME_VERBOSITY 20
#define AODV_REPLY_FOR_ME "MyREP"
#define AODV_SUPPRESS_REPLY_VERBOSITY 10
#define AODV_SUPPRESS_REPLY "SREP"
#define AODV_DROP_RTR_NO_ROUTE_VERBOSITY 10
#define DROP_RTR_NO_ROUTE "NRTE"
#define AODV_RECV_ERROR_VERBOSITY 20
#define AODV_RECV_ERROR "RERR"
#define AODV_RECV_HELLO_VERBOSITY 20
#define AODV_RECV_HELLO "RHELLO"
#define AODV_DROP_RTR_NO_ROUTE "NRTE"  // no route
#define AODV_DROP_RTR_NO_ROUTE_VEROSITY 10
#define DROP_RTR_MAC_CALLBACK           "CBK"   // MAC callback (from cmu-trace.h, ns2 standard distribution)
#define DROP_RTR_MAC_CALLBACK_VERBOSITY 10

#define REV_ROUTE_LIFE          6				// 5  seconds
#define ARP_DELAY 0.01      // fixed delay to keep arp happy
#define HELLO_INTERVAL          1               // 1000 ms
#define ALLOWED_HELLO_LOSS      3               // packets
#define ACTIVE_ROUTE_TIMEOUT    10				// 50 seconds
#define MY_ROUTE_TIMEOUT        10                      	// 100 seconds


// Should be set by the user using best guess (conservative) 
#define NETWORK_DIAMETER        30             // 30 hops

#define RREQ_RETRIES            3  
// timeout after doing network-wide search RREQ_RETRIES times
#define MAX_RREQ_TIMEOUT	10.0 //sec


/* Various constants used for the expanding ring search */
#define TTL_START     5
#define TTL_THRESHOLD 7
#define TTL_INCREMENT 2 

// This should be somewhat related to arp timeout
#define NODE_TRAVERSAL_TIME     0.03             // 30 ms
//packet_t PT_PDATA;
class AodvModule;

/*
  Timers (Broadcast ID, Hello, Neighbor Cache, Route Cache)
*/
class MrclBroadcastTimer : public Handler {
public:
        MrclBroadcastTimer(AodvModule* a) : module(a) {}
        void	handle(Event* p);
private:
        AodvModule    *module;
	Event	intr;
};

class MrclHelloTimer : public Handler {
public:
        MrclHelloTimer(AodvModule* a) : module(a) {}
        void	handle(Event* p);
private:
        AodvModule    *module;
	Event	intr;
};

class MrclNeighborTimer : public Handler {
public:
        MrclNeighborTimer(AodvModule* a) : module(a) {}
        void	handle(Event* p);
private:
        AodvModule    *module;
	Event	intr;
};

class MrclRouteCacheTimer : public Handler {
public:
        MrclRouteCacheTimer(AodvModule* a) : module(a) {}
        void	handle(Event* p);
private:
        AodvModule    *module;
	Event	intr;
};

class MrclLocalRepairTimer : public Handler {
public:
        MrclLocalRepairTimer(AodvModule* a) : module(a) {}
        void	handle(Event* p);
private:
        AodvModule    *module;
	Event	intr;
};



class AodvModule : public MrclRouting
{
	friend class MrclBroadcastTimer;
	friend class MrclHelloTimer;
	friend class MrclNeighborTimer;
	friend class MrclRouteCacheTimer;
	friend class MrclLocalRepairTimer;

public:
	AodvModule();
	virtual ~AodvModule();
	virtual int command(int argc, const char *const *argv);
	
	void rt_ll_failed(Packet *p);
	void handle_link_failure(char* id);
	void local_rt_repair(Aodv_rt_entry *rt, Packet *p);
	

protected:
	virtual int controlPacket(Packet *p);
	virtual void forward(Packet *p);
	virtual void resolve(Packet *p);
	virtual void forward(Packet *p, Aodv_rt_entry *re, double delay = 0.0);

	virtual char *getNextHop(Packet *p);
	virtual int canIReach(char *a, Metric ***m);
	
	virtual void recvAodv(Packet *p);
	virtual void recvRequest(Packet *p);
	virtual void recvReply(Packet *p);
	virtual void recvError(Packet *p);
	virtual void recvHello(Packet *p);
	
	virtual void sendHello();
	
// 	virtual void enque(Aodv_rt_entry *rt, Packet *p);
// 	virtual Packet* deque(Aodv_rt_entry *rt);

	virtual void sendReply(char *dst, int hop_count, char *rp_dst, int rpseq, int lifetime, double timestamp);
	virtual void sendRequest(char *dst);
	virtual void sendError(Packet *p, int jitter = 1);
	int id_lookup(char *addr, int bid);
	void id_insert(char *addr, int bid);
// 	void id_delete(char *id);
	void id_delete(AddrList *a);
	void id_purge(void);
	void rt_update(Aodv_rt_entry *rt, int seqnum, int metric, char *nexthop, double expire_time);
	void rt_down(Aodv_rt_entry *rt);
	void rt_purge();
	Aodv_rt_entry* rtable_lookup(char *addr) {return (rtable_.rt_lookup(addr));};
// 	void rt_resolve(Packet *p);

	void nb_insert(char *id);
	AddrList *nb_lookup(char *id);
	void nb_delete(char *id);
	void nb_delete(AddrList *a);
	void nb_purge();
 
	double perHopTime(Aodv_rt_entry *rt);
	
	double minHelloInterval() {return (helloInterval_ * minHelloIntervalRatio_);};
	double maxHelloInterval() {return (helloInterval_ * maxHelloIntervalRatio_);};
	double routeCacheFrequency() {return (routeCacheFrequency_);};
	
	int debug(){return (debug_);};

	HopCountMetric **m_;
	AddrList *ncache_;		// neighbour cache
	AddrList *bcache_;		// broadcast ID cache
	RoutingQueue rqueue_;
	Aodv_rtable rtable_;
	PriQueue* ifqueue_;	// pointer to radio interface queue (used to delete packet just 
	// resolved destinated(or which exploits) to link failed) 
	MrclAddress *broadcastAddr_;
	int seqno_;
	int RREQ_GRAT_RREP;
	int bid_;

	int AODV_LINK_LAYER_DETECTION;
	// TRUE:		allows AODV to use link-layer (802.11) feedback in determining when links are up/down.
	// FALSE: 	when standard AODV link layer detection has to be used (HELLO packets)

	int AODV_LOCAL_REPAIR;			// when TRUE allows local repair of routes (default = TRUE)

	int AODV_MIRACLE_ROUTING;			// when TRUE 
												// AODV can fully exploit MrclRouting funtionalities
												// and therefore relay packet to other routing module within
												// the same MIRACLE routing framework in case of they can
												// solve the packet, the route request is estended to all
												// the routign modules.
												// 
												// when FALSE
												// AODV works as in standard ns2, this meas that it do not propagate
												// route request to (possible) others routing module within the 
												// same MIRACLE routing framework and therefore realaying across
												// different interface is avoided.
	// Timers
	MrclBroadcastTimer  btimer;
	MrclHelloTimer      htimer;
	MrclNeighborTimer   ntimer;
	MrclRouteCacheTimer rtimer;
	MrclLocalRepairTimer lrtimer;

	// Timers Variable
	double minHelloIntervalRatio_;
	double maxHelloIntervalRatio_;
	double helloInterval_;
	double routeCacheFrequency_;
	
	int debugout;
};


#endif

