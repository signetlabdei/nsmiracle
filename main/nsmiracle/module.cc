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
 

#include <string.h>
#include "module.h"
#include "deprecated.h"


/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
// static class ModuleClass : public TclClass {
// public:
// 	ModuleClass() : TclClass("Module") {}
// 	TclObject* create(int, const char*const*) {
// 		return (new Module());
// 	}
// } class_module;

Module::Module() : upLayerSAP_(0),upLaySAPnum_(0), downLayerSAP_(0), downLaySAPnum_(0)
{
	// variables could be bound: no one...
}

Module::~Module()
{
}

// // TCL command intepreter
int Module::command(int argc, const char* const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc==3)
	{
		// install a SAP to an above module
		if (strcasecmp(argv[1],"addupsap")==0)
		{
			 SAP *sap = dynamic_cast<SAP*>(TclObject::lookup(argv[2]));
			addUpSAP(sap);
			if(debug_ > 5)
				printf("addUpsap\n");
			return (TCL_OK);
		}
		// install a SAP to a bottom module
		if (strcasecmp(argv[1],"adddownsap")==0)
		{
			SAP *sap = dynamic_cast<SAP*>(TclObject::lookup(argv[2]));
			addDownSAP(sap);
			if(debug_ > 5)
				printf("addDownsap\n");
			return (TCL_OK);
		}
		if (strcasecmp(argv[1],"setlayer")==0)
		{
			int val = atoi(argv[2]);
			setLayer(val);
			return TCL_OK;
		}
	}
	return PlugIn::command(argc, argv);
}

int Module::getUpLaySAPnum()
{
	return(upLaySAPnum_);
}

SAP* Module::getUpLaySAP(int i)
{
	if (i<0 || i>=upLaySAPnum_)
		return (0);
	else 
		return (upLayerSAP_[i]);
}


int Module::getDownLaySAPnum()
{
	return(downLaySAPnum_);
}

SAP* Module::getDownLaySAP(int i)
{
	if (i<0 || i>=downLaySAPnum_)
		return (0);
	else 
		return (downLayerSAP_[i]);
}


// install a new SAP to an above module
void Module::addUpSAP(SAP* sap)
{
	SAP** temp = new SAP*[++upLaySAPnum_];
	for (int i = 0; i<(upLaySAPnum_-1); i++)
		temp[i] = upLayerSAP_[i];
	temp[upLaySAPnum_ - 1] = sap;
	
	if (upLayerSAP_)
		delete [] upLayerSAP_;
	
	if(debug_>1)
	{
		printf("Module::addUpSAP(%p) -- upLaySAPnum_=%i\n", sap, upLaySAPnum_);
		if(debug_ > 1)
		{
			for(int i = 0; i<upLaySAPnum_; i++)
				printf("\ttemp[%i]=%p\n",i,temp[i]);
		}
	}
	upLayerSAP_ = temp;
}

// install a new SAP to a bottom module
void Module::addDownSAP(SAP* sap)
{
	SAP** temp = new SAP*[++downLaySAPnum_];
	for (int i = 0; i<(downLaySAPnum_-1); i++)
		temp[i] = downLayerSAP_[i];
	temp[downLaySAPnum_ - 1] = sap;
	if (downLayerSAP_)
		delete [] downLayerSAP_;
	downLayerSAP_ = temp;
	if(debug_ >= 1)
	{
		printf("(%p) Module %i downLayerSAP_[%i] = %p \n", this, getId(), (downLaySAPnum_ - 1), sap);
	}
}


void Module::swapUpLaySAP(int i, int j)
{
	
	if ( (i<0) || (i>=upLaySAPnum_) || (j<0) || (j>=upLaySAPnum_) || (i==j) )
	{
		fprintf(stderr, "Module::swapUpLaySAP, try to swap uncoherent SAPs (%d <-> %d)",i,j);
		exit(1);
	}
	SAP* temp;
	temp = upLayerSAP_[i];
	upLayerSAP_[i] = upLayerSAP_[j];
	upLayerSAP_[j] = temp;
	return;
}

void Module::swapDownLaySAP(int i, int j)
{
	if ( (i<0) || (i>=downLaySAPnum_) || (j<0) || (j>=downLaySAPnum_) || (i==j) )
	{
		fprintf(stderr, "Module::swapDownLaySAP, try to swap uncoherent SAPs (%d <-> %d)",i,j);
		exit(1);
	}
	SAP* temp;
	temp = downLayerSAP_[i];
	downLayerSAP_[i] = downLayerSAP_[j];
	downLayerSAP_[j] = temp;
	return;

}



