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
 * \file clcommontracer.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 8-21-06
 * 
 * This file contains a simple ClMessage tracer which print the module id of the sendere module and the destination module 
 * (if the ClMessage is unicast). This module is contained in separate library called <i>libcltrace.so</i>
 * 
 */
#ifndef _CLCOMMONTRACE_
#define _CLCOMMONTRACE_

#include "cltracer.h"
/**
 * The ClCommonTracer is born for tracing the common attributes of the ClMessages, i.e. the suorce and the destination 
 * address. The class extend ClMessageTracer, but it is compiled and linked in another library. Thus, user which want
 * to change the output format of the trace file can write a new class and plug their library in the simulation 
 * (instead this). When the library are loaded a ClCommonTracer are instanced and is plugged to the ClTracer by invoking
 * the ClSap addCommonTracer method.
 * 
 * @see ClMessageTracer, ClTracer, ClSAP
 **/
class ClCommonTracer : public ClMessageTracer
{
	public:
		/**
		 * The class constructor initialize the clType to 0. This value is not relevant because this class 
		 * is a common ClMessageTracer thus it has not to checked the incoming ClMessage type.
		 * 
		 * @see ClMessageTracer, ClMessage
		 **/
		ClCommonTracer();
		/**
		 * This method writes a string in the trace file with the following format: <br>
		 * 
		 * <center>[CMN: <i>id src dstType dst direction</i>]</center>
		 * 
		 * where <ul>
		 * <li> <i>id</i> is the message ID
		 * <li> <i>src</i> is the source module id
		 * <li> <i>dstType</i> is the destination type (UNI or BRD)
		 * <li> <i>dst</i> is the destination (modul id, layer number or all layers)
		 * <li> <i>direction</i> is the message direction (N=to node core, M=to module)
		 * </ul>
		 * 
		 * @param m pointer to the ClMessage to be traced
		 * @param sap pointer to the ClSAP instance which ask for the trace
		 * 
		 **/
		void format(ClMessage *m, ConnectorTrace *sap);
};


#endif
