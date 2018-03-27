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

The AODV code developed by the CMU/MONARCH group was optimized and tuned by Samir Das and Mahesh Marina, University of Cincinnati. The work was partially done in Sun Microsystems. Modified for gratuitous replies by Anant Utgikar, 09/16/02.

*/

/**
 * Ported to NS-Miracle by Marco Miozzo, 2007-2008
 * 
 */


#include <random.h>
#include<float.h>
#include<node-core.h>
#include "aodv-pkt.h"
#include "module-aodv.h"


#define CURRENT_TIME (Scheduler::instance().clock())
#define max(a,b)        ( (a) > (b) ? (a) : (b) )

//#define DEBUG 99

extern packet_t PT_MCBR;

packet_t PT_MRCL_AODV;

static class AodvModuleClass : public TclClass {
public:
	AodvModuleClass() : TclClass("Routing/AODV") {}
	TclObject* create(int, const char*const*) {
		return (new AodvModule());
	}
} class_aodv_module;


/*------------------------------------------------------------
				TIMERS
--------------------------------------------------------------*/

void MrclBroadcastTimer::handle(Event* p)
{
	module->id_purge();
	Scheduler::instance().schedule(this, &intr, BCAST_ID_SAVE);
}

void MrclHelloTimer::handle(Event* p)
{
	module->sendHello();
	double interval = module->minHelloInterval() + 
				((module->maxHelloInterval() - module->minHelloInterval()) * Random::uniform());
	assert(interval >= 0);
	Scheduler::instance().schedule(this, &intr, interval);
}


void MrclNeighborTimer::handle(Event* p)
{
	module->nb_purge();
	Scheduler::instance().schedule(this, &intr, HELLO_INTERVAL);
}

void MrclRouteCacheTimer::handle(Event* p)
{
	module->rt_purge();
	Scheduler::instance().schedule(this, &intr, module->routeCacheFrequency());
}


void MrclLocalRepairTimer::handle(Event* p)
{
	// SRD: 5/4/99
	RoutingHdr *rhdr = HDR_ROUTING((Packet *)p);
	Aodv_rt_entry *rt = module->rtable_lookup(rhdr->daddr());

	/* you get here after the timeout in a local repair attempt */
	/*	fprintf(stderr, "%s\n", __FUNCTION__); */

	if (rt && rt->getFlags() != RTF_UP) {
		// route is yet to be repaired
		// I will be conservative and bring down the route
		// and send route errors upstream.
		/* The following assert fails, not sure why */
		/* assert (rt->rt_flags == RTF_IN_REPAIR); */
		
		//rt->rt_seqno++;
		module->rt_down(rt);
		// send RERR
// 		if (module->debug())
// 			fprintf(stderr, "Node %d: Dst - %s, failed local repair\n", index , rt->getDst() );
	
	}
	Packet::free((Packet *)p);
}

AodvModule::AodvModule() : 
	bcache_(0),
	ncache_(0),
	rqueue_(this),
	bid_(1),
	btimer(this),
	htimer(this),
	ntimer(this),
	rtimer(this),
	lrtimer(this),
	AODV_LINK_LAYER_DETECTION(TRUE),		// standardly disabled in ns2 (actived thanks to "AodvLinkLayerDetection" Tcl command)
	AODV_LOCAL_REPAIR(TRUE),
	AODV_MIRACLE_ROUTING(TRUE),
	ifqueue_(0),
	debugout(1),
	broadcastAddr_(0)
{
	seqno_ = 2;
	bind_bool("RREQ_GRAT_RREP", &RREQ_GRAT_RREP);
	bind("minHelloIntervalRatio_",&minHelloIntervalRatio_);
	bind("maxHelloIntervalRatio_",&maxHelloIntervalRatio_);
	bind("helloInterval_",&helloInterval_);
	bind("routeCacheFrequency_",&routeCacheFrequency_);
	m_ = new HopCountMetric*;
	m_[0] = new HopCountMetric();
}

AodvModule::~AodvModule()
{
	delete m_[0];
	delete m_;
}

int AodvModule::command(int argc, const char *const *argv)
{
// 	Tcl& tcl = Tcl::instance();
	if (argc==2)
	{
		if(strcmp(argv[1], "start") == 0)
		{
			btimer.handle((Event*) 0);
			if (!AODV_LINK_LAYER_DETECTION)
			{
				htimer.handle((Event*) 0);
				ntimer.handle((Event*) 0);
			}
			rtimer.handle((Event*) 0);
			return TCL_OK;
		}
		else if(strcmp(argv[1], "AodvLinkLayerDetection") == 0)
		{
			AODV_LINK_LAYER_DETECTION = FALSE;
			return TCL_OK;
		}
		else if(strcmp(argv[1], "AodvMiracleRouting") == 0)
		{
			AODV_MIRACLE_ROUTING = TRUE;
			return TCL_OK;
		}
		else if(strcmp(argv[1], "AodvStandardNs2") == 0)
		{
			AODV_MIRACLE_ROUTING = FALSE;
			return TCL_OK;
		}
	}
	else if (argc==3)
	{
		if (strcmp(argv[1],"setBroadcastAddr")==0)
		{
			MrclAddress *addr = (MrclAddress *)(TclObject::lookup(argv[2]));
			if (addr==0)
			{
				printf("AodvModule::command(%s), error in address lookup\n", argv[1]);
				return TCL_ERROR;
			}
			broadcastAddr_ = addr;
			int len = addr->strlen() + 2;
			char *str = new char[len];
			addr->toString(str, len);
			if(debug_>5)
			{
				printf("AodvModule::command(%s) -- setBroadcastAddr(%d) = %s\n", argv[1], nAddresses(), str);
			}
			return TCL_OK;
		}
		else if(strcmp(argv[1], "if_queue") == 0)
		{
			ifqueue_ = (PriQueue*) TclObject::lookup(argv[2]);
			if(ifqueue_ == 0)
				return TCL_ERROR;
			return TCL_OK;
		}
	}
	return MrclRouting::command(argc, argv);
}


/*
  Link Failure Management Functions
*/

static void aodv_rt_failed_callback(Packet *p, void *arg)
{
	((AodvModule*) arg)->rt_ll_failed(p);
}

/*
 * This routine is invoked when the link-layer reports a route failed.
 */
void AodvModule::rt_ll_failed(Packet *p)
{
	struct hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);

	Aodv_rt_entry *rt;
	char* broken_nbr = rhdr->nexthop();

	if (debug_>5)
	{
		char addr[20];
		char saddr[20];
		int ip;
		memcpy(&ip, rhdr->nexthop() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(saddr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::rt_ll_failed] Node %s  %s failed pkt %d\n", saddr, addr, ch->uid());
	}

	if (!AODV_LINK_LAYER_DETECTION)
	{
 		drop(p, DROP_RTR_MAC_CALLBACK_VERBOSITY, DROP_RTR_MAC_CALLBACK);
		return;
	}
	/*
	* Non-data packets and Broadcast Packets can be dropped.
	*/
	if(! (DATA_PACKET(ch->ptype())||(ch->ptype()==PT_MCBR)) || MrclAddress::areEqual(rhdr->daddr(), broadcastAddr_->getAddr()) )
	{
		drop(p, DROP_RTR_MAC_CALLBACK_VERBOSITY, DROP_RTR_MAC_CALLBACK);
		return;
	}
// 	log_link_broke(p);
	if((rt = rtable_.rt_lookup(rhdr->daddr())) == 0)
	{
		drop(p, DROP_RTR_MAC_CALLBACK_VERBOSITY, DROP_RTR_MAC_CALLBACK);
		return;
	}
// 	log_link_del(ch->next_hop_);

	if (AODV_LOCAL_REPAIR)
	{
		/* if the broken link is closer to the dest than source, 
		attempt a local repair. Otherwise, bring down the route. */
		if (ch->num_forwards() > rt->getHops())
		{
			local_rt_repair(rt, p); // local repair
			// retrieve all the packets in the ifq using this link,
			// queue the packets for which local repair is done, 
			return;
		}else if (rt->getFlags()==RTF_IN_REPAIR) return;		// broken link is in repairing phase
	}
	nb_delete(broken_nbr);
	drop(p, DROP_RTR_MAC_CALLBACK_VERBOSITY, DROP_RTR_MAC_CALLBACK);
	// Do the same thing for other packets in the interface queue using the
	// broken link -Mahesh
	if (ifqueue_)
	{
		while((p = ifqueue_->filter((nsaddr_t)(*(broken_nbr+sizeof(int))))))
		{
			drop(p, DROP_RTR_MAC_CALLBACK_VERBOSITY, DROP_RTR_MAC_CALLBACK);
		}
	}
// 	nb_delete(broken_nbr);
}


