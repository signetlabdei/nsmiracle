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
 * \file clsap.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 8-2-06
 */
#ifndef _CLSAP_
#define _CLSAP_

#include <connector-trace.h>
 
class NodeCore;
class Position;
class PlugIn;
/**
 * The ClSAP class is a ConnectorTrace which  connect a Plugin with 
 * the NodeCore belonging to, in order to exchange cross layer messagges
 * It has tracing functionality, thus
 * it could trace the ClMessage which pass through.
 *
 * @see ClMessageTracer, ClTracer, Plugin, NodeCore
 **/
 class ClSAP : virtual public ConnectorTrace{
 public:
	/**
	 * ClSAP constructor
	 **/
	ClSAP();
	/**
	 * ClSAP destructor
	 **/
	virtual ~ClSAP();
	
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * <ul>
	 *  <li><b>module &lt;<i>plugin instance</i>&gt;</b>: 
	 *  	initializes the <i>pluginPtr_</i> pointer to the given Plugin</li>
	 *  <li><b>nodeCore &lt;<i>Node core instance</i>&gt;</b>: 
	 *  	initializes the <i>nodeCorePtr_</i> class attribute</li>
	 *  <li><b>downmodule &lt;<i>module instance</i>&gt;</b>: 
	 *  	initializes the <i>downModule_</i> pointer to the given module</li>
	 *  <li><b>traceToModule &lt;<i>to plugin prefix char for trace</i>&gt;</b>: 
	 *  	initializes the <i>dirToPlugin_</i> class attribute</li>
	 *  <li><b>traceToNodeCore &lt;<i>to node core prefix char for trace</i>&gt;</b>: 
	 *  	initializes the <i>dirToNodeCore_</i> class attribute</li>
	 *  <li><b>preambleToPlugin &lt;<i>to plugin preamble string</i>&gt;</b>: 
	 *  	initializes the <i>preambleToPlugin_</i> class attribute</li>
	 *  <li><b>preambleToNodeCore &lt;<i>to node core preamble string</i>&gt;</b>: 
	 *  	initializes the <i>preambleToNodeCore_</i> class attribute</li>
	 * </ul>
	 * 
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
	 * 
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	 * 
	 * @see NodeCore, Plugin
	 **/
	virtual int command(int argc, const char*const* argv);
	
	/**
	 * Sends a ClMessage to the plugin adding a delay if it is neccessary.
	 * 
	 * @param m a pointer to the ClMessage which has to send to the plugin
	 * @param delay the delivery delay (it must be greater or equal 0)
	 * 
	 **/
	void sendModule(ClMessage* m, double delay);
	/**
	 * Sends a synchronous ClMessage to the plugin.
	 * 
	 * @param m a pointer to the ClMessage which has to send to the plugin
	 * 
	 **/
	void sendSynchronousModule(ClMessage* m);
	/**
	 * Sends a ClMessage to the NodeCore adding a delay if it is neccessary.
	 * 
	 * @param m a pointer to the ClMessage which has to send to the NodeCore
	 * @param delay the delivery delay (it must be greater or equal 0)
	 * 
	 **/
	void sendClLayer(ClMessage* m, double delay);
	/**
	 * Sends a synchronous ClMessage to the NodeCore.
	 * 
	 * @param m a pointer to the ClMessage which has to send to the NodeCore
	 * 
	 **/
	void sendSynchronousClLayer(ClMessage* m);
	/**
	 * Returns the ID of the attached plugin
	 * 
	 * @return the up plugin ID
	 **/
	int getPluginId();
	/**
	 * This method is called when the ClSAP has been scheduled in order to give a delay to a ClMessage
	 * 
	 * @param e pointer to the event which has this ClSAP for handler. It is a ClMessage
	 * 
	 **/
	virtual void handle(Event *e);
	/**
	* Return the pointer to instance of the Position installed
	* 
	* @return the pointer to instance of the Position installed
	* 
	**/
	Position *getPosition();
protected:
	/** Define the level of depth of tracing for the messages directed to the nodecore*/
	int depthNC_;
	/** Define the level of depth of tracing for the messages directed to a plugin*/
	int depthP_;
private:
	/** pointer to the plugin */
	PlugIn* pluginPtr_;
	/** pointer to the NodeCore */
	NodeCore* nodeCorePtr_;
};
#endif
