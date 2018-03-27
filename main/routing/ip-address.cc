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

#include "ip-address.h"

static class IPAddressClass : public TclClass {
public:
	IPAddressClass() : TclClass("Address/IP") {}
	TclObject* create(int argc, const char *const *argv) 
	{
		int i=0;
		while((strcmp(argv[i],"Address/IP"))&&(i<argc))
		{
			i++;
		}
		if(argc - i - 1 == 2)
			{
				return (new IPAddress(argv[i+2]));
			}
// 		else if(argc - i - 1 > 2)
// 			return (new IPAddress(argv[i+2], argv[i+3]));
		else
			return (new IPAddress());
	}
} class_ip_address;

IPAddress::IPAddress() : ipAddr_(0)
{
}

IPAddress::IPAddress(const char *ip)
{
	setAddress(ip);
// 	ipAddr_ = (int*)(ip + sizeof(int));
// 	generateNetmask();
}

IPAddress::~IPAddress()
{
}

int IPAddress::toString(char *str, int strlen)
{
	char addr[20];
	int ip;
	getAddr(&ip, sizeof(int));
	sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
	strncpy(str, addr, strlen);
	if(::strlen(addr)>= strlen)
		return 1;
	return 0;
}

int IPAddress::strlen()
{
	char addr[20];
	int ip;
	getAddr(&ip, sizeof(int));
	sprintf(addr,"%d.%d.%d.%d", (ip & 0xff000000)>>24,(ip & 0x00ff0000)>>16, (ip & 0x0000ff00)>>8, (ip & 0x000000ff));
	return ::strlen(addr);
}

void IPAddress::setAddress(const char *addr)
{
	int level[4] = {0,0,0,0};
	char tmp[20];
	strncpy(tmp,addr,19);
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

	setAddr(&ip, sizeof(int));
	ipAddr_ = ip;

}


int IPAddress::command(int argc, const char * const *argv)
{
// 	Tcl& tcl = Tcl::instance();
	return MrclAddress::command(argc, argv);
}


int IPAddress::INT(char* str)
{
	if (str==0)
		return (ipAddr_);
	else
	{
		int addr;
		memcpy(&addr, str + sizeof(int), sizeof(int));
		return (addr);
	}
}

int IPAddress::AND(char* str1, char *str2)
{
	int addr1;
	int addr2;
	int ret;
	memcpy(&addr1, str1 + sizeof(int), sizeof(int));
// 	addr->getAddr(&addr2, sizeof(int));
// 	ret = (addr & addr2);
// 	char *ret1 = new char[MRCL_ADDRESS_MAX_LEN];
// 	sprintf(ret1,"%d.%d.%d.%d", (ret & 0xff000000)>>24,(ret & 0x00ff0000)>>16, (ret & 0x0000ff00)>>8, (ret & 0x000000ff));
	if (str2==0)
		return(ipAddr_ & addr1);
	else
	{
		memcpy(&addr2, str2 + sizeof(int), sizeof(int));
		return(addr1 & addr2);
	}
}

int IPAddress::OR(char* str1, char *str2)
{
	int addr1;
	int addr2;
	int ret;
	memcpy(&addr1, str1 + sizeof(int), sizeof(int));
// 	addr->getAddr(&addr2, sizeof(int));
// 	ret = (addr & addr2);
// 	char *ret1 = new char[MRCL_ADDRESS_MAX_LEN];
// 	sprintf(ret1,"%d.%d.%d.%d", (ret & 0xff000000)>>24,(ret & 0x00ff0000)>>16, (ret & 0x0000ff00)>>8, (ret & 0x000000ff));
	if (str2==0)
	return(ipAddr_ | addr1);
	else
	{
		memcpy(&addr2, str2 + sizeof(int), sizeof(int));
		return(addr1 | addr2);
	}
}