void AodvModule::handle_link_failure(char* id)
{
	if (debug_>5)
	{
		char addr[20];
		int ip;
		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		char addr2[20];
		memcpy(&ip, id + sizeof(int), sizeof(int));
		sprintf(addr2,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::handle_link_failure] Node %s link failure %s\n", addr, addr2);
	}
	Aodv_rt_entry *rt, *rtn;
	Packet *rerr = Packet::alloc();
// 	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(rerr);
// 	hdr_mrcl_aodv_error *re = (hdr_mrcl_aodv_error *)(ah->body);
	hdr_mrcl_aodv_error *re = HDR_MRCL_AODV_ERROR(rerr);

	re->DestCount = 0;
	for(rt = rtable_.head(); rt; rt = rtn)
	{
		// for each rt entry
		rtn = rtable_.next(rt);
	if ((rt->getHops() != INFINITY2) && (MrclAddress::areEqual(rt->getNexthop(),id) ) )
	{
		assert (rt->getFlags() == RTF_UP);
		assert((rt->getSeqno()%2) == 0);
		rt->incrSeqno();
		MrclAddress::storeAddr(&(re->unreachable_dst[re->DestCount * MRCL_ADDRESS_MAX_LEN]), rt->getDst());
		re->unreachable_dst_seqno[re->DestCount] = rt->getSeqno();
// 	if (debug_)
// 	{
// 		fprintf(stderr, "[AodvModule::handle_link_failure] %s(%f): %d\t(%d\t%u\t%d)\n", __FUNCTION__, CURRENT_TIME,
// 				index, re->unreachable_dst[re->DestCount],
// 				re->unreachable_dst_seqno[re->DestCount], (char *)(rt->getNexthop()+ sizeof(int)));
// 	}

	re->DestCount += 1;
	rt_down(rt);
	}
	// remove the lost neighbor from all the precursor lists
	rt->pc_delete(id);
	}
	
	if (re->DestCount > 0) {
// #ifdef DEBUG
// 		fprintf(stderr, "%s(%f): %d\tsending RERR...\n", __FUNCTION__, CURRENT_TIME, index);
// #endif // DEBUG
		sendError(rerr, false);
	}
	else
	{
		Packet::free(rerr);
	}
}

void AodvModule::local_rt_repair(Aodv_rt_entry *rt, Packet *p)
{
	if (debug_)
	{
		fprintf(stderr,"[AodvModule::local_rt_repair] %s: Dst - %d\n", __FUNCTION__, (char *)(rt->getDst()+ sizeof(int)));
	}
  // Buffer the packet 
  rqueue_.enque(p);

  // mark the route as under repair 
  rt->setFlags(RTF_IN_REPAIR);

  sendRequest(rt->getDst());

  // set up a timer interrupt
  Scheduler::instance().schedule(&lrtimer, p->copy(), rt->rt_req_timeout);
}



/*
  Route Handling Functions
*/


void AodvModule::rt_update(Aodv_rt_entry *rt, int seqnum, int metric, char *nexthop, double expire_time)
{
	rt->setSeqno(seqnum);
	rt->setHops(metric);
	rt->setFlags(RTF_UP);
	rt->setNexthop(nexthop);
	rt->setExpire(expire_time);
}

void AodvModule::rt_down(Aodv_rt_entry *rt)
{
	/*
	*  Make sure that you don't "down" a route more than once.
	*/

	if(rt->getFlags() == RTF_DOWN) 
	{
		return;
	}

	// assert (rt->rt_seqno%2); // is the seqno odd?
	rt->setLastHopCount(rt->getHops());
	rt->setHops(INFINITY2);
	rt->setFlags(RTF_DOWN);
	rt->resetNexthop();
	rt->setExpire(0);
}


void AodvModule::rt_purge()
{
	Aodv_rt_entry *rt, *rtn;
	double now = CURRENT_TIME;
	double delay = 0.0;
	Packet *p;

	char addr[20];
	int ip;
	if (debug_>5)
	{	memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("Node %s check route table at %f\n", addr, CURRENT_TIME);
	}
	for(rt = rtable_.head(); rt; rt = rtn)
	{  // for each rt entry
// 		rtn = rt->rt_link.le_next;			// standard ns version: not used
		rtn = rtable_.next(rt);
		if (debug_>5)
		{
			memcpy(&ip, rt->getDst() + sizeof(int), sizeof(int));
			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
			printf("\tdst %s expire at %f\n", addr, rt->getExpire());
		}
		if ((rt->getFlags() == RTF_UP) && (rt->getExpire() < now))
		{
			// if a valid route has expired, purge all packets from 
			// send buffer and invalidate the route.                    
			assert(rt->getHops() != INFINITY2);
			while((p = rqueue_.deque(rt->getDst())))
			{
				if (debug_)
					fprintf(stderr, "%s: calling drop()\n", __FUNCTION__);
				drop(p, AODV_DROP_RTR_NO_ROUTE_VERBOSITY, AODV_DROP_RTR_NO_ROUTE);
			}
			rt->incrSeqno();
			assert (rt->getSeqno()%2);
			rt_down(rt);
		}
		else if (rt->getFlags() == RTF_UP)
		{
			// If the route is not expired,
			// and there are packets in the sendbuffer waiting,
			// forward them. This should not be needed, but this extra 
			// check does no harm.
			assert(rt->getHops() != INFINITY2);
			while((p = rqueue_.deque(rt->getDst())))
			{
				forward (p, rt, delay);
				delay += ARP_DELAY;
			}
		}
		else if (rqueue_.find(rt->getDst()))
			// If the route is down and 
			// if there is a packet for this destination waiting in
			// the sendbuffer, then send out route request. sendRequest
			// will check whether it is time to really send out request or not.
			// This may not be crucial to do it here, as each generated 
			// packet will do a sendRequest anyway.
			sendRequest(rt->getDst()); 
	}

}



int AodvModule::controlPacket(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);

	if(ch->ptype() != PT_AODV)
		return 0;
	
	recvAodv(p);
	return 1;
}


