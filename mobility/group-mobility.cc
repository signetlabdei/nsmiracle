/*
 * Copyright (c) 2008 Regents of the SIGNET lab, University of Padova.
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

#include"group-mobility.h"
#include<node-core.h>
#include<rng.h>

/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class GroupMobPositionClass : public TclClass {
	public:
		GroupMobPositionClass() : TclClass("Position/GroupMob") {}
		TclObject* create(int, const char*const*) {
			return (new GroupMobPosition());
		}
} class_groupmobposition;


void UpdatePosTimer::expire(Event *e)
{
	module->update(Scheduler::instance().clock());
}

GroupMobPosition::GroupMobPosition() : 
	Position(),
	xFieldWidth_(0),
	yFieldWidth_(0),
	xprec_(),
	yprec_(),
	speedMean_(0),
	sigmaSpeed_(0),
	speedM_(0),
	speedS_(0),
	eta_(0),
	alpha_(0),
	charge_(0),
	steps_(),
	bound_(REBOUNCE),
	updateTime_(0),
	updateTmr_(this),
	speed_(0),
	gammaOld_(),
	memoryM_(0),
	memoryS_(0),
	thetaMax_(),
	debug_(0),
	leader_(0)
{
	bind("xFieldWidth_", &xFieldWidth_);
	bind("yFieldWidth_", &yFieldWidth_);
	bind("alpha_", &alpha_);
	bind("updateTime_", &updateTime_);
	bind("speedMean_",&speedMean_);
	bind("sigmaSpeed_",&sigmaSpeed_);
	bind("speedM_",&speedM_);
	bind("speedS_",&speedS_);
	bind("eta_",&eta_);
	bind("charge_",&charge_);
	bind("memoryM_",&memoryM_);
	bind("memoryS_",&memoryS_);
	bind("thetaMax_",&thetaMax_);
	bind("debug_", &debug_);
}

GroupMobPosition::~GroupMobPosition()
{
}


int GroupMobPosition::command(int argc, const char*const* argv)
{
	if (argc == 2)
	{
		if(strcasecmp(argv[1], "move") == 0)
		{
			if (updateTime_<0)
			{
				fprintf(stderr,"Error GroupMobPosition(constructor) updateTime_ <= 0");
				return TCL_ERROR;
			}
			speed_ = speedMean_;
			steps_ = memoryM_;
			gammaOld_ = RNG::defaultrng()->uniform_double() * (2*pi);
			updateTmr_.resched(updateTime_);
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "stop") == 0)
		{
			updateTmr_.cancel();
			return TCL_OK;
		}
	}
	else if(argc == 3)
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
		else if(strcasecmp(argv[1], "leader") == 0)
		{
			Position* lead = (Position *)TclObject::lookup(argv[2]);
			leader_ = lead;
			return TCL_OK;
		}
	}
	return Position::command(argc, argv);
}


double GroupMobPosition::getX()
{
	return (x_);
}

double GroupMobPosition::getY()
{
	return (y_);
}


double GroupMobPosition::distance(Position* pos1, Position* pos2)
{
	double xdiff,ydiff;
    
	xdiff = pos1->getX() - pos2->getX();
	ydiff = pos1->getY() - pos2->getY();
	// if bounds are spheric, compute the shortest distance separating 
	// the two nodes in the spheric (wrapped) 2D plane
	if(bound_==SPHERIC){
		xdiff = (fabs(xdiff)<((xFieldWidth_)/2))? xdiff : (xFieldWidth_-fabs(xdiff));
		ydiff = (fabs(ydiff)<((yFieldWidth_)/2))? ydiff : (yFieldWidth_-fabs(ydiff));
	}
	//printf("Compute Distance:%f\n",sqrt(xdiff*xdiff + ydiff*ydiff));
	return (sqrt(xdiff*xdiff + ydiff*ydiff));
}


double GroupMobPosition::mirror_posx(double xnode, double xleader)
{

	double d, dref;

	d	= fabs(xnode - xleader);
	dref	= (xFieldWidth_)/2.;
	
	// if the two nodes are separated more than half of the max.
	// distance, then we consider the wrapped coordinate of the leader
	// to compute the attraction between leader and the current node
	// the same applies in the procedure below

	if ( (d>dref) && (bound_==SPHERIC) )
		return ( xleader - (xFieldWidth_)*(sgn(xleader-xnode)) );

	return xleader;
}

double GroupMobPosition::mirror_posy(double ynode, double yleader)
{

	double d, dref;

	d	= fabs(ynode - yleader);
	dref	= (yFieldWidth_)/2.;
	
	// if the two nodes are separated more than half of the max.
	// distance, then we consider the wrapped coordinate of the leader
	// to compute the attraction between leader and the current node
	// the same applies in the procedure below

	if ( (d>dref) && (bound_==SPHERIC) ) 
		return ( yleader - (yFieldWidth_)*(sgn(yleader-ynode)) );

	return yleader;
}


double GroupMobPosition::MobGaussian(double avrg, double sigma)
{

	double x1, x2, w, y1;
	static double y2;
	static int use_last = 0;

	if(sigma==0.0)
		return(avrg);

	if (use_last) {
		y1 = y2;
		use_last = 0;
	}
	else {
		do {
			x1 = 2.0 * RNG::defaultrng()->uniform_double() - 1.0;
			x2 = 2.0 * RNG::defaultrng()->uniform_double() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}
	if(y1*sigma+avrg<0.0)
		return(0.0);
	else
		return ((double)(y1*sigma+avrg));
}


// double Gauss(double m,double sigma,int type){
// 	static int cache=0;
// 	static float v1;
// 	float v2,w,y;
// 
// 	if(cache){
// 		cache=0;
// 		return v1*sigma+m;
// 	}
// 	else{
// 		do{
// 			v1=2.0*RNG::defaultrng()->uniform_double()-1.0;
// 			v2=2.0*RNG::defaultrng()->uniform_double()-1.0;
// 			w=(v1*v1)+(v2*v2);
// 		}while(w>1.0);
// 		y=sqrt((-2.0*log(w))/w);
// 		v1=v1*y;
// 		cache=1;
// 		return v2*y*sigma+m;
// 	}
// }


void GroupMobPosition::update(double now)
{
	if (debug_>10) printf("old pos (%f,%f)...", getX(), getY());
	double gamma,rho,delta;
	int rmem;
	double newx,newy,xPrec,yPrec;

	speed_ = speed_*(1.-eta_) + 
			eta_*MobGaussian(speedMean_, sigmaSpeed_);
	rho = speed_ * updateTime_;

	gamma = gammaOld_;
	if (steps_>=1) steps_--;
	else
	{
		rmem = (int)((RNG::defaultrng()->uniform_double()*2*memoryS_)-memoryS_);
		steps_ = (int)((rmem<memoryM_)?0:memoryM_+rmem);
		delta = (RNG::defaultrng()->uniform_double()*thetaMax_);
		gamma += 2*delta-thetaMax_;
		gammaOld_ = gamma;
	}
	
	if (gamma>pi)	gamma -= 2*pi;
	if (gamma<pi)	gamma += 2*pi;
	
	// Compute new indepent position
	x_ += rho*cos(gamma);
	y_ += rho*sin(gamma);
	
	// Add Leader Attraction
	if (leader_!=0)
	{
		double cx,cy,dist;

		cx = mirror_posx(x_,leader_->getX());
		cy = mirror_posy(y_,leader_->getY());

		dist = distance(leader_,this);
		if ((alpha_>0)&&(dist<1)) dist =1;		// limit the force field to avoid explosion of velocity
		rho = updateTime_*MobGaussian(speedM_,speedS_)*charge_*((GroupMobPosition*)leader_)->getCharge()*pow(dist,-alpha_);
		
		if (cx-x_==0) gamma = pi/2*sgn((cy-y_));
		else gamma = atan((cy-y_)/(cx-x_));
		if ((cx-x_)<=0.0) gamma += (y_-cy)>=0.0?pi:-pi;
		
		x_ += rho*cos(gamma);
		y_ += rho*sin(gamma);

	}// End Leader Attraction

	// adjust new position according to bounds behaviour

	xPrec= xprec_;
	yPrec= yprec_;
	newx = x_;
	newy = y_;


	if ((newy>yFieldWidth_) || (newy<0)){
		switch (bound_) {
			case SPHERIC:		yPrec		-=yFieldWidth_*(sgn(newy));
			newy		-=yFieldWidth_*(sgn(newy));
			break;
			case THOROIDAL:	xPrec		= (xFieldWidth_/2) + xPrec - newx;
			yPrec		= (yFieldWidth_/2) + yPrec - newy;
			newx		= xFieldWidth_/2;
			newy		= yFieldWidth_/2;
			break;
			case HARDWALL:		newy=newy<0?0:yFieldWidth_; break;
			case REBOUNCE:		if (newy>yFieldWidth_){
				newy	= 2*yFieldWidth_ - newy;
				yPrec	= 2*yFieldWidth_ - yPrec;
			}else{
				newy	= 0 - newy;
				yPrec	= 0 - yPrec;
			}
								
			gammaOld_ *= -1;
									/*if (newy<0){
			if (newx-xPrec>0)
			plStPtr->gammaOld = -1 * plStPtr->gammaOld;
			else plStPtr->gammaOld = -1 * plStPtr->gammaOld;
		}else{
			if (newx-xPrec>0)
			plStPtr->gammaOld = -1 * plStPtr->gammaOld;
			else plStPtr->gammaOld = -1 * plStPtr->gammaOld;
		}*/
			break;
		}
	}
	if ((newx>xFieldWidth_) || (newx<0)){
		switch (bound_) {
			case SPHERIC:		xPrec		-=xFieldWidth_*(sgn(newx));
			newx		-=xFieldWidth_*(sgn(newx));
			break;
			case THOROIDAL:	xPrec		= (xFieldWidth_/2) + xPrec - newx;
			yPrec		= (yFieldWidth_/2) + yPrec - newy;
			newx		= xFieldWidth_/2;
			newy		= yFieldWidth_/2;
			break;
			case HARDWALL:		newx=newx<0?0:xFieldWidth_; break;
			case REBOUNCE:		if (newx>xFieldWidth_){
				newx	= 2*xFieldWidth_ - newx;
				xPrec	= 2*xFieldWidth_ - xPrec;
			}else{
				newx	= 0 - newx;
				xPrec	= 0 - xPrec;
			}
			if (newy==yPrec){
				if (newx>xPrec)
					gammaOld_ = 0;
				else 
					gammaOld_ = pi;
			}else{
				if (newy>yPrec)
					gammaOld_ = pi - gammaOld_;
				else 
					gammaOld_ = -pi - gammaOld_;
			}
			break;
		}
	}


	if ((newx<0.0)||(newx>xFieldWidth_)|| (newy<0.0)||(newy>yFieldWidth_))
	{
		puts("[GroupMobPosition::update] Errate position! Abort!");
		exit(1);
	}

	x_ = newx;
	y_ = newy;
	if (debug_>10) printf("new pos (%f,%f)\n", x_, y_);
	updateTmr_.resched(updateTime_);
	
// 	FILE *fd;
// 	char filename[100];
// 	sprintf(filename,"movementTraceNode%d.out",debug_);
// 	fd = fopen(filename,"a");
// 	fprintf(fd,"%f\t%f\n", x_, y_);
// 	fclose(fd);
}
