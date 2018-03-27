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

#include "routing-module.h"
#include<ip.h>

#define DEBUG

RouteReachable::RouteReachable(int source) : ClMessage(ROUTEREACHABLE_VERBOSITY, CL_ROUTEMESSAGE, UNICAST, 0), info_(0), nInfo_(0), infoLen_(0), modules_(0), nModules_(0), myIP_(0), processed_(0)
{
	memset(addr_, 0, MRCL_ADDRESS_MAX_LEN);
	setSource(source);
	printf("[RR Constr] setto addr_ a 0 (%d)\n", addr_);
}

RouteReachable::~RouteReachable()
{
	if(infoLen_)
		delete [] info_;
}

void RouteReachable::setAddress(char *a)
{
	MrclAddress::storeAddr(addr_, a);

}

char *RouteReachable::getAddress()
{
	return addr_;
}

void RouteReachable::addRouteInfo(RouteInfo *i)
{

/// NOTE where do I check for reachability?


 	int id = i->getModuleId();

	if(id < nModules_ && modules_[id]>=0)
	{
		info_[modules_[id]] = i;
		printf("modules_[%d] %d - info_[modules_[id]] = %d\n",id, modules_[id], info_[modules_[id]]);
		return;
	}
	if(nInfo_ >= infoLen_)
	{printf("Route info nInfo(%d) > infoLen(%d)\n", nInfo_, infoLen_);
		RouteInfo **tmp = new RouteInfo*[infoLen_ + ROUTE_REACHABLE_ALLOC_INFO_PER_TIME];
		for(int j = 0; j < infoLen_; j++)
		{
			if(j < nInfo_)
				tmp[j] = info_[j];
		}
		if(infoLen_ > 0)
			delete [] info_;
		info_ = tmp;
		infoLen_ += ROUTE_REACHABLE_ALLOC_INFO_PER_TIME;
	}

	info_[nInfo_++] = i;
	printf("Ora nInfo %d, infoLen %d\n", nInfo_, infoLen_);
	//int id = i->getModuleId();
	if(id >= nModules_)
	{
		int *tmp = new int[id + 1];
		for(int j = 0; j <= id; j++)
		{
			if(j < nModules_)
				tmp[j] = modules_[j];
			else
				tmp[j] = -1;
		}
		if(nModules_ > 0)
			delete [] modules_;
		modules_ = tmp;
		nModules_ = id + 1;
	}


	modules_[id] = nInfo_ - 1;

}

int RouteReachable::length()
{
	return nInfo_;
}

RouteInfo *RouteReachable::getRouteInfo(int id)
{
	if(id >= nModules_)
		return 0;
	int i = modules_[id];
 	//printf("module_[%d] %d\n", id, modules_[id]);
	if(i < 0)
		return 0;
 	//printf("info_[i] %p\n", info_[i]);
	return info_[i];
}

RouteInfo **RouteReachable::getRouteInfo()
{
	return info_;
}

void RouteReachable::empty()
{
	printf("[EMPTY] nInfo %d, infoLen %d\n", nInfo_, infoLen_);
	//memset(info_, 0, sizeof(RouteInfo) * nInfo_);
	if(infoLen_!=0)
	delete [] info_;
	myIP_ = 0;
	nInfo_ = 0;
	infoLen_ = 0;
	for(int j = 0; j < nModules_; j++)
		modules_[j] = -1;
// andrea mior dic 16th 2008
	nModules_ = 0;
}

void RouteReachable::setMyIP()
{
	myIP_ = 1;
}

int RouteReachable::isMyIP()
{
	return myIP_;
}

void RouteReachable::processed(int value)
{
	processed_ = value;
}

int RouteReachable::processed()
{
	return processed_;
}

int RouteReachable::getIndex(int id)
{
	if(id >= nModules_)
		return -1;
	return modules_[id];
}

// RouteReachableTracer::RouteReachableTracer() : ClMessageTracer(CL_ROUTEMESSAGE)
// {
// }
// 
// void RouteReachableTracer::format(ClMessage *m, ClSAP *sap)
// {
// 	if (m->type()!=CL_ROUTEMESSAGE)
// 		return;
// 	
// 	writeTrace(sap, " [CL_ROUTE_MSG] ");
// }




