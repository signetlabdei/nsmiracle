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

#include <stdarg.h>
#include "cltracer.h"
#include "clsap.h"

/*-------------------------------------------------------------------------------------------------------
	methods for clTracer class
---------------------------------------------------------------------------------------------------------*/ 

ClTracer::ClTracer() : commonTr_(0)
{
	for (int i = 0; i < MAXHASHINDEX; i++)
		tr_[i] = 0;
}

// add a new tracer (insert sorted by level, decreasing from the max)
// (a new tracer of the same level is inserted in tail of the already inserted tracer)
void ClTracer::addTracer(ClMessageTracer *newTr)
{
	if(!newTr)
		return;
	int i = newTr->clType() % MAXHASHINDEX;
	addTracer(newTr, &(tr_[i]));
}

void ClTracer::addCommonTracer(ClMessageTracer *newTr)
{
	if(!newTr)
		return;
	commonTr_ = newTr;
}

void ClTracer::addTracer(ClMessageTracer *newTr, ClMessageTracer **tr)
{
	if(!*tr)
	{
		/*
		newTr is the first ClMessageTracer with this key
		*/
		*tr = newTr;
		(*tr)->next(newTr);
		return;
	}
	newTr->next(*tr);
	ClMessageTracer *cur;
	
	for (cur = *tr; cur->next() != *tr; cur = cur->next());
	
	cur->next(newTr);
	
}

// begin the trace of the packet that is crossing the ClSAP
void ClTracer::trace(ClMessage *m, ConnectorTrace *clsap)
{
	if(commonTr_)
		commonTr_->format(m, clsap);
	int i = m->type() % MAXHASHINDEX;
	ClMessageTracer *cur;
	for (cur = tr_[i]; cur && cur->next() != tr_[i]; cur = cur->next())
	{
		if(cur->clType() == m->type())
		{
			tr_[i] = cur;
			cur->format(m, clsap);
			return;
		}
	}
	if(cur && cur->clType() == m->type())
	{
		tr_[i] = cur;
		cur->format(m, clsap);
	}
}

/*-------------------------------------------------------------------------------------------------------
	methods for ClMessageTracer class
---------------------------------------------------------------------------------------------------------*/ 

ClMessageTracer::ClMessageTracer(ClMessage_t clType) : clType_(clType)
{
	next_ = 0;
}

ClMessage_t ClMessageTracer::clType()
{
	return (clType_);
}

ClMessageTracer* ClMessageTracer::next()
{
	return (next_);
}

void ClMessageTracer::next(ClMessageTracer *tr)
{
	next_ = tr;
}

void ClMessageTracer::writeTrace(ConnectorTrace *clsap, char *s, ...)
{
	va_list ap;
	va_start(ap,s);
	clsap->vWriteTrace(s, ap);
	//printf(s,ap);
	va_end(ap);
}


