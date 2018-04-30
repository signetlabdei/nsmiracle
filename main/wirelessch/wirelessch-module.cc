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

#include "wirelessch-module.h"

static class WirelessChModuleClass : public TclClass {
public:
	WirelessChModuleClass() : TclClass("Module/WirelessCh") {}
	TclObject* create(int, const char*const*) {
	return (new WirelessChModule());

}
} class_wirelesschmodule;

WirelessChModule::WirelessChModule() : ChannelModule(), sorted_(0)
{
	bind("CSThresh_", &CSThresh_);
	bind("freq_", &freq_);
	bind("L_", &L_);
	lambda_ = SPEED_OF_LIGHT / freq_;

}

WirelessChModule::~WirelessChModule()
{
}



int WirelessChModule::command(int argc, const char*const* argv)
{
	//Tcl& tcl = Tcl::instance();
	
// 	if (argc==3)
// 	{
// 		// Over-ride of a channel-module tcl command:
// 		// 1) and set up the array of the nodes used for reduce complexity in packet trasmission
// 		// 2) install a SAP to an above module (call channel-module tcl command)
// 		if (strcasecmp(argv[1],"addsap")==0)
// 		{
// 			ChSAP *chsap = dynamic_cast<ChSAP*>(TclObject::lookup(argv[2]));
// 			if (!chsap)
// 				return TCL_ERROR;
// 			addNode(chsap);
// 			Module::command(argc, argv);
// 		}
// 		
// 	}
	return ChannelModule::command(argc, argv);
}


void WirelessChModule::sortChSAPList()
{
	if (!getChSAPnum())
		return;

	sorted_ = true;
	
	int n = getChSAPnum();
	// Bubble Sort
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < (n - 1 - i); j++)
		{
		  if ( ( ((ChSAP *)getChSAP(j+1))->getPosition())->getX() < (((ChSAP *)getChSAP(j))->getPosition())->getX() )
                        {
                                // swap i-el and i-el
                                swapChSAP(j+1,j);
                        }
		}
	}

	
	// BUBBLE SORT ORIGINAL VERSION (from www.cs.princeton.edu/~ah/alg_anim/gawain-4.0/BubbleSort.html)
// 	for (i=0; i<n-1; i++) {
//   for (j=0; j<n-1-i; j++)
//     if (a[j+1] < a[j]) {  /* compare the two neighbors */
//       tmp = a[j];         /* swap a[j] and a[j+1]      */
//       a[j] = a[j+1];
//       a[j+1] = tmp;
//   }
}



int* WirelessChModule::getInfluencedNodes(Position *p, double radius,  int *numInfluencedNodes)
{
	double xmin, xmax, ymin, ymax;
	int n = 0;
	
	int nodesNum = getChSAPnum();
	
	if (!nodesNum) {
		*numInfluencedNodes=-1;
		fprintf(stderr, "WirelessChModule::getInfluencedNodes, no ChSAP installed when trying to send!!!\n");
		return NULL;
	}
	
	xmin = p->getX() - radius;
	xmax = p->getX() + radius;
	ymin = p->getY() - radius;
	ymax = p->getY() + radius;
	
	double xpos,ypos;
	double xprec = 0;
	
	// First allocate as much as possible needed
	int* tmpList = new int[nodesNum];
	
	for(int i = 0; i < nodesNum; i++)
	{
		xpos = ( ((ChSAP *)getChSAP(i))->getPosition() )->getX();
		ypos = ( ((ChSAP *)getChSAP(i))->getPosition() )->getY();
		//printf("Node from ChSAP %p has position (%f,%f)\n",((ChSAP *)getChSAP(i)),xpos,ypos);
		
		if (xpos < xprec)
		{
			delete [] tmpList;
			// re-sort the chsap list by x-pos and re-do this function
			sortChSAPList();
			return (getInfluencedNodes(p, radius, numInfluencedNodes));
		}
		
		if (	(xpos >= xmin) &&  (xpos <= xmax) )
		{
			if (( ypos >= ymin) && ( ypos <= ymax) )
			{
				tmpList[n++] = i;
			}
		}
		if (xpos > xmax)
			break;				// end of the inteference area

		xprec = xpos;
	}
	
	
	int* list = new int[n];
	memcpy(list, tmpList, n * sizeof(int));
	delete [] tmpList;
	*numInfluencedNodes = n;
	return list;
}


double WirelessChModule::getPropDelay(Position *source, Position* dest)
{
	return	(sqrt( ((source->getX()-dest->getX())*(source->getX()-dest->getX())) + 
				((source->getY()-dest->getY())*(source->getY()-dest->getY())) ) / SPEED_OF_LIGHT);
}

void WirelessChModule::sendUpPhy(Packet *p,ChSAP *chsap)
{

	
	Scheduler &s = Scheduler::instance();
	struct hdr_cmn *hdr = HDR_CMN(p);
	
	hdr->direction() = hdr_cmn::UP;
	
	double Pt = p->txinfo_.getTxPr();
	// calculate max radius
	double radius = sqrt(( Pt * lambda_ * lambda_) / (L_ * CSThresh_)) /  (4 * PI);

	if(!sorted_){
		sortChSAPList();		// first packet trasmission sorting
	}
	
	Position *sourcePos = chsap->getPosition();
	ChSAP *dest;
	
	int *affectedNodes;
	int numInfluencedNodes = -1, i;
	affectedNodes = getInfluencedNodes(chsap->getPosition(), radius + /* safety */ 5, &numInfluencedNodes);
	
	for (i=0; i < numInfluencedNodes; i++) {
		
		dest = (ChSAP*)getChSAP(affectedNodes[i]);
		if (chsap == dest)		// it's the source node -> skip it
			continue;
		
		s.schedule(dest, p->copy(), getPropDelay(sourcePos, dest->getPosition()));
	}
	
	delete [] affectedNodes;

	Packet::free(p);
}

void WirelessChModule::recv(Packet *p, ChSAP* chsap)
{
	sendUpPhy(p, chsap);
}
