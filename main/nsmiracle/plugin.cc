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
 
#include "plugin.h"
#include "scheduler.h"

#include "deprecated.h" 
#include "clmsg-discovery.h"
#include "clmsg-stats.h"
// #include <string>

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class PlugInClass : public TclClass {
public:
	PlugInClass() : TclClass("PlugIn") {}
	TclObject* create(int, const char*const*) {
		return (new PlugIn());
	}
} class_plugin;

PlugIn::PlugIn() :  clsap_(0), binPtr_(0), uLayerId_(0), stack_id(0), stats_ptr(NULL)
{
	sprintf(tag_, "---");
}

PlugIn::~PlugIn()
{
}

// TCL command interpreter
int PlugIn::command(int argc, const char*const* argv)
{
// 	printf("PlugIn::command -- %s (%d)\n", argv[1], argc);
	Tcl& tcl = Tcl::instance();
	if (argc==2)
	{
		if (strcasecmp(argv[1],"Id_")==0)
		{
			tcl.resultf("%d", getId());
			return (TCL_OK);
		}
		else if (strcasecmp(argv[1],"gettag")==0)
		{
			tcl.resultf("%s",tag_);
			return TCL_OK;
		}
		else if (strcasecmp(argv[1],"getstackid")==0)
		{
			tcl.resultf("%d", getStackId());
			return (TCL_OK);
		}
	}
	if (argc==3)
	{
		if (strcasecmp(argv[1],"Id_")==0)
		{
			int val = atoi(argv[2]);
			setId(val);
			return (TCL_OK);
		}
		if (strcasecmp(argv[1],"setstackid")==0)
		{
			int val = atoi(argv[2]);
			setStackId(val);
			return (TCL_OK);
		}
		if (strcasecmp(argv[1],"addclsap")==0)
		{
			ClSAP *clsap = dynamic_cast<ClSAP*>(TclObject::lookup(argv[2]));
			if (clsap_!=0)
			{
				tcl.resultf("Error PlugIn::command = %s:  a crLay is already installed", argv[1]);
				return (TCL_ERROR);
			}
			clsap_ = clsap;
			return (TCL_OK);
		}
		if (strcasecmp(argv[1],"addbin")==0)
		{
			Bin *bin = dynamic_cast<Bin*>(TclObject::lookup(argv[2]));
			if (binPtr_!=0)
			{
				tcl.resultf("Error PlugIn::command = %s:  a bin is already installed", argv[1]);
				return (TCL_ERROR);
			}
			binPtr_ = bin;
			return (TCL_OK);
		}
		// set the plugin tag
		if (strcasecmp(argv[1],"settag")==0)
		{
			if (strlen(argv[2])>MAX_TAG_LENGTH)
			{
				tcl.resultf("Error PlugIn::command cmd = %s: plugin tag too long", argv[1]);
				return (TCL_ERROR);
			}
			strcpy(tag_, argv[2]);
			return TCL_OK;
		}
	}
	return NsObject::command(argc, argv);
}


int PlugIn::getId()
{
	return(uid_);
}

void PlugIn::setId(int val)
{
	uid_ = val;
}

int PlugIn::getStackId()
{
	return(stack_id);
}

void PlugIn::setStackId(int val)
{
	stack_id = val;
}

// it returns -1 if it's impossible to return the tag
// it returns 0 if the size of the buffer is not sufficient (but the string is copied)
// if returns >0 if all is ok
int PlugIn::getTag(char* buf, int size)
{
  if ((buf==0)||(size<=0)||(tag_==0))
    {
      return (-1);
    }

  strncpy(buf, tag_, size);
  if (buf[size-1] != '\0')
    {
      buf[size-1] = '\0';
      return 0;
    } 
  else 
    return 1;

}

        
int PlugIn::recvAsyncClMsg(ClMessage* m)
{
  //  cerr << "PlugIn::recvAsyncClMsg() NOT IMPLEMENTED" << endl;

  // NOT deallocating message since if this is not implemented then
  // crLayCommand() will for sure be called with the same parameter

  return RETVAL_NOT_IMPLEMENTED;
}


// standard cross-layer messages intepreter
// by default an unknown cross-layer message does not interest the
// plugin, so we just throw it away
// ATTENTION: remember to dellocate the message after the use
int PlugIn::crLayCommand(ClMessage* m)
{
  delete m;
  return RETVAL_NOT_IMPLEMENTED;
}

int PlugIn::getLayer()
{
	return(uLayerId_);
}

void PlugIn::setLayer(int val)
{
	uLayerId_ = val;
}



int PlugIn::recvSyncClMsg(ClMessage* m)
{
  if (m->type() == CLMSG_DISCOVERY) 
  {
     Tcl& tcl = Tcl::instance();
     tcl.evalf("%s info class", name() );
     const char* class_name = tcl.result();
     (dynamic_cast<ClMsgDiscovery*>(m))->addData((const PlugIn*)this, getLayer(), getStackId(), getId(), class_name, getTag());
     return 0;
  }
  else if (m->type() == CLMSG_STATS)
  {
    (dynamic_cast<ClMsgStats*>(m))->setStats(stats_ptr);
    return 0;
  }
  else return RETVAL_NOT_IMPLEMENTED;
}


int PlugIn::crLaySynchronousCommand(ClMessage* m)
{
  return RETVAL_NOT_IMPLEMENTED;
}



void PlugIn::sendCl(ClMessage* m, double delay)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  sendAsyncClMsg(m, delay);
}

void PlugIn::sendAsyncClMsg(ClMessage* m, double delay)
{
	if (clsap_==0)
	{
		fprintf(stderr, "Error, PlugIn.sendCl: crLayBus_ is not yet installed\n");
		exit(1);
	}
	m->setSource(getId()); // this makes src IDs more spoofing-proof
	clsap_->sendClLayer(m, delay);
}


void PlugIn::sendSynchronousCl(ClMessage* m)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  sendSyncClMsg(m);
}

void PlugIn::sendSyncClMsg(ClMessage* m)
{
	if (clsap_==0)
	{
		fprintf(stderr, "Error, PlugIn.sendCl: crLayBus_ is not yet installed\n");
		exit(1);
	}
	m->setSource(getId()); // this makes src IDs more spoofing-proof
	clsap_->sendSynchronousClLayer(m);
}


void PlugIn::drop(ClMessage* m, const char* reason)
{
	binPtr_->drop(m, reason);
}


void PlugIn::recv(Packet* p, Handler* callback)
{
	// an FE can receive only cross-layer messages
	// but recv must be implemeted since a FunctionalEntity is a NsObject child
}

Position* PlugIn::getPosition()
{
	return clsap_->getPosition();
}

