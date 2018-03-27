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
 * \file headertracer.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 8-28-06
 * 
 * This file contains a tracer for the Packet common header and a tracer for the IP header. The output format is similar to
 * the CMU-Monarch format.  These modules are contained in separate library called <i>libtrace.so</i>
 * 
 */
#ifndef _HEADERTRACER_
#define _HEADERTRACER_
#include "packettracer.h"

/**
 * The CommonHeaderTracer is born for tracing the common header of a Packet, which is present in all packets because it
 * contains:
 * <ul>
 * <li> the packet ID: it is used to distinguish one packet from another because it is unique
 * <li> the packet type: a string which explains the type of the packet (tcp, udp, cbr,...)
 * <li> the packet size: the size of the packet in bytes
 * </ul>
 * The class extend Tracer, but it is compiled and linked in another library. Thus, users which want
 * to change the output format of the trace file can write a new class and plug their library in the simulation 
 * (instead this). When the library are loaded a CommonHeaderTracer are instanced and is plugged to the PktTracer by invoking
 * the Sap addTracer static method.
 * 
 * @see Tracer, PktTracer, SAP, format
 **/
class CommonHeaderTracer : public Tracer
{
	public:
		/**
		 * The class constructor initialize the level to 1. This means that it will be the first Tracer which will 
		 * process the incoming packets
		 * 
		 * @see Tracer
		 **/
		CommonHeaderTracer();
		
		/**
		 * This method writes a string in the trace file with the following format: <br>
		 * 
		 * <center><i>UID TYPE SIZE DIRECTION</i></center>
		 * 
		 * where <ul>
		 * <li> <i>UID</i> is the packet ID
		 * <li> <i>TYPE</i> is the packet type
		 * <li> <i>SIZE</i> is the size of the packet in bytes
		 * <li> <i>DIRECTION</i> is the packet direction (u=to up module, d=to down module)
		 * </ul>
		 * 
		 * @param p pointer to the packet to be traced
		 * @param sap pointer to the SAP instance which ask for the trace
		 * 
		 **/
	// ad hoc tracer for common header
		void format(Packet *p, SAP *sap);

};

/**
 * The IpHeaderTracer is born for tracing the IP header of a Packet, which contains:
 * <ul>
 * <li> the IP address of the destination and the source of the packet
 * <li> the port of the destination and the source of the packet
 * <li> the remain TTL
 * <li> the next hop
 * </ul>
 * The class extend Tracer, but it is compiled and linked in another library. Thus, users which want
 * to change the output format of the trace file can write a new class and plug their library in the simulation 
 * (instead this). When the library are loaded a IpHeaderTracer are instanced and is plugged to the PktTracer by invoking
 * the Sap addTracer static method.
 * 
 * @see Tracer, PktTracer, SAP, format
 **/
class IpHeaderTracer : public Tracer
{
	public:
		/**
		 * The class constructor initialize the level to 3.
		 * 
		 * @see Tracer
		 **/
		IpHeaderTracer();
		
		/**
		 * This method writes a string in the trace file with the following format: <br>
		 * 
		 * <center>--IP-- <i>SRC:SPORT DST:DPORT TTL NH</i></center>
		 * 
		 * where <ul>
		 * <li> <i>SRC</i> is IP address of the source node
		 * <li> <i>SPORT</i> is the port of the source node (TCP or UDP)
		 * <li> <i>DST</i> is the IP address of the destination node
		 * <li> <i>DPORT</i> is the port of the destination node (TCP or UDP)
		 * <li> <i>TTL</i> is the remain Time To Live
		 * <li> <i>NH</i> is the IP address of the next hop node
		 * </ul>
		 * 
		 * @param p pointer to the packet to be traced
		 * @param sap pointer to the SAP instance which ask for the trace
		 * 
		 **/
		// ad hoc tracer for common header
		void format(Packet *p, SAP *sap);

};

#endif
