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


#include<iostream>
#include "bmposition.h"


/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class BMPositionClass : public TclClass {
public:
	BMPositionClass() : TclClass("Position/BM") {}
	TclObject* create(int, const char*const*) {
		return (new BMPosition());
	}
} class_bmposition;

BMPosition::BMPosition() : Position(), trgTime_(-1), Xsorg_(0), Ysorg_(0), Xdest_(0), Ydest_(0), speed_(0), lastUpdateTime_(0)
{
	bind("debug_", &debug_);
}

BMPosition::~BMPosition()
{
}

int BMPosition::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 5)
	{
		if(strcasecmp(argv[1], "setdest") == 0)
		{
		  if (debug_ > 10)
		    cerr << NOW << "BMPosition::command(setdest, "
			 << argv[2] << ", "
			 << argv[3] << ", "
			 << argv[4] << ")"
			 << endl;
		      
			trgTime_ = Scheduler::instance().clock();
			if(trgTime_ <= 0.0)
			  cerr << "Warning: calling set dest at time <= 0 will not work" << endl;
			Xdest_ = atof(argv[2]);
			Ydest_ = atof(argv[3]);
			speed_ = atof(argv[4]);
			Xsorg_ = x_;
			Ysorg_ = y_;
			return TCL_OK;
		}
	}
	return Position::command(argc, argv);
}


void BMPosition::update(double now)
{
	double gamma;

	if (Xdest_-Xsorg_==0 && Ydest_-Ysorg_==0) {
		x_ = Xsorg_;
		y_ = Ysorg_;
	} else {

		if (Xdest_-Xsorg_==0)
			gamma = pi/2*sgn(Ydest_-Ysorg_);
		else 
			gamma = atan((Ydest_-Ysorg_)/(Xdest_-Xsorg_));
		if ((Xdest_-Xsorg_)<0.0) gamma += (Ysorg_-Ydest_)>=0.0?pi:-pi;
		//printf("At %f prec pos (%f,%f)",now,x_,y_);
		x_ = Xsorg_ + (speed_*(now - trgTime_) )*cos(gamma);
		y_ = Ysorg_ + (speed_*(now - trgTime_) )*sin(gamma);
		if (debug_>50)
			printf("New pos (%f,%f), dest(%f,%f), speed %f sen(%f)=%f\n",x_,y_,Xdest_,Ydest_,speed_,gamma, sin(gamma));
	}
		
		lastUpdateTime_ = now;
}

double BMPosition::getX()
{
	double now = Scheduler::instance().clock();
	if ((trgTime_>0.)&&(now>lastUpdateTime_+1e-6))
		update(now);
	return (x_);
}

double BMPosition::getY()
{
	double now = Scheduler::instance().clock();
	if ((trgTime_>0.)&&(now>lastUpdateTime_+1e-6))
		update(now);
	return (y_);
}
