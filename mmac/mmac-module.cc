/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: mmac-module.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "mmac-module.h"
#include "mmac-clmsg.h"

static int MacIndex = 0;

MMacModule::MMacModule() : index_(MacIndex++), ifq_(0)
{
}

MMacModule::~MMacModule()
{
}

int MMacModule::command (int argc, const char *const *argv)
{
	Tcl& tcl = Tcl::instance();
  
	if(argc == 2)
	{
		if (strcasecmp(argv[1],"getIfq")==0)
		{
			if(ifq_)
			{
				tcl.result(ifq_->name());
				return TCL_OK;
			}
			else return TCL_ERROR;
		}
	}
	else if(argc == 3)
	{
		if(strcasecmp(argv[1], "setIfq")==0)
		{
			ifq_ = dynamic_cast<Queue*>(TclObject::lookup(argv[2]));

			if(!ifq_)
				return TCL_ERROR;

			tcl.evalf("%s target %s",ifq_->name(), name());

			return TCL_OK;
		}
	}

	/* If command is unknown, fallback to parent command intepreter */
	return  Module::command(argc,argv);
  
}

void MMacModule::recv (Packet *p)
{
	hdr_cmn *ch = HDR_CMN(p);

	if(ch->direction() == hdr_cmn::UP)
	{
		rx(p);
	}
	else
	{
		if(ifq_)
			ifq_->recv(p, (Handler*) 0);
		else
			tx(p);
	}
}

void MMacModule::recv(Packet *p, Handler*)
{
	hdr_cmn *ch = HDR_CMN(p);

	if(ch->direction() == hdr_cmn::UP)
		rx(p);
	else
		tx(p);
}

int MMacModule::crLaySynchronousCommand(ClMessage* m)
{
	if(m->type() == MAC_CLMSG_GET_ADDR)
	{
		if(debug_ > 5)
			printf("MMac(1), rx a MAC_CLMSG_GET_ADDR; -> return addr\n");
		
		((MacClMsgGetAddr *)m)->setAddr(addr());
		return 1;
	}
	return Module::crLaySynchronousCommand(m);
}


int MMacModule::recvSyncClMsg(ClMessage* m)
{
	if(m->type() == MAC_CLMSG_GET_ADDR)
	{
		if(debug_ > 5)
			printf("MMac(2), rx a MAC_CLMSG_GET_ADDR; -> return addr\n");
		
		((MacClMsgGetAddr *)m)->setAddr(addr());
		return 1;
	}
	return Module::recvSyncClMsg(m);
}


int MMacModule::addr()
{
	return index_;
}

void MMacModule::resume()
{
	if(ifq_)
		ifq_->resume();
}