void AodvModule::forward(Packet *p)
{

	struct hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);

	int saddrLen;
	memcpy(&saddrLen, rhdr->saddr(), sizeof(int));
	if (saddrLen==0) return; // routing addresses not yet initialized
	char saddr[MRCL_ADDRESS_MAX_LEN] = "";
	char temp[10];
	for(int i=saddrLen-1; i>=0; i--)
	{
		if (i==0)
			sprintf(temp,"%d", rhdr->saddr()[i+sizeof(int)]);
		else
			sprintf(temp,"%d.", rhdr->saddr()[i+sizeof(int)]);
		strcat(saddr,temp);
	}
	strcat(saddr,"\0");
	char daddr[MRCL_ADDRESS_MAX_LEN] = "";
	int daddrLen;
	if (daddrLen>0)
	{
		memcpy(&daddrLen, rhdr->daddr(), sizeof(int));
		strcpy(temp,"");
		for(int i=daddrLen-1; i>=0; i--)
		{
			if (i==0)
				sprintf(temp,"%d", rhdr->daddr()[i+sizeof(int)]);
			else
				sprintf(temp,"%d.", rhdr->daddr()[i+sizeof(int)]);
			strcat(daddr,temp);
		}
	}
	strcat(daddr,"\0");

	// check destination
	assert(nAddresses() > 0);
	assert(broadcastAddr_);
		
	if (ch->ptype() != PT_AODV && ch->direction() == hdr_cmn::UP &&
		(broadcastAddr_->isEqual(rhdr->daddr()))
		|| isMyAddress(rhdr->daddr())) 
	{
		// packet for me (unicast or broadcast)
		//dmux_->recv(p,0);
		if (debug_>5)
		{
			char addr[20];
			int ip;
			memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
			Position* pos = getPosition();
			printf("[AodvModule::forward1] %s My packet %d -> send up (%f,%f) at %f ttl %d\n", addr, ch->uid(), pos->getX(), pos->getY(), CURRENT_TIME, rhdr->ttl());
		}
		sendUp(p);
		return;
	}
	#ifdef DEBUG
		printf("packet not for me -> try to solve the address\n");
	#endif
	if ((MrclAddress::areEqual(rhdr->daddr(),rhdr->nexthop()))&&!(isMyAddress(rhdr->daddr())))
	{
		return;
	}
	int nRoute = getRoute(rhdr->daddr(), p, 0);

	int saddrLen1;
	memcpy(&saddrLen1, rhdr->saddr(), sizeof(int));
	if (saddrLen1==0) return; // routing addresses not yet initialized
	char saddr1[MRCL_ADDRESS_MAX_LEN] = "";
	char temp1[10];
	for(int i=saddrLen1-1; i>=0; i--)
	{
		if (i==0)
			sprintf(temp1,"%d", rhdr->saddr()[i+sizeof(int)]);
		else
			sprintf(temp1,"%d.", rhdr->saddr()[i+sizeof(int)]);
		strcat(saddr1,temp1);
	}
	strcat(saddr1,"\0");
	char daddr1[MRCL_ADDRESS_MAX_LEN] = "";
	int daddrLen1;
	if (daddrLen1>0)
	{
		memcpy(&daddrLen1, rhdr->daddr(), sizeof(int));
		strcpy(temp1,"");
		for(int i=daddrLen1-1; i>=0; i--)
		{
			if (i==0)
				sprintf(temp1,"%d", rhdr->daddr()[i+sizeof(int)]);
			else
				sprintf(temp1,"%d.", rhdr->daddr()[i+sizeof(int)]);
			strcat(daddr1,temp1);
		}
	}
	strcat(daddr1,"\0");

	#ifdef DEBUG
		char addr[20];
		//char saddr[20];
		int ip;
		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		memcpy(&ip, rhdr->saddr() + sizeof(int), sizeof(int));
		sprintf(saddr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::forward] Node %s Try to solve new packet from %s at %.10f\n", addr, saddr, CURRENT_TIME);
		printf("[AodvModule::forward] found %d route(s)\n", nRoute);
	#endif
	if (nRoute != 0)
	{
		if (rhdr->getModuleRoute(rhdr->moduleRouteCount() - 1) == getId())
		{
			if ((isMyAddress(rhdr->saddr()))&&(ch->num_forwards()==0))
				rhdr->ttl_ = NETWORK_DIAMETER;
			else
				rhdr->ttl_--;

			if (nRoute > 0)
			{
				if (AODV_MIRACLE_ROUTING==FALSE)
				{
					// standard AODV working -> try to forward directly the packet despite possible better paths via others routing module
					int i = 0;
					while((rhdr->getModuleRoute(rhdr->moduleRouteCount() - 1)!=getId())||(i<nRoute-1))
					{
						getRoute(rhdr->daddr(), p, ++i);
					}
					if (rhdr->getModuleRoute(rhdr->moduleRouteCount() - 1)==getId())
					{
						// route known by this module
						Aodv_rt_entry *rt = rtable_.rt_lookup(rhdr->daddr());
						if (debug_>5)
						{
							char saddr[20];
							int ip;
							memcpy(&ip, rt->getNexthop() + sizeof(int), sizeof(int));
							sprintf(saddr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
							printf("[AodvModule::forward1] forward to %s\n", saddr);
						}
						forward(p, rt,0);
					}
					else
					{
						// route does not known by this module -> try ro solve it
						#ifdef DEBUG
							printf("route does not known by this module -> try ro solve it\n");
						#endif
						resolve(p);
						return;
					}
				}
				else
				{
					if (rhdr->getModuleRoute(rhdr->moduleRouteCount() - 1)==getId())
					{///rhdr->getModuleRoute(rhdr->moduleRouteCount() - 1) == getId() NON VA! C'E' GIA' PRIMA
					 /// e quindi non entro mai nell'else!
						printf("// this AODV module has the better metric to forward this packet\n");
						Aodv_rt_entry *rt = rtable_.rt_lookup(rhdr->daddr());
						rt->pc_show();
						forward(p, rt,0);
					}
					else
					{
						#ifdef DEBUG
							printf("// route known by another routing module this packet has to be forwarded to other routing module\n");
						#endif
						getRoute(rhdr->daddr(), p, 0);
						if (debug_>5)
							printf("[AodvModule::forward] sendUp to another routing module %d %d \n", rhdr->moduleRouteCount(), getId());
						sendUp(p);
					}
				}
			}
			else
			{
				#ifdef DEBUG
					printf("resolve, nRoute<=0\n");
				#endif
				resolve(p);
				return;
			}
		}
		else
		{
			// this packet has to be forwarded to another routing module
			if (debug_)
				printf("[AodvModule::forward] sendUp to another routing module \n");
// 		mrhdr->setSendup();
			sendUp(p);
		}
	}
	else
	{
		// packet can not be forwarded
		if (debug_>5)
			printf("[AodvModule::forward1]  NO ROUTE to HOST %s -> DROP packet\n", rhdr->daddr());
		drop(p, AODV_DROP_RTR_NO_ROUTE_VERBOSITY, DROP_RTR_NO_ROUTE);
	}
}



void AodvModule::resolve(Packet *p)
{
	RoutingHdr *rhdr = HDR_ROUTING(p);

	if (debug_>5)
	{
		char addr[20];
		int ip;
		memcpy(&ip, rhdr->daddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::resolve] route unknown to %s -> start Route Discovery and put the packet in queue\n", addr);
	}
	Aodv_rt_entry *rt = rtable_.rt_lookup(rhdr->daddr());

	if(rt == 0)
	{
		rt = rtable_.rt_add(rhdr->daddr());
	}
	/*
	*  if I am the source of the packet, then do a Route Request.
	*/
	if(isMyAddress(rhdr->saddr()))
	{
		rqueue_.enque(p);
		sendRequest(rt->getDst());
	}
	else
	{
		if (rt->getFlags() == RTF_IN_REPAIR)
		{
			/*
			*	A local repair is in progress. Buffer the packet. 
			*/
			rqueue_.enque(p);
		}
		else
		{
			Packet *rerr = Packet::alloc();
// 			hdr_mrcl_aodv *ah = HDR_MRCL_AODV(rerr);
// 			hdr_mrcl_aodv_error *re = (hdr_mrcl_aodv_error *)(ah->body);
			hdr_mrcl_aodv_error *re = HDR_MRCL_AODV_ERROR(rerr);
			/* 
			* For now, drop the packet and send error upstream.
			* Now the route errors are broadcast to upstream
			* neighbors - Mahesh 09/11/99
			*/
	
			assert(rt->getFlags() == RTF_DOWN);
			re->DestCount = 0;
			MrclAddress::storeAddr(&(re->unreachable_dst[re->DestCount * MRCL_ADDRESS_MAX_LEN]), rt->getDst());
			re->unreachable_dst_seqno[re->DestCount] = rt->getSeqno();
			re->DestCount += 1;
			if (debug_)
			{
				fprintf(stderr, "[AodvModule::resolve] %s: sending RERR...\n", __FUNCTION__);
			}
			sendError(rerr, false);
			drop(p, AODV_DROP_RTR_NO_ROUTE_VEROSITY, AODV_DROP_RTR_NO_ROUTE);
		}
	}
}

void AodvModule::recvAodv(Packet *p)
{
	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);

	switch(ah->ah_type)
	{
		case AODVTYPE_RREQ:
			recvRequest(p);
			break;
		case AODVTYPE_RREP:
			recvReply(p);
			break;
		case AODVTYPE_RERR:
			recvError(p);
			break;
		case AODVTYPE_HELLO:
			recvHello(p);
			break;
        	default:
			fprintf(stderr, "Invalid AODV type (%x)\n", ah->ah_type);
			exit(1);
	}
}

int AodvModule::id_lookup(char *addr, int bid)
{
	for(AddrList *cur = bcache_; cur; cur = cur->next)
	{
		if(MrclAddress::areEqual(cur->addr,addr) && bid == cur->id)
		{
			return 1;
		}
	}
	return 0;
}


void AodvModule::id_delete(AddrList *a)
{
	if(a == bcache_)
	{
		bcache_ = a->next;
		if(bcache_)
			bcache_->prev = 0;
	}
	else
	{
		a->prev->next = a->next;
		if(a->next)
			a->next->prev = a->prev;
	}
	delete a;
}

