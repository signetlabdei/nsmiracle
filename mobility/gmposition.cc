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

#include<rng.h>
#include "gmposition.h"


/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class GMPositionClass : public TclClass {
public:
	GMPositionClass() : TclClass("Position/GM") {}
	TclObject* create(int, const char*const*) {
		return (new GMPosition());
	}
} class_gmposition;

GMPosition::GMPosition() : 
	Position(),
	xFieldWidth_(0),
	yFieldWidth_(0),
	alpha_(0),
	speedMean_(0),
	directionMean_(0),
	bound_(REBOUNCE),
	updateTime_(0),
	nextUpdateTime_(0.0),
	speed_(0),
	direction_(0),
	debug_(0)
	{
	bind("xFieldWidth_", &xFieldWidth_);
	bind("yFieldWidth_", &yFieldWidth_);
	bind("alpha_", &alpha_);
	bind("directionMean_", &directionMean_);
	bind("updateTime_", &updateTime_);
	bind("debug_", &debug_);
}

GMPosition::~GMPosition()
{
}

int GMPosition::command(int argc, const char*const* argv)
{
	if(argc == 3)
	{
		if(strcasecmp(argv[1], "bound") == 0)
		{
			if (strcasecmp(argv[2],"SPHERIC")==0)
				bound_ = SPHERIC;
			else
			{
				if (strcasecmp(argv[2],"THOROIDAL")==0)
					bound_ = THOROIDAL;
				else
				{
					if (strcasecmp(argv[2],"HARDWALL")==0)
						bound_ = HARDWALL;
					else
					{
						if (strcasecmp(argv[2],"REBOUNCE")==0)
							bound_ = REBOUNCE;
						else
						{
							fprintf(stderr,"GMPosition::command(%s), unrecognized bound_ type (%s)\n",argv[1],argv[2]);
							exit(1);
						}
					}
				}
			}
			return TCL_OK;
		}
		if(strcasecmp(argv[1], "speedMean") == 0)
		{
			speedMean_ = speed_ = atof(argv[2]);
			return TCL_OK;
		}
	}
	return Position::command(argc, argv);
}


double GMPosition::Gaussian()
{
	double x1, x2, w, y1;
	static double y2;
	static int use_last = 0;

	if (use_last)             
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do
		{
// 			x1 = 2.0 * ((float)rand()/(float)RAND_MAX) - 1.0;
// 			x2 = 2.0 * ((float)rand()/(float)RAND_MAX) - 1.0;
			x1 = 2.0 * RNG::defaultrng()->uniform_double() - 1.0;
			x2 = 2.0 * RNG::defaultrng()->uniform_double() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}
	return(y1);
}

void GMPosition::update(double now)
{
	double t;
	for(t = nextUpdateTime_; t < now; t += updateTime_)
	{
		// calculate new sample of speed and direction
		if (debug_>10)
			printf("Update at %.3f(%.3f) old speed %.2f old direction %.2f", now, t, speed_, direction_);
		
		speed_ = (alpha_*speed_) + (((1.0-alpha_))*speedMean_) + (sqrt(1.0-pow(alpha_,2.0))*Gaussian());
		direction_ = (alpha_*direction_) + (((1.0-alpha_))*directionMean_) + (sqrt(1.0-pow(alpha_,2.0))*Gaussian());
		//direction_ = (alpha_*direction_) + (sqrt(1.0-pow(alpha_,2.0))*Gaussian());
		if (debug_>10)
			printf(" new speed %.2f new direction %.2f \n", speed_, direction_);
		// calculate new position
		double newx = x_ + (speed_*cos(direction_)*updateTime_);
		double newy = y_ + (speed_*sin(direction_)*updateTime_);
		if (debug_>10)
			printf("X:%.3f->%.3f Y:%.3f->%.3f\n", x_, newx, y_, newy);
		// verify whether the new position has to be re-computed in order
		// to maintain node position within the simulation field	
		
		if ((newy>yFieldWidth_) || (newy<0))
		{
			switch (bound_)
			{
			case SPHERIC:
				y_ -= yFieldWidth_*(sgn(newy));
				newy -= yFieldWidth_*(sgn(newy));
				break;
			case THOROIDAL:
				x_ = (xFieldWidth_/2) + x_ - newx;
				y_ = (yFieldWidth_/2) + y_ - newy;
				newx = xFieldWidth_/2;
				newy = yFieldWidth_/2;
				break;
			case HARDWALL:
				newy = newy<0?0:yFieldWidth_; 
				break;
			case REBOUNCE:
				if (newy>yFieldWidth_)
				{
					newy = 2*yFieldWidth_ - newy;
					y_ = 2*yFieldWidth_ - y_;
				}else{
					newy = 0 - newy;
					y_ = 0 - y_;
				}
				direction_ *= -1;
				/*if (newy<0){
					if (newx-x_>0)
						plStPtr->gammaOld = -1 * plStPtr->gammaOld;
					else plStPtr->gammaOld = -1 * plStPtr->gammaOld;
				}else{
					if (newx-x_>0)
						plStPtr->gammaOld = -1 * plStPtr->gammaOld;
					else plStPtr->gammaOld = -1 * plStPtr->gammaOld;
				}*/
				break;
			}
		}
		if ((newx>xFieldWidth_) || (newx<0))
		{
			switch (bound_)
			{
			case SPHERIC:
				x_ -= xFieldWidth_*(sgn(newx));
				newx -= xFieldWidth_*(sgn(newx));
				break;
			case THOROIDAL:
				x_ = (xFieldWidth_/2) + x_ - newx;
				y_ = (yFieldWidth_/2) + y_ - newy;
				newx = xFieldWidth_/2;
				newy = yFieldWidth_/2;
				break;
			case HARDWALL:
				newx = newx<0?0:xFieldWidth_;
				break;
			case REBOUNCE:
				if (newx>xFieldWidth_)
				{
					newx = 2*xFieldWidth_ - newx;
					x_ = 2*xFieldWidth_ - x_;
				}else{
					newx = 0 - newx;
					x_ = 0 - x_;
				}
				if (newy==y_)
				{
					if (newx>x_)
						direction_ = 0;
					else
						direction_ = pi;
				}else{
					if (newy>y_)
						direction_ = pi - direction_;
					else 
						direction_ = -pi - direction_;
				}
				break;
			}
		}
		x_ = newx;
		y_ = newy;
		directionMean_ = direction_;
	}
	nextUpdateTime_ = t;
	if (debug_>10)
		printf("nextUpdateTime = %f, now %f, updateTime %f\n", nextUpdateTime_, now, updateTime_);
}

double GMPosition::getX()
{
	double now = Scheduler::instance().clock();
	if (now > nextUpdateTime_)
		update(now);
	return (x_);
}

double GMPosition::getY()
{
	double now = Scheduler::instance().clock();
	if (now > nextUpdateTime_)
		update(now);
	return (y_);
}
