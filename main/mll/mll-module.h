/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: mll-module.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef MLL_MODULE_H
#define MLL_MODULE_H

#include <module.h>
#include <packet.h>
#include <arp.h>
#include <mac.h>

#include <vector>
#include <algorithm>

#include "marptable.h"
#include <ipmodule.h>

/**
 * Module for ARP-resolve.
 * Should live between one or more IPModule and one MMacModule
 * NOTE: You should only use one MMacModule below this module, else behavior is undef
 */
class MLLModule : public Module
{
	public:
		/** Constructor */
		MLLModule();
		/** Desctructor */
		~MLLModule();

		/**
		 * Receive asyncronous commands
		 * In Practive only IPModule telling us its IP-address
		 * @param m Pointer to message object
		 * @return 1 on success
		 */
		virtual int crLayCommand(ClMessage* m);
		virtual int recvAsyncClMsg(ClMessage* m);
    virtual int recvSyncClMsg(ClMessage* m);

		/** TCL Command handler */
		virtual int command(int argc, const char*const* argv);

		/** Retrieve packets from other modules */
		virtual void recv(Packet* p);

		/**
		 * Retrieve packets from other modules
		 * This method is used to know which mac-module to send ARP-replies to
		 */
		virtual void recv(Packet *p, int idSrc);

	protected:
		/** Handle packet going down */
		virtual void sendDown(Packet* p);

		/** Handle packet going up */
		virtual void sendUp(Packet* p);

		/** Process a ARP-request/-response packet */
		virtual void processARP(Packet* p, int idSrc);

		/**
		 * Check if a given IP-address is present in our stack
		 * Sends cl-messages to upper layer requesting their IP-addresses
		 * @param addr IP address to look for
		 * @return True if addr exists
		 */
		virtual bool netAddrPresent(nsaddr_t addr);

		/**
		 * Fill the table of upper layer network addresses
		 */
		virtual void fillNetAddrTable();

		/**
		 * Retrieve MAC address for lower layer
		 * @param downId id of downward module
		 * 		if> -1, only the module with id downId will be asked,
		 * 		else request is broadcasted
		 * @return the MAC address of lower layer
		 */
		virtual int getDownAddr(int downId = -1);

		/**
		 * Resolve MAC address for given dst address
		 * @param dst IP destination address
		 * @param p packet which requested the resolv, will be cached
		 * @return 0 if ARP request successfully sent, otherwise some error number
		 */
		virtual int arpResolve(nsaddr_t dst, Packet* p);

		/**
		 * Builds and sends an ARP request for the given dst IP address
		 * @param src Our source address
		 * @param dst The destination address we are looking for
		 */
		virtual void arpRequest(nsaddr_t src, nsaddr_t dst);

		/** Pointer to an arptable */
		MARPTable *arptable_;

		/** List of IP address to our upper layers */
		vector<nsaddr_t> netAddr;

		/** Link layer sequence number */
		int seqno_;

		/** ACK received so far */
		int ackno_;
};

#endif
