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
 * @file   clmessage.h
 * @author Maguolo Federico, Miozzo Marco, Baldo Nicola
 * @date   Mon Nov 26 02:03:42 2007
 * 
 * @brief  
 * 
 * 
 */


#ifndef _COMMONCLMSG_
#define _COMMONCLMSG_

#include <string.h>
#include <scheduler.h>

/**
 * \def MAX_MODULE_TAG
 * \brief Maximum size in characters of the module tag
 *
 **/
#define MAX_MODULE_TAG 6
//NOT USED ... #define MAX_CR_LAY_TAG 8			// size in character of the exchanged string in cross-layer command
/**
 * \def INVALID_DESTINATION
 * \brief Error code for invalid destination
 *
 **/
#define INVALID_DESTINATION -1
//NOT USED.. #define ALL_LEVEL 0					// broadcasr address for cross-layer messages directed to all layers
/**
 * \def NO_MODULE
 * \brief Return value when the module ID can not be sent
 *
 **/
#define NO_MODULE -1


 /**
 * \def CLBROADCASTADDR
 * \brief Defines the value used to codify the BROADCAST address for cross-layer message within the architecture
 */
#define CLBROADCASTADDR -1



 /**
 * \def CLPLUGINLAYERNUM
 * \brief Defines the layer number used for PlugIns (which are not within any layer of the protocol stack)
 */
#define CLPLUGINLAYERNUM 0

/**
 * \enum DestinationType
 * \brief Defines the type of destination endpoint of the cross layer message (unique or multiple)
 *
 **/
enum DestinationType{
	UNICAST,
	BROADCAST
};
/**
 * \enum ClDirection
 * \brief Defines the istantaneous direction of the message (i.e., whether it is directed to the NodeCore or to module/plugin
 *
 **/
enum ClDirection{
	TONODECORE,
	TOMODULE,
	UP,
	DOWN
};
/**
 * \typedef ClMessage_t
 * \brief Defines that a ClMessage type is an unsigned int
 *
 **/
typedef unsigned int ClMessage_t;

/*
 * the father class of all cross layer messages
*/
/**
 * This is the father class of all cross layer messages (i.e., by extending it
 * it is possible to define an ad hoc messages with any type of parameter in 
 * order to allow cross any type of cross layer communication).
 * There are 2 message type:
 * <ul>
 * <li><b>UNICAST</b>:
 * 	a message directed to a specific module/plugin recipient </li>
 * <li><b>BROADCAST</b>:
 * a message directed to all the modules of a specific layer or of the whole architecture</li>
 * </ul>
 *
 * @see ClTracer, ClMessageTracer, ClSAP, NodeCore, PlugIn, Module
 **/
class ClMessage : public Event{
public:
	/**
	 * Broadcast ClMessage constructor.
	 *
	 * This is the parent constructor that should be called by a
	 * class inheriting from ClMessage and implementing a
	 * broadcast message. 
	 * 
	 * @param verbosity define the level of verbosity of the cross-layer message
	 * @param type define the type of the cross-layer message (this parameter is given by
	 * the simulator during the initialization phase, i.e., it is the value returned by the 
	 * <i>addClMessage</i> method)
	 *
	 **/
	ClMessage(int verbosity, ClMessage_t type);

	/**
	 * Copy-by-reference constructor.
	 * 
	 * \warning this constructor must be redefined for each child in order
	 *  to obtain a specular copy of the clmessage (including
	 *  possible new attributes)
	 *
	 * @param m a cross-layer message thas has to initialize the new one
	 *
	 **/
	ClMessage(ClMessage *m);


	/**
	 * Standard ClMessage constructor.
	 *
	 * This constructor allows settings all the relevant
	 * parameters of ClMessage, and therefore can be used to
	 * create all types of messages (unicast, broadcast, layercast).
	 * 
	 * @param verbosity define the level of verbosity of the cross-layer message
	 * @param type define the type of the cross-layer message (this parameter is given by
	 * the simulator during the initialization phase, i.e., it is the value returned by the 
	 * <i>addClMessage</i> method)
	 * @param dtype define the type of the destination (UNICAST or BROADCAST)
	 * @param value its intepretation is function of the <i>dtype</i> paramter:
	 * <ul>
	 * <li><i>dtype</i>=UNICAST:
	 * 	the id of the destination recipient</li>
	 * <li><i>dtype</i>=BROADCAST:
	 * 	the number of the layer to which the message has to be sent,
	 * 	or <i>CLBROADCASTADDR</i> if the message is directed
	 * to all the modules of the whole architecture</li> 
	 * </ul> 
	 *
	 **/
	ClMessage(int verbosity, ClMessage_t type, DestinationType dtype, int value);

