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

#include"mrcl-routing-static.h"
#include"routing-module.h"
#include<hop-count-metric.h>
#include<ip.h>

static class MrclRoutingStaticClass : public TclClass {
public:
	MrclRoutingStaticClass() : TclClass("Routing/MrclRoutingStatic") {}
	TclObject* create(int, const char*const*) {
		return (new MrclRoutingStatic());
	}
} class_mrcl_routing_static;

MrclRoutingStatic::MrclRoutingStatic() 
  : 
    routes_(0),
    tail_(0),
    m_(0)
{
	m_ = new HopCountMetric*;
	m_[0] = new HopCountMetric();
	m_[0]->setHopCount(1);
}


MrclRoutingStatic::~MrclRoutingStatic()
{
	delete m_[0];
	delete m_;
}


int MrclRoutingStatic::command(int argc, const char*const* argv)
{
//	Tcl& tcl = Tcl::instance();
// 	if(argc == 2)
// 	{
// 		if (strcasecmp(argv[1],"addr")==0)
// 		{
// 			tcl.resultf("%d", ipAddr_);
// 			return TCL_OK;
// 		}
// 	}
// 	else if (argc == 3)
// 	{
// 		if (strcmp(argv[1],"setAddr")==0)
// 		{
// 			IPAddress *addr = (IPAddress *)(TclObject::lookup(argv[2]));
// 			if (addr==0)
// 			{
// 				printf("AodvModule::command(%s), error in address lookup\n", argv[1]);
// 				return TCL_ERROR;
// 			}
// 			addr_ = addr;
// 			return TCL_OK;
// 		}
// 		else if (strcasecmp (argv[1], "metric") == 0)
// 		{
// 			SandboxMetric *m = new SandboxMetric();
// 			SandboxMetric **mp = new SandboxMetric*[1];
// 			mp[0] = m;
// 			myMetric_ = mp;
// 			myMetric_[0]->setValue(atof(argv[2]));
// 			Packet *p;
// 			if (debug_>5) printf("IPmodule %d has metric %f\n", ipAddr_, myMetric_[0]->value(p));
// 			return TCL_OK;
// 		}
// 	}
	if (argc == 6) 
	{
		if (strcasecmp (argv[1], "addRoute") == 0)
		{
			IPAddress *net = (IPAddress *)(TclObject::lookup(argv[2]));
			IPAddress *mask = (IPAddress *)(TclObject::lookup(argv[3]));
			IPAddress *nextHop = (IPAddress *)(TclObject::lookup(argv[4]));
			if ((net==0)||(mask==0)||(nextHop==0))
			{
				printf("MrclRoutingStatic::command(%s), error in address lookup\n", argv[1]);
				return TCL_ERROR;
			}
			int module = atoi(argv[5]);
			MRS_RouteEntry  *r = new MRS_RouteEntry;
			r->net = net;
			r->mask = mask;
			r->module = module;
			r->next_hop = nextHop;
			r->next = 0;
			if(tail_)
			{
				tail_->next = r;
			}
			else
			{
				routes_ = r;
			}
			tail_ = r;
			int len = net->strlen() + 2;
			char *neta = new char[len];
			net->toString(neta, len);
			len = mask->strlen() + 2;
			char *maska = new char[len];
			mask->toString(maska, len);
			len = nextHop->strlen() + 2;
			char *nexthopa = new char[len];
			nextHop->toString(nexthopa, len);
			printf("Add Route: net %s mask %s nexthop %s\n", neta, maska, nexthopa);
			return TCL_OK;
		}
	}
	return (MrclRouting::command(argc,argv));
}