MrclRouting::MrclRouting() : rr_(0), lastGetConfiguration_(-1), delayUp_(0.0), delayDown_(0.0), nAddresses_(0)
{
	bind("overheadLength_", &overheadLength_);
}

MrclRouting::~MrclRouting()
{
	if(rr_)
		delete rr_;
}

int MrclRouting::command(int argc, const char *const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 3)
	{
		if(strcasecmp(argv[1], "id_") == 0)
		{
			int id = atoi(argv[2]);
			rr_ = new RouteReachable(id);
		}
		else if (strcasecmp(argv[1],"getAddr")==0)
		{
			int addrNum = atoi(argv[2]);
			if (addrNum-1>nAddresses())
			{
				printf("Error AodvModule %s command, ask for an unexisting address (%d, while max is %d)\n", argv[1], addrNum, nAddresses());
				return TCL_ERROR;
			}
			MrclAddress *addr = getAddress(addrNum-1);
			if (addr==0)
				return TCL_ERROR;
			int len = addr->strlen() + 2;
			char *str = new char[len];
			addr->toString(str, len);
			tcl.resultf("%s", str);
			return TCL_OK;
		}
		else 	if (strcasecmp(argv[1],"setAddr")==0)
		{
			MrclAddress *addr = (MrclAddress *)(TclObject::lookup(argv[2]));
			if (addr==0)
			{
				printf("RoutingModule::command(%s), error in address lookup\n", argv[1]);
				return TCL_ERROR;
			}
			int len = addr->strlen() + 2;
			char *str = new char[len];
			addr->toString(str, len);
			addAddress(addr);
			if(debug_>5)
			{
				printf("RoutingModule::command(%s) -- setAddr(%d) = %s\n", argv[1], nAddresses(), str);
			}
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}

int MrclRouting::getConfiguration(char *a, int source)
{
	for(int t = 0; t < nAddresses_; t++)
	{
		if(MrclAddress::areEqual(a, addresses_[t]->getAddr()))
		{
			return -1;
		}
	}

	// this empty() call had caused a memory leak (now fixed) because all Routing tables refer through pointers to rr_ values. The rr_ deleting, invalidates pointers.
	if(source < 0){
		routes_.clear();
		rr_->empty();
	}
	
	rr_->setAddress(a);
	Metric **m;
	int r = canIReach(a, &m);

	RouteInfo *i = new RouteInfo();
	i->setModuleId(getId());
	if(r != 0)
	{
		if(r > 0)
		{
			#ifdef DEBUG
				printf("ROUTE REACHABLE\n");
			#endif
			i->setReachability(REACHABLE);
		}
		else
		{
			#ifdef DEBUG
				printf("ROUTE MAYBE REACHABLE\n");
			#endif
			i->setReachability(MAYBE);
			r *= -1;
		}

		for(int j = 0; j < r; j++)
		{
			i->addMetric(m[j]);
		}
	}
	else
	{
		#ifdef DEBUG
			printf("ROUTE UNREACHABLE\n");
		#endif
		i->setReachability(UNREACHABLE);	
	}


/// NOTE here we do addRouteInfo even if route unreachable!!!

	//if(i->reachability()!=UNREACHABLE){

	rr_->addRouteInfo(i);
	//}
	int nFathers = 0;
	int nChildren = 0;
	#ifdef DEBUG
		printf("Try to find Children(%d) and fathers(%d) - id: %d\n", getDownLaySAPnum(), getUpLaySAPnum(), getId());
	#endif
	for(int j = 0; j < getUpLaySAPnum(); j++)
	{
		int id = getUpLaySAP(j)->getModuleUpId();
		#ifdef DEBUG
 			printf("Try to upSAP %d module %d source %d\n", j, id, source);
		#endif
		if(id != source)
		{
			rr_->processed(0);
			#ifdef DEBUG
 				printf("-Father(s)- Try to send to %d - id %d\n", id, getId());
			#endif
			rr_->setDest(id);
			sendSyncClMsgUp(rr_);
			if(rr_->isMyIP())
			{
				return -1;
			}
		}
		#ifdef DEBUG
			printf("father Processed: %d - id %d\n", rr_->processed(), getId());
		#endif
		if(rr_->processed() || (id >= 0 && id == source))
		{	nFathers++;
			#ifdef DEBUG
				printf("\t\t\tmodule %d addfather %d (source=%d) %p\n",getId(),id,source, rr_->getRouteInfo(id));
			#endif
			rr_->getRouteInfo(getId())->addFather(rr_->getRouteInfo(id));
		}
		else
		{
			#ifdef DEBUG
				printf("RM %d module %d is not a father :-(\n", getId(), getDownLaySAP(j)->getModuleUpId());
			#endif
		}
	}
	for(int j = 0; j < getDownLaySAPnum(); j++)
	{
		int id = getDownLaySAP(j)->getModuleDownId();
		if(id != source)
		{
			rr_->processed(0);
			#ifdef DEBUG
				printf("-Child- Try to send to %d - id %d\n", id, getId());
			#endif
			rr_->setDest(id);
			sendSyncClMsgDown(rr_);
			if(rr_->isMyIP())
			{
				return -1;
			}
		}
		#ifdef DEBUG
			printf("child Processed: %d - id %d\n", rr_->processed(), getId());
		#endif
		if(rr_->processed() || (id >= 0 && id == source))
		{	nChildren++;
			#ifdef DEBUG
				printf("\t\t\tmodule %d addchild %d (source=%d)\n",getId(),id,source);
			#endif
			rr_->getRouteInfo(getId())->addChild(rr_->getRouteInfo(id));
		}
		else
		{
			#ifdef DEBUG
				printf("RM %d Module %d is not child :-(\n", getId(), getDownLaySAP(j)->getModuleDownId());
			#endif
		}
	}

// Mior, 12 2008.

	if(nFathers + nChildren <= 1 && (i->reachability() == UNREACHABLE)){
		#ifdef DEBUG
		printf("I'm a module with no enough connections:\n\tI'm a leaf/node with no connections and, through me, route is unreachable\n\tfathers %d child %d, route %s\n\tI should return %d\n\n", nFathers, nChildren, (i->reachability()==UNREACHABLE)?"UNREACHABLE":"(M)Reachable", rr_->length());
		#endif
/// NOTE I must tell the sender of the message that I'm NOT a good module to complete the path toward destination!
/// doing this trick This module is not included in the path. Anyway, I would prefer a better solution...
/// NOTE devo anche dire che per questa via la strada e' unreachable!
/// Il problema e' nato nel caso in cui il modulo che ha la route e' interrogato dopo il modulo che NON ha la route. Il modulo senza route viene inserito nel path e successivamente viene inserito ANCHE quello con la route, sebbene i due moduli, inseriti uno di seguito all'altro, non siano direttamente comunicanti.
		return rr_->length()-1;
	}
	else
	return rr_->length();

}

int MrclRouting::recvSyncClMsg(ClMessage *m)
{
	if(m->type() == CL_ROUTEMESSAGE)
	{
		RouteReachable *r = (RouteReachable *)m;
		r->processed(1);
		RouteInfo **info = r->getRouteInfo();
		routes_.clear();
		rr_->empty();
		for(int i = 0; i < r->length(); i++)
		{
			rr_->addRouteInfo(info[i]);
		}

		#ifdef DEBUG
			printf("RoutingModule %d has rx a CL_ROUTEMESSAGE\n", getId());
									/// ATTENZIONE!
			printf("recvSyncClMsg. Call getConfiguration with addr %d\n", ((RouteReachable *)m)->getAddress());
		#endif
		int nInfo = getConfiguration(((RouteReachable *)m)->getAddress(), m->getSource());
		#ifdef DEBUG
			printf("\tFill the SyncClMsg with nInfo = %d\n", nInfo);
		#endif
		if(nInfo < 0)
		{
			((RouteReachable *)m)->setMyIP();
		}
		else
		{
			#ifdef DEBUG
				printf("nInfo!=0 (%d)\n", nInfo);
			#endif
			for(int i = 0; i < nInfo; i++)
			{
				((RouteReachable *)m)->addRouteInfo((rr_->getRouteInfo())[i]);
			}
		}

		return 0;
	}
	return Module::recvSyncClMsg(m);
}

int MrclRouting::findNextLeaf(int i)
{
	RouteInfo **info = rr_->getRouteInfo();
	for(int j = i + 1; j < rr_->length(); j++)
	{
 		//printf("\t\ti=%d, j=%d, id=%d, nch=%d\n", i, j, info[j]->getModuleId(), info[j]->getNChild());
		if(info[j]->getNChild() == 0)
			return info[j]->getModuleId();
	}
	return -1;
}

class MetricComparator : public Comparator
{
public:
	virtual int isLess(void *a, void *b)
	{
		ModuleRoute *ma = (ModuleRoute *)a;
		ModuleRoute *mb = (ModuleRoute *)b;
		return (ma->w < mb->w);
	}
};

int MrclRouting::getRoute(MrclAddress *a, Packet *p, int i)
{
	getRoute(a->getAddr(), p, i);
}

int MrclRouting::getRoute(char *a, Packet *p, int i)
{
#ifdef DEBUG
//####################
	int saddrLen;
	memcpy(&saddrLen, a, sizeof(int));
	//if (saddrLen==0) return; // routing addresses not yet initialized
	char saddr[MRCL_ADDRESS_MAX_LEN] = "";
	char temp[10];
	for(int ki=saddrLen-1; ki>=0; ki--)
	{
		if (ki==0)
			sprintf(temp,"%d", a[ki+sizeof(int)]);
		else
			sprintf(temp,"%d.", a[ki+sizeof(int)]);
		strcat(saddr,temp);
	}
	strcat(saddr,"\0");
	char daddr[MRCL_ADDRESS_MAX_LEN] = "";
	int daddrLen;
	if (daddrLen>0)
	{
		memcpy(&daddrLen, rr_->getAddress(), sizeof(int));
		strcpy(temp,"");
		for(int ki=daddrLen-1; ki>=0; ki--)
		{
			if (ki==0)
				sprintf(temp,"%d", rr_->getAddress()[ki+sizeof(int)]);
			else
				sprintf(temp,"%d.", rr_->getAddress()[ki+sizeof(int)]);
			strcat(daddr,temp);
		}
	}
	strcat(daddr,"\0");
//####################
	printf("Last config %f = %f now   &&   a %s = %s\n", lastGetConfiguration_, Scheduler::instance().clock(), saddr, daddr);
#endif
	if(lastGetConfiguration_ == Scheduler::instance().clock() && MrclAddress::areEqual(a, rr_->getAddress()))
	{
		//if we have already compute the path
		#ifdef DEBUG
			printf("if we have already computed the path\n");
		#endif
		if(rr_->isMyIP())
		{
			RoutingHdr *rhdr = HDR_ROUTING(p);
			rhdr->setSendup();
			return -1;
		}
		if(i < routes_.count())
		{
			RoutingHdr *rhdr = HDR_ROUTING(p);
			rhdr->clearModuleRoute();
			ModuleRoute *mr = (ModuleRoute *)routes_.get(i);
			for(int k = mr->modules.count() - 1; k >= 0; k--)
			{
				RouteInfo *info = (RouteInfo *)(mr->modules.get(k));
				rhdr->addModuleRoute(mr->modules.count() - 1 - k, info->getModuleId());
			}
		}

		return routes_.count();
	}
	//else
	//getConfiguration(a, getId());

	#ifdef DEBUG
		printf("If path not computed (or too old), call getConfiguration(a, -1)\n");
	#endif

	//routes_.clear();
	int nm = getConfiguration(a);

	lastGetConfiguration_ = Scheduler::instance().clock();

	if(rr_->isMyIP())
	{
		RoutingHdr *rhdr = HDR_ROUTING(p);
		rhdr->setSendup();
		return -1;
	}
	int j = findNextLeaf();

	// Dealloc
	printf("RoutesCount %d, next leaf %d. Sono nel modulo %d\n",routes_.count(), j, getId());

// Mior 01, 2009 This delete statement has been removed

// 	for(int k = 0; k < routes_.count(); k++)
// 	{	printf("k= %d\n", k);
// 		ModuleRoute *tmp = (ModuleRoute *)routes_.get(k);
// 		printf("Get, now delete (tmp %p R %f)\n", tmp, ((ModuleRoute *)routes_.get(k))->w);
// 		//we cannot use delete function here! It generates a memory leak! I try to delete a pointer already deallocated! It becomes inconsistent when I empty the routing message rr_ (they are all pointers to pointers)
// 		delete tmp;
// 		printf("Successfully deleted\n");
// 	}

// this function has been modified to avoid the previous "delete tmp;" statement.
// The clear() function includes a delete[] call
	routes_.clear();
	int c = 0;
	while(j >= 0)
	{
		RouteInfo *info = rr_->getRouteInfo(j);
		if(info->reachability() != UNREACHABLE)
		{	
			//printf("route ! Unreachable\n");
			RouteInfo *f = info->getFather();
			MetricList l;
			ModuleRoute *mr = new ModuleRoute;
			mr->modules.set(0,(void *)info);
			for(int k = 0; k < info->getNMetric(); k++)
			{
				l.addMetric(info->getMetric(k));
			}
			int d = 1;
			while(f)
			{
				mr->modules.set(d++,(void *)f);
				#ifdef DEBUG
 					printf("\tFIND A FATHER %d (%i)\n", f->getModuleId(), mr->modules.count());
				#endif
				if(f->getNMetric() > 0)
				{
					for(int k = 0; k < f->getNMetric(); k++)
					{
						f->getMetric(k)->clearParam();
						f->getMetric(k)->insertParam(&l);
					}
					l.empty();
					for(int k = 0; k < f->getNMetric(); k++)
					{
						l.addMetric(f->getMetric(k));
					}
				}
				f = f->getFather();
			}
			double w = 0.0;
			for(int k = 0; k < l.length(); k++)
			{
				w += l.getMetric(k)->value(p);
			}
			mr->w = w;
			mr->reachability = info->reachability();
 			//if (debug_>10)
				int ci = c+1;
				#ifdef DEBUG
					printf("\n\n[RoutingModule] Module %d Add new %dth route (j=%d) metric %f\n\n", getId(), ci, j, mr->w);
				#endif

			routes_.set(c,(void *)mr);
			c++;
		}
		j = findNextLeaf(rr_->getIndex(j));
	}
	// Resort Routes
	MetricComparator comp;
//	for(int k = 0; k< routes_.count(); k++)
//		printf("BEFORE_SORT route %d metric %f\n", k, ((ModuleRoute *)routes_.get(k))->w);
	routes_.sort(&comp);
//	for(int k = 0; k< routes_.count(); k++)
//		printf("AFTER_SORT route %d etric %f (%d) pointer %p\n", k, ((ModuleRoute *)routes_.get(k))->w, ((ModuleRoute *)routes_.get(k))->modules.count(), routes_.get(k));

	// Return the modules
	int ret = routes_.count();

	if(i < ret)
	{
		RoutingHdr *rhdr = HDR_ROUTING(p);
		rhdr->clearModuleRoute();
		rhdr->setSendup();
		ModuleRoute *mr = (ModuleRoute *)routes_.get(i);
		for(int k = mr->modules.count() - 1; k >= 0; k--)
		{
			RouteInfo *info = (RouteInfo *)(mr->modules.get(k));

			rhdr->addModuleRoute(mr->modules.count() - 1 - k, info->getModuleId());
			#ifdef DEBUG
				printf("[RoutingModule::getRoute] - ADDMODULEROUTE %d (%d)\n", info->getModuleId(), mr->modules.count());
			#endif
		}
		if (mr->reachability==MAYBE){
			#ifdef DEBUG
				printf("[getRoute] Reachability maybe\n");
			#endif
			ret *= -1;}
	}
	return ret;
}


void MrclRouting::recv(Packet *p)
{

	if (controlPacket(p))
	{
		// this is a control packet of this routing module -> just processed it -> drop it
		//drop(p, CTRL_PKT_DROP_VERBOSITY, CTRL_PKT_DROP_MSG);
			printf("[MrclRouting::recv] received a Control Packet\n");
		return;
	}

	RoutingHdr* mrhdr = HDR_ROUTING(p);
	hdr_cmn* ch = hdr_cmn::access(p);
	hdr_ip* iph = hdr_ip::access(p);

#ifdef DEBUG
//////////////////////////////////////////////////////////////////////////////
	int saddrLen;
	memcpy(&saddrLen, mrhdr->saddr(), sizeof(int));
	if (saddrLen==0) return; // routing addresses not yet initialized
	char saddr[MRCL_ADDRESS_MAX_LEN] = "";
 	char temp[10];
	for(int i=saddrLen-1; i>=0; i--)
	{
		if (i==0)
			sprintf(temp,"%d", mrhdr->saddr()[i+sizeof(int)]);
		else
			sprintf(temp,"%d.", mrhdr->saddr()[i+sizeof(int)]);
		strcat(saddr,temp);
	}
	strcat(saddr,"\0");
	char daddr[MRCL_ADDRESS_MAX_LEN] = "";
	int daddrLen;
	if (daddrLen>0)
	{
		memcpy(&daddrLen, mrhdr->daddr(), sizeof(int));
		strcpy(temp,"");
		for(int i=daddrLen-1; i>=0; i--)
		{
			if (i==0)
				sprintf(temp,"%d", mrhdr->daddr()[i+sizeof(int)]);
			else
				sprintf(temp,"%d.", mrhdr->daddr()[i+sizeof(int)]);
			strcat(daddr,temp);
		}
	}
	strcat(daddr,"\0");

	char raddr[MRCL_ADDRESS_MAX_LEN] = "";
	int raddrLen;
	char temp1[10];
	if (raddrLen>0)
	{
		memcpy(&raddrLen, rr_->getAddress(), sizeof(int));
		strcpy(temp1,"");
		for(int i=raddrLen-1; i>=0; i--)
		{
			if (i==0)
				sprintf(temp1,"%d", rr_->getAddress()[i+sizeof(int)]);
			else
				sprintf(temp1,"%d.", rr_->getAddress()[i+sizeof(int)]);
			strcat(raddr,temp1);
		}
	}
	strcat(raddr,"\0");
////////////////////////////////////////

	printf("%s saddr is %s and daddr is %s. Currently I have a route for -%s-\n",__PRETTY_FUNCTION__, saddr,daddr,raddr);
#endif
	// Mior, 12 2008
	// I collect the number of modules to be crossed from the cross layer message,
	// which has the information for the last computed path. Subsequently I have to verify
	// if the path computed is suitable for the destination of the packet just arrived.
	int nModule = rr_->length();//routes_.count();//mrhdr->moduleRouteCount();

	// Mior, 12 2008
	if(!MrclAddress::areEqual(mrhdr->daddr(), rr_->getAddress())){
		printf("Maybe I have a path (nMod %d),\nbut for another destination: daddr -%d-  raddr -%d-\n\n", nModule, mrhdr->daddr(), rr_->getAddress());
		// I have to restart the discovery, reset parameters regarding the old path
		nModule=0;
		routes_.clear();
		rr_->empty();
	}

	#ifdef DEBUG
		printf("\t\tModule %d - recv - nmodule=%d\n", getId(), nModule);
	#endif

	int hdrWrite = 0;    /// NOTE ci vorrebbe qualcosa di meglio...tipo se il pkt arriva da fuori...

#ifdef DEBUG
// This slice of code serves to check whether all useful fields are complete or not...
// They stores the sequence of modules to be crossed
	// packet header
	for(int j=0; j<10; j++){
		printf("mrhdr->getModuleRoute(%d) = %d\n",j,mrhdr->getModuleRoute(j));
		if(mrhdr->getModuleRoute(j)!=0)
			hdrWrite = 1;
	}
	// sendUp field
	printf("Pkt header sendup field is set to %d\n", mrhdr->sendup() );
	
	// crossLayerMessage
	for(int t=0; t<10; t++){
		printf("modules_[%d] %d\n", t,rr_->getIndex(t));
	}

	// nextLeaf
	for(int hj = -1; hj<4; hj++){
		printf("nextLeaf(%d) %d\n", hj, findNextLeaf(hj));
	}
#endif


	if (nModule<=1)
	{
		if (mrhdr->sendup()==0)
		{
			// Packet has not been yet solved -> try to solve it
			// here the packet has to be solved for the 1st time
			if(ch->direction() == hdr_cmn::DOWN)
			{
				// first routing module which processes the packet set the source address
				assert(nAddresses()>0);
				MrclAddress::storeAddr(mrhdr->saddr(), addresses_[0]->getAddr());
			}
			printf("[MrclRouting::recv] call forward\n");
			forward(p);
		}
		else
		{printf("[MrclRouting::recv] send up\n");
			// Packet has been solved -> not for me -> send Up
			sendUp(p, delayUp_);
		}
	}
	else
	{

	/// WAIT A MINUTE!!!! Here we have to verify if nModules is != 0 BUT we have to verify
 	/// also if the header is not empty.
// la strategia da realizzare consiste in info su route, derivate da scoperta con cl msg, vanno memorizzate in ciascun nodo, in qualche modo. Il pkt si muove all'interno dello stesso nodo con le info che trasporta nell'header. Il fatto che ci siano routes non implica che l'header del pkt sia riempito, si riempie durante discovery. I pkt che arrivano possono essere brad new, come per aodv che prima ci sono le req che scatenano la route discovery e poi arrivano gli altri pkt, totalmente ignoranti.
// Attenzione poi a generalizzare nel caso di piu' destinazioni possibili

// 		if(nModule == 1){//non ho alternative
// 			sendDown(p);
// 		}
// 		else{
		#ifdef DEBUG
			printf("I have to scan header to check whether is empty (%d)\n", hdrWrite);
		#endif
		if(hdrWrite == 0){
			// If I'm here, I have a route to the destination, but I haven't
			// teached the packet the succession of modules to traverse
			#ifdef DEBUG
				printf("WARNING nModule!=0 but header empty!!\n");
			#endif
			
			// 10 is choosed randomly, even if it depends on DEPTH_TREE value
			for(int t=0; t<10; t++){
				if(rr_->getIndex(t)>0)
					mrhdr->addModuleRoute(rr_->getIndex(t)-1, t);
			}
		}
		// go through the Module Id list
		int nextModule = -1;
		for(int i=0; i<nModule-1; i++)
		{
			if (mrhdr->getModuleRoute(i)==getId())
			{
				if (i < nModule-2)
				{
					#ifdef DEBUG
						printf("Get the next ModuleId in the path\n");
					#endif
					nextModule = mrhdr->getModuleRoute(i+1);
					#ifdef DEBUG
						printf("nextModule %d\n", nextModule);
					#endif
				}
				else
				{
					#ifdef DEBUG
						printf("This is the last Routing Module: clean the Module Id list in the packet header and solve the routing\n");
					#endif
					nextModule = mrhdr->getModuleRoute(i);
					mrhdr->clearModuleRoute();
					char *nextHop = getNextHop(p);
					if(nextHop)
					{	
						MrclAddress::storeAddr(mrhdr->nexthop(), nextHop);
						/// NOTE store ch->next_hop();
						char *nh;
						//MrclAddress::storeAddr(, nextHop);
						//printf("next hop nh %d, %d\n", nh, mrhdr->nexthop());
						//############
						int nhLen;
						memcpy(&nhLen, nextHop, sizeof(int));
						//if (saddrLen==0) return; // routing addresses not yet initialized
						char nha[MRCL_ADDRESS_MAX_LEN] = "";
						char temp[10];
						for(int ki=nhLen-1; ki>=0; ki--)
						{
							if (ki==0)
								sprintf(temp,"%d", nextHop[ki+sizeof(int)]);
							else
								sprintf(temp,"%d.", nextHop[ki+sizeof(int)]);
							strcat(nha,temp);
						}
						strcat(nha,"\0");
						printf("NextHopAddress is %s\n", nha);

//1.0.0.8 to 16777224 per il ch->next_hop()


						int level[4] = {0,0,0,0};
						char tmp[20];
						strncpy(tmp,nha,19);
						tmp[19] = '\0';
						char *p = strtok(tmp, ".");
						for(int i = 0; p && i < 4; p = strtok(NULL, "."), i++)
						{
							level[i] = atoi(p);
							if(level[i] > 255)
								level[i] = 255;
							else if(level[i] < 0)
								level[i] = 0;
						}
						int ip = 0;
						for(int i = 0; i < 4; i++)
						{
							ip += (level[i] << 8 * (3 - i));
						}
						
 						ch->next_hop() = (nsaddr_t)(ip);
						//mrhdr->nexthop() = (nsaddr_t)(ip);
						printf("\tSetto il next hop mh %d, ch %d\n", mrhdr->nexthop(), ch->next_hop());
						//################
						//ch->next_hop() = (nsaddr_t)Address::instance().get_nodeaddr(nextHop);
// 						setOldAddresses(p);
						// here the packet has to be solved from the leaf module
					}
/// NOTE maybe this is wrong. If I'm the last module, I'm the last hope of a previous search. If nether I find a route I can just drop the packet. This behavior can cause an infinite "resolve(p)" loop, trying to find a non-existing path.
					else
					{
						#ifdef DEBUG
						printf("maybe someone has chosen this module when he said that the destination is MAYBE reacheable, but no route was found!\n");
						#endif
						//resolve(p); //Mior changed dic 10 08
						return;
					}
					
				}
				break;
			}
		}
		#ifdef DEBUG
			printf("\t\tModule %d - recv - nextModule=%d\n",getId(), nextModule);
		#endif
		if (nextModule==-1)
		{    //if(nModule>1){
			#ifdef DEBUG
				printf("This is an intermediate node -> send the packet to upper module\n");
			#endif
			sendUp(p, delayUp_);
// 		     }
// 		     else{
// 			mrhdr->clearModuleRoute();
// 			char *nextHop = getNextHop(p);
// 				if(nextHop)
// 				{
// 					MrclAddress::storeAddr(mrhdr->nexthop(), nextHop);
// // 					setOldAddresses(p);
// 					// here the packet has to be solved from the leaf module
// 				}
// 				else
// 				{printf("Maybe reachable\n");
// 					// maybe someone has chosen this module when he said that the destination is MAYBE reacheable
// 					resolve(p);
// 				}
// /// NOTE mi manca chi scrive la source e la dest del pacchetto nell'header routing
// 			printf("Ho solo una rotta a disposizione, reperisco info: nexthop %d\n", nextHop);
// 			MrclAddress::storeAddr(mrhdr->saddr(), addresses_[0]->getAddr());
// 			//sendDown(p);
// 			forward(p);
// 		     }
		}
		else if(nextModule == getId())
		{
			#ifdef DEBUG
				printf("If nextModule==getId() sendDown\n");
			#endif
			sendDown(p, delayDown_);
		}
		else
		{
			#ifdef DEBUG
				printf("Send the packet to the next Module in the path -> try with the above modules\n");
			#endif
			
			sendDown(nextModule, p, delayDown_);
		} // end nextModule==-1
	} // end nModule!=0
	//}//end else nModule == 1
}

void MrclRouting::addAddress(MrclAddress *a)
{// contando che lo chiamo una volta....Quando dovrei chiamarlo piu' volte?
	addresses_ = (MrclAddress **)realloc(addresses_, nAddresses_ * sizeof(MrclAddress *), (nAddresses_ + 1) * sizeof(MrclAddress *));
	addresses_[nAddresses_] = a;
	nAddresses_=nAddresses_+1;
}

MrclAddress *MrclRouting::getAddress(int i)
{
	if(i < 0 || i >= nAddresses_)
		return 0;
	return addresses_[i];
}

int MrclRouting::nAddresses()
{
	return nAddresses_;
}

int MrclRouting::isMyAddress(char *addr)
{
	for(int i = 0; i < nAddresses();i++)
	{
		if(getAddress(i)->isEqual(addr))
			return 1;
	}
	return 0;
}
