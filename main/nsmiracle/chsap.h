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
 * \file chsap.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 8-2-06
 * 
 */
 
#ifndef _CHSAP_
#define _CHSAP_

#include "sap.h"
#include "node-core.h"

class ChannelModule;

/**
 * The ChSAP class is a SAP which is used to connect a modules 
 * to a channel-module. It has tracing functionality, thus
 * it could trace the packets which pass through.
 * 
 * @see ChannelModule, Module, NodeCore, Position
 **/
class ChSAP : public SAP{
	public:
		/**
		* ChSAP constructor
		**/
		ChSAP();
		/**
		* ChSAP destructor
		**/
		virtual ~ChSAP();
		
		/**
		* TCL command interpreter. It implements the following OTcl methods:
		* <ul>
		*  <li><b>module &lt;<i>module instance</i>&gt;</b>: 
		*  	initializes the <i>upModule_</i> pointer to the given module</li>
		*  <li><b>channel &lt;<i>module instance</i>&gt;</b>: 
		*  	initializes the <i>downModule_</i> pointer to the given module</li>
		*  <li><b>nodeCore &lt;<i>Node core instance</i>&gt;</b>: 
		*  	initializes the <i>nodeCorePtr_</i> class attribute</li>
		* </ul>
		* 
		* Moreover it inherits all the OTcl method of SAP
		* 
		* @param argc number of arguments in <i>argv</i>
		* @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
		* 
		* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
		* 
		* @see NodeCore
		**/
		virtual int command(int argc, const char*const* argv);
		
		/**
		* <i>Overload of the SAP method</i>: send a packet to the channel adding a delay if it is neccessary
		* (it does not send the module id of the sender)
		* Note: this method sets the direction of the packet to <b>DOWN</b>
		* 
		* @param p a pointer to the packet which has to send down
		* @param delay the delivery delay (it must be greater or equal 0)
		* 
		**/
		void sendDown(Packet* p, double delay);
		/**
		* <i>Overload of the SAP method</i>: send a packet to the up module adding a delay if it is neccessary.
		* Note: this method sets the direction of the packet to <b>UP</b>
		* 
		* @param p a pointer to the packet which has to send up
		* @param delay the delivery delay (it must be greater or equal 0)
		* 
		**/
		virtual void sendUp(Packet* p, double delay);
		/**
		* <i>Overload of the SAP method</i>: This method is called when the ChSAP has been scheduled in order to give a delay to a packet
		* 
		* @param e pointer to the event which has this SAP for handler. It is a Packet
		* 
		**/
		void handle(Event* e);
		/**
		 * Returns the Position of the node
		 * 
		 * @returns the Position of the node
		 * 
		 * @see NodeCore, Position
		 **/
		Position* getPosition();
	protected:
		/** Instance of the node core this ChSAP belongs to */
		NodeCore* nodeCorePtr_;
		
		ChannelModule *channel_;
};

#endif
