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
 * \file packettracer.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 8-2-06
 * 
 * This file contains the classes used for give a readable Packet trace during the simulation. 
 * Note that, there is only an instace of PktTracer (similarly to the scheduler) in the process which contains
 * a list of Tracer that know how format some header of the given Packet
 */
#ifndef _PktTracer_
#define _PktTracer_

class Packet;
class SAP;

/**
 * A Tracer is an object which know how to trace a Packet header (or a Packet) 
 * and whether this header is contained in the given packet.
 * There is only one instance of Tracer for each header (or Packet) during the simulation
 * and the order with which a Packet pass through the Tracer objects is given by the Level (ascending).
 * Tracer has an abstract method <i>format</i> which must be overloaded by the derived classes. 
 * This method can permorm the string that will be writen in the trace file with the <i>writeTrace</i>
 * protected method.
 *
 * @see PktTracer, SAP
 **/

class Tracer
{
	public:
		/**
		* Tracer constructor. The Level param indicates the position 
		* which this tracer will be stored in the system packet tracer list
		* 
		* @param level position int the packet tracer list (0 will be the first tracer 
		* 	that will process the packet)
		* 
		* @remarks If two Tracer objects have the same level the insertion order will be taken into account
		* 
		* @see PktTracer
		**/
		Tracer(int level);
		
		/**
		 * This method is called by the PktTracer in order to trace the packet. This method call the format method
		 * and then the trace method of the next_ Tracer (if it is not null)
		 * 
		 * @param p pointer to the Packet to be traced
		 * @param sap pointer to the SAP instance which ask for the trace
		 * 
		 * @remarks This method is not virtual, thus it can not be overloadable. Whoever wants to 
		 * 	write a Tracer <b>must</b> overload the format method
		 * 	
		 * @see format, SAP
		 * 
		 **/
		void trace(Packet *p, SAP *sap);
		
		/**
		 * This method has to write a formatted string with the packet's information
		 * using the method writeTrace of the given SAP.
		 * 
		 * @param p pointer to the Packet to be traced
		 * @param sap pointer to the SAP instance which ask for the trace
		 * 
		 * @remarks This method is virtual, thus it <b>must</b> be overloaded. The Tracer class is a 
		 * 	SAP friend class, but a derived class can not call the protected method of SAP. This mean that 
		 * 	when we wants to write a string in the trace file, we have to call the Tracer <i>writeTrace</i>
		 * 	method passing the SAP instance.
		 * 
		 * @see writeTrace, ConnectorTrace, SAP
		 **/
		virtual void format(Packet *p, SAP *sap) = 0;
		/**
		 * Returns the level_ of the tracer
		 * 
		 * @return the level_ attribute of the Tracer
		 * 
		 * @see level_
		 **/
		int level();
		
		/**
		 * Returns the pointer to the next Tracer in the Tracer chain
		 * 
		 * @return tha pointer of the next tracer of the Tracer list
		 * 
		 * @see PktTracer
		 **/
		Tracer *next();
		
		/**
		 * Set the next_ Tracer in the Tracer objects list.
		 * 
		 * @param tr pointer to the next Tracer
		 * 
		 **/
		void next(Tracer *tr);
	protected:
		/**
		 * This method write a string in the trace file using the method <i>vWriteTRace</i> of the given SAP
		 * instance
		 * 
		 * @param sap the SAP instance where the string will write.
		 * @param s string to be write in the trace file. The sintax is equal to the printf C function
		 * 
		 * @see vWriteTRace, ConnectorTrace, SAP
		 **/
		void writeTrace(SAP *sap, char* s,...);
	private:
		/**
		 * The Tracer storage level
		 **/
		int level_;
		/**
		 * The next Tracer in the PktTracer chain. The last tracer has this attribute equal to 0
		 **/
		Tracer *next_;
};

/**
 * The PktTracer class contains a list of Tracer. A Tracer is added to a single linked list using an 
 * ascending order of Tracer level. If two or more Tracer abjects have the same level, the PktTrace insert the 
 * new Tracer after all the Tracer with the same level.
 * 
 * @see Tracer
 **/
class PktTracer
{
	public:
		/**
		 * PktTracer constructor. It reset the tr_ class attribute
		 * 
		 * @see tr_
		 **/
		PktTracer();
		
		/**
		 * This method adds a new tracer sorted by level ascending. A new tracer 
		 * of the same level is inserted in tail of the already inserted tracer
		 * 
		 * @param newTr pointer to the tracer which has to be added
		 **/
		void addTracer(Tracer *newTr);
		
		/**
		 * Begin the trace of the packet that is crossing the SAP. The Packet will pass through
		 * all the Tracer objects in the list.
		 * 
		 * @param p pointer to the Packet to trace
		 * @param sap pointer to the SAP instance which call this method (useful to write in the BaseTrace)
		 * 
		 * @see ConnectorTrace, SAP
		 **/
		void trace(Packet *p, SAP *sap);

	private:
		/**
		 * First Tracer of the Tracer objects list
		 **/
		Tracer* tr_;
};


#endif