void Module::sendUp(Packet* p,  double delay)
{
	// by default a module send a packet to all the modules of the above layer
	if (upLaySAPnum_<=0)
	{
		fprintf(stderr,"Error Module.sendUp(): no SAP installed\n");
		exit(1);
	}
	if(upLaySAPnum_ == 1)
	{
		if(debug_ >= 1)
		{
			printf("\t %f upLayerSAP_[0]=%p (this=%p id=%i)\n", Scheduler::instance().clock(), upLayerSAP_[0], this, getId());
			fflush(stdout);
		}
		upLayerSAP_[0]->sendUp(p, delay);
	}
	else
	{
		for(int i=0; i<upLaySAPnum_; i++)
		{
			upLayerSAP_[i]->sendUp(copy(p), delay);
		}
		Packet::free(p);
	}
}


 void Module::sendDown(Packet* p,  double delay)
 {
	// by default a module send a packet to all the modules of the bottom layer
	if (downLaySAPnum_<=0)
	{
		fprintf(stderr,"Error Module.sendDown(): no SAP installed\n");
		exit(1);
	}
	if(debug_ > 10)
		printf("Module::sendDown(%p,%f) ---- downLaySAPnum_=%i\n", p, delay, downLaySAPnum_);
	if(downLaySAPnum_ == 1)
	{
		if(debug_ >= 1)
		{
			printf("\t %f downLayerSAP_[0]=%p (this=%p id=%i)\n", Scheduler::instance().clock(), downLayerSAP_[0], this, getId());
			fflush(stdout);
		}
		downLayerSAP_[0]->sendDown(p, delay);
	}
	else
	{
		for(int i=0; i<downLaySAPnum_; i++)
		{
			if(debug_ >= 1)
			{
				printf("\tdownLayerSAP_[%i]=%p\n", i, downLayerSAP_[i]);
				fflush(stdout);
			}
			Packet *pkt = copy(p);
			downLayerSAP_[i]->sendDown(pkt, delay);
		}
		Packet::free(p);
	}
}

// send a packet to the above specified module
void Module::sendUp(int moduleId, Packet* p,  double delay)
 {
 	bool find = FALSE;
 	SAP* sap = 0;
	for(int i = 0; i<upLaySAPnum_; i++)
	{
		sap = getUpLaySAP(i);
		if(debug_ > 1)
			printf("Module::sendUp - trovato module con id=%i\n", sap->getModuleDownId());
		if (sap->getModuleUpId()==moduleId)
		{
			find = TRUE;
			break;
		}
	}
	if (find)
	{
		sap->sendUp(p, delay);
		return;
	} else {
		fprintf(stderr,"Error Module.sendUp(): invalid moduleId %d\n",moduleId);
		char tag[20];
		getTag(tag,20);
		fprintf(stderr,"This module: tag \"%s\", id %d, %d SAPs above and %d below\n",
			tag,getId(),getUpLaySAPnum(),getDownLaySAPnum());
		exit(1);
	}
 }

// send a packet to the bottom specified module
 void Module::sendDown(int moduleId, Packet* p,  double delay)
 {
	
	bool find = FALSE;
	SAP* sap = 0;
	for(int i = 0; i<downLaySAPnum_; i++)
	{
		sap = getDownLaySAP(i);
		if (sap->getModuleDownId()==moduleId)
		{
			find = TRUE;
			break;
		}
	}
	if (find)
	{
		sap->sendDown(p, delay);
		return;
	} else {
		fprintf(stderr,"Error Module.sendDown(): invalid moduleId specified\n");
		exit(1);
	}
}

void Module::sendUp(ClMessage* m,  double delay)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  sendAsyncClMsgUp(m, delay);
}

void Module::sendDown(ClMessage* m,  double delay)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  sendAsyncClMsgDown(m, delay);
}


void Module::sendDown(int moduleId, ClMessage* m,  double delay)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  m->setDest(moduleId);
  sendAsyncClMsgDown(m, delay);
}


void Module::sendUp(int moduleId, ClMessage* m,  double delay)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  m->setDest(moduleId);
  sendAsyncClMsgUp(m, delay);
}


void Module::sendSynchronousUp(ClMessage* m)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  sendSyncClMsgUp(m);
}


void Module::sendSynchronousDown(ClMessage* m)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  sendSyncClMsgDown(m);
}


void Module::sendSynchronousDown(int moduleId, ClMessage* m)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  m->setDest(moduleId);
  sendSyncClMsgDown(m);
}

void Module::sendSynchronousUp(int moduleId, ClMessage* m)
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  m->setDest(moduleId);
  sendSyncClMsgUp(m);
}


