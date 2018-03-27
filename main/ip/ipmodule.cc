/*
 * Copyright (c) 2007 Regents of the SIGNET lab, University of Padova.
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

#include "ipmodule.h"
#include "ip.h"
#include "ip-clmsg.h"

static class IPModuleClass : public TclClass {
public:
	IPModuleClass() : TclClass("Module/IP") {}
	TclObject* create(int, const char*const*) {
	return (new IPModule());

}
} class_ipmodule;

nsaddr_t lastIP = 0;

IPModule::IPModule() : Module()
{
	ipAddr_ = ++lastIP;
}

IPModule::~IPModule()
{
}

int IPModule::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 2)
	{
		if (strcasecmp(argv[1],"addr")==0)
		{
			tcl.resultf("%d", ipAddr_);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1],"subnet")==0)
		{
			tcl.resultf("%d", subnet_);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1],"addr-string")==0)
		{
			tcl.resultf("%d.%d.%d.%d", (ipAddr_ & 0xff000000)>>24,(ipAddr_ & 0x00ff0000)>>16, (ipAddr_ & 0x0000ff00)>>8, (ipAddr_ & 0x000000ff));
			return TCL_OK;
		}
		else if (strcasecmp(argv[1],"subnet-string")==0)
		{
			tcl.resultf("%d.%d.%d.%d", (subnet_ & 0xff000000)>>24,(subnet_ & 0x00ff0000)>>16, (subnet_ & 0x0000ff00)>>8, (subnet_ & 0x000000ff));
			return TCL_OK;
		}
	}
	else if (argc == 3) {
                if (strcasecmp (argv[1], "addr") == 0) {
                        ipAddr_ = str2addr((char *)argv[2]);
			if (ipAddr_ == 0)
			  {
			    fprintf(stderr,"0.0.0.0 is not a valid IP address");
			    return TCL_ERROR;
			  }
                        subnet_ = get_subnetaddr(ipAddr_);
                        return TCL_OK;
                }
		else if (strcasecmp (argv[1], "subnet") == 0) {
                        subnet_ = str2sub((char *)argv[2]);
                        return TCL_OK;
                }
	}
	return Module::command(argc, argv);
}

void IPModule::recv(Packet *p)
{
	hdr_ip *iph = HDR_IP(p);
	hdr_cmn *ch = HDR_CMN(p);
	if(ch->direction() == hdr_cmn::UP)
	{
		if(iph->daddr() == ipAddr_ || iph->daddr() == IP_BROADCAST)
		{
			ch->size() -= IP_HDR_LEN;
			sendUp(p);
		}
		else
		{
			/*
			This is a simple IP module. If you want to add routing feature, put here the code for forward the packets.
			*/
			drop(p, NOT_FOR_ME_DEPTH, NOT_FOR_ME_REASON);
		}
	}
	else
	{

		iph->saddr() = ipAddr_;
		iph->ttl() = IP_DEF_TTL;
		ch->addr_type()	= NS_AF_INET;
		ch->size() += IP_HDR_LEN;
		if ((u_int32_t)iph->daddr() == IP_BROADCAST)
			ch->next_hop()	= IP_BROADCAST;
		else {
			ch->next_hop() = iph->daddr();
		}
		sendDown(p);
	}
}

nsaddr_t IPModule::str2addr(const char *str)
{
	int level[4] = {0,0,0,0};
	char tmp[20];
	strncpy(tmp,str,19);
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
	nsaddr_t addr = 0;
	for(int i = 0; i < 4; i++)
	{
		addr += (level[i] << 8 * (3 - i));
	}
	return addr;
}

nsaddr_t IPModule::str2sub(const char *str)
{
	int level[4] = {0,0,0,0};
	char tmp[20];
	strncpy(tmp,str,19);
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
	nsaddr_t sub = 0;
	for(int i = 0; i < 4; i++)
	{
		sub += (level[i] << 8 * (3 - i));
	}
	return sub;
}

nsaddr_t IPModule::get_subnetaddr(nsaddr_t addr)
{
	int c = 0;
	nsaddr_t a = (addr & 0xff000000)>>24;
	if (a < 128)
		c = 1;
	else if (a < 192)
		c = 2;
	else if (a < 224)
		c = 3;
	else if (a < 240)
		c = 4;
	else
		c = 5;
	
	if (c > 3)
		return 0;
	
	switch(c)
	{
		case 1:
			return (nsaddr_t)0xff000000;
		case 2:
			return (nsaddr_t)0xffff0000;
		case 3:
			return (nsaddr_t)0xffffff00;
	}
}


int IPModule::recvSyncClMsg(ClMessage* m)
{
	if(m->type() == IP_CLMSG_SEND_ADDR)
	{ 
    IPClMsgSendAddr *c = new IPClMsgSendAddr(UNICAST, m->getSource());
    c->setAddr(ipAddr_);
//     sendDown(m->getSource(), c);		// DEPRECATED method
    sendAsyncClMsg(c);

    ((IPClMsgSendAddr*)m)->setAddr(ipAddr_);
    return 0;
	}
	return Module::recvSyncClMsg(m);
}

