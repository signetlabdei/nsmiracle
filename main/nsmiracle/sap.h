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
 * \file sap.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 8-2-06
 */


#ifndef _SAP_
#define _SAP_

#include <connector-trace.h>
#include <packettracer.h>

class Module;

/**
 * The SAP class is a ConnectorTrace which  is used to connect two modules of adjacent layers. 
 * It has tracing functionality, thus
 * it could trace the packets which pass through.
 *
 * @see Module, PktTracer, Tracer
 **/
 
 class SAP : virtual public ConnectorTrace{
 public:
 	friend class Tracer;
	/**
	 * SAP constructor
	 **/
	
	SAP();
	
	/**
	 * SAP destructor
	 **/
	
	virtual ~SAP();
	
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * <ul>
	 *  <li><b>upmodule &lt;<i>module instance</i>&gt;</b>: 
	 *  	initializes the <i>upModule_</i> pointer to the given module</li>
	 *  <li><b>downmodule &lt;<i>module instance</i>&gt;</b>: 
	 *  	initializes the <i>downModule_</i> pointer to the given module</li>
	 *  <li><b>traceUp &lt;<i>up prefix char</i>&gt;</b>: 
	 *  	initializes the <i>dirUp_</i> class attribute</li>
	 *  <li><b>traceDown &lt;<i>down prefix char</i>&gt;</b>: 
	 *  	initializes the <i>dirDown_</i> class attribute</li>
	 *  <li><b>preambleUp &lt;<i>up preamble string</i>&gt;</b>: 
	 *  	initializes the <i>preambleUp_</i> class attribute</li>
	 *  <li><b>preambleDown &lt;<i>down preamble string</i>&gt;</b>: 
	 *  	initializes the <i>preambleDown_</i> class attribute</li>
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
	 * Send a packet to the up module adding a delay if it is neccessary.
	 * Note: this method sets the direction of the packet to <b>UP</b>
	 * 
	 * @param p a pointer to the packet which has to send up
	 * @param delay the delivery delay (it must be greater or equal 0)
	 * 
	 **/
	virtual void sendUp(Packet* p, double delay);
	
	/**
	 * Send a packet to the down module with the sender module ID and
	 * adding a delay if it is neccessary
	 * Note: this method sets the direction of the packet to <b>DOWN</b>
	 * 
	 * @param p a pointer to the packet which has to send down
	 * @param delay the delivery delay (it must be greater or equal 0)
	 * 
	 **/
	virtual void sendDown(Packet* p, double delay);
	
	/**
	 * Send a ClMessage to the up module adding a delay if it is neccessary.
	 * Note: this method sets the direction of the packet to <b>UP</b>
	 * 
	 * @param m a pointer to the ClMessage which has to send up
	 * @param delay the delivery delay (it must be greater or equal 0)
	 * 
	 **/
	virtual void sendUp(ClMessage* m, double delay);
	
	/**
	 * Send a ClMessage to the up module adding a delay if it is neccessary.
	 * Note: this method sets the direction of the packet to <b>UP</b>
	 * 
	 * @param m a pointer to the ClMessage which has to send down
	 * @param delay the delivery delay (it must be greater or equal 0)
	 * 
	 **/
	virtual void sendDown(ClMessage* m, double delay);
	
	/**
	 * Send a Synchronous ClMessage to the up module adding a delay if it is neccessary.
	 * Note: this method sets the direction of the packet to <b>UP</b>
	 * 
	 * @param m a pointer to the ClMessage which has to send up
	 * 
	 **/
	virtual void sendSynchronousUp(ClMessage* m);
	
	/**
	 * Send a Synchronous ClMessage to the up module adding a delay if it is neccessary.
	 * Note: this method sets the direction of the packet to <b>UP</b>
	 * 
	 * @param m a pointer to the ClMessage which has to send down
	 * @param delay the delivery delay (it must be greater or equal 0)
	 * 
	 **/
	virtual void sendSynchronousDown(ClMessage* m);
	
	/**
	 * Adds a new tracer to the packet tracer list (pktTr_)
	 * 
	 * @param tr a pointer to the tracer instance which has to insert in the tracer list
	 * 
	 * @see PktTracer, Tracer
	 **/
	static void addTracer(Tracer *tr);
	
	/**
	 * Returns the ID of the up module
	 * 
	 * @return the up module ID
	 **/
	int getModuleUpId();
	
	/**
	 * Returns the ID of the down module
	 * 
	 * @return the down module ID
	 **/
	int getModuleDownId();
	
	/**
	 * This method is called when the SAP has been scheduled in order to give a delay to a packet
	 * 
	 * @param e pointer to the event which has this SAP for handler. It is a Packet
	 * 
	 **/
	virtual void handle(Event *e);
	/**
	 * Return the value of depthUp_
	 *
	 * @return The value of depthUp_
	 **/
	int depthUp();
	/**
	 * Return the value of depthDown_
	 *
	 * @return The value of depthDown_
	 **/
	int depthDown();
protected:
	/** The packet tracer list */
	static PktTracer *pktTr_;
	/** Pointer to the up module */
	Module* upModule_;
	/** Pointer to the down module */
	Module* downModule_;
	
	/**  
	 * Write a line in the system trace file  which contains the trace information of a packet
	 * 
	 * @param p pointer to the packet which has to be traced
	 * 
	 **/
	void trace(Packet *p);
	/**  
	 * Write a line in the system trace file  which contains the trace information of a ClMessage
	 * 
	 * @param m pointer to the ClMessage which has to be traced
	 * 
	 **/
	void trace(ClMessage* m);

	/** Define the level of tracing for packet with up direction**/
	int depthUp_;
	/** Define the level of tracing for packet with down direction**/
	int depthDown_;
};
#endif
