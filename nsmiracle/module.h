/* -*- Mode:C++ -*- */
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
 * @file   module.h
 * @author Maguolo Federico, Miozzo Marco, Baldo Nicola
 * @date   Mon Nov 26 02:04:38 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef MODULE_H
#define MODULE_H


#include "plugin.h"



/**
 * The MODULE class is a PlugIn which is forced to be in a stack
 * It may have several connections both to module(s) of the up layer in the stack (through SAP)
 * and to the modules(s) of the above layer.
 * It may also have a connection to a cross-layer bus (through ClSAP) in order to communicate
 * to module of un-adjacent layer (by means of ad-hoc messages, extended by ClMessage class)
 *
 * @see PlugIn, SAP, CLSAP, ClMessage, NodeCore
 **/
 

class Module : public PlugIn {
public:
	/**
	* Module constructor
	**/
	Module();
	/**
	* Module destructor
	**/
	virtual ~Module();
	
	/**
	* TCL command interpreter. It implements the following OTcl methods:
	* <ul>
	*  <li><b>addupsap &lt;<i>SAP instance</i>&gt;</b>: 
	*  	add a new SAP instance to the <i>upLayerSAP_</i> pointers array</li>
	*  <li><b>adddownsap &lt;<i>SAP instance</i>&gt;</b>: 
	*  	add a new SAP instance to the <i>downLayerSAP_</i> pointers array</li>
	* </ul>
	* 
	* Moreover it inherits all the OTcl method of PlugIn
	* 
	* 
	* @param argc number of arguments in <i>argv</i>
	* @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
	* 
	* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	* 
	* @see PlugIn
	**/
	virtual int command(int argc, const char*const* argv);

	/**
	* Call the recv method with only the Packet parameter in order to hide the callback (not used in this implementation)
	* and mantain compatibility with the old ns2 code
	* 
	* @param p pointer to the packet will be received
	* @param callback pointer to a Handler class 
	* 
	* @see SAP, ChSAP
	**/
	virtual void recv(Packet* p, Handler* callback);
	/**
	* Call the recv method with only the Packet parameter, by extending it it is possible to use 
	* the information stored in idSrc to understand with module has sent this packet
	* 
	* @param p pointer to the packet will be received
	* @param idSrc unique id of the module that has sent the packet
	* 
	* @see SAP, ChSAP
	**/
	virtual void recv(Packet *p, int idSrc);
	/**
	* Abstract method which has to be filled with the ad-hoc operations that the module has to do
	* to the packet received
	* 
	* @param p pointer to the packet will be received
	* 
	* @see SAP, ChSAP
	**/
	virtual void recv(Packet *p)=0;

	/**
	* Returns the number of the SAP installed in the above layer side
	* 
	* @returns the number od SAP installed in the above layer side
	* 
	* @see SAP
	**/
	int getUpLaySAPnum();
	/**
	* Returns the pointer to the instance of the i-SAP installed in the above layer side
	* 
	* @param i id of the SAP (SAPs are logically placed as an array)
	*
	* @returns the pointer to the instance of the i-SAP installed in the above layer side
	* 
	* @see SAP
	**/
	SAP* getUpLaySAP(int i);
	/**
	* Returns the number of the SAP installed in the bottom layer side
	* 
	* @returns the number od SAP installed in the bottom layer side
	* 
	* @see SAP
	**/
	int getDownLaySAPnum();
	/**
	* Returns the pointer to the instance of the i-SAP installed in the bottom layer side
	* 
	* @param i id of the SAP (SAPs are logically placed as an array)
	*
	* @returns the pointer to the instance of the i-SAP installed in the bottom layer side
	* 
	* @see SAP
	**/
	SAP* getDownLaySAP(int i);
	/**
	* Swap the SAPs in the above layer indicated, i.e., i becomes the id of the instance of the j-SAP and
	* j becomes the instance of the i-SAP
	* 
	* @param i id of first SAP
	* @param j id of second SAP
	* 
	* @see SAP
	**/
	void swapUpLaySAP(int i, int j);
	/**
	* Swap the SAPs in the bottom layer indicated, i.e., i becomes the id of the instance of the j-SAP and
	* j becomes the instance of the i-SAP
	* 
	* @param i id of first SAP
	* @param j id of second SAP
	* 
	* @see SAP
	**/
	void swapDownLaySAP(int i, int j);

protected:

	/**
	* Install a new SAP in the above layer side
	* 
	* @param sap instance of the SAP to be installed
	* 
	* @see SAP
	**/
	void addUpSAP(SAP* sap);

	/**
	* Install a new SAP in the bottom layer side
	* 
	* @param sap instance of the SAP to be installed
	* 
	* @see SAP
	**/
	void addDownSAP(SAP* sap);

	/**
	* Send a packet to the module(s) of the above layer after the delay specified (if they are more than one,
	* it sends a copy to each one)
	* 
	* 
	* @param p pointer of the packet which has to be sent
	* @param delay (optional) delay introduced before transmission [sec.]
	* 
	* @see SAP
	**/
	void sendUp(Packet* p, double delay=0);

	/**
	* Send a packet to the module(s) of the bottom layer after the delay specified (if they are more than one,
	* it sends a copy to each one)
	* 
	* 
	* @param p pointer of the packet which has to be sent
	* @param delay (optional) delay introduced before transmission [sec.]
	* 
	* @see SAP
	**/
	void sendDown(Packet* p,  double delay=0);

