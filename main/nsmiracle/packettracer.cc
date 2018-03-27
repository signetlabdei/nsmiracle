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
#include "packettracer.h"
#include "sap.h"

/*-------------------------------------------------------------------------------------------------------
	methods for Tracer class
---------------------------------------------------------------------------------------------------------*/ 

Tracer::Tracer(int level) : level_(level)
{
	next_ = 0;
}

int Tracer::level()
{
	return (level_);
}

Tracer* Tracer::next()
{
	return (next_);
}

void Tracer::next(Tracer *tr)
{
	next_ = tr;
}

void Tracer::writeTrace(SAP *sap, char *s, ...)
{
	va_list ap;
	va_start(ap,s);
	sap->vWriteTrace(s, ap);
	//printf(s,ap);
	va_end(ap);
}


void Tracer::trace(Packet *p, SAP *sap)
{
// 	printf("Tracer::trace(%p,%p) --- this=%p next_=%p\n", p, sap, this, next_);
	if(p)
		format(p, sap);
	if(next_)
		next_->trace(p, sap);
}



/*-------------------------------------------------------------------------------------------------------
	methods for PktTracer class
---------------------------------------------------------------------------------------------------------*/ 

PktTracer::PktTracer(void) 
{
	tr_ = 0;
// 	printf("PktTracer::PktTracer() --- tr_=%p\n", tr_);
// 	fflush(stdout);
}

// add a new tracer (insert sorted by level)
// (a new tracer of the same level is inserted in tail of the already inserted tracer)
void PktTracer::addTracer(Tracer *newTr)
{
// 	printf("PktTracer::addTracer(%p) --- newTr->next=%p newTr->level()=%i tr=%p\n", newTr, newTr->next(), newTr->level(), tr_);
	
	if (newTr->level()<=0)
	{
		fprintf(stderr, "Error PktTracer: try to install  a tracer in a level <= 0");
		exit(1);
	}
	Tracer*pt = tr_;
	Tracer*last = tr_;
	
	if (pt==0)
	{
		// first Tracer added
		tr_ = newTr;
		//newTr->next(0);
		return;
	}
	// insert sorted
	bool inserted = FALSE;
	while (pt && !inserted)
	{
		if (pt->level() > newTr->level())
		{
			// find the position to insert the new tracer
			if (pt==tr_)
			{
				// insert in head of list
				newTr->next(tr_);
				tr_ = newTr;
				inserted = TRUE;
			}
			else
			{
				// insert in the middle
				newTr->next(last->next());
				last->next(newTr);
// 				newTr->next(pt->next());
				inserted = TRUE;
			}
		}
		else
		{
			last = pt;
			pt = pt->next();
		}
	}
	if (!inserted)
	{
		// insert in tail
		last->next(newTr);
		newTr->next(0);
	}
// 	tr_->trace(0,0);
}

// begin the trace of the packet that is crossing the SAP
void PktTracer::trace(Packet *p, SAP *sap)
{
	if (tr_!=0)
	{
//  		printf("PktTracer::trace(%p,%p) --- tr_=%p\n", p, sap, tr_);
		tr_->trace(p, sap);
	}
	else
	{
		fprintf(stderr,"Error PktTracer: there isn't any tracer installed'");
		exit(1);
	}
}
