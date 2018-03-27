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

#include"sandbox-rm1.h"
#include"sandbox-metric.h"
#include"../routing-module.h"
#include<ip.h>

static class SandboxRM1Class : public TclClass {
public:
	SandboxRM1Class() : TclClass("Module/SandboxRM1") {}
	TclObject* create(int, const char*const*) {
		return (new SandboxRM1());
	}
} class_sandbox_rm1;

nsaddr_t lastIP = 0;

SandboxRM1::SandboxRM1() : tail_(0), routes_(0)
{
	ipAddr_ = ++lastIP;
}


SandboxRM1::~SandboxRM1()
{
}


int SandboxRM1::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 2)
	{
		if (strcasecmp(argv[1],"addr")==0)
		{
			tcl.resultf("%d", ipAddr_);
			return TCL_OK;
		}
	}
	else if (argc == 3)
	{
		if (strcasecmp (argv[1], "metric") == 0) {
			SandboxMetric *m = new SandboxMetric();
			SandboxMetric **mp = new SandboxMetric*[1];
			mp[0] = m;
			myMetric_ = mp;
			myMetric_[0]->setValue(atof(argv[2]));
			Packet *p;
			if (debug_>5) printf("IPmodule %d has metric %f\n", ipAddr_, myMetric_[0]->value(p));
			return TCL_OK;
		}
	}
	else if (argc == 6) 
	{
		if (strcasecmp (argv[1], "addRoute") == 0) {
// 			if (debug_>10) printf("AddRoute %d:\n", ipAddr_);
			SandboxAddress net;
			net.setAddress((char *)argv[2]);
// 			if (debug_>10) printf("net %s %d ", (char *)argv[2], net.getIpAddr());
			SandboxAddress subnet;
			subnet.setAddress((char *)argv[3]);
// 			if (debug_>10) printf("subnet %s %d ", (char *)argv[3], subnet.getIpAddr());
			int module = atoi(argv[4]);
			SandboxAddress next_hop;
			next_hop.setAddress((char *)argv[5]);
// 			if (debug_>10) printf("netxhop %s %d ", (char *)argv[5], next_hop.getIpAddr());
// 			if (debug_>10) printf("-> module %d\n", module);
			
			RouteEntry  *r = new RouteEntry;
			r->net = net;
			r->mask = subnet;
			r->module = module;
			r->next_hop = next_hop;
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
			return TCL_OK;
		}
	}
	return (MrclRouting::command(argc,argv));
}


char* SandboxRM1::getNextHop(Packet *p)
{
	hdr_ip *iph = HDR_IP(p);
	hdr_cmn *ch = HDR_CMN(p);
	//printf("[SandboxRM1::getNextHop] Ip %d Try to find nexthop for %d:\n", ipAddr_, iph->daddr());
	for(RouteEntry *cur = routes_; cur; cur = cur->next)
	{
		//printf("try with %d -> ", cur->next_hop.getIpAddr());
		if((iph->daddr() & cur->mask.getIpAddr()) == cur->net.getIpAddr())
		{
			//printf("GOT IT\n");
			return(cur->next_hop.getAddr());
		}
		//printf("NO\n");
	}
	drop(p, 1, STATICROUTE_NO_ROUTE);
}


int SandboxRM1::canIReach(char *a, Metric ***m)
{
	//printf("[SandboxRM1::canIReach] Ip %d Try to find nexthop for %d:\n", ipAddr_, ((SandboxAddress *)a)->getIpAddr());
	for(RouteEntry *cur = routes_; cur; cur = cur->next)
	{
		//printf("try with %d -> ", cur->next_hop.getIpAddr());
		if((((SandboxAddress *)a)->getIpAddr() & cur->mask.getIpAddr()) == cur->net.getIpAddr())
		{
			Packet *p;
			//printf("I CAN REACH %f\n", myMetric_[0]->value(p));
			*m = (Metric **)(myMetric_);
			return(1);
		}
		//printf("NO\n");
	}
	//printf("Can not reach such a destination\n");
	return(0);
}