void Module::sendAsyncClMsgUp(ClMessage* m,  double delay)
{
  assert(m);
  m->setSource(getId()); // this makes src IDs more spoofing-proof
  int dest = m->getDest();
  bool is_broadcast = ( m->getDestType() == BROADCAST);

  
  if (is_broadcast && (dest != CLBROADCASTADDR) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " broadcast message with dest_ != CLBROADCASTADDR (dest_=" << dest << ")" << endl;

  if ((upLaySAPnum_<=0) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " upLaySAPnum_ <= 0 " << endl;

  bool found = false;

  for(int i=0; i<upLaySAPnum_; i++)
    {
      SAP* sap = getUpLaySAP(i);

      if (is_broadcast || (dest == sap->getModuleUpId()))
	{      
	  // Note that we cannot send m because we need it for further copies
	  // If we send m then it might have been deleted when we try to make a copy of it
	  sap->sendUp(m->copy(), delay); 
	  found = true;
	}
    }

  delete m;

  if (!found)
    {

      if  (debug_ >= 0)
	cerr << __PRETTY_FUNCTION__ << " no destination found" << endl;
    }      
    

}

void Module::sendAsyncClMsgDown(ClMessage* m,  double delay)
{

  assert(m);
  m->setSource(getId()); // this makes src IDs more spoofing-proof
  int dest = m->getDest();
  bool is_broadcast = ( m->getDestType() == BROADCAST);
  
  if (is_broadcast && (dest != CLBROADCASTADDR) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " broadcast message with dest_ != CLBROADCASTADDR (dest_=" << dest << ")" << endl;

  if ((downLaySAPnum_<=0) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " downLaySAPnum_ <= 0 " << endl;

  bool found = false;

  for(int i=0; i<downLaySAPnum_; i++)
    {
      SAP* sap = getDownLaySAP(i);

      if (is_broadcast || (dest == sap->getModuleDownId()))
	{
	  // Note that we cannot send m because we need it for further copies
	  // If we send m then it might have been deleted when we try to make a copy of it
	  sap->sendDown(m->copy(), delay);
	  found = true;
	}
    }

  delete m;

  if (!found)
    {
      if  (debug_ >= 0)
	cerr << __PRETTY_FUNCTION__ << " no destination found" << endl;
    }      
    
}

void Module::sendSyncClMsgUp(ClMessage* m)
{
  assert(m);
  m->setSource(getId()); // this makes src IDs more spoofing-proof
  int dest = m->getDest();
  bool is_broadcast = ( m->getDestType() == BROADCAST);
  
  if (is_broadcast && (dest != CLBROADCASTADDR) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " broadcast message with dest_ != CLBROADCASTADDR (dest_=" << dest << ")" << endl;

  if ((upLaySAPnum_<=0) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " upLaySAPnum_ <= 0 " << endl;

  bool found = false;

  for(int i=0; i<upLaySAPnum_; i++)
    {
      SAP* sap = getUpLaySAP(i);

      if (is_broadcast || (dest == sap->getModuleUpId()))
	{
	  sap->sendSynchronousUp(m); // first message
	  found = true;
	}
    }

  if ((!found) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " no destination found" << endl;
}      

void Module::sendSyncClMsgDown(ClMessage* m)
{
  assert(m);
  m->setSource(getId()); // this makes src IDs more spoofing-proof
  int dest = m->getDest();
  bool is_broadcast = ( m->getDestType() == BROADCAST);
  
  if (is_broadcast && (dest != CLBROADCASTADDR) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " broadcast message with dest_ != CLBROADCASTADDR (dest_=" << dest << ")" << endl;

  if ((downLaySAPnum_<=0) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " downLaySAPnum_ <= 0 " << endl;

  bool found = false;

  for(int i=0; i<downLaySAPnum_; i++)
    {
      SAP* sap = getDownLaySAP(i);

      if (is_broadcast || (dest == sap->getModuleDownId()))
	{
	  sap->sendSynchronousDown(m); // first message
	  found = true;
	}
    }

  if ((!found) && (debug_ >= 0))
    cerr << __PRETTY_FUNCTION__ << " no destination found" << endl;
}      
    



void Module::drop(Packet* p, int depth, const char* reason)
{
	if (binPtr_==0)
	{
		fprintf(stderr, "Error Module.drop(packet): no bin installed\n");
		exit(1);
	}
	if(debug_ > 1)
		printf("Module::drop(%p,%i,%s)\n", p, depth, reason);
	char *temp = new char[MAX_TAG_LENGTH + strlen(reason)+2];
	strcpy(temp, tag_);
	strcat(temp, " ");
	strcat(temp, reason);
	if(debug_ > 1)
		printf("\ttemp=%s\n", temp);
	binPtr_->drop(p, depth, temp);
	delete [] temp;
}

void Module::recv(Packet *p, Handler* callback)
{
	recv(p);
}

void Module::recv(Packet *p, int idSrc)
{
	recv(p);
}

Packet *Module::copy(Packet *p)
{
	Packet *pkt = p->copy();
	pkt->txinfo_.RxPr = p->txinfo_.RxPr;   	
	pkt->txinfo_.CPThresh = p->txinfo_.CPThresh;

	return pkt;
}
