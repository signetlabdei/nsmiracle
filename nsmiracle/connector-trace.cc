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
#include "connector-trace.h"
 
/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class ConnectorTraceClass : public TclClass {
public:
	ConnectorTraceClass() : TclClass("ConnectorTrace") {}
	TclObject* create(int, const char*const*) {
		return (new ConnectorTrace());
	}
} class_connector_trace;

/*
Static attributes and methods for ConnectorTrace
*/
ClTracer* ConnectorTrace::clTr_ = 0;

void ConnectorTrace::addTracer(ClMessageTracer *tr)
{
	if(!clTr_)
		clTr_ = new ClTracer();
	clTr_->addTracer(tr);
}

void ConnectorTrace::addCommonTracer(ClMessageTracer *tr)
{
	if(!clTr_)
		clTr_ = new ClTracer();
	clTr_->addCommonTracer(tr);
}



ConnectorTrace::ConnectorTrace() : dirToPlugin_('P'), dirToNodeCore_('N'), dirUp_('r'), dirDown_('s')
{
	depth_ = DEFAULTDEPTH;
	bind("depth_", &depth_);
	
	pt_ = new BaseTrace;
	pt_->buffer()[0] = 0;
	bind("debug_", &debug_);
}

ConnectorTrace::~ConnectorTrace()
{
	delete pt_;
	printf("Distruttore ConnectorTrace\n");
}


int ConnectorTrace::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	// SAME SET OF TRACE CLASS

	if (argc == 2) {
		if (strcmp(argv[1], "detach") == 0) {
			pt_->channel(0) ;
			pt_->namchannel(0) ;
			return (TCL_OK);
		}
		if (strcmp(argv[1], "flush") == 0) {
			Tcl_Channel ch = pt_->channel();
			Tcl_Channel namch = pt_->namchannel();
			if (ch != 0) 
				pt_->flush(ch);
				//Tcl_Flush(pt_.channel());
			if (namch != 0)
				//Tcl_Flush(pt_->namchannel());
				pt_->flush(namch);
			return (TCL_OK);
		}
                if (strcmp(argv[1], "tagged") == 0) {
			tcl.resultf("%d", pt_->tagged());
                        return (TCL_OK);
                }
	} else if (argc == 3) {
		if (strcmp(argv[1], "annotate") == 0) {
			if (pt_->channel() != 0)
				annotate(argv[2]);
			return (TCL_OK);
		}
		if (strcmp(argv[1], "attach") == 0) {
			int mode;
			const char* id = argv[2];
			Tcl_Channel ch = Tcl_GetChannel(tcl.interp(), (char*)id,
						  &mode);
			pt_->channel(ch); 
			if (pt_->channel() == 0) {
				tcl.resultf("trace: can't attach %s for writing", id);
				return (TCL_ERROR);
			}
			return (TCL_OK);
		}
		if (strcmp(argv[1], "namattach") == 0) {
			int mode;
			const char* id = argv[2];
			Tcl_Channel namch = Tcl_GetChannel(tcl.interp(), 
							   (char*)id, &mode);
			pt_->namchannel(namch); 
			if (pt_->namchannel() == 0) {
				tcl.resultf("trace: can't attach %s for writing", id);
				return (TCL_ERROR);
			}
			return (TCL_OK);
		}
		if (strcmp(argv[1], "ntrace") == 0) {
			if (pt_->namchannel() != 0) 
				write_nam_trace(argv[2]);
			return (TCL_OK);
		}
		if (strcmp(argv[1], "tagged") == 0) {
                        int tag;
			if (Tcl_GetBoolean(tcl.interp(),
					   (char*)argv[2], &tag) == TCL_OK) {
				pt_->tagged(tag);
				return (TCL_OK);
			} else return (TCL_ERROR);
                }
		if (strcasecmp(argv[1],"traceToModule")==0)
		{
			dirToPlugin_ = argv[2][0];
			return TCL_OK;
		}
		if (strcasecmp(argv[1],"traceToNodeCore")==0)
		{
			dirToNodeCore_ = argv[2][0];
			return TCL_OK;
		}
		if (strcasecmp(argv[1],"traceUp")==0)
		{
			dirUp_ = argv[2][0];
			return TCL_OK;
		}
		if (strcasecmp(argv[1],"traceDown")==0)
		{
			dirDown_ = argv[2][0];
			return TCL_OK;
		}
		if (strcmp(argv[1], "preambleToPlugin") == 0)
		{
			sprintf(preambleToPlugin_, "%s", argv[2]);
			return (TCL_OK);
		}
		if (strcmp(argv[1], "preambleToNodeCore") == 0)
		{
			sprintf(preambleToNodeCore_, "%s", argv[2]);
			return (TCL_OK);
		}
		if (strcmp(argv[1], "preambleUp") == 0)
		{
			sprintf(preambleUp_, "%s", argv[2]);
			return (TCL_OK);
		}
		if (strcmp(argv[1], "preambleDown") == 0)
		{
			sprintf(preambleDown_, "%s", argv[2]);
			return (TCL_OK);
		}

	}
	return TclObject::command(argc, argv);
}