void SandboxRM1::forward(Packet *p)
{
	RoutingHdr* mrhdr = HDR_ROUTING(p);
	hdr_ip *iph = HDR_IP(p);
	hdr_cmn *ch = HDR_CMN(p);
	char destAddr[MRCL_ADDRESS_MAX_LEN];
	sprintf(destAddr,"%d.%d.%d.%d", (iph->daddr() & 0xff000000)>>24,(iph->daddr() & 0x00ff0000)>>16, (iph->daddr() & 0x0000ff00)>>8, (iph->daddr() & 0x000000ff));
	printf("[SandboxRM1::forward] Ip %d Try to find nexthop for %d (%s):\n", ipAddr_, iph->daddr(), destAddr);
	SandboxAddress dest;
	dest.setAddress(destAddr);
	if (dest.getIpAddr()==ipAddr_)
	{
		printf("My packet %d -> send up\n", ch->uid());
		sendUp(p);
	}
	else
	{
		resolve(p);
	}
}

void SandboxRM1::resolve(Packet* p)
{
	// packet not for me -> try to solve it
	RoutingHdr* mrhdr = HDR_ROUTING(p);
	hdr_ip *iph = HDR_IP(p);
	hdr_cmn *ch = HDR_CMN(p);
	char destAddr[MRCL_ADDRESS_MAX_LEN];
	sprintf(destAddr,"%d.%d.%d.%d", (iph->daddr() & 0xff000000)>>24,(iph->daddr() & 0x00ff0000)>>16, (iph->daddr() & 0x0000ff00)>>8, (iph->daddr() & 0x000000ff));
	printf("[SandboxRM1::forward] Ip %d Try to find nexthop for %d (%s):\n", ipAddr_, iph->daddr(), destAddr);
	SandboxAddress dest;
	dest.setAddress(destAddr);
	int nRoute = getRoute(&dest, p, 0);
	if (nRoute>0)
	{
		// packet can be forwarded
		printf("IP %d Discovered routes %d:\n", ipAddr_, nRoute);
		printf("\t0 to reach %s through module %d\n", mrhdr->daddr(), mrhdr->getModuleRoute(mrhdr->moduleRouteCount() - 1));
		for(int i=1; i<nRoute; i++)
		{
			getRoute(&dest, p, i);
			printf("\t%d to reach %s through module %d\n", i, mrhdr->daddr(), mrhdr->getModuleRoute(mrhdr->moduleRouteCount() - 1));
		}
		// forward to the 1st one
		getRoute(&dest, p, 0); // re-set packet header parameters
		if (mrhdr->getModuleRoute(mrhdr->moduleRouteCount() - 1)==getId())
		{
			// this module can forward the packet directly
			mrhdr->clearModuleRoute();
			char *nextHop = getNextHop(p);
			if(nextHop)
			{
				MrclAddress::storeAddr(mrhdr->nexthop(), nextHop);
				ch->next_hop() = (nsaddr_t)(mrhdr->nexthop()+sizeof(int));
				// here the packet has to be solved from the leaf module
			}
			for(RouteEntry *cur = routes_; cur; cur = cur->next)
			{
				if((dest.getIpAddr() & cur->mask.getIpAddr()) == cur->net.getIpAddr())
				{
					printf("[FORWARDING1] IP %d to %s module %d nexthop %d\n\n", ipAddr_, destAddr, cur->module, cur->next_hop.getIpAddr());
					hdr_ip *iph = HDR_IP(p);
					hdr_cmn *ch = HDR_CMN(p);
					int id = 0;
					for(RouteEntry *cur = routes_; cur; cur = cur->next)
					{
						if((iph->daddr() & cur->mask.getIpAddr()) == cur->net.getIpAddr())
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
			printf("[FORWARDING2] IP %d sendUp to another routing module %d\n\n", ipAddr_, dest.getIpAddr());
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




int SandboxRM1::controlPacket(Packet *p)
{
	return(FALSE);
}
