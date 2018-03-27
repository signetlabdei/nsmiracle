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

/**
* \file dropinterface.h
* \author Federico Maguolo
* \date Wed Jan 24 9:30:00 2007
**/

#ifndef _DROPINTERFACE_
#define _DROPINTERFACE_

#include "link_channel.h"

#define DROPINTERFACE_TRACE_DEPTH 5
#define DROPINTERFACE_TRACE_REASON "DROP"

/**
* The DropInterface class is a NsObject which receives packets and send them to the BIN of the associated Module
**/
class DropInterface : public NsObject
{
public:
	/**
	* Class constructor
	**/
	DropInterface();
	/**
	* Class destructor
	**/
	virtual ~DropInterface();
	/**
	* Entry point for the packets. This metod call the drop method of the associated LinkChannelModule ch_.
	* If threre is not LinkChannelModule associated it free the packet
	* @param p packet which can be dropped
	* @param h this parameter is not considered
	**/
	virtual void recv(Packet *p, Handler *h = 0);
	virtual int command(int argc, const char*const* argv);
protected:
	LinkChannelModule *ch_;
};

#endif
