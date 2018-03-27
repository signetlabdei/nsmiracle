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
 * \file metric.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 7-13-07
 */

#ifndef _ROUTINGMETRIC_
#define _ROUTINGMETRIC_

#include <packet.h>

enum Reachability
{
	REACHABLE,
	UNREACHABLE,
	MAYBE
};

class Metric;

#define METRIC_ALLOC_PARAM_PER_TIME 1

class MetricList
{
public:
	MetricList();
	virtual ~MetricList();
	void addMetric(Metric *m);
	void empty();
	int length();
	Metric *getMetric(int i);
	Metric *getMetric(const char *name);
protected:
	void addNewMetric(Metric *m);
	int find(const char *name);
	Metric **list_;
	int listLength_;
	int metricCount_;
};

class Metric : public TclObject
{
public:
	Metric(const char* name);
	virtual ~Metric();
	virtual double value(Packet *p) = 0;
	const char *getName();
	//virtual Metric *copy() = 0;
	/**
	 * TCL command interpreter. It implements the following OTcl methods:
	 * <ul>
	 *  <li><b>addr</b>: 
	 *  	return the string formatted address
	 *  <li><b>addr &lt;<i>address</i>&gt;</b>: 
	 *  	set the address converting the string <i>address</i>
	 * </ul>
	 * 
	 * @param argc number of arguments in <i>argv</i>
	 * @param argv array of strings which are the command parameters (Note that argv[0] is the name of the object)
	 * 
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	 * 
	 * @see Module
	 **/
	virtual int command(int argc, const char*const* argv);
	void insertParam(MetricList *m);
	void clearParam();
private:
	const char *name_;
	MetricList list_;
};

#define ROUTEINFO_DEFAULT_REACHABILITY UNREACHABLE
#define ROUTEINFO_ALLOC_CHILD_PER_TIME 1

/**
 * This structure contains information about the accessibility of a particular MrclAddress.
 * When a RoutingModule wants to know whether any other RoutingModule can reach a particular 
 * address, it calls the method isReachable which returns an array of these structures
 *
 */
class RouteInfo
{
public:
	RouteInfo();
	virtual ~RouteInfo();
	void addChild(RouteInfo *c);
	int getNChild();
	RouteInfo *getChild(int i);
	void addFather(RouteInfo *f);
	RouteInfo *getFather();
	void setModuleId(int id);
	int getModuleId();
	void addMetric(Metric *m);
	Metric *getMetric(const char *m);
	Metric *getMetric(int i);
	int getNMetric();
	Reachability reachability();
	void setReachability(Reachability a);
protected:
	RouteInfo *father_;
	RouteInfo **child_;
	int nChild_;
	int lengthChild_;
	/**
	 * A reachability awareness which can be used in order to give information about how the 
	 * destination can be reached (e.g. for a reactive routing protocol it could say "I can
	 * try")
	 *
	 */
	Reachability reachability_;
	/**
	 * The list of the eported metrics
	 */
	MetricList list_;
	int moduleId_;
};



#endif


