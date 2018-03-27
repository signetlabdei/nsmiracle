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

/* -*-	Mode:C++ -*- */
#include "mrcl_freespace.h"

#include <node-core.h>
#include <stdio.h>


#include "mrcl_wireless-phy.h"
#include "wirelessphy-module.h"


class PacketStamp;

static class MrclFreeSpaceClass: public TclClass {
public:
	MrclFreeSpaceClass() : TclClass("Propagation/MrclFreeSpace") {}
	TclObject* create(int, const char*const*) {
		return (new MrclFreeSpace);
	}                   
} class_mrclfreespace;


double MrclFreeSpace::Pr(Packet *p, PacketStamp *t, PacketStamp *r, double L, double lambda)
{
	Position *sourcePos;
  Position *destPos;

	double Xt, Yt, Zt;		// location of transmitter
	double Xr, Yr, Zr;		// location of receiver

	// old position paramters (from MobileNode)
// 	t->getNode()->getLoc(&Xt, &Yt, &Zt);
// 	r->getNode()->getLoc(&Xr, &Yr, &Zr);
	hdr_MrclWrlPhy *wph = HDR_MRCLWRLPHY(p);
	sourcePos = wph->sourcePos_;
  	destPos = wph->destPos_;
  

  Xr = destPos->getX();
  Yr = destPos->getY();
  Zr = 0.;
  Xt = sourcePos->getX();
  Yt = sourcePos->getY();
  Zt = 0.;
	// Is antenna position relative to node position?
	Xr += r->getAntenna()->getX();
	Yr += r->getAntenna()->getY();
	Zr += r->getAntenna()->getZ();
	Xt += t->getAntenna()->getX();
	Yt += t->getAntenna()->getY();
	Zt += t->getAntenna()->getZ();

	double dX = Xr - Xt;
	double dY = Yr - Yt;
	double dZ = Zr - Zt;
	double d = sqrt(dX * dX + dY * dY + dZ * dZ);

	// get antenna gain
	double Gt = t->getAntenna()->getTxGain(dX, dY, dZ, lambda);
	double Gr = r->getAntenna()->getRxGain(dX, dY, dZ, lambda);

	double Pt = t->getTxPr();

	// calculate receiving power at distance
	double Pr = Friis(Pt, Gt, Gr, lambda, L, d);
	       			
	//printf("%lf: d: %lf, Pr: %e\n", Scheduler::instance().clock(), d, Pr);

	// This is a Free Space model, which is valid only in the far field.
	// This part prevents Pr to be > Pt for d close to 0
	if (Pr<Pt)
		return Pr;
	else
		return Pt;
}

double MrclFreeSpace::getDist(double Pr, double Pt, double Gt, double Gr, double hr, double ht, double L, double lambda)
{
        return sqrt((Pt * Gt * Gr * lambda * lambda) / (L * Pr)) /
                (4 * PI);
}

