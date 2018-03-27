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

#include "mbposition.h"

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class MBPositionClass : public TclClass {
public:
	MBPositionClass() : TclClass("Position/MB") {}
	TclObject* create(int, const char*const*) {
		return (new MBPosition());
	}
} class_mbposition;

MBPosition::MBPosition() : Position()
{
}

MBPosition::~MBPosition()
{
}

int MBPosition::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 3)
	{
		if(strcasecmp(argv[1], "mobileNode") == 0)
		{
			node_ = (MobileNode *)tcl.lookup(argv[2]);
			if(!node_)
				return TCL_ERROR;
			return TCL_OK;
		}
	}
	return Position::command(argc, argv);
}

double MBPosition::getX()
{
	if(node_)
		return node_->X();
	return 0.0;
}

double MBPosition::getY()
{
	if(node_)
		return node_->Y();
	return 0.0;
}

void MBPosition::getLoc(double *x, double *y, double *z)
{
	if(!node_)
	{
		*x = 0.0;
		*y = 0.0;
		*z = 0.0;
		return;
	}
	node_->getLoc(x, y, z);
}

void MBPosition::setX(double x)
{
	if(node_)
	{
		Tcl& tcl = Tcl::instance();
		tcl.evalf("%s set X_ %f", node_->name(), x);
	}
}

void MBPosition::setY(double y)
{
	if(node_)
	{
		Tcl& tcl = Tcl::instance();
		tcl.evalf("%s set Y_ %f", node_->name(), y);
	}
}
