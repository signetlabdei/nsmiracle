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

#include <math.h>

//#include <delay.h>
#include <packet.h>
#include <node-core.h>

//#include <packet-stamp.h>
#include <antenna.h>
//#include <mobilenode.h>
//#include <mrcl_propagation.h>
//#include <mrcl_wireless-phy.h>
#include "simplepathloss.h"
#include "wirelessphy-module.h"


static class SimplePathLossClass: public TclClass {
public:
        SimplePathLossClass() : TclClass("Propagation/SimplePathLoss") {}
        TclObject* create(int, const char*const*) {
                return (new SimplePathLoss);
        }
} class_simplepathloss;

SimplePathLoss::SimplePathLoss()
{
	bind("gamma_", &gamma_);
	bind("d0_", &d0_);
}

// use Friis at less than crossover distance
// use two-ray at more than crossover distance
//static double
double SimplePathLoss::spl(double Pt, double d0, double gamma, double lambda, double d)
{
        /*
         * Simple path loss propagation model.
	 *
         *		     gamma
         *	     	[ d ]
         *  Pr = Pt * K [---]
         *           	[ d0]
	 *
	 *           	      2
         *	   (  lambda )
         * with K= (---------)
	 * 	   ( 4 pi d0 )
	 *
	 * Goldsmith A. "Wireless communication" sez 2.6 p 46
         */
	double K = (lambda / 4 / PI / d0) * (lambda / 4 / PI / d0);
	//printf("lambda=%f d0=%f K=%f Pt=%f d=%f gamma=%f pow=%f\n", lambda, d0, K, Pt, d, gamma, pow(d / d0, gamma));
	return Pt * K * pow(d0 / d, gamma);
}

double SimplePathLoss::Pr(Packet *p, PacketStamp *t, PacketStamp *r, double L, double lambda)
{
	Position *sourcePos;
	Position *destPos;
	double rX, rY, rZ;		// location of receiver
	double tX, tY, tZ;		// location of transmitter
	double d;				// distance
	double Pr;			// received signal power


	// new position Parameters
	hdr_MrclWrlPhy *wph = HDR_MRCLWRLPHY(p);
	sourcePos = wph->sourcePos_;
	destPos = wph->destPos_;
  
	rX = destPos->getX();
	rY = destPos->getY();
	rZ = 0.;
	tX = sourcePos->getX();
	tY = sourcePos->getY();
	tZ = 0.;

	rX += r->getAntenna()->getX();
	rY += r->getAntenna()->getY();
	tX += t->getAntenna()->getX();
	tY += t->getAntenna()->getY();

	d = sqrt((rX - tX) * (rX - tX) 
	   	+ (rY - tY) * (rY - tY) 
	   	+ (rZ - tZ) * (rZ - tZ));

	if(d >= d0_)
	{
		Pr = spl(t->getTxPr(), d0_, gamma_, lambda, d);
	}
	else
	{
		double Gt = t->getAntenna()->getTxGain(rX - tX, rY - tY, rZ - tZ, t->getLambda());
		double Gr = r->getAntenna()->getRxGain(tX - rX, tY - rY, tZ - rZ, r->getLambda());
		Pr = Friis(t->getTxPr(), Gt, Gr, lambda, L, d);
	}
	//printf("PI=%f d=%f pr = %f\n", PI, d, Pr);
	//fflush(stdout);
	return Pr;
}

/*double MrclTwoRayGround::getDist(double Pr, double Pt, double Gt, double Gr, double hr, double ht, double L, double lambda)
{
       // Get quartic root
       return sqrt(sqrt(Pt * Gt * Gr * (hr * hr * ht * ht) / Pr));
}*/

