/*
 * Copyright (c) 2007 Regents of the SIGNET lab, University of Padova.
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

#include "metric.h"

/*	 _______________________________________
 *	/					\
 *	|		MetricList		|
 *	\_______________________________________/
 */

MetricList::MetricList() : list_(0), listLength_(0), metricCount_(0)
{
}

MetricList::~MetricList()
{
	if(listLength_ > 0)
		delete [] list_;
}

void MetricList::addMetric(Metric *m)
{
	int i = find(m->getName());
	if(i == metricCount_ || strcmp(list_[i]->getName(), m->getName()) != 0)
		addNewMetric(m);
	else
		list_[i] = m;
}

void MetricList::addNewMetric(Metric *m)
{
	if(metricCount_ < listLength_)
	{
		int i = find(m->getName());
		for(int j = metricCount_; j >= i ; j--)
		{
			list_[j + 1] = list_[j];
		}
		list_[i] = m;
	}
	else
	{
		Metric **tmp = new Metric*[listLength_ + METRIC_ALLOC_PARAM_PER_TIME];
		int offset = 0;
		for(int i = 0; i < listLength_; i++)
		{
			if(!offset)
			{
				if(strcmp(list_[i]->getName(), m->getName()) > 0)
				{
					offset = 1;
					tmp[i] = m;
				}
			}
			tmp[i + offset] = list_[i];
		}
		for(int i = listLength_; i < listLength_ + METRIC_ALLOC_PARAM_PER_TIME; i++)
		{
			tmp[i] = 0;
		}
		if(!offset)
			tmp[listLength_] = m;
		if(listLength_ > 0)
			delete [] list_;
		list_ = tmp;
		listLength_ += METRIC_ALLOC_PARAM_PER_TIME;
	}
	metricCount_++;
}

int MetricList::find(const char *name)
{
	if(metricCount_ == 0)
		return 0;
	if(metricCount_ == 1)
	{
		if(strcmp(list_[0]->getName(), name) >= 0)
			return 0;
		else
			return 1;
	}
	int max = metricCount_ - 1;
	int min = 0;
	while(max - min > 0)
	{
		int i = min + (max - min) / 2;
		int c1 = -1;
		int c2 = strcmp(list_[i]->getName(), name);
		if(i > 0)
			c1 = strcmp(list_[i-1]->getName(), name);
		
		if(i > 0 && c2 >= 0 && c1 < 0)
		{
			return i;
		}
		else if(i == 0 && c2 >= 0)
		{
			return 0;
		}
		else if(i == metricCount_ - 1 && c2 < 0)
		{
			return i + 1;
		}
		else if(c2 < 0)
		{
			min = i + 1;
		}
		else
		{
			max = i - 1;
		}

	}
	int c1 = -1;
	int c2 = strcmp(list_[min]->getName(), name);
	if(min > 0)
		c1 = strcmp(list_[min-1]->getName(), name);
	
	if(min > 0 && c2 >= 0 && c1 < 0)
	{
		return min;
	}
	else if(min == 0 && c2 >= 0)
	{
		return 0;
	}
	else if(min == metricCount_ - 1 && c2 < 0)
	{
		return min + 1;
	}
}

int MetricList::length()
{
	return metricCount_;
}

Metric *MetricList::getMetric(int i)
{
	if(i < 0 || i >= metricCount_)
		return 0;
	return list_[i];
}

Metric *MetricList::getMetric(const char *name)
{
	int i = find(name);
	if(i == metricCount_ || strcmp(list_[i]->getName(), name) != 0)
		return 0;
	return list_[i];
}

void MetricList::empty()
{
	for(int i = 0; i < metricCount_; i++)
		list_[i] = 0;
	metricCount_ = 0;
}

/*	 _______________________________________
 *	/					\
 *	|		Metric			|
 *	\_______________________________________/
 */

Metric::Metric(const char *name) : name_(name), list_()
{
}

Metric::~Metric()
{
}

const char*Metric::getName()
{
	return name_;
}

int Metric::command(int argc, const char *const* argv)
{
	Tcl &tcl = Tcl::instance();
	if(argc == 2)
	{
		if(strcasecmp(argv[1], "type"))
		{
			tcl.resultf("%s", name_);
			return TCL_OK;
		}
	}
}

void Metric::insertParam(MetricList *m)
{
	for(int i = 0; i < m->length(); i++)
		list_.addMetric(m->getMetric(i));
}

void Metric::clearParam()
{
	list_.empty();
}

/*	 _______________________________________
 *	/					\
 *	|		RouteInfo		|
 *	\_______________________________________/
 */

RouteInfo::RouteInfo() : nChild_(0), father_(0), list_(), lengthChild_(0)
{
	reachability_ = ROUTEINFO_DEFAULT_REACHABILITY;
}

RouteInfo::~RouteInfo()
{
	if(lengthChild_ > 0)
		delete [] child_;
}

void RouteInfo::addChild(RouteInfo *c)
{
	if(lengthChild_ <= nChild_)
	{
		RouteInfo **tmp = new RouteInfo*[lengthChild_ + ROUTEINFO_ALLOC_CHILD_PER_TIME];
		for(int i = 0; i < lengthChild_; i++)
			tmp[i] = child_[i];
		if(lengthChild_ > 0)
			delete [] child_;
		child_ = tmp;
		lengthChild_ += ROUTEINFO_ALLOC_CHILD_PER_TIME;
	}
	child_[nChild_++] = c;
}

int RouteInfo::getNChild()
{
	return nChild_;
}

RouteInfo *RouteInfo::getChild(int i)
{
	if(i < 0 || i >= nChild_)
		return 0;
	
	return child_[i];
}

void RouteInfo::addFather(RouteInfo *f)
{
	father_ = f;
}

RouteInfo *RouteInfo::getFather()
{
	return father_;
}

void RouteInfo::setModuleId(int id)
{
	moduleId_ = id;
}

int RouteInfo::getModuleId()
{
	return moduleId_;
}

void RouteInfo::addMetric(Metric *m)
{
	list_.addMetric(m);
}

Metric *RouteInfo::getMetric(const char *m)
{
	return list_.getMetric(m);
}

Metric *RouteInfo::getMetric(int i)
{
	return list_.getMetric(i);
}

int RouteInfo::getNMetric()
{
	return list_.length();
}

Reachability RouteInfo::reachability()
{
	return reachability_;
}

void RouteInfo::setReachability(Reachability r)
{
	reachability_ = r;
}

