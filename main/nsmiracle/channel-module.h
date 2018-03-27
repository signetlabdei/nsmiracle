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
 * \file channel-module.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 9-7-06
 */

#ifndef _CHANNELMODULE_
#define _CHANNELMODULE_
#include "module.h"
#include "chsap.h"
/**
 * The ChannelModule class is a module which has connection (SAP) only to the above layer side
 * through the apposite ChSAP
 *
 * @see ChSAP, Module
 **/

class ChannelModule : public Module {
	public:
		/**
		* ChannelModule constructor
		**/
		ChannelModule();
		/**
		* ChannelModule destructor
		**/
		virtual ~ChannelModule();
		/**
		* TCL command interpreter. It implements the following OTcl methods:
		* <ul>
		*  <li><b>addsap &lt;<i>ChSAP instance</i>&gt;</b>: 
		*  	add a new SAP instance to the <i>upLayerSAP_</i> pointers array</li>
		* </ul>
		* 
		* Moreover it inherits all the OTcl method of Module
		* 
		* 
		* @param argc number of arguments in <i>argv</i>
		* @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
		* 
		* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
		* 
		* @see Module, ChSAP, PlugIn
		**/
		virtual int command(int argc, const char*const* argv);
		/**
		* Call the recv method casting the Handler parameter as a ChSAP in order to interface old version of code with 
		* the new one
		* 
		* @param p pointer to the packet will be received
		* @param h pointer to a Handler class 
		*
		* @see ChSAP
		**/
		virtual void recv(Packet* p, Handler *h);
		/**
		* Method that intefaces the basic <i>recv</i> method (with only packet as parameter) with the specific
		* Channel Module <i>recv</i> method, which uses the <i>ChSAP</i> to have knowledges about the source
		* 
		* @param p pointer to the packet will be received
		* 
		* @see ChSAP
		**/
		virtual void recv(Packet* p);
		/**
		* Abstract method used to receive packet from the module of the above layer, it gives also knowledges about
		* the source through the instance of <i>ChSAP</i> which connected them
		* 
		* @param p pointer to the packet will be received
		* @param chsap pointer of the instance which connects the ChannelModule with the source Module
		* 
		* @see ChSAP, Module
		**/
		virtual void recv(Packet* p, ChSAP *chsap) = 0;
		/**
		* Return a pointer to instance of the i-<i>ChSAP</i>
		* 
		* @param i index of the ChSAP requested
		* 
		* @see ChSAP
		**/
		ChSAP* getChSAP(int i);
		/**
		* Swap two instances of the <i>ChSAP</i> installed, i.e., the i-<i>ChSAP</i> one with the j-<i>ChSAP</i> one
		* 
		* @param i index of the first ChSAP to be swapped
		* @param j index of the second ChSAP to be swapped
		* 
		* @see ChSAP
		**/
		void swapChSAP(int i, int j);
		/**
		* Return the number of <i>ChSAP</i> installed
		* 
		* @return the number of ChSAP installed
		* 
		* @see ChSAP
		**/
		int getChSAPnum();
		
};

#endif
