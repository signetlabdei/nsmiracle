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

#include <assert.h>
#include "mrcl-address.h"

packet_t PT_MRCL_ROUTING;

MrclAddress::MrclAddress()
{
}

MrclAddress::MrclAddress(char *addr)
{
	memcpy(addr_, addr, MRCL_ADDRESS_MAX_LEN);
}

MrclAddress::~MrclAddress()
{
}

void MrclAddress::setAddr(void *addr, int addrLen)
{
	assert(addrLen <= (int)MRCL_ADDRESS_MAX_LEN - (int)sizeof(int));
	memset(addr_, 0, MRCL_ADDRESS_MAX_LEN);
	memcpy(addr_, &addrLen, sizeof(int));
	memcpy(addr_ + sizeof(int), addr, addrLen);
}

void MrclAddress::write(void *addr, int addrLen)
{
	int len;
	memcpy(&len, addr_, sizeof(int));
	memcpy(addr, addr_, addrLen > (int)len + (int)sizeof(int) ? len + (int)sizeof(int): addrLen);
}

void MrclAddress::getAddr(void *addr, int addrLen, int offset)
{
	assert(offset + addrLen <= (int)MRCL_ADDRESS_MAX_LEN - (int)sizeof(int));
	memcpy(addr, addr_ + sizeof(int) + offset, addrLen);
	
}

bool MrclAddress::operator==(const MrclAddress &a)
{
	return (memcmp(addr_, a.addr_, MRCL_ADDRESS_MAX_LEN) == 0);
}

int MrclAddress::isEqual(char *addr)
{
	return (memcmp(addr_, addr, MRCL_ADDRESS_MAX_LEN) == 0);
}

int MrclAddress::areEqual(char *addr1, char *addr2)
{
	return (memcmp(addr1, addr2, MRCL_ADDRESS_MAX_LEN) == 0);
}

void MrclAddress::storeAddr(char *dst, char *addr)
{
	memcpy(dst, addr, MRCL_ADDRESS_MAX_LEN);
}

int MrclAddress::command(int argc, const char *const *argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 2)
	{
		if(strcasecmp(argv[1],"addr") == 0)
		{
			int len = strlen() + 2;
			char *str = new char[len];
			toString(str, len);			
			tcl.resultf("%s",str);
			delete [] str;
			return TCL_OK;
		}
	}
	else if(argc == 3)
	{
		if(strcasecmp(argv[1],"addr") == 0)
		{
			setAddress(argv[2]);
			return TCL_OK;
		}
	}
	return TclObject::command(argc, argv);
}

char *MrclAddress::getAddr()
{
	return addr_;
}