void AodvModule::id_purge()
{
	double now = CURRENT_TIME;
	AddrList *cur = bcache_;
	while(cur)
	{
		if(cur->expire <= now)
		{
			if(cur != bcache_)
			{
				cur = cur->prev;
				id_delete(cur->next);
				cur = cur->next;
			}
			else
			{
				id_delete(cur);
				cur = ncache_;
			}
		}
		else
			cur = cur->next;
	}
}


void AodvModule::id_insert(char *addr, int bid)
{
	AddrList *a = new AddrList;
	MrclAddress::storeAddr(a->addr, addr);
	a->id = bid;
	a->expire = Scheduler::instance().clock() + BCAST_ID_SAVE;
	a->next = bcache_;
	a->prev = 0;
	if(bcache_)
		bcache_->prev = a;
	bcache_ = a;
}

void AodvModule::recvRequest(Packet *p)
{
	#ifdef DEBUG
		char addr[20];
		int ip;
		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		char saddr[20];
		RoutingHdr* mrhdr = HDR_ROUTING(p);
		memcpy(&ip, mrhdr->saddr() + sizeof(int), sizeof(int));
		sprintf(saddr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::recvRequest] Node %s recv REQ from %s\n", addr, saddr);
	#endif
// 	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);
// 	hdr_mrcl_aodv_request *rq = (hdr_mrcl_aodv_request *)(ah->body);
	hdr_mrcl_aodv_request *rq = HDR_MRCL_AODV_REQUEST(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);
	/*
	* Drop if:
	*      - I'm the source
	*      - I recently heard this request.
	*/
	if(isMyAddress(rq->rq_src))
	{
		drop(p, AODV_GOT_MY_REQUEST_VERBOSITY, AODV_GOT_MY_REQUEST);
		return;
	}

	if(id_lookup(rq->rq_src, rq->rq_bcast_id))
	{
		drop(p, AODV_DISCARD_REQUEST_VERBOSITY, AODV_DISCARD_REQUEST);
		return;
	}
	/*
	* Cache the broadcast ID
	*/
	id_insert(rq->rq_src, rq->rq_bcast_id);
	/* 
	* We are either going to forward the REQUEST or generate a
	* REPLY. Before we do anything, we make sure that the REVERSE
	* route is in the route table.
	*/
	Aodv_rt_entry *rt0 = rtable_.rt_lookup(rq->rq_src); //rt0 is the reverse route
 	if(rt0 == 0) 
	{ /* if not in the route table */
		// create an entry for the reverse route.
		#ifdef DEBUG
			printf("if not in the route table, create an entry for the reverse route.\n");
		#endif
		rt0 = rtable_.rt_add(rq->rq_src);
	}
	rt0->pc_show();

  	rt0->setExpire(max(rt0->getExpire(), (CURRENT_TIME + REV_ROUTE_LIFE)));

	if ( (rq->rq_src_seqno > rt0->getSeqno() ) || ((rq->rq_src_seqno == rt0->getSeqno()) && (rq->rq_hop_count < rt0->getHops())) ) 
	{
		// If we have a fresher seq no. or lesser #hops for the 
		// same seq no., update the rt entry. Else don't bother.
		rt_update(rt0, rq->rq_src_seqno, rq->rq_hop_count, rhdr->saddr(),
			max(rt0->getExpire(), (CURRENT_TIME + REV_ROUTE_LIFE)) );
		if (rt0->rt_req_timeout > 0.0) 
		{
			// Reset the soft state and 
			// Set expiry time to CURRENT_TIME + ACTIVE_ROUTE_TIMEOUT
			// This is because route is used in the forward direction,
			// but only sources get benefited by this change
			rt0->rt_req_cnt = 0;
			rt0->rt_req_timeout = 0.0; 
			rt0->setReqLastTtl(rq->rq_hop_count);
			rt0->setExpire(CURRENT_TIME + ACTIVE_ROUTE_TIMEOUT);
		}
		/* Find out whether any buffered packet can benefit from the 
		* reverse route.
		* May need some change in the following code - Mahesh 09/11/99
		*/
		assert (rt0->getFlags() == RTF_UP);
		Packet *buffered_pkt;
		while ((buffered_pkt = rqueue_.deque(rt0->getDst()))) 
		{
			if (rt0 && (rt0->getFlags() == RTF_UP)) 
			{
				assert(rt0->getHops() != INFINITY2);
				forward(buffered_pkt, rt0);
			}
		}
	}
	// End for putting reverse route in rt table

	/*
	* We have taken care of the reverse route stuff.
	* Now see whether we can send a route reply. 
	*/
	Aodv_rt_entry *rt = rtable_.rt_lookup(rq->rq_dst);

	// First check if I am the destination ..
	if(isMyAddress(rq->rq_dst)) 
	{

		if(debug_)
			fprintf(stderr, "%s: destination sending reply\n",
				 __FUNCTION__);
		// Just to be safe, I use the max. Somebody may have
		// incremented the dst seqno.
		seqno_ = max(seqno_, rq->rq_dst_seqno)+1;
		if (seqno_ % 2) 
			seqno_++;

		assert(nAddresses() > 0);

		sendReply(rq->rq_src,			// IP Destination
             			1,			// Hop Count
				getAddress(0)->getAddr(),			// Dest IP Address
				seqno_,			// Dest Sequence Num
				MY_ROUTE_TIMEOUT,	// Lifetime
				rq->rq_timestamp);	// timestamp
		drop(p, AODV_RECEIVED_REQUEST_VERBOSITY, AODV_RECEIVED_REQUEST);
	}
	// I am not the destination, but I may have a fresh enough route.
	else
	{
		if (AODV_MIRACLE_ROUTING==FALSE)
		{
			// standard ns2 behaviour -> answer with the info only of the local routing table
			if (rt && (rt->getHops() != INFINITY2) && 
				(rt->getSeqno() >= rq->rq_dst_seqno) ) 
			{
				//assert (rt->rt_flags == RTF_UP);
				assert(MrclAddress::areEqual(rq->rq_dst, rt->getDst()));
				//assert ((rt->rt_seqno%2) == 0);	// is the seqno even?
				sendReply(rq->rq_src,
					rt->getHops() + 1,
					rq->rq_dst,
					rt->getSeqno(),
					(int)(rt->getExpire() - CURRENT_TIME),
					rq->rq_timestamp);
				// Insert nexthops to RREQ source and RREQ destination in the
				// precursor lists of destination and source respectively
				rt->pc_insert(rt0->getNexthop()); // nexthop to RREQ source
				rt0->pc_insert(rt->getNexthop()); // nexthop to RREQ destination
		
				if(RREQ_GRAT_RREP)
					sendReply(rq->rq_dst,
						rq->rq_hop_count,
						rq->rq_src,
						rq->rq_src_seqno,
						(int)(rt->getExpire() - CURRENT_TIME),
						rq->rq_timestamp);
		
			
				// TODO: send grat RREP to dst if G flag set in RREQ using rq->rq_src_seqno, rq->rq_hop_counT
			
				// DONE: Included gratuitous replies to be sent as per IETF aodv draft specification. As of now, G flag has not been dynamically used and is always set or reset in aodv-packet.h --- Anant Utgikar, 09/16/02.

				drop(p, AODV_RECEIVED_REQUEST_VERBOSITY, AODV_RECEIVED_REQUEST_RESOLVED);
			}
			/*
			* Can't reply. So forward the  Route Request
			*/
			else 
			{
				assert(nAddresses() > 0);
				assert(broadcastAddr_);
				MrclAddress::storeAddr(rhdr->saddr(), getAddress(0)->getAddr());
				MrclAddress::storeAddr(rhdr->daddr(), broadcastAddr_->getAddr());
				rq->rq_hop_count += 1;
				// Maximum sequence number seen en route
				if (rt)
				{
					printf("rp dst seqno assumes max %d, %d RecvRQT mrclFALSE\n",rt->getSeqno(), rq->rq_dst_seqno);
					rq->rq_dst_seqno = max(rt->getSeqno(), rq->rq_dst_seqno);
				}
				forward(p, (Aodv_rt_entry *)0);
			}
		}
		else
		{
			// enhanced beahviour mode -> test all the routing modules within the MIRACLE routing framework
	char daddr[MRCL_ADDRESS_MAX_LEN] = "";
	int daddrLen;
	char temp[10];
	if (daddrLen>0)
	{
		memcpy(&daddrLen, rhdr->daddr(), sizeof(int));
		strcpy(temp,"");
		for(int ki=daddrLen-1; ki>=0; ki--)
		{
			if (ki==0)
				sprintf(temp,"%d", rhdr->daddr()[ki+sizeof(int)]);
			else
				sprintf(temp,"%d.", rhdr->daddr()[ki+sizeof(int)]);
			strcat(daddr,temp);
		}
	}
	strcat(daddr,"\0");

//##
// Mior, 12, 2008, daddr is 255.255.255.255! It is a req! Changed with rq_dest!
//##
	char daddrc[MRCL_ADDRESS_MAX_LEN] = "";
	//int daddrLen;
	char tempc[10];
	if (daddrLen>0)
	{
		memcpy(&daddrLen, rq->rq_dest(), sizeof(int));
		strcpy(tempc,"");
		for(int ki=daddrLen-1; ki>=0; ki--)
		{
			if (ki==0)
				sprintf(tempc,"%d", rq->rq_dest()[ki+sizeof(int)]);
			else
				sprintf(tempc,"%d.", rq->rq_dest()[ki+sizeof(int)]);
			strcat(daddrc,tempc);
		}
	}
	strcat(daddrc,"\0");
//                      andrea mior dic 12 2008
			int nRoute = getRoute(rq->rq_dest(), p, 0);

			if (nRoute>0)
			{
				if (rhdr->getModuleRoute(rhdr->moduleRouteCount() - 1)==getId())
				{
					Aodv_rt_entry *rt0 = rtable_.rt_lookup(rhdr->daddr());

					if(rt0 && (rt0->getHops() != INFINITY2)&& (rt->getSeqno() >= rq->rq_dst_seqno) )
					{
						// Directly
						//assert (rt->rt_flags == RTF_UP);
						assert(MrclAddress::areEqual(rq->rq_dst, rt->getDst()));
						//assert ((rt->rt_seqno%2) == 0);	// is the seqno even?
						sendReply(rq->rq_src,
							rq->rq_hop_count + 1,
							rq->rq_dst,
							rq->rq_dst_seqno,
							ACTIVE_ROUTE_TIMEOUT,
							rq->rq_timestamp);
					}
				}
				else
				{
					// via another routing module within this miracle routing framework
// 						if (rt)
// 						printf("rt %p\n", rt);
// 						sendReply(rq->rq_src,
// 						rt->getHops() + 1,
// 						rq->rq_dst,
// 						rt->getSeqno(),
// 						(int)(rt->getExpire() - CURRENT_TIME),
// 						rq->rq_timestamp);
// 					// Insert nexthops to RREQ source and RREQ destination in the
// 					// precursor lists of destination and source respectively
// 					rt->pc_insert(rt0->getNexthop()); // nexthop to RREQ source
// 					rt0->pc_insert(rt->getNexthop()); // nexthop to RREQ destination
					#ifdef DEBUG
						printf("Via another routing module within this miracle framework\n");
						printf("Send Reply @ %f\n", Scheduler::instance().clock());
					///if(RREQ_GRAT_RREP) DA TOGLIERE ANCHE NEL REPOS
						printf("What am I sending? dstseqno %d hopcount %d (seqno %d)\n", rq->rq_dst_seqno, rq->rq_hop_count, seqno_);
					#endif
						sendReply(rq->rq_src,
							rq->rq_hop_count,
							rq->rq_dst,
							rq->rq_dst_seqno,
							ACTIVE_ROUTE_TIMEOUT,
							rq->rq_timestamp);

				}

				drop(p, AODV_RECEIVED_REQUEST_VERBOSITY, AODV_RECEIVED_REQUEST_RESOLVED);

			}
			else
			{
				assert(nAddresses() > 0);
				assert(broadcastAddr_);
				MrclAddress::storeAddr(rhdr->saddr(), getAddress(0)->getAddr());
				MrclAddress::storeAddr(rhdr->daddr(), broadcastAddr_->getAddr());
				rq->rq_hop_count += 1;
				// Maximum sequence number seen en route
				if (rt){printf("rq dst seqno assumes max %d, %d RecvRQT mrclTRUE\n",rt->getSeqno(), rq->rq_dst_seqno);
					rq->rq_dst_seqno = max(rt->getSeqno(), rq->rq_dst_seqno);
				}

				forward(p, (Aodv_rt_entry *)0);
			}
		}
	}
}

