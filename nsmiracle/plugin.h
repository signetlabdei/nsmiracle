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
 * @file   plugin.h
 * @author Maguolo Federico, Miozzo Marco, Baldo Nicola
 * @date   Mon Nov 26 02:02:49 2007
 *
 */

#ifndef _PLUGIN_
#define _PLUGIN_

#include "logger.h"
#include <bin.h>
#include <memory>
#include <object.h>
#include <sstream>
/**
 * \def MAX_TAG_LENGTH
 * \brief Defines the maximum size in characters of the PlugIn tag
 */
#define MAX_TAG_LENGTH 10

using LogLev = Logger::LogLevel;

class ClSAP;

class Stats;

/**
 * The PLUGIN class is a NsObject which has to be an entity in the architecture
 *that has to be modelled, to facilitate its managment it has an id (and a tag
 *for tracing purposes) It may have a connection to NodeCore in order to be able
 *to exchange messages through the cross-layer bus to others plugin
 *
 * @see CLSAP, NodeCore, ClMessage
 **/
class PlugIn : public NsObject
{
public:
	/**
	 * PlugIn constructor
	 **/
	PlugIn();
	/**
	 * PlugIn destructor
	 **/
	virtual ~PlugIn();

	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * <ul>
	 *  <li><b>Id_ &lt; &gt;</b>:
	 *  	return the id of the PlugIn</li>
	 *  <li><b>Id_ &lt;<i>integer id value</i>&gt;</b>:
	 *  	set the PlugIn id to the requested value</li>
	 *  <li><b>addclsap &lt;<i>ClSAP instance</i>&gt;</b>:
	 *		initializes the <i>clSap_</i> pointer to the given ClSAP</li>
	 *  <li><b>addbin &lt;<i>Bin instance</i>&gt;</b>:
	 *		initializes the <i>binPtr_</i> pointer to the given Bin</li>
	 *  <li><b>settag &lt;<i>tag string</i>&gt;</b>:
	 *  	initializes the <i>tag_</i> class attribute</li>
	 * </ul>
	 *
	 * Moreover it inherits all the OTcl method of NsObject
	 *
	 *
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the comand parameters (Note that
	 *argv[0] is the name of the object)
	 *
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 *succesfully or no
	 *
	 * @see Nodecore, ClSAP, Bin
	 **/
	virtual int command(int argc, const char *const *argv);

	/**
	 * Cross-Layer messages asynchronous interpreter.
	 *
	 * It has to be properly extended in order to
	 * interpret custom cross-layer messages used by this particular plug-in.
	 * This type of communication does not necessarily need a reply.
	 *
	 * @note Each implementation of this method is responsible for
	 * deleting the ClMessage instance referred to by ClMessage* m
	 * when the message is received
	 *
	 * Normally, classes inheriting from other classes should call
	 * the recvAsyncClMsg() method of the parent when an unknown
	 * ClMsg is detected, in order to allow the parent to handle
	 * unknown message types.
	 *
	 * A  very importan exception to this rule are classes
	 * inheriting directly from either Plugin or Module. These
	 * classes should NOT call  neither Plugin::recvAsyncClMsg()
	 * nor Module::recvAsyncClMsg() for unknown messages; instead,
	 * they should just free the memory associated with ClMessage* m
	 *
	 * @param m an instance of <i>ClMessage</i> that represent the message
	 *received
	 *
	 * @return 0 if the method was re-implemented by somebody,
	 * RETVAL_NOT_IMPLEMENTED if it is the implementation provided
	 * by the parent Plugin class (note that Module does not
	 * re-implement it, so also Module::recvAsyncClMsg() returns
	 * RETVAL_NOT_IMPLEMENTED)
	 *
	 * @see NodeCore, ClMessage, ClSAP, ClTracer
	 **/
	virtual int recvAsyncClMsg(ClMessage *m);

	/**
	 * Cross-Layer messages synchronous interpreter. It has to be properly
	 *extended in order to interpret custom cross-layer messages used by this
	 *particular plug-in. This type of communication need to be directly
	 *answered in the message exchanged in order to be synchronous with the
	 *source.
	 *
	 * @param m an instance of <i>ClMessage</i> that represent the message
	 *received and used for the answer
	 *
	 * @return zero if successful
	 *
	 * @see NodeCore, ClMessage, ClSAP, ClTracer
	 **/
	virtual int recvSyncClMsg(ClMessage *m);

	/**
	 * Method to send the log message as a stringstream to the logger.
	 *
	 * @param log_level LogLevel representing the amout of logs.
	 * @param module String name of the plugin/module.
	 * @param message Stringstream log message.
	 *
	 */
	virtual void printOnLog(LogLev log_level, const std::string &module,
			const std::stringstream &message) const;

	/**
	 * Method to send the log message string to the logger.
	 *
	 * @param log_level LogLevel representing the amout of logs.
	 * @param module String name of the plugin/module.
	 * @param message String log message.
	 *
	 */
	virtual void printOnLog(LogLev log_level, const std::string &module,
			const std::string &message) const;

