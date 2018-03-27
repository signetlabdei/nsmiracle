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

#include"ip-address-codec.h"
#include"ip-address.h"
#include <ip.h>

static class IPAddressCodecClass : public TclClass {
	public:
		IPAddressCodecClass() : TclClass("Module/MrclIpAddressCodec") {}
		TclObject* create(int, const char*const*) {
			return (new IPAddressCodec());
		}
} class_ip_address_codec_module;


IPAddressCodec::IPAddressCodec(): up_(TRUE)
{
}

IPAddressCodec::~IPAddressCodec()
{
}

int IPAddressCodec::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc == 2)
	{
		if (strcmp(argv[1], "up") == 0)
		{
			up_=TRUE;
			return TCL_OK;
		}else if (strcmp(argv[1], "down") == 0)
		{
			up_=FALSE;
			return TCL_OK;
		}
	}
	else if (argc == 3)
	{
		if(strcmp(argv[1], "ns_addr_t2mrcl_addr") == 0)
		{
			int ip = atoi(argv[2]);
			char addr[20];
			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
			tcl.resultf("%s", addr);
			return TCL_OK;
		}
		else if(strcmp(argv[1], "mrcl_addr2ns_addr_t") == 0)
		{
			IPAddress *addr = (IPAddress *)TclObject::lookup(argv[2]);
			int level[4] = {0,0,0,0};
			char tmp[20];
			strncpy(tmp,addr->getAddr()+sizeof(int),19);
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
			memcpy(&ip, addr->getAddr()+sizeof(int), sizeof(int));
			tcl.resultf("%d", ip);
			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}

void IPAddressCodec::recv(Packet *p)
{printf("%s\n", __PRETTY_FUNCTION__);
	hdr_cmn *ch = HDR_CMN(p);
	if (ch->direction()==hdr_cmn::UP)
	{	
		sendUp(p, 0);
	}
	else
	{
		if (up_==TRUE)
		{
			// convert old ip addresses to new one
			struct hdr_ip *ih = HDR_IP(p);
			RoutingHdr *rhdr = HDR_ROUTING(p);
			int source = ih->saddr();
			int addrLen = sizeof(int);
			memcpy(rhdr->saddr(), &addrLen, sizeof(int));
			memcpy(rhdr->saddr() + sizeof(int), &source, addrLen);
			int dest = ih->daddr();
			memcpy(rhdr->daddr(), &addrLen, sizeof(int));
			memcpy(rhdr->daddr() + sizeof(int), &dest, addrLen);
 			char addr[20];
 			int ip;
 			memcpy(&ip, rhdr->saddr() + sizeof(int), sizeof(int));
 			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
 			printf("[IPAddressCodecUP(down)]source address %s, old %d\n", addr, source);
 			memcpy(&ip, rhdr->daddr()+ sizeof(int), sizeof(int));
 			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
 			printf("[IPAddressCodecUP]destination address %s, old %d\n", addr, dest);
		}
		else
		{
			// converto from IPAddress -> ns_addr_t
			struct hdr_ip *ih = HDR_IP(p);
			RoutingHdr *rhdr = HDR_ROUTING(p);
			int temp;
			memcpy(&temp, rhdr->saddr() + sizeof(int), sizeof(int));
			ih->saddr() = temp;
			memcpy(&temp, rhdr->daddr() + sizeof(int), sizeof(int));
			ih->daddr() = temp;
// 			char addr[20];
// 			int ip;
// 			memcpy(&ip, rhdr->saddr() + sizeof(int), sizeof(int));
// 			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
// 			printf("[IPAddressCodecDOWN]source address %s, old %d\n", addr, ih->saddr());
// 			memcpy(&ip, rhdr->daddr()+ sizeof(int), sizeof(int));
// 			sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
// 			printf("[IPAddressCodecDOWN(down)]destination address %s, old %d\n", addr, ih->daddr());
		}
		printf("SendDown\n");
		sendDown(p, 0);
	}
}