void AodvModule::recvReply(Packet *p)
{
	RoutingHdr *rhdr = HDR_ROUTING(p);
// 	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);
// 	hdr_mrcl_aodv_reply *rp = (hdr_mrcl_aodv_reply *)(ah->body);
	hdr_mrcl_aodv_reply *rp = HDR_MRCL_AODV_REPLY(p);
	char suppress_reply = 0;
	double delay = 0.0;
	
// 	if(debug_)
// 		fprintf(stderr, "%d - %s: received a REPLY\n", index, __FUNCTION__);

	/*
	*  Got a reply. So reset the "soft state" maintained for 
	*  route requests in the request table. We don't really have
	*  have a separate request table. It is just a part of the
	*  routing table itself. 
	*/
	// Note that rp_dst is the dest of the data packets, not the
	// the dest of the reply, which is the src of the data packets.

	Aodv_rt_entry *rt = rtable_.rt_lookup(rp->rp_dst);

	/*
	*  If I don't have a rt entry to this host... adding
	*/
	if(rt == 0) 
	{
		rt = rtable_.rt_add(rp->rp_dst);
	}
	/*
	* Add a forward route table entry... here I am following 
	* Perkins-Royer AODV paper almost literally - SRD 5/99
	*/

	if ( (rt->getSeqno() < rp->rp_dst_seqno) ||   // newer route 
		((rt->getSeqno() == rp->rp_dst_seqno) &&  (rt->getHops() > rp->rp_hop_count)) ) 
	{ // shorter or better route
		#ifdef DEBUG
			printf("Add new forward Route\n");
		#endif
		// Update the rt entry 
		rt_update(rt, rp->rp_dst_seqno, rp->rp_hop_count,
			rp->rp_src, CURRENT_TIME + rp->rp_lifetime);
		// reset the soft state
		rt->rt_req_cnt = 0;
		rt->rt_req_timeout = 0.0; 
		rt->setReqLastTtl(rp->rp_hop_count);
  		rt->pc_show();
		if (isMyAddress(rhdr->daddr())) 
		{ // If I am the original source
			// Update the route discovery latency statistics
			// rp->rp_timestamp is the time of request origination
		
			rt->setDiscLatency((unsigned char)rt->getHistIndx(), (CURRENT_TIME - rp->rp_timestamp)
                                         / (double) rp->rp_hop_count);
			// increment indx for next time
			rt->setHistIndx((char)((rt->getHistIndx() + 1) % MAX_HISTORY));
		}	

		/*
		* Send all packets queued in the sendbuffer destined for
		* this destination. 
		* XXX - observe the "second" use of p.
		*/
		Packet *buf_pkt;
		while((buf_pkt = rqueue_.deque(rt->getDst()))) 
		{
			if(rt->getHops() != INFINITY2) 
			{
				assert (rt->getFlags() == RTF_UP);
				//RoutingHdr* rh = RoutingHdr::access(buf_pkt);




//////////////////////////////////////////////////////////////////////////////
RoutingHdr* h = HDR_ROUTING(buf_pkt);
	
	int saddrLen;
	memcpy(&saddrLen, h->saddr(), sizeof(int));
	if (saddrLen==0) return; // routing addresses not yet initialized
	char saddr[MRCL_ADDRESS_MAX_LEN] = "";
	char temp[10];
	for(int i=saddrLen-1; i>=0; i--)
	{
		if (i==0)
			sprintf(temp,"%d", h->saddr()[i+sizeof(int)]);
		else
			sprintf(temp,"%d.", h->saddr()[i+sizeof(int)]);
		strcat(saddr,temp);
	}
	strcat(saddr,"\0");
	char daddr[MRCL_ADDRESS_MAX_LEN] = "";
	int daddrLen;
	if (daddrLen>0)
	{
		memcpy(&daddrLen, h->daddr(), sizeof(int));
		strcpy(temp,"");
		for(int i=daddrLen-1; i>=0; i--)
		{
			if (i==0)
				sprintf(temp,"%d", h->daddr()[i+sizeof(int)]);
			else
				sprintf(temp,"%d.", h->daddr()[i+sizeof(int)]);
			strcat(daddr,temp);
		}
	}
	strcat(daddr,"\0");
/////////////////////////////////////////////////////////////////////////////////



				// Delay them a little to help ARP. Otherwise ARP 
				// may drop packets. -SRD 5/23/99
				forward(buf_pkt, rt, delay);
				delay += ARP_DELAY;
			}
		}
	}
	else 
	{
		suppress_reply = 1;
	}

	/*
	* If reply is for me, discard it.
	*/
	if(isMyAddress(rhdr->daddr())) 
	{
		char addr[20];
		int ip;
		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		#ifdef DEBUG
			printf("[AodvModule::recvReply] %s recv my RREP -> discard it\n", addr);
		#endif
		drop(p, AODV_REPLY_FOR_ME_VERBOSITY, AODV_REPLY_FOR_ME);
	}
	else if(suppress_reply) 
	{
// 		char addr[20];
// 		int ip;
// 		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
// 		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
// 		printf("[AodvModule::recvReply] %s <suppress_reply> -> discard it\n", addr);

		drop(p, AODV_SUPPRESS_REPLY_VERBOSITY, AODV_SUPPRESS_REPLY);
	}
 	/*
	* Otherwise, forward the Route Reply.
	*/
	else 
	{
		#ifdef DEBUG
			printf("Not for me and not suppress_reply\n");
		#endif
	/*	char addr[20];
		int ip;
		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::recvReply] %s forward RREP\n", addr);
	*/	// Find the rt entry
		Aodv_rt_entry *rt0 = rtable_.rt_lookup(rhdr->daddr());

		// If the rt is up, forward
		if(rt0 && (rt0->getHops() != INFINITY2)) 
		{
			assert (rt0->getFlags() == RTF_UP);
			rp->rp_hop_count += 1;
			assert(nAddresses() > 0);
			MrclAddress::storeAddr(rp->rp_src, getAddress(0)->getAddr());
			forward(p, rt0);
			// Insert the nexthop towards the RREQ source to 
			// the precursor list of the RREQ destination
			rt->pc_insert(rt0->getNexthop()); // nexthop to RREQ source
      
		}
		else 
		{
			// I don't know how to forward .. drop the reply. 
			if(debug_)
				fprintf(stderr, "%s: dropping Route Reply\n", __FUNCTION__);
			drop(p, AODV_DROP_RTR_NO_ROUTE_VERBOSITY, DROP_RTR_NO_ROUTE);
		}
	}
}

