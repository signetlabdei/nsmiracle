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
 */

/*
 * Split clmsg code from mmac/mmac-module.h distributed in ns2MiracleWimax-0.0.1 
 */



#ifndef MMAC_CLMSG_H
#define MMAC_CLMSG_H



#define MAC_CLMSG_VERBOSITY 5	// verbosity of this message

#include <clmessage.h>


extern ClMessage_t MAC_CLMSG_GET_ADDR;

/**
 * Message to get the mac address for a mac module
 */
class MacClMsgGetAddr : public ClMessage
{
	public:
		/** Constructor #1*/
		MacClMsgGetAddr();
		    
		/** Constructor #2*/
		MacClMsgGetAddr(DestinationType dtype, int value);

		/** Copy constructor */
		MacClMsgGetAddr(MacClMsgGetAddr *m);

    virtual ~MacClMsgGetAddr() { }

		/**
		 * Creates a copy of the object
		 * @return Pointer to a copy of the object
		 */
		ClMessage* copy();

		/**
		 * Sets the address for mac layer
		 * @param addr Address for mac layer
		 */
		void setAddr(int addr);

		/**
		 * Get the the address for mac layer
		 * @return Address for mac layer
		 */
		int getAddr();
	private:
		/** Address for mac layer */
		int addr_;
	
};






#endif /*  MMAC_CLMSG_H */