	/**
	 * ClMessage destructor
	 *
	 **/
	virtual ~ClMessage();

	/**
	 * Return the destination type of the cross layer message (i.e., UNICAST or BROADCAST)
	 * 
	 * @return the destination type of the cross layer message
	 **/
	DestinationType getDestType();

	/**
	 * Return the destination of the cross layer message. According to its destination-type it
	 * is the id of the destination module/plugin in case of UNICAST messase, while it is the 
	 * layer id or the broadcast address in case of BROADCAST message
	 * 
	 * @return the destination of the cross layer message
	 **/
	int getDest();

	/**
	 * Return the source id of the cross layer message
	 * 
	 * @return the source id of the cross layer message
	 **/
	int getSource();

	/**
	 * Set the source id of the cross layer message
	 * 
	 * @param src the source id of the cross layer message
	 */
	void setSource(int src);

	/**
	 * Set the destination id of the cross layer message
	 * 
	 * @param src the destination id of the cross layer message
	 */
	void setDest(int dst);


	/**
	 * Return a copy of the message
	 * 
	 * \warning this method must be always implemented by child-classes 
	 * in order to get the copy of all new added attributes
	 *
	 *
	 **/
	virtual ClMessage *copy();

	/**
	 * Return the verbosity of the cross layer message
	 * 
	 * @return the verbosity of the cross layer message
	 **/
	int verbosity();

	/**
	 * Return the type of the cross layer message (i.e., its own unique id returned 
	 * from  <i>addClMessage</i> method during the initialization phase)
	 * 
	 * @return the type of the cross layer message
	 **/
	ClMessage_t type();

	/**
	 * Return the direction of the cross layer message, in detail:
	 * <ul>
	 * <li>TOMODULE: when the cross layer message is directed to a module(s)/plugin(s)</li>
	 * <li>TONODECORE: when the cross layer message is directed to the <i>NodeCore</i></li>
	 * </ul>
	 * 
	 * @return the verbosity of the cross layer message
	 **/
	ClDirection direction();

	/**
	 * Set the direction of the cross layer message
	 * 
	 * @param dir the direction requested (i.e., TOMODULE or TONODECORE)
	 *
	 **/
	void direction(ClDirection dir);

	/**
	 * Return the unique id of the cross layer message (each message generated has
	 * an integer id unique in the simulation)
	 * 
	 * @return the type of the cross layer message
	 **/
	unsigned int uid();

	/**
	 * Static method to add a new type of cross layer message during the initialization phase
	 * 
	 * @return the type of the message (i.e., a unique id to identify it during the simulation)
	 *
	 **/
	static ClMessage_t addClMessage();

protected:
	/** Static variable used to generate the id of all the cross layer message used in the 
		simulation */
	static ClMessage_t nType_;

	/** Define the type of the message (i.e., unique id returned from the <i>addClMessage</i> during 
	the initialization phase) */
	ClMessage_t type_;

	/** Define the destination type of the message (i.e., UNICAST or BROADCAST) to correctly 
	codify the <i>dest_</i> address*/
	DestinationType destType_;

	/** Define the source id) */
	int source_;

	/** Define the destination id (it has to be codified according to <i>destType_</i> attribute) */
	int dest_;

	/** Define the verbosity of the message (i.e., the depht of tracing requested for this message) */
	int verbosity_;

	/** Define the direction of the message (i.e., TOMODULE or TONODECORE) */
	ClDirection direction_;

	/** Define the unique id of the message */
	unsigned int uid_;









  /******************************
   * Deprecated Methods
   * 
   */

public:
        /** 
	 * \deprecated This method is deprecated since the source ID is now filled in automatically.
	 * use ClMessage(int verbosity, ClMessage_t type, DestinationType dtype, int value) instead.
	 * 
	 * @param verbosity 
	 * @param type 
	 * @param dtype 
	 * @param source 
	 * @param value 
	 * 
	 * @return 
	 */
	ClMessage(int verbosity, ClMessage_t type, DestinationType dtype, int source, int value);

};

#endif