void AodvModule::recvError(Packet *p)
{
	RoutingHdr *rhdr = HDR_ROUTING(p);
// 	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);
// 	hdr_mrcl_aodv_error *re = (hdr_mrcl_aodv_error *)(ah->body);
	hdr_mrcl_aodv_error *re = HDR_MRCL_AODV_ERROR(p);

	Aodv_rt_entry *rt;
	int i;
	Packet *rerr = Packet::alloc();
// 	hdr_mrcl_aodv *ah_err = HDR_MRCL_AODV(rerr);
// 	hdr_mrcl_aodv_error *nre = (hdr_mrcl_aodv_error *)(ah_err->body);
	hdr_mrcl_aodv_error *nre = HDR_MRCL_AODV_ERROR(rerr);

	if (debugout)
	{
		char addr[20];
		int ip;
		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		char saddr[20];
		RoutingHdr* mrhdr = HDR_ROUTING(p);
		memcpy(&ip, mrhdr->saddr() + sizeof(int), sizeof(int));
		sprintf(saddr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::recvError] Node %s recv RERR from %s\n", addr, saddr);
	}
		
 	nre->DestCount = 0;

	for (i=0; i<re->DestCount; i++) 
	{
		// For each unreachable destination
		rt = rtable_.rt_lookup(&(re->unreachable_dst[i]));
		if ( rt && (rt->getHops() != INFINITY2) &&
			(MrclAddress::areEqual(rt->getNexthop(), rhdr->saddr())) &&
			(rt->getSeqno() <= re->unreachable_dst_seqno[i]) ) 
		{
			if (rt->getFlags() == RTF_IN_REPAIR) continue; // this node has just recognized the link failure and it's trying to repair the link
			assert(rt->getFlags() == RTF_UP);
			assert((rt->getSeqno()%2) == 0); // is the seqno even?
     			rt->setSeqno(re->unreachable_dst_seqno[i]);
			rt_down(rt);

			// Not sure whether this is the right thing to do
			if (ifqueue_)
			{
				Packet *pkt;
				struct hdr_ip *ih = HDR_IP(p);
				while((pkt = ifqueue_->filter(ih->saddr()))) 
				{
					drop(pkt, DROP_RTR_MAC_CALLBACK_VERBOSITY, DROP_RTR_MAC_CALLBACK);
				}
			}

			// if precursor list non-empty add to RERR and delete the precursor list
			if (!rt->pc_empty()) 
			{
				MrclAddress::storeAddr(&(nre->unreachable_dst[nre->DestCount * MRCL_ADDRESS_MAX_LEN]), rt->getDst());
 				nre->unreachable_dst_seqno[nre->DestCount] = rt->getSeqno();
				nre->DestCount += 1;
				rt->pc_delete();
			}
		}
	} 

	if (nre->DestCount > 0) 
	{
		if(debug_)
			fprintf(stderr, "%s(%f)\t sending RERR...\n", __FUNCTION__, CURRENT_TIME);
   		sendError(rerr);
	}
	else 
	{
		Packet::free(rerr);
	}

	drop(p, AODV_RECV_ERROR_VERBOSITY, AODV_RECV_ERROR);
}

void AodvModule::recvHello(Packet *p)
{
	hdr_mrcl_aodv_reply *rp = HDR_MRCL_AODV_REPLY(p);
	AddrList *nb = nb_lookup(rp->rp_dst);
	if(nb == 0) 
	{
		nb_insert(rp->rp_dst);
	}
	else 
	{
		nb->expire = CURRENT_TIME + (1.5 * ALLOWED_HELLO_LOSS * HELLO_INTERVAL);
	}

	drop(p, AODV_RECV_HELLO_VERBOSITY, AODV_RECV_HELLO);
}


void AodvModule::sendReply(char *dst, int hop_count, char *rp_dst, int rpseq, int lifetime, double timestamp)
{
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);
// 	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);
// 	hdr_mrcl_aodv_reply *rp =(hdr_mrcl_aodv_reply *)(ah->body);
	hdr_mrcl_aodv_reply *rp = HDR_MRCL_AODV_REPLY(p);
	Aodv_rt_entry *rt = rtable_.rt_lookup(dst);

// 	if(debug_)
// 		fprintf(stderr, "sending Reply from %d at %.2f\n", index, Scheduler::instance().clock());

	assert(rt);

	rp->rp_type = AODVTYPE_RREP;
	rp->rp_hop_count = hop_count;
	MrclAddress::storeAddr(rp->rp_dst, rp_dst);
	rp->rp_dst_seqno = rpseq;
	assert(nAddresses() > 0);
// 	getAddress(0)->getAddr(rp->rp_src);
	MrclAddress::storeAddr(rp->rp_src, getAddress(0)->getAddr());
	rp->rp_lifetime = lifetime;
	rp->rp_timestamp = timestamp;
   
	// ch->uid() = 0;
	ch->ptype() = PT_AODV;
	ch->size() = overheadLength_ + rp->size();
	// [Ali Hamidian bug solve] check for PT_TCP and PT_ACK packet types, they do not need to include IP header length (just addedd by TCP Agent) and overheadLenght_ just include it by default
	if((ch->ptype() == PT_TCP || ch->ptype() == PT_ACK) && (ch->size()>= IP_HDR_LEN)) ch->size()-=IP_HDR_LEN;
	ch->iface() = -2;
 	ch->error() = 0;
	ch->addr_type() = NS_AF_INET;
	ch->next_hop_ = *((int *)(rt->getNexthop()) + 1);
	int a;
	getAddress(0)->getAddr(&a, sizeof(int), sizeof(int));
	ch->prev_hop_ = a;          // AODV hack

	MrclAddress::storeAddr(rhdr->saddr(), getAddress(0)->getAddr());
	MrclAddress::storeAddr(rhdr->daddr(), dst);
	rhdr->ttl() = NETWORK_DIAMETER;
	#ifdef DEBUG
		printf("Send reply aodv down\n");
	#endif
// 	setOldAddresses(p);
	sendDown(p);
}


