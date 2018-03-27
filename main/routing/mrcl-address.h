/*
 * Copyright (c) 2006 Regents of the SIGNET lab, University of Padova.
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

/**
 * \file mrcl-address.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 7-5-07
 */

#ifndef _MRCL_ADDRESS_
#define _MRCL_ADDRESS_

#include <packet.h>

#define MRCL_ADDRESS_MAX_LEN 288

#define MRCL_ADDRESS_MAX_ROUTING_TREE_DEPTH 10

#define HDR_ROUTING(p)		(RoutingHdr::access(p))

typedef struct RoutingHdr {
	char saddr_[MRCL_ADDRESS_MAX_LEN];
	char daddr_[MRCL_ADDRESS_MAX_LEN];
	char nexthop_[MRCL_ADDRESS_MAX_LEN];
	int moduleRoute_[MRCL_ADDRESS_MAX_ROUTING_TREE_DEPTH];
	int moduleRouteCount_;
	int sendup_;
	int ttl_;
	int newPkt_;	

	inline int isNew() { return newPkt_; }
	inline void setNew() { newPkt_ = 1; }
	inline char *saddr() { return saddr_; }
	inline char *daddr() { return daddr_; }
	inline char *nexthop() { return nexthop_; }
	inline int &ttl() { return ttl_; }
	inline void addModuleRoute(int i, int val) 
	{
		assert(i < MRCL_ADDRESS_MAX_ROUTING_TREE_DEPTH);
		moduleRoute_[i] = val;
		if(i >= moduleRouteCount_)
			moduleRouteCount_ = i + 1;
	}
	inline int moduleRouteCount() { return moduleRouteCount_; }
	inline int getModuleRoute(int i) { return moduleRoute_[i]; }
	inline void clearModuleRoute()
	{
		moduleRouteCount_ = 0;
		memset(moduleRoute_, 0, sizeof(int) * MRCL_ADDRESS_MAX_ROUTING_TREE_DEPTH);
		sendup_ = 0;
		newPkt_ = 0;
	}
	inline int sendup() { return sendup_; }
	inline void setSendup() { sendup_ = 1; }

	
	static int offset_;
	inline static int& offset() { return offset_; }
	inline static struct RoutingHdr* access(const Packet* p) {
		return (struct RoutingHdr*)p->access(offset_);
	}
} RoutingHdr;


class MrclAddress : public TclObject
{
public:
	MrclAddress();
	MrclAddress(char *addr);
	virtual ~MrclAddress();

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * <ul>
	 *  <li><b>addr</b>: 
	 *  	return the string formatted address
	 *  <li><b>addr &lt;<i>address</i>&gt;</b>: 
	 *  	set the address converting the string <i>address</i>
	 * </ul>
	 * 
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
	 * 
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	 * 
	 * @see Module
	 **/
	virtual int command(int argc, const char*const* argv);
	/**
	 * Convert the Address to a string and write it into the str buffer.
	 * @param str Buffer into which will be written the address
	 * @param strlen Length of str
	 *
	 * @return 0 if the buffer is smaller than converted string address, 1 otherwise
	 */
	virtual int toString(char *str, int strlen) = 0;
	/**
	 * @return The length, in number of char, of the address converted to string
	 */
	virtual int strlen() = 0;
	
	/**
	 * Set the address converting the null terminated string.
	 *
	 * @param addr Null terminated string which represent the address
	 */
	virtual void setAddress(const char *addr) = 0;

	void setAddr(void *addr, int addrlen);

	void getAddr(void *addr, int addrle = MRCL_ADDRESS_MAX_LEN - sizeof(int), int offset = 0);

	char *getAddr();

	void write(void *addr, int addrlen);

	bool operator==(const MrclAddress &a);

	int isEqual(char *addr);

	static int areEqual(char *addr1, char *addr2);
	static void storeAddr(char *dst, char *addr);
private:
	char addr_[MRCL_ADDRESS_MAX_LEN];
};

#endif