char* MrclRoutingStatic::getNextHop(Packet *p)
{
//	hdr_ip *iph = HDR_IP(p);
//	hdr_cmn *ch = HDR_CMN(p);
	RoutingHdr *rhdr = HDR_ROUTING(p);
	//printf("[MrclRoutingStatic::getNextHop] Ip %d Try to find nexthop for %d:\n", ipAddr_, iph->daddr());
	for(MRS_RouteEntry *cur = routes_; cur; cur = cur->next)
	{
		//printf("try with %d -> ", cur->next_hop.getIpAddr());
// 		if((iph->daddr() & cur->mask->getIpAddr()) == cur->net.getIpAddr())
		int len = cur->net->strlen() + 2;
		char *neta = new char[len];
		cur->net->toString(neta, len);
		len = cur->mask->strlen() + 2;
		char *maska = new char[len];
		cur->mask->toString(maska, len);
		len = cur->next_hop->strlen() +2;
		char *nexthopa = new char[len];
		cur->next_hop->toString(nexthopa, len);
		char addr[20];
		int ip;
		memcpy(&ip, rhdr->daddr() + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));

		#ifdef DEBUG
			printf("STATIC-getnetxhop\n");
			printf("\tckecking cur %p net %s mask %s addr %s netxhop %s\n", cur, neta, maska, addr, nexthopa);
			printf("\tcur->mak->AND(a)=%d cur->net->INT()=%d\n", cur->mask->AND(rhdr->daddr()), cur->net->INT());
		#endif

		if (cur->mask->AND(rhdr->daddr()) == cur->net->INT())
		{
			#ifdef DEBUG
				printf("GOT IT! return\n");
			#endif
			//ch->next_hop()=cur->net->INT();
			return(cur->next_hop->getAddr());
		}
		#ifdef DEBUG
			printf("NO matches...\n");
		#endif
	}
	printf("No route, drop\n");

	/// Mior 10 dic 2008 No more drop! If I drop a packet and then I try to solve it (because I have no nextHop), is wrong! Maybe reachable, if route definitively unreachable, drop...
	//printf("%s drop\n", __PRETTY_FUNCTION__);
	drop(p, 1, STATICROUTE_NO_ROUTE);
	return 0;
}


int MrclRoutingStatic::canIReach(char *a, Metric ***m)
{
	//Sprintf("[MrclRoutingStatic::canIReach] Ip %d Try to find nexthop for %d:\n", ipAddr_, ((IPAddress *)a)->getIpAddr());
	for(MRS_RouteEntry *cur = routes_; cur; cur = cur->next)
	{
		//printf("try with %d -> ", cur->next_hop.getIpAddr());
// 		if((((IPAddress *)a)->getIpAddr() & cur->mask.getIpAddr()) == cur->net.getIpAddr())

		int len = cur->net->strlen() + 2;
		char *neta = new char[len];
		cur->net->toString(neta, len);
		len = cur->mask->strlen() + 2;
		char *maska = new char[len];
		cur->mask->toString(maska, len);
		char addr[20];
		int ip;
		memcpy(&ip, a + sizeof(int), sizeof(int));
		sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
		printf("ckecking cur %p net %s mask %s addr %s\n", cur, neta, maska, addr);
		printf("cur->mak->AND(a)=%d cur->net->INT()=%d\n", cur->mask->AND(a), cur->net->INT());
		if ( cur->mask->AND(a) == cur->net->INT())
		{
			Packet *p = NULL;
			printf("mrs I CAN REACH %f\n", m_[0]->value(p));
			*m = (Metric **)m_;
			return(1);
		}
		printf("NO reachability\n");
	}
	printf("Can not reach such a destination\n");
	return(0);
}