void AodvModule::sendRequest(char *dst)
{

	if (debugout)
	{
		char addr[20];
		int ip;
		memcpy(&ip, dst + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		Position *pos = getPosition();
		printf("[AodvModule::sendRequest] to %s\n", addr);
	}
	// Allocate a RREQ packet 
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);
// 	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);
// 	hdr_mrcl_aodv_request *rq =(hdr_mrcl_aodv_request *)(ah->body);
	hdr_mrcl_aodv_request *rq = HDR_MRCL_AODV_REQUEST(p);
	Aodv_rt_entry *rt = rtable_.rt_lookup(dst);

	assert(rt);

	/*
	*  Rate limit sending of Route Requests. We are very conservative
	*  about sending out route requests. 
	*/

	if (rt->getFlags() == RTF_UP) 
	{
		assert(rt->getHops() != INFINITY2);
		Packet::free(p);
		return;
	}

	if (rt->rt_req_timeout > CURRENT_TIME) 
	{
		Packet::free(p);
		return;
	}

	// rt_req_cnt is the no. of times we did network-wide broadcast
	// RREQ_RETRIES is the maximum number we will allow before 
	// going to a long timeout.

	if (rt->rt_req_cnt > RREQ_RETRIES) 
	{
		rt->rt_req_timeout = CURRENT_TIME + MAX_RREQ_TIMEOUT;
		rt->rt_req_cnt = 0;
		Packet *buf_pkt;
		while ((buf_pkt = rqueue_.deque(rt->getDst()))) 
		{
			drop(buf_pkt, AODV_DROP_RTR_NO_ROUTE_VERBOSITY, DROP_RTR_NO_ROUTE);
		}
		Packet::free(p);
		return;
	}

	if(debug_)
		fprintf(stderr, "(module %d) Sending Route Request, dst\n", getId());

	// Determine the TTL to be used this time. 
	// Dynamic TTL evaluation - SRD

	rt->setReqLastTtl(max(rt->getReqLastTtl(),rt->getLastHopCount()));

	if (rt->getReqLastTtl() == 0)
	{
		// first time query broadcast
		rhdr->ttl() = TTL_START;
	}
	else 
	{
		// Expanding ring search.
		if (rt->getReqLastTtl() < TTL_THRESHOLD)
			rhdr->ttl() = rt->getReqLastTtl() + TTL_INCREMENT;
		else 
		{
			// network-wide broadcast
			rhdr->ttl() = NETWORK_DIAMETER;
			rt->rt_req_cnt += 1;
		}
	}

	// remember the TTL used  for the next time
	rt->setReqLastTtl(rhdr->ttl());

	// PerHopTime is the roundtrip time per hop for route requests.
	// The factor 2.0 is just to be safe .. SRD 5/22/99
	// Also note that we are making timeouts to be larger if we have 
	// done network wide broadcast before. 
	rt->rt_req_timeout = 2.0 * (double) rhdr->ttl() * perHopTime(rt); 
	if (rt->rt_req_cnt > 0)
		rt->rt_req_timeout *= rt->rt_req_cnt;
	rt->rt_req_timeout += CURRENT_TIME;

	// Don't let the timeout to be too large, however .. SRD 6/8/99
	if (rt->rt_req_timeout > CURRENT_TIME + MAX_RREQ_TIMEOUT)
		rt->rt_req_timeout = CURRENT_TIME + MAX_RREQ_TIMEOUT;
	rt->setExpire(0);

	// Fill out the RREQ packet 
	// ch->uid() = 0;
	ch->ptype() = PT_AODV;
	ch->size() = overheadLength_ + rq->size();
	// [Ali Hamidian bug solve] check for PT_TCP and PT_ACK packet types, they do not need to include IP header length (just addedd by TCP Agent) and overheadLenght_ just include it by default
	if((ch->ptype() == PT_TCP || ch->ptype() == PT_ACK) && (ch->size()>= IP_HDR_LEN)) ch->size()-=IP_HDR_LEN;
	ch->iface() = -2;
	ch->error() = 0;
	ch->addr_type() = NS_AF_NONE;
	ch->prev_hop_ = *((int *)(getAddress(0)->getAddr()) + 1);          // AODV hack
	assert(nAddresses()>0);
	MrclAddress::storeAddr(rhdr->saddr(), getAddress(0)->getAddr());
	assert(broadcastAddr_!=0);
	MrclAddress::storeAddr(rhdr->daddr(), broadcastAddr_->getAddr());
	// Fill up some more fields. 
	rq->rq_type = AODVTYPE_RREQ;
	rq->rq_hop_count = 1;
	rq->rq_bcast_id = bid_++;
	MrclAddress::storeAddr(rq->rq_dst, dst);
	rq->rq_dst_seqno = (rt ? rt->getSeqno() : 0);
	MrclAddress::storeAddr(rq->rq_src, getAddress(0)->getAddr());
	seqno_ += 2;
	assert ((seqno_%2) == 0);
	rq->rq_src_seqno = seqno_;
	rq->rq_timestamp = CURRENT_TIME;

	//  Scheduler::instance().schedule(target_, p, 0.);
	sendDown(p);

}

void AodvModule::sendError(Packet *p, int jitter)
{
	hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);
// 	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);
// 	hdr_mrcl_aodv_error *re = ((hdr_mrcl_aodv_error *)ah->body);
	hdr_mrcl_aodv_error *re = HDR_MRCL_AODV_ERROR(p);
    
	if(debug_)
		fprintf(stderr, "sending Error from %d at %.2f\n", getId(), Scheduler::instance().clock());

	re->re_type = AODVTYPE_RERR;
	//re->reserved[0] = 0x00; re->reserved[1] = 0x00;
	// DestCount and list of unreachable destinations are already filled

	// ch->uid() = 0;
	ch->ptype() = PT_AODV;
	ch->size() = overheadLength_ + re->size();
	// [Ali Hamidian bug solve] check for PT_TCP and PT_ACK packet types, they do not need to include IP header length (just addedd by TCP Agent) and overheadLenght_ just include it by default
	if((ch->ptype() == PT_TCP || ch->ptype() == PT_ACK) && (ch->size()>= IP_HDR_LEN)) ch->size()-=IP_HDR_LEN;
	ch->iface() = -2;
	ch->error() = 0;
	ch->addr_type() = NS_AF_NONE;
	ch->next_hop_ = 0;
	ch->prev_hop_ = *((int *)(getAddress(0)->getAddr()) + 1);          // AODV hack

	MrclAddress::storeAddr(rhdr->saddr(), getAddress(0)->getAddr());
	MrclAddress::storeAddr(rhdr->daddr(), broadcastAddr_->getAddr());

	rhdr->ttl() = 1;
	/*setOldAddresses(p);*/

	// Do we need any jitter? Yes
	if (jitter)
 		sendDown(p, 0.01*Random::uniform());
	else
 		sendDown(p);

}



void AodvModule::forward(Packet *p, Aodv_rt_entry *re, double delay)
{
	struct hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);

	if(rhdr->ttl_ == 0) 
	{
		drop(p, AODV_DROP_RTR_TTL_VERBOSITY, AODV_DROP_RTR_TTL);
		return;
	}
	/*
	if (ch->ptype() != PT_AODV && ch->direction() == hdr_cmn::UP &&
		broadcastAddr_->isEqual(rhdr->daddr()) || isMyAddress(re->getDst())) 
	{
		printf("ATTENTION!!! forward is sending up a packet!!!!\n");
		sendUp(p);
		return;
	}*/

	if (re) 
	{
		assert(re->getFlags() == RTF_UP);
		re->setExpire(CURRENT_TIME + ACTIVE_ROUTE_TIMEOUT);
		MrclAddress::storeAddr(rhdr->nexthop(), re->getNexthop());
		ch->next_hop_ = *((int *)(re->getNexthop()) + 1);
 		ch->addr_type() = NS_AF_INET;
 	}
	else 
	{ // if it is a broadcast packet
		// assert(ch->ptype() == PT_AODV); // maybe a diff pkt type like gaf
		assert(broadcastAddr_->isEqual(rhdr->daddr()));
		ch->addr_type() = NS_AF_NONE;
	}
	ch->xmit_failure_ = aodv_rt_failed_callback;
	ch->xmit_failure_data_ = (void*) this;
	rhdr->clearModuleRoute();
	if (broadcastAddr_->isEqual(rhdr->daddr())) 
	{// If it is a broadcast packet
		assert(re == 0);
		 // According to Marco Fiore proposal: this avoids random jitter on non-AODV broadcasts
		if(ch->ptype() == PT_AODV) {
			/*
			*  Jitter the sending of broadcast packets by 10ms
			*/
			sendDown(p, 0.01 * Random::uniform());
		}
		else
		{
			sendDown(p);
		}
 	}
	else 
	{ // Not a broadcast packet 
		if (debug_>5)
		{
			char addr[20];
			char nexthop[20];
			int ip;
			memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
			memcpy(&ip, rhdr->nexthop() + sizeof(int), sizeof(int));
			sprintf(nexthop,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
			Position* pos = getPosition();
			printf("[AodvModule::forward] node %s send down to %s pkt %d ttl %d num_forwards %d (%f,%f)\n", 
						addr, nexthop, ch->uid(), rhdr->ttl(), ch->num_forwards(), pos->getX(), pos->getY());
		}
		sendDown(p, delay);
   }
}


