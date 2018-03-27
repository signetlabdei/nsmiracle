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
 * \file routing-module.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 7-5-07
 */

#ifndef _ROUTING_MODULE_
#define _ROUTING_MODULE_
#include <module.h>
#include <clmessage.h>
#include <cltracer.h>
#include "mrcl-address.h"
#include "metric.h"
#include "utility.h"
#include "routing-queue.h"

extern ClMessage_t CL_ROUTEMESSAGE;

#define ROUTEREACHABLE_VERBOSITY 5
#define ROUTE_REACHABLE_ALLOC_INFO_PER_TIME 10

#define CTRL_PKT_DROP_VERBOSITY 10
#define CTRL_PKT_DROP_MSG "CTRL"

/**
 * The cross layer message is used in order to discover if there is any module which know the route
 * to the destination
 */
class RouteReachable : public ClMessage
{
public:
	/**
	 * Class constructor
	 *
	 * @param source Id of the module which send the message
	 */
	RouteReachable(int source);
	/**
	 * Class destructor
	 */
	virtual ~RouteReachable();
	/**
	 * Set the Address for which the route can be found
	 *
	 * @param a pointer to the destination address
	 */
	void setAddress(char *a);
	/**
	 * @return The address for which the route can be found
	 */
	char *getAddress();
	/**
	 * Add a RouteInfo in the internal data structure. This method is used by the modules which 
	 * receives the message in order to add newer route information
	 *
	 * @param i the new Routeinf which can be added
	 */
	void addRouteInfo(RouteInfo *i);
	/**
	 * @return The number of RouteInfo which are stored in the internal data structure
	 */
	int length();
	/**
	 * @param id id of the module which you looking for the RouteInfo
	 *
	 * @return The pointer to the RouteInfo with id equal to id, 0 whether the RouteInfo 
	 * is not found.
	 *
	 * @remarks The returned array cannot be deallocated
	 */
	RouteInfo *getRouteInfo(int id);
	/**
	 * @return The list of the discovered RouteInfo.
	 */
	RouteInfo **getRouteInfo();
	/**
	 * Empty the internal data structure
	 */
	void empty();
	int isMyIP();
	void setMyIP();
	void processed(int value);
	int processed();
	int getIndex(int id);
private:
	/**
	 * Array which contain the collected RouteInfo
	 */
	RouteInfo **info_;
	/**
	 * Number of RouteInfo stored in info_
	 */
	int nInfo_;
	/**
	 * Array which contain the position of a modules RouteInfo in the info_ array
	 */
	int *modules_;
	/**
	 * Length of modules_;
	 */
	int nModules_;
	/**
	 * Length of info_
	 */
	int infoLen_;
	/**
	 * The address for which the route can be found
	 */
	char addr_[MRCL_ADDRESS_MAX_LEN];
	int myIP_;
	int processed_;
};



// class RouteReachableTracer : public ClMessageTracer
// {
// 	public:
// 		RouteReachableTracer();
// 		void format(ClMessage *m, ClSAP *sap);
// };


struct ModuleRoute
{
	DinArray modules;
	double w;
	Reachability reachability;
};

/**
 * This is the class which can be exteded in order to develop a new routing module for miracle
 */
class MrclRouting : public Module
{
	friend class RoutingQueue;
public:
	/**
	 * Class constructor
	 */
	MrclRouting();
	/**
	 * Class destructor
	 */
	virtual ~MrclRouting();
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * 
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
	 * 
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	 * 
	 * @see Module
	 **/
	virtual int command(int argc, const char*const* argv);
	/**
	 * Entry point for the synchronous cross layer message. It implements the answer to
	 * the CL_ROUTEMESSAGE.
	 *
	 * @param m pointer to the receives cross layer message
	 */
	virtual int recvSyncClMsg(ClMessage *m);
	void recv(Packet* p);
protected:
	/**
	 * Abstract method to be extended by the specific routing algorithm in order to get all the info it needed
	 * from its control packets
	 *
	 * @param p pointer to the received packet
	 *
	 * @return TRUE when the packet pointed by *p is a control packet
	 */
	virtual int controlPacket(Packet *p) = 0;
	/**
	 * Abstract method used to solve the routing of the packet as function of the specific routing algorithm
	 *
	 * @param p pointer to the received packet
	 *
	 * @return The address of the next hop node
	 */
	virtual char *getNextHop(Packet *p) = 0;
	int getRoute(MrclAddress *a, Packet *p, int i);
	int getRoute(char *addr, Packet *p, int i);
	/**
	 * This method is called by isReachable and it say if a particular destination can be 
	 * reach by the current module.
	 *
	 * @param a Pointer to the address of the node which can be reached
	 * @param m [out] reference to a ponter of metric returned by the method
	 *
	 * @return The number of returned metrics
	 *
	 * @remarks The returned metrics <b>have not to be dellocated</b>
	 */
	virtual int canIReach(char *a, Metric ***m)=0;
	virtual void forward(Packet *p) = 0;
	virtual void resolve(Packet *p) = 0;
	void addAddress(MrclAddress *a);
	MrclAddress *getAddress(int i);
	int nAddresses();
	int isMyAddress(char *addr);
	int overheadLength_; /// NON VIENE UTILIZZATA!!!!
private:	
	/**
	 * This method has to be called in order to understand whether a particular destination
	 * can be reached towards the current module or another module in the node.
	 *
	 * @param a Pointer to the address of the node which can be reached
	 * @param source Id of the module at which do not propagates the accessibility request. 
	 * 	If it assumes negative values the request will be propagated to all the adjacentes
	 * 	modules
	 *
	 * @return The length of the array pointed out to *info. If the address a is an address
	 * of the current module the returned value is negative and *info will be null.
	 *
	 * @remarks The array pointed out to *info wil be not deallocated
	 */
	int getConfiguration(char *a, int source = -1);
	int findNextLeaf(int i = -1);
	/**
	 * Array of the addresses of the current module
	 */
	MrclAddress **addresses_;
	/**
	 * Length of addresses_
	 */
	int nAddresses_;
	/**
	 * Pointer to the cross layer message which will be propagated toward the adjectes modules
	 * in ordert to retrive information about the accessibility of a particular destination. It
	 * is allocated as soon as an id is assigned to the current module.
	 */
	RouteReachable *rr_;
	/**
	 * Last instant when the configuration has been computed
	 */
	double lastGetConfiguration_;
	/**
	 * Last computed routes
	 */
	DinArray routes_;
	/**
	 * The delay introduced by the module before transmit to the upper module
	 */
	double delayUp_;
	/**
	 * The delay introduced by the module before transmit to the lower modules
	 */
	double delayDown_;
};


#endif