void MrclRoutingStatic::forward(Packet *p)
{
	RoutingHdr* mrhdr = HDR_ROUTING(p);
	hdr_ip *iph = HDR_IP(p);
	hdr_cmn *ch = HDR_CMN(p);
	char destAddr[MRCL_ADDRESS_MAX_LEN];
	char myaddr[MRCL_ADDRESS_MAX_LEN];
	int ip;
	sprintf(destAddr,"%d.%d.%d.%d", (iph->daddr() & 0xff000000)>>24,(iph->daddr() & 0x00ff0000)>>16, (iph->daddr() & 0x0000ff00)>>8, (iph->daddr() & 0x000000ff));
	memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
	sprintf(myaddr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
	printf("[MrclRoutingStatic::forward] Ip %s Try to find nexthop for %d (%s):\n", myaddr, iph->daddr(), destAddr);

	if (isMyAddress(mrhdr->daddr()))
	{
		printf("My packet %d -> send up\n", ch->uid());
		sendUp(p);
	}
	else
	{
		resolve(p);
	}
}

void MrclRoutingStatic::resolve(Packet* p)
{
	// packet not for me -> try to solve it
	RoutingHdr* mrhdr = HDR_ROUTING(p);
	hdr_ip *iph = HDR_IP(p);
	hdr_cmn *ch = HDR_CMN(p);
	char destAddr[MRCL_ADDRESS_MAX_LEN];
	char myaddr[MRCL_ADDRESS_MAX_LEN];
	int ip;
	sprintf(destAddr,"%d.%d.%d.%d", (iph->daddr() & 0xff000000)>>24,(iph->daddr() & 0x00ff0000)>>16, (iph->daddr() & 0x0000ff00)>>8, (iph->daddr() & 0x000000ff));
	memcpy(&ip, getAddress(0)->getAddr() + sizeof(int), sizeof(int));
	sprintf(myaddr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
	printf("[MrclRoutingStatic::resolve] Ip %s Try to find nexthop for %d (%s):\n", myaddr, iph->daddr(), destAddr);
	
	int nRoute = getRoute(mrhdr->daddr(), p, 0);
	if (nRoute>0)
	{
		// packet can be forwarded
		printf("IP %s Discovered routes %d:\n", myaddr, nRoute);
		printf("0 to reach %s through module %d\n", mrhdr->daddr(), mrhdr->getModuleRoute(mrhdr->moduleRouteCount() - 1));
		for(int i=1; i<nRoute; i++)
		{
			getRoute(mrhdr->daddr(), p, i);
			printf("\t%d to reach %s through module %d\n", i, mrhdr->daddr(), mrhdr->getModuleRoute(mrhdr->moduleRouteCount() - 1));
		}
		// forward to the 1st one
		getRoute(mrhdr->daddr(), p, 0); // re-set packet header parameters
		if (mrhdr->getModuleRoute(mrhdr->moduleRouteCount() - 1)==getId())
		{
			// this module can forward the packet directly
			mrhdr->clearModuleRoute();
			char *nextHop = getNextHop(p);
			if(nextHop)
			{
				IPAddress::storeAddr(mrhdr->nexthop(), nextHop);
//				ch->next_hop() = (nsaddr_t)(mrhdr->nexthop()+sizeof(int)); COMPILER ERROR ON 64 BIT MACHINES 
				ch->next_hop() = *((nsaddr_t*)(mrhdr->nexthop()+sizeof(int)));
				// here the packet has to be solved from the leaf module
			}
			for(MRS_RouteEntry *cur = routes_; cur; cur = cur->next)
			{
// 				if((dest.getIpAddr() & cur->mask.getIpAddr()) == cur->net.getIpAddr())
				if ( cur->mask->AND(mrhdr->daddr()) == cur->net->INT())
				{
					printf("[FORWARDING1] IP %s to %s module %d nexthop %d\n\n", myaddr, destAddr, cur->module, cur->next_hop->INT());
					hdr_ip *iph = HDR_IP(p);
					int id = 0;
					for(MRS_RouteEntry *cur = routes_; cur; cur = cur->next)
					{
						if((iph->daddr() & cur->mask->INT()) == cur->net->INT())
						{
							id = cur->module;
						}
					}
					sendDown(id,p);
					return;
				}
			}
		}
		else
		{
			// this packet has to be forwarded to other routing module
			printf("[FORWARDING2] IP %s sendUp to another routing module\n", myaddr);
		// 				mrhdr->setSendup();
			sendUp(p);
		}
	}
	else
	{
		// packet can not be forwarded
		drop(p, STATICROUTE_NO_ROUTE_VERBOSITY, STATICROUTE_NO_ROUTE);
	}
}




int MrclRoutingStatic::controlPacket(Packet *p)
{
	return(FALSE);
}