void AodvModule::nb_insert(char *id)
{
	AddrList *nb = new AddrList;
	MrclAddress::storeAddr(nb->addr, id);

 	//assert(nb);
	nb->expire = CURRENT_TIME + (1.5 * ALLOWED_HELLO_LOSS * HELLO_INTERVAL);
	nb->prev = 0;
	if(!ncache_)
	{
		nb->next = 0;
		ncache_ = nb;
	}
	else
	{
		nb->next = ncache_;
		ncache_->prev = nb;
		ncache_ = nb;
	}
	
 	seqno_ += 2;             // set of neighbors changed
 	assert ((seqno_%2) == 0);
}

AddrList *AodvModule::nb_lookup(char *id)
{
	for(AddrList *cur = ncache_; cur; cur = cur->next)
	{
		if(MrclAddress::areEqual(id, cur->addr))
			return cur;
	}
	return 0;
}

void AodvModule::nb_delete(char *id)
{
	if (debug_>5)
	{
		char addr[20];
		int ip;
		memcpy(&ip, id + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::nb_delete] delete id %s at %.2f (triggered from LL)\n", addr, Scheduler::instance().clock());
	}
	AddrList *nb = nb_lookup(id);
	if(nb)
		nb_delete(nb);
	handle_link_failure(id);
}

void AodvModule::nb_delete(AddrList *a)
{
	if (debug_>5)
	{
		char addr[20];
		int ip;
		memcpy(&ip, a->addr + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::nb_delete] delete id %s at %.2f (triggered from HELLO)\n", addr, Scheduler::instance().clock());
	}
	if(a == ncache_)
	{
		ncache_ = a->next;
		if(ncache_)
			ncache_->prev = 0;
	}
	else
	{
		a->prev->next = a->next;
		if(a->next)
			a->next->prev = a->prev;
	}
	handle_link_failure(a->addr);
	delete a;
}

void AodvModule::nb_purge()
{
	double now = CURRENT_TIME;
	AddrList *cur = ncache_;
	while(cur)
	{
		if (debug_>5)
		{
			char addr[20];
			int ip;
			memcpy(&ip, cur->addr + sizeof(int), sizeof(int));
			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
			char saddr[20];
			memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
			sprintf(saddr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
			printf("Node %s checking %s expire at %f (now %f)\n", saddr, addr, cur->expire, CURRENT_TIME);
		}
		if(cur->expire <= now)
		{
			if(cur != ncache_)
			{
				cur = cur->prev;
				nb_delete(cur->next);
				cur = cur->next;
			}
			else
			{
				nb_delete(cur);
				cur = ncache_;
			}
		}
		else
			cur = cur->next;
	}
}

double AodvModule::perHopTime(Aodv_rt_entry *rt) 
{
	int num_non_zero = 0;
	int i;
	double total_latency = 0.0;

	if (!rt)
		return ((double) NODE_TRAVERSAL_TIME );

	for (i=0; i < MAX_HISTORY; i++) 
	{
		if (rt->getDiscLatency(i) > 0.0) 
		{
			num_non_zero++;
			total_latency += rt->getDiscLatency(i);
		}
	}
	if (num_non_zero > 0)
		return(total_latency / (double) num_non_zero);
	else
	return((double) NODE_TRAVERSAL_TIME);
}

char *AodvModule::getNextHop(Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);
	Aodv_rt_entry *rt = rtable_.rt_lookup(rhdr->daddr());

#ifdef DEBUG
	if (rt==0)
	{
		printf("[AodvModule::getNextHop]  no nexthop\n");
	}
	else
	{
		char addr[20];
		int ip;
		memcpy(&ip, rt->getNexthop() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::getNextHop] forward to %s, path will expire at %f\n", addr, rt->getExpire());
	}
#endif
	return rt->getNexthop();
}

int AodvModule::canIReach(char *a, Metric ***m)
{
	Aodv_rt_entry *rt = rtable_.rt_lookup(a);

	if(!rt)
	{
		#ifdef DEBUG
			printf("if rt = 0, return a fake metric, in order maintain RoutingModule specific\n");
		#endif
		// return a fake metric, in order maintain RoutingModule specific
		HopCountMetric **fake = new HopCountMetric*;
		fake[0] = new HopCountMetric();
		fake[0]->setHopCount(DBL_MAX);
		*m = (Metric **) fake;
		Packet* p;
		return -1;
	}
	if ((rt->getFlags() == RTF_DOWN) || (rt->getFlags() == RTF_IN_REPAIR))
	{
		// return a fake metric, in order maintain RoutingModule specific
		HopCountMetric **fake = new HopCountMetric*;
		fake[0] = new HopCountMetric();
		fake[0]->setHopCount(DBL_MAX);
		*m = (Metric **) fake;
		Packet* p;
		return -1;
	}	
	m_[0]->setHopCount(rt->getHops());
// 	Packet* p;
	*m = (Metric **)m_;
	#ifdef DEBUG
		char addr[20];
		int ip;
		memcpy(&ip, a + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::canIReach] reachability to %s, path will expire at %f\n", addr, rt->getExpire());
	#endif
	return 1;
}


/*
   Neighbor Management Functions
*/

void AodvModule::sendHello()
{
// 	if ((debug_==9)&&(CURRENT_TIME>2.0)) return;			// used in link failure debug
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);
// 	hdr_mrcl_aodv *ah = HDR_MRCL_AODV(p);
// 	hdr_mrcl_aodv_reply *rp =(hdr_mrcl_aodv_reply *)(ah->body);
	hdr_mrcl_aodv_reply *rp = HDR_MRCL_AODV_REPLY(p);
	
	if (debug_)
	{
		char addr[20];
		int ip;
		memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("[AodvModule::sendHello] sending Hello from %s at %.2f\n", addr, Scheduler::instance().clock());
	}
	
	
	rp->rp_type = AODVTYPE_HELLO;
	//rh->rp_flags = 0x00;
	rp->rp_hop_count = 1;
// 	rp->rp_dst = index;
	rp->rp_dst_seqno = seqno_;
	rp->rp_lifetime = (1 + ALLOWED_HELLO_LOSS) * HELLO_INTERVAL;
	MrclAddress::storeAddr(rp->rp_src, getAddress(0)->getAddr());
	MrclAddress::storeAddr(rp->rp_dst, getAddress(0)->getAddr());
	
	// ch->uid() = 0;
	ch->ptype() = PT_AODV;
	ch->size() = overheadLength_ + rp->size();
	// [Ali Hamidian bug solve] check for PT_TCP and PT_ACK packet types, they do not need to include IP header length (just addedd by TCP Agent) and overheadLenght_ just include it by default
	if((ch->ptype() == PT_TCP || ch->ptype() == PT_ACK) && (ch->size()>= IP_HDR_LEN)) ch->size()-=IP_HDR_LEN;
	ch->iface() = -2;				// WHY???
	ch->error() = 0;
	ch->addr_type() = NS_AF_NONE;
	int a;
	getAddress(0)->getAddr(&a, sizeof(int), sizeof(int));
	ch->prev_hop_ = a;          // AODV hack
	
	MrclAddress::storeAddr(rhdr->saddr(), getAddress(0)->getAddr());
	MrclAddress::storeAddr(rhdr->daddr(), broadcastAddr_->getAddr());
	rhdr->ttl() = NETWORK_DIAMETER;
	sendDown(p);
}

