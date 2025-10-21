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
 * $Id: mmac-module.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef MMAC_MODULE_H
#define MMAC_MODULE_H

#include <module.h>
#include <queue.h>

/**
 * Base class for mac layer modules
 */
class MMacModule : public Module
{
	public:
		/** Constructor */
		MMacModule();
		/** Desctructor */
		virtual ~MMacModule() = 0;

		/** TCL commander */
		virtual int command (int argc, const char *const *argv);

		/**
		 * Recieve method, Used in Miracle communication
		 * If ifq_ is set, send downgoing packets through the queue
		 */
		virtual void recv (Packet *p);

		/**
		 * Recieve method, used for packets comming from queue
		 */
		virtual void recv(Packet *p, Handler* callback);

		/**
		 * Recieve packet from layers below
		 * Needs to be implemented in implementing classes
		 * @param p The received packet
		 */
		virtual void rx(Packet *p) = 0;

		/**
		 * Transmit packet from layers above
		 * Needs to be implemented in implementing classes
		 * @param p The packet to transmit
		 */
		virtual void tx(Packet *p) = 0;

		/**
		 * Receive syncronous commands (DEPRECATED interface)
		 * @param m The incomming message
		 */
		int crLaySynchronousCommand(ClMessage* m);
		
		/**
		 * Receive syncronous commands (DEPRECATED interface)
		 * @param m The incomming message
		 */
		int recvSyncClMsg(ClMessage* m);
		
		/**
		 * Get the MAC address
		 * @return The MAC address
		 */
		virtual int addr();

		/**
		 * Tell a queue that we are clear to send new packets
		 * Should be called after sendDown()
		 */
		virtual void resume();

	private:
		/**
		 * MAC address
		 */
		int index_;

		/** Pointer to a queue used to queue incomming packets */
		Queue *ifq_;
};

#endif
