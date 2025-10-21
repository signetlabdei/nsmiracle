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
 * \file connector-trace.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 9-19-06
 */
#ifndef _CONNECTORTRACE_
#define _CONNECTORTRACE_

#include <math.h> // floor
#include "packet.h"
#include "basetrace.h"
#include "cltracer.h"

/**
 * \def DEFAULTDEPTH
 * \brief Default value of the depth when each connector trace is initialized
 *
 **/
#define DEFAULTDEPTH 0
/**
 * \def MAX_PREAMBLE_LENGTH
 * \brief Maximum size in characters of the layer tag
 *
 **/
#define MAX_PREAMBLE_LENGTH 50
/**
 * \def BASICTRACE_BUFFERLEN
 * \brief Size of the buffer in the BasicTrace class of ns. It is 1026 in ns-2.29.
 **/
#define BASICTRACE_BUFFERLEN 1026
/**
 * The ConnectorTrace class is a TclObject and Handler which is the base of all the
 * classes that want the functionality of tracing. Moreover each ConnectorTrace is able
 * to trace ClMessage
 *
 * @see SAP, ClSAP, ChSAP
 **/
class ConnectorTrace : public TclObject, public Handler
{
 	/**
	 * ClMessageTracer is a friend class of ConnectorTrace because it can be able to write string in the trace file 
	 * 
	 * @see ClMessageTracer
	 **/
 	friend class ClMessageTracer;
public:
	/**
	* ConnectorTrace constructor
	**/
	ConnectorTrace();
	/**
	* ConnectorTrace destructor
	**/
	virtual ~ConnectorTrace();
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * <ul>
	 *  <li><b>detach</b>: 
	 *  	initializes the <i>pt_->channel_</i> and the <i>pt_->namchannel_</i> pointers to null value</li>
	 *  <li><b>flush</b>: 
	 *  	flushes the <i>pt_->channel_</i> and the <i>pt_->namchannel_</i> pointers</li>
	 *  <li><b>tagged</b>: 
	 *  	returns to TCL the string in <i>pt_->tagged</i></li>
	 *  <li><b>annotate &lt;<i>annotation string</i>&gt;</b>: 
	 *  	writes the annotation requested in the trace file</li>
	 *  <li><b>attach &lt;<i>channel string</i>&gt;</b>: 
	 *  	initializes the <i>pt->channel_</i> class attribute to the channel specified</li>
	 *  <li><b>namattach &lt;<i>nam channel string</i>&gt;</b>: 
	 *  	initializes the <i>pt->namchannel_</i> class attribute to the nam channel specified</li>
	 *  <li><b>ntrace &lt;<i>nam trace string</i>&gt;</b>: 
	 *  	writes the nam trace specified</li>
	 *  <li><b>tagged &lt;<i>boolean tag</i>&gt;</b>: 
	 *  	set the tag boolean variable to the requested value</li>
	 * </ul>
	 * 
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
	 * 
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	 *
	 * @see SAP, ChSAP, ClSAP
	 **/
	virtual int command(int argc, const char*const* argv);
	/**
	* Return the current value of depth in tracing
	* 
	* @return the current value of depth
	* 
	**/
	int depth();
	/**
	 * This method is called when the ConnectorTrace has been scheduled in order to give a delay to a ClMessage
	 * 
	 * @param e pointer to the event which has this ConnectorTrace for handler
	 * 
	 **/
	virtual void handle(Event *e);
	/**
	 * Adds a new tracer to the cross layer message tracer list (clTr_). 
	 * The given tracer is used only when ClMessage type is equal to the tracer clType attribute
	 * 
	 * @param tr a pointer to the tracer instance which has to insert in the tracer list
	 * 
	 * @see ClMessageTracer, ClTracer
	 **/
	static void addTracer(ClMessageTracer *tr);
	/**
	 * Adds a common tracer to the cross layer message tracer list (clTr_).
	 * This tracer will be used to process all the ClMessage which pass thorugh the ClSap
	 * 
	 * @param tr a pointer to the tracer instance which has to insert in the tracer list
	 * 
	 * @see ClMessageTracer, ClTracer
	 **/
	static void addCommonTracer(ClMessageTracer *tr);
	
protected:
	/**
	* Write down the trace requested
	* 
	* @param s source of the trace
	* @param ... extended input format for (i.e., writes a sequence of arguments to the given buffer 
	* formatted as the format argument specifies)
	* 
	**/
	void writeTrace(char* s,...);
	/**
	* Write down the trace requested
	* 
	* @param s source of the trace
	* @param ap extended input format for (i.e., writes a sequence of arguments to the given buffer 
	* formatted as the format argument specifies)
	* 
	**/
	void vWriteTrace(char* s, va_list ap);
// 	void writePreamble();
	/**
	* Empty the buffer
	* 
	**/
	void dump();
	/**
	* Write down the requested nam trace
	* 
	* @param s string to write down
	* 
	**/
	void write_nam_trace(const char *s);
	/**
	* Write down the annotation specified in the parameter
	* 
	* @param s annotation to write down
	* 
	**/
	void annotate(const char* s);
	/**  
	 * Write a line in the system trace file  which contains the trace information of a ClMessage
	 * 
	 * @param m pointer to the ClMessage which has to be traced
	 * 
	 **/
	void trace(ClMessage* m);
	/**  
	 * Write a line in the system trace file  which contains the trace information of a synchronous ClMessage
	 * 
	 * @param m pointer to the ClMessage which has to be traced
	 * 
	 **/
	void traceSync(ClMessage* m);

	/** Support for pkt tracing */
	BaseTrace *pt_;
	/** Define the level of depth of tracing */
	int depth_;
	/** Define the level of debug (i.e., for verbosity trace during the simulaztion)*/
	int debug_;
	/** The ClMessage tracer list */
	static ClTracer *clTr_;

	/** Prefix used in the trace of a ClMessage if its direction is <b>to Plugin</b> */
	char dirToPlugin_;
	/** Prefix used in the trace of a ClMessage if its direction is <b>to NodeCore</b> */
	char dirToNodeCore_;
	/** Prefix used in the trace of a ClMessage if its direction is <b>to the up module</b> */
	char dirUp_;
	/** Prefix used in the trace of a ClMessage if its direction is <b>to the down module</b> */
	char dirDown_;
	/** String added before dirToPlugin_ in the trace of the ClMessage (used if the ClMessage direction is <b>to Plugin</b>) */
	char preambleToPlugin_[MAX_PREAMBLE_LENGTH];
	/** String added before dirToNodeCore_ in the trace of the ClMessage (used if the ClMessage direction is <b>to NodeCore</b>) */
	char preambleToNodeCore_[MAX_PREAMBLE_LENGTH];
	/** String added before dirUp_ in the trace of the ClMessage (used if the ClMessage direction is <b>up</b>) */
	char preambleUp_[MAX_PREAMBLE_LENGTH];
	/** String added before dirDown_ in the trace of the ClMessage (used if the ClMessage direction is <b>down</b>) */
	char preambleDown_[MAX_PREAMBLE_LENGTH];


};
#endif