	/**
	* Send a packet to the requested module of the above layer after the delay specified (if they are more than one,
	* it sends a copy to each one)
	* 
	* @param moduleId ID of the up module at which you would send the packet
	* @param p pointer of the packet which has to be sent
	* @param delay (optional) delay introduced before transmission [sec.]
	* 
	* @see SAP
	**/
	void sendUp(int moduleId, Packet* p, double delay=0);

	/**
	* Send a packet to the requested module of the bottom layer after the delay specified (if they are more than one,
	* it sends a copy to each one)
	* 
	* @param moduleId ID of the down module at which you would send the packet
	* @param p pointer of the packet which has to be sent
	* @param delay (optional) delay introduced before transmission [sec.]
	* 
	* @see SAP
	**/
	void sendDown(int moduleId, Packet* p,  double delay=0);





	/**
	* Send a ClMessage to the module(s) of the above layer after the delay specified (if they are more than one,
	* it sends a copy to each one)
	* 
	* 
	* @param m pointer of the ClMessage which has to be sent
	* @param delay (optional) delay introduced before transmission [sec.]
	* 
	* @see SAP
	**/
	void sendAsyncClMsgUp(ClMessage* m, double delay=0);


	/**
	* Send a ClMessage to the module(s) of the bottom layer after the delay specified (if they are more than one,
	* it sends a copy to each one)
	* 
	* 
	* @param m pointer of the ClMessage which has to be sent
	* @param delay (optional) delay introduced before transmission [sec.]
	* 
	* @see SAP
	**/
	void sendAsyncClMsgDown(ClMessage* m, double delay=0);


	/**
	* Send a synchronous ClMessage to the module(s) of the bottom layer 
	* (it sends the same the synchronous ClMessage to each one)
	* 
	* 
	* @param m pointer of the ClMessage which has to be sent
	* 
	* @see SAP
	**/
	void sendSyncClMsgDown(ClMessage* m);


	/**
	* Send a  synchronous ClMessage to the module(s) of the above layer
	* (it sends the same the synchronous ClMessage to each one)
	* 
	* @param m pointer of the ClMessage which has to be sent
	* 
	* @see SAP
	**/
	void sendSyncClMsgUp(ClMessage* m);



	/**
	* Drop a packet, i.e., send it to the instance of the Bin in order to manage its tracing and deallocation
	* 
	* 
	* @param p pointer of the packet which has to be dropped
	* @param depth define the depth of the tracing
	* @param reason stores the string in which is indicated the motivation of the drop
	* 
	* @see Bin, Tracer, PacketTracer
	**/
	void drop(Packet* p, int depth, const char* reason = "---");
	
	/**
	* Return a copy of the given packet
	* 
	* 
	* @param p pointer of the packet which has to be copied
	* 
	* @return A copy of the original packet
	* 
	**/
	virtual Packet *copy(Packet *p);

private:
	/** Array of the pointers to the instance of the SAP installed in the above layer side */	
	SAP** upLayerSAP_;
	/** Number of SAP installed in the above layer side */
	int upLaySAPnum_;
	/** Array of the pointers to the instance of the SAP installed in the bottom layer side */
	SAP** downLayerSAP_;
	/** Number of SAP installed in the bottom layer side */
	int downLaySAPnum_;
	
	
protected:

  /**********************
   * Deprecated methods 
   * 
   */

	/** 
	 * \deprecated use sendAsyncClMsgUp(ClMessage* m, double delay=0)
	 * 
	 * @param m 
	 * @param delay 
	 */
	void sendUp(ClMessage* m, double delay=0);

	/** 
	 * \deprecated use sendAsyncClMsUp(ClMessage* m, double delay=0) 
	 * and specify the destination module Id using
	 * ClMessage->dest_
	 * 
	 * @param moduleId 
	 * @param m 
	 * @param delay 
	 */
	void sendUp(int moduleId, ClMessage* m, double delay=0);

	/** 
	 * \deprecated use sendAsyncClMsgDown(ClMessage* m, double delay=0)
	 * 
	 * @param m 
	 * @param delay 
	 */
	void sendDown(ClMessage* m, double delay=0);

	/** 
	 * \deprecated use sendAsyncClMsgDown(ClMessage* p,  double delay=0) 
	 * and specify the destination module Id using ClMessage->dest_
	 * 
	 * @param moduleId 
	 * @param p 
	 * @param delay 
	 */
	void sendDown(int moduleId, ClMessage* p,  double delay=0);

	/** 
	 * \deprecated use sendSyncClMsgDown(ClMessage* m)
	 * 
	 * @param m 
	 */
	void sendSynchronousDown(ClMessage* m);

	/** 
	 * \deprecated use sendSyncClMsgUp(ClMessage* m);
	 * 
	 * @param m 
	 */
	void sendSynchronousUp(ClMessage* m);

	/** 
	 * \deprecated use sendSyncClMsgUp(int moduleId, ClMessage* m);
	 * and specify the destination module Id using ClMessage->dest_
	 *  
	 * @param moduleId 
	 * @param m 
	 */
	void sendSynchronousUp(int moduleId, ClMessage* m);

	/** 
	 * \deprecated use sendSyncClMsgDown(int moduleId, ClMessage* m);
	 * and specify the destination module Id using ClMessage->dest_
	 * 
	 * @param moduleId 
	 * @param m 
	 */
	void sendSynchronousDown(int moduleId, ClMessage* m);





};

#endif /* MODULE_H */