int ConnectorTrace::depth()
{
	return (depth_);
}


void ConnectorTrace::write_nam_trace(const char *s)
{
	sprintf(pt_->nbuffer(), "%s", s);
	pt_->namdump();
}

void ConnectorTrace::annotate(const char* s)
{
	if (pt_->tagged()) {
		sprintf(pt_->buffer(),
			"v " TIME_FORMAT " -e {sim_annotation %g %s}",
			Scheduler::instance().clock(), 
			Scheduler::instance().clock(), s);
	} else {
		sprintf(pt_->buffer(),
			"v " TIME_FORMAT " eval {set sim_annotation {%s}}", 
			pt_->round(Scheduler::instance().clock()), s);
	}
	pt_->dump();
	//callback();
	sprintf(pt_->nbuffer(), "v -t " TIME_FORMAT " -e sim_annotation %g %s", 
		Scheduler::instance().clock(), 
		Scheduler::instance().clock(), s);
	pt_->namdump();
}

void ConnectorTrace::writeTrace(char* s, ...)
{
	va_list ap;
	va_start(ap, s);
	int offset = strlen(pt_->buffer());
	int remain = BASICTRACE_BUFFERLEN - offset - 2;
	vsnprintf(pt_->buffer()+offset, remain, s, ap);
	va_end(ap);
}

void ConnectorTrace::vWriteTrace(char* s, va_list ap)
{
	int offset = strlen(pt_->buffer());
	int remain = BASICTRACE_BUFFERLEN - offset - 2;
	vsnprintf(pt_->buffer()+offset, remain, s, ap);
}

void ConnectorTrace::dump()
{
	if(pt_)
	{
		pt_->dump();
		pt_->buffer()[0] = 0;
	}
}


void ConnectorTrace::handle(Event *e)
{	
}

void ConnectorTrace::trace(ClMessage* m)
{
	if(m->direction() == TONODECORE)
		writeTrace((char*)"%c %.9f %s ASYNC",dirToNodeCore_, Scheduler::instance().clock(), preambleToNodeCore_);
	else if(m->direction() == TOMODULE)
		writeTrace((char*)"%c %.9f %s ASYNC",dirToPlugin_, Scheduler::instance().clock(), preambleToPlugin_);
	else if(m->direction() == UP)
		writeTrace((char*)"%c %.9f %s ASYNC",dirUp_, Scheduler::instance().clock(), preambleUp_);
	else
		writeTrace((char*)"%c %.9f %s ASYNC",dirDown_, Scheduler::instance().clock(), preambleDown_);
	if(clTr_)
		clTr_->trace(m, this);
	dump();
}

void ConnectorTrace::traceSync(ClMessage* m)
{
	if(m->direction() == TONODECORE)
		writeTrace((char*)"%c %.9f %s SYNC",dirToNodeCore_, Scheduler::instance().clock(), preambleToNodeCore_);
	else
		writeTrace((char*)"%c %.9f %s SYNC",dirToPlugin_, Scheduler::instance().clock(), preambleToPlugin_);
	if(clTr_)
		clTr_->trace(m, this);
	dump();
}