	/**
	 * Method inherited from NsObject, it is not used in this level of
	 *inheritance since PlugIn Class use only cross-layer messages to
	 *communicate with the other entities in the architecture
	 *
	 * @param p pointer to the packet will be received
	 * @param callback pointer to a Handler class
	 *
	 * @see NodeCore, ClMessage, ClSAP, ClTracer
	 **/
	void recv(Packet *p, Handler *callback = 0);
	/**
	 * Return the PlugIn id
	 *
	 * @return the PlugIn id [integer]
	 *
	 **/
	int getId();
	/**
	 * Return the PlugIn stack id
	 *
	 * @return the PlugIn stack id [integer]
	 *
	 **/
	int getStackId();
	/**
	 * Set the PlugIn id
	 *
	 * @param val the integer value that will be the PlugIn id
	 *
	 **/
	void setId(int val);
	/**
	 * Set the PlugIn stack id
	 *
	 * @param val the integer value that will be the PlugIn stack id
	 *
	 **/
	void setStackId(int val);
	/**
	 * Return the PlugIn tag
	 *
	 * @return the PlugIn tag string
	 *
	 **/
	int getTag(char *buf, int size);

	/**
	 * Return a const pointer to the PlugIn tag
	 *
	 * @return const char pointer to the char array
	 *
	 **/
	const char *
	getTag()
	{
		return tag_;
	}
	/**
	 * Return the pointer to the Position instance within the NodeCore
	 *
	 * @return the pointer to the Position instance within the NodeCore
	 *
	 **/
	Position *getPosition();

	/**
	 * Set the PlugIn layer id
	 *
	 * @param val the integer value that will be the PlugIn layer id
	 *
	 **/
	void setLayer(int val);
	/**
	 * Return the PlugIn layer id
	 *
	 * @return the PlugIn layer id
	 *
	 **/
	int getLayer();

protected:
	/**
	 * Send an asyncronous cross-layer message, i.e., it does not require a
	 *direct answer from the target. It is possible to insert a delay before the
	 *communcation will start.
	 *
	 * @param m an instance of <i>ClMessage</i> to be sent
	 * @param delay (optional) delay [sec.] to introduce before the comunication
	 *starts
	 *
	 * @see NodecCore, ClMessage, ClTracer
	 **/
	void sendAsyncClMsg(ClMessage *m, double delay = 0);

	/**
	 * Send a syncronous cross-layer message, i.e., the target entity has to
	 *directly answer through the message exchanged
	 *
	 * @param m an instance of <i>ClMessage</i> to be sent (and in which will be
	 *the target answer)
	 *
	 * @see NodecCore, ClMessage, ClTracer
	 **/
	void sendSynchronousCl(ClMessage *m);

	void sendSyncClMsg(ClMessage *m);

	/**
	 * Drop a  cross-layer message, i.e., send it to the instance of the Bin in
	 *order to manage its tracing and deallocation
	 *
	 *
	 * @param m pointer of the cross-layer message which has to be dropped
	 * @param reason stores the string in which is indicated the motivation of
	 *the drop
	 *
	 * @see Bin, Tracer, PacketTracer
	 **/
	void drop(ClMessage *m, const char *reason = "---");

	int uLayerId_; /**< PlugIn layer identifier. */
	int stack_id; /**< Sub-stack identifier, useful in multi-stack case*/
	int node_id; /**< Node identifier. */
	int enable_log; /**< Enable the log of a specific module/plugin. */
	Bin *binPtr_; /**< Pointer to the instance of the global Bin. */
	Stats *stats_ptr; /**< Pointer to the metrics container. By default it is
					   * null, it should be dynamically allocatated in the
					   * derived classes */
	/** String identifies the plug-in in the bin-tracing */
	char tag_[MAX_TAG_LENGTH];

private:
	/** PlugIn id */
	int uid_;
	/** Pointer to the manager of the cross layer bus and cross layer messages
	 */
	ClSAP *clsap_;

protected:
	static Logger logger; /**< Logger class used to print log messages. By
						   * default the log level is set to NONE. */

	/******************************
	 * Deprecated Methods
	 *
	 */
	/**
	 * \deprecated use sendAsyncClMsg(ClMessage* m, double delay) instead
	 *
	 * @param m
	 * @param delay
	 */
	void sendCl(ClMessage *m, double delay = 0);

public:
	/**
	 * \deprecated  use recvAsyncClMsg(ClMessage* m) instead
	 *
	 * @param m
	 *
	 * @return
	 */
	virtual int crLayCommand(ClMessage *m);

	/**
	 * \deprecated  use recvSyncClMsg(ClMessage* m) instead
	 *
	 * @param m
	 *
	 * @return
	 */
	virtual int crLaySynchronousCommand(ClMessage *m);
};
#endif
