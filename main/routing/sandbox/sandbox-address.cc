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

#include"sandbox-address.h"



SandboxAddress::SandboxAddress() : MrclAddress()
{
	
}

SandboxAddress::~SandboxAddress()
{
	
}


int SandboxAddress::toString(char *str, int strlen)
{
	if (strlen<ADDRESS_MAX_LEN)
	{
		strncpy(str, addrStr_, strlen);
		return(0);
	}
	else
	{
		strncpy(str, addrStr_, ADDRESS_MAX_LEN);
		return(1);
	}
}

int SandboxAddress::strlen()
{
	return(ADDRESS_MAX_LEN);
}


nsaddr_t str2addr(const char *str)
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




void SandboxAddress::setAddress(const char *addr)
{
	strncpy(addrStr_, addr, ADDRESS_MAX_LEN);
	ipAddr_ = str2addr(addr);
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
	char *temp = new char[ADDRESS_MAX_LEN];
	for(int i = 0; i < 4; i++)
	{
		temp += (level[i] << 8 * (3 - i));
	}
	setAddr(temp, ADDRESS_MAX_LEN);
}

nsaddr_t SandboxAddress::getIpAddr()
{
	return(ipAddr_);
}
