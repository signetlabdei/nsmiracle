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
 * \file bin.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 8-2-06
 */
#ifndef _BIN_H_
#define _BIN_H_

#include <sap.h>
#include <clsap.h>
 
 
/**
 * The BIN class is a SAP and a ClSAP which  is used to drop packets or ClMessages. 
 * It has tracing functionality, thus
 * it could trace the packets which pass through.
 *
 * @see ClMessageTracer, ClSAP, ClTracer, Module,  NodeCore, PktTracer, Plugin, Tracer, SAP
 **/
 
class Bin : public SAP, public ClSAP{
	public:
		/**
		* Bin constructor
		**/
		Bin();
		/**
		* Bin destructor
		**/
		virtual ~Bin();
		
		/**
		* TCL command interpreter. It implements the following OTcl methods:
		* <ul>
		*  <li><b>preamble &lt;<i>preamble string</i>&gt;</b>: 
		*  	initializes the <i>preamble_</i> class attribute</li>
		* </ul>
		* 
		* Note that it inherits all the OTcl method of ConnectTrace (not SAP or ClSAP !!!)
		* 
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
		 * Write the trace for the dropping of a Packet and free the packet
		 * 
		 * @param p pointer of the packet which has to be dropped
		 * @param depth verbosity of the trace. If this param is less or equal to the <i>depth_</i> class attribute a new line on the trace file will be add
		 * @param reason reason for the drop
		 * 
		 **/
		void drop(Packet *p, int depth,const char *reason = "---");
		/**
		 * Write the trace for the dropping of a cross-layer command message
		 * 
		 * @param m pointer of the ClMessage which has to be dropped
		 * @param reason reason for the drop
		 * 
		 **/
		void drop(ClMessage* m, const char* reason = "---");
		/**
		 * This method has an empty body; it is here for multi inherits troubles
		 **/
		void handle(Event *e);
	protected:
		/** String added in the trace of the packet or the CLMessage */
		char preamble_[MAX_PREAMBLE_LENGTH];
		/** Reference verbosity level (binded in Tcl) */
		int depth_;
};
#endif
