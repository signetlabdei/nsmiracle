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
 * \file cltracer.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 8-21-06
 * 
 * This file contains the classes used for give a readable Cross Layer Message (ClMessage) trace during the simulation. 
 * Note that, there is only an instace of ClTracer (similarly to the scheduler) in the process which contains
 * ClMessageTracer that know how format the given ClMessage. 
 * 
 * 
 */
#ifndef _CLTRACER_
#define _CLTRACER_
#include "basetrace.h"
#include "clmessage.h"
/**
 * \def MAXHASHINDEX
 * \brief Defines the maximun value of the hash index used in the hash table in which are placed the ClMessageTracer
 *
 **/
#define MAXHASHINDEX 10

class ConnectorTrace;

/**
 * A ClMessageTracer is an object which know how to trace a ClMessage. The ClMessage delivery is done by the ClTracer, which
 * check the incoming ClMessage ID and give it to the correctly ClMessageTracer. Hence the incoming ClMessage ID is equal to 
 * the ClMessageTracer clType_. <br>
 * There is only one instance of ClMessageTracer for ClMessage during the simulation.
 * ClMessageTracer has an abstract method <i>format</i> which must be overloaded by the derived classes. 
 * This method can permorm the string that will be writen in the trace file with the <i>writeTrace</i>
 * protected method.
 *
 * @see ClMessage, ClSAP, ClTracer, clType_
 **/

class ClMessageTracer
{
	public:
		/**
		* ClMessageTracer constructor. The clType param indicates the ID of the ClMessage which this class can trace.
		* 
		* @param clType the ID of the ClMessage which this class can trace
		* 
		* @remarks If two ClMessageTracer have the same clType_ the simulation does not return erro, but the output
		* 	trace can have some lines with a trace format and some lines with the other.
		* 
		* @see ClTracer
		**/
		ClMessageTracer(ClMessage_t clType);

		/**
		 * This method has to write a formatted string with the ClMessage's information
		 * using the method writeTrace of the given ClSAP.
		 * 
		 * @param m pointer to the ClMessage to be traced
		 * @param sap pointer to the ClSAP instance which ask for the trace
		 * 
		 * @remarks This method is virtual, thus it <b>must</b> be overloaded. The ClMessageTracer class is a 
		 * 	ClSAP friend class, but a derived class can not call the protected method of ClSAP. This mean that 
		 * 	when we wants to write a string in the trace file, we have to call the ClMessageTracer 
		 * 	<i>writeTrace</i> method passing the ClSAP instance.
		 * 
		 * @see writeTrace, ConnectorTrace, SAP
		 **/
		virtual void format(ClMessage *m, ConnectorTrace *sap) = 0;
		
		/**
		 * Returns the clType_ of the ClMessageTracer
		 * 
		 * @return the clType_ of the ClMessageTracer
		 * 
		 * @see clType_
		 **/
		ClMessage_t clType();
		
		/**
		 * Set the next_ ClMessageTracer in the ClTracer objects list (The are stored in a HashTable, but the ClMessageTracer
		 * with the same key are stored in a circular list).
		 * 
		 * @param next pointer to the next ClMessageTracer
		 * 
		 * @see ClTracer
		 **/
		void next(ClMessageTracer *next);
		/**
		 * Returns the pointer to the next ClMessageTracer in the ClTracer list
		 * 
		 * @return tha pointer of the next ClMessageTracer of the ClTracer list
		 * 
		 * @see ClTracer
		 **/
		ClMessageTracer *next();
	protected:
		/**
		 * This method write a string in the trace file using the method <i>vWriteTRace</i> of the given ClSAP
		 * instance
		 * 
		 * @param sap the ClSAP instance where the string will write.
		 * @param s string to be write in the trace file. The sintax is equal to the printf C function
		 * 
		 * @see ConnectorTrace::vWriteTrace, ConnectorTrace, ClSAP
		 **/
		void writeTrace(ConnectorTrace *sap, char* s,...);
	private:
		/**
		 * The ID of the ClMessage which this class can trace
		 **/
		ClMessage_t clType_;
		/**
		 * The next ClMessageTracer in the CLTracer chain
		 **/
		ClMessageTracer *next_;
};
/**
 * The ClTracer class contains the ClMessageTracer. The ClMessageTracer are stored in an Hash Table
 * in order to improve the performance. The key of the Hash Table is the rest of the division of the Message ID
 * by the constant MAXHASHINDEX.
 * If two or more ClMessageTracer objects have the same key, the ClTrace insert these objects in an circular list.
 * Note that if two or more ClMessageTracer have the same clType_ the simulator does not give error, but the trace file 
 * could be unexpected.
 * It is possible to declare a common ClMessageTracer, which proccess all the incoming ClMessage. Note that there could 
 * be at most one common ClMessageTracer in the simultation.
 * 
 * @see ClMessageTracer
 **/
class ClTracer
{
	public:
		/**
		 * ClTracer constructor. It set tr_[i]=0 for i=0,...,MAXHASHINDEX-1 and commonTr_=0
		 * 
		 * @see tr_
		 **/
		ClTracer();
		
		// add a new tracer (insert sorted by level, decreasing from the max)
		// (a new tracer of the same level is inserted in tail of the already inserted tracer)
		/**
		 * This method adds a new ClMessageTracer. It compute the key of the ClMessageTracer and call 
		 * addTracer(ClMessageTracer *, ClMessageTracer**) private method at which it pass the 
		 * ClMessageTracer that can be added and the pointer of the position in the hash table
		 * 
		 * @param newTr pointer to the ClMessageTracer which has to be added
		 * 
		 * @see addTracer(ClMessageTracer *, ClMessageTracer**)
		 **/
		void addTracer(ClMessageTracer *newTr);
		/**
		 * This method adds a new common ClMessageTracer. It set commonTr_ equal to newTr
		 * 
		 * @param newTr pointer to the ClMessageTracer which has to be added
		 **/
		void addCommonTracer(ClMessageTracer *newTr);
		// begin the trace of the packet that is crossing the SAP
		/**
		 * Begin the trace of the ClMessage that is crossing the ClSAP. First The ClMessage is processed by the 
		 * common ClMessageTracer and then by the specific ClMessageTracer in the hash table.
		 * 
		 * @param m pointer to the ClMessage to trace
		 * @param clsap pointer to the ClSAP instance which call this method (useful to write in the BaseTrace)
		 * 
		 * @see ConnectorTrace, ClSAP
		 **/
		void trace(ClMessage *m, ConnectorTrace *clsap);

	private:
		/**
		 * This method adds a new ClMessageTracer to a circular list which have the first node pointed by *tr
		 * 
		 * @param newTr pointer to the ClMessageTracer which has to be added
		 * @param tr pointer to the pointer of the first element in the circular list (i.e. &tr_[i])
		 * 
		 * @see addTracer(ClMessageTracer *)
		**/
		void addTracer(ClMessageTracer *newTr, ClMessageTracer** tr);
		/**
		 * ClMessageTracer hash table. Each entry of the array points to the first element of a circular list
		 **/
		ClMessageTracer* tr_[MAXHASHINDEX];
		/**
		 * Pointer to the common ClMessageTracer. Each incoming ClMessage is processed by the common ClMessageTracer
		 * at first, and then by the corresponding ClMessageTRec er in the hash table (if exists)
		 **/
		ClMessageTracer* commonTr_;
};


#endif
