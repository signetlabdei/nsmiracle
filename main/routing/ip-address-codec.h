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
 * \file ip-address-codec.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 12-11-07
 */
 
#ifndef _MRCL_IPADDRESS_CODEC_MODULE_
#define _MRCL_IPADDRESS_CODEC_MODULE_

#include <module.h>


 /**
 * This class implements the module which converts the IPAddress (MrclAddress) to the old ns
 * address format (ns_addr_t) and correspondly sets the opportune header fields in order to
 * guarantee backward compatibility with ns modules which exploit old ns address format
 */
class IPAddressCodec : public Module
{
public:
	/**
	* Class constructor
	 */
	IPAddressCodec();
	/**
	 * Class destructor
	 */
	~IPAddressCodec();
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * 
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
	 * 
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	 * 
	 * @see Module
	 **/
	virtual int command(int argc, const char*const* argv);
	
	virtual void recv(Packet* p);
	
	private:
	/**
	 * TRUE defines that the codec is placed above the routing module
	 */
		int up_;
	
};

#endif
