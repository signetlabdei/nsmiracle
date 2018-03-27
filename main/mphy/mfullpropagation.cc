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

#include <node-core.h>
#include<rng.h>
#include "mfullpropagation.h"
#include "mphy_pktheader.h"
#include "mspectralmask.h"

/// SIDE_NUM is the number in which the side of the simulation field is divided in shadowing computation
#define SIDE_NUM 20
#define N_RAYS 1


#define twopi 6.28318530718
#define pi 3.14159265359


static class MFullPropagationClass : public TclClass {
public:
  MFullPropagationClass() : TclClass("MPropagation/FullPropagation") {}
  TclObject* create(int, const char*const*) {
    return (new MFullPropagation());
  }
} class_mfullpropagation_class;


void SampleTimer::expire(Event *e)
{
  module->simulationStep();
}

MFullPropagation::MFullPropagation()
	: amp_(NULL),
	incr_(NULL),
	phases_(NULL),
	fad_(NULL),
	maxDopplerShift_(6.0),
	xFieldWidth_(0),
	yFieldWidth_(0),
	d_(-1),
	A2_(NULL),
	N0_(4),
	shadowMat_(NULL),
	beta_(3.4),
	shadowSigma_(0.0),
	refDistance_(1.0),
	rayleighFading_(1),
	sampleTimer_(this),
	timeUnit_(-100),
	nodesNum_(0),
	nodesIndexArray_(NULL),
	debug_(0)
{
	// binding to TCL variables
	bind("maxDopplerShift_", &maxDopplerShift_);
	bind("beta_", &beta_);
	bind("shadowSigma_", &shadowSigma_);
	bind("refDistance_", &refDistance_);
	bind("rayleighFading_", &rayleighFading_);
	bind("timeUnit_", &timeUnit_);
	bind("debug_", &debug_);
	bind("xFieldWidth_", &xFieldWidth_);
	bind("yFieldWidth_", &yFieldWidth_);
}

// TCL command interpreter
int MFullPropagation::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc==2)
	{
		if(strcasecmp(argv[1], "Init")==0)
		{
			// launch the initialization method
			Init();
			return TCL_OK;
		}
	}
	if(argc==3)
	{
		if(strcasecmp(argv[1], "newNode")==0)
		{
			// dynamic cast returns NULL if object is not of the correct type
			Position* pos = dynamic_cast<Position*> (TclObject::lookup(argv[2]));
			if(!pos)
			{
				fprintf(stderr, "Error MFullPropagation::command, does not recognize the Position class instance");
				return TCL_ERROR;
			}
			// keep track of the new user
			if (debug_)
				printf("MFullPropagation: add new simulated node (id %d, pt. %p), position (%.2f,%.2f), tot %d\n", nodesNum_, pos, pos->getX(), pos->getY(), nodesNum_+1);
			Position** temp = new Position*[nodesNum_+1];
			if (nodesNum_>0)
			{
				memcpy(temp, nodesIndexArray_, (sizeof(Position*))*nodesNum_);
				delete [] nodesIndexArray_;
			}
			nodesIndexArray_ = temp;
			nodesIndexArray_[nodesNum_++] = pos;
			return TCL_OK;
		}
	}
	return MPropagation::command(argc, argv);
}


double MFullPropagation::Gaussian()
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


void MFullPropagation::initialize_common(unsigned long int N0, double d, double **amp, double *incr)
{
	unsigned long int i,N=4*N0+2;
	
	int n_users = nodesNum_;
	int tot_bases =nodesNum_;
	
	nRays_ = 1;
  
	for(i=0;i<=N0;i++){ 
		incr[i]=twopi*cos(twopi*i/N)*d;
/* phase increment corresponding to a time increment */
		amp[i][0]=2*cos(pi*i/(N0+1))/sqrt(2*N0);
		amp[i][1]=2*sin(pi*i/(N0+1))/sqrt(2*N0+2);
/* amplitudes of the quadrature oscillators */
  }
  amp[0][0]=amp[0][0]/sqrt(2.0);
  amp[0][1]=amp[0][1]/sqrt(2.0);
}


void MFullPropagation::initialize_phases(unsigned long int N0, double *phases)
{
  unsigned long int i;
  double temp;
  
  for(i=0;i<=N0;i++){ 
       //temp=((double)rand() / RAND_MAX);
       temp = RNG::defaultrng()->uniform_double();
       phases[i]=twopi*temp;
  }
}


void MFullPropagation::initialize_all_phases(unsigned long int N0,double ****phases)
{
	int n_users = nodesNum_;
	int tot_bases =nodesNum_;
	unsigned long int i,c,l;

	for(i=0;i<n_users;i++)
		for(c=0;c<tot_bases;c++)
			for(l=0;l<nRays_;l++)
				initialize_phases(N0,phases[i][c][l]);
}


void MFullPropagation::oscillators(unsigned long int N0,double *phases,double *incr, double **amp,double *x)
{
  unsigned long int i;
  double temp,temp0,temp1,osc;
  
  temp0=0;
  temp1=0;
  for(i=0;i<=N0;i++){ 
       temp=phases[i]+incr[i];
       for(;temp>twopi;)temp-=twopi;
       phases[i]=temp;
       osc=cos(temp);
       temp0+=amp[i][0]*osc;
       temp1+=amp[i][1]*osc;
  }
  x[0]=temp0;
  x[1]=temp1;
}


void MFullPropagation::compute_fading(unsigned long int N0,double ****phases,double *incr, double **amp,double **fad,double ***A2)
{
	int n_users = nodesNum_;
	int tot_bases =nodesNum_;
	unsigned long int i,c,l;
	double fading_comps[2];

	for(i=0;i<n_users;i++)
		for(c=0;c<tot_bases;c++)
		{
			fad[i][c]=0.0;
			for(l=0;l<nRays_;l++){
				oscillators(N0,phases[i][c][l],incr,amp,fading_comps);
				A2_[i][c][l]=(pow(fading_comps[0],2.0)+pow(fading_comps[1],2.0));
				fad[i][c]+=A2[i][c][l];
			}
		}
}

void MFullPropagation::ShadowInit()
{
unsigned int sourceX,sourceY,destX,destY;

	shadowMat_ = (double*)malloc(SIDE_NUM*SIDE_NUM*SIDE_NUM*SIDE_NUM*sizeof(double));
	if(shadowMat_==NULL) {
		fprintf(stderr, "MFullPropagation::ShadowInit, malloc error\n");
		exit(1);
	}

	for(sourceX=0; sourceX<SIDE_NUM; sourceX++)
	{
		for(sourceY=0; sourceY<SIDE_NUM; sourceY++)
		{
			for(destX=0; destX<SIDE_NUM; destX++)
			{
				for(destY=0; destY<SIDE_NUM; destY++)
				{
					bool flag=FALSE;
					unsigned int temp;
				
					if(sourceX-destX<0)
						temp = destX - sourceX;
					else 
						temp = sourceX - destX;
					if(temp<=3)
						flag=TRUE;
					if(sourceY-destY<0)
						temp = destY - sourceY;
					else
						temp= sourceY - destY;
					if(temp<=3)
						flag=TRUE;

					if(temp==TRUE)
						shadowMat_[sourceX*SIDE_NUM*SIDE_NUM*SIDE_NUM+
						sourceY*SIDE_NUM*SIDE_NUM+destX*SIDE_NUM+destY]=
						0.0;
					else
						if((sourceX>destX)||
							((sourceX==destX)&&(sourceY>destY))) {
							// we consider simmetric shadowing, i.e., 
							// there is the same shadowing between two certains
							// cells independently from where is placed the
							// source and the destination.
							shadowMat_[sourceX*SIDE_NUM*SIDE_NUM*SIDE_NUM+
								sourceY*SIDE_NUM*SIDE_NUM+destX*SIDE_NUM+
								destY] = shadowMat_[destX*SIDE_NUM*SIDE_NUM*
								SIDE_NUM+destY*SIDE_NUM*SIDE_NUM+sourceX*
								SIDE_NUM+sourceY];
						}
						else {
						// new value (not yet calculated)
						shadowMat_[sourceX*SIDE_NUM*SIDE_NUM*SIDE_NUM+
						sourceY*SIDE_NUM*SIDE_NUM+destX*SIDE_NUM+destY]=
						Gaussian()*shadowSigma_;
/* old commented code
		gaus=shadowMat[sourceX*SIDE_NUM*SIDE_NUM*SIDE_NUM+
							sourceY*SIDE_NUM*SIDE_NUM+destX*SIDE_NUM+destY];
		if(gaus<-40.0) dati[0]=dati[0]+1;
		else if(gaus>40.0) dati[801]=dati[801]+1;
		else for(j=0; j<801; j++) {
			if((-40.0+j*0.1 < gaus)&&(gaus < -40.0+(j+1)*0.1)) {
				dati[j+1]=dati[j+1]+1;
				break;
			}
		}
end (old commented code) */
					}
				}
			}
		}
	}
}




void MFullPropagation::FadingInit() {

	unsigned long int n_user = nodesNum_;
	unsigned long int tot_bases = nodesNum_;
	unsigned long int n_rays = N_RAYS;
	unsigned long int i,c,l;
 
	//amp_=(double **)malloc((N0_+1)*sizeof(double *));
	amp_ = new double*[N0_+1];
	if(amp_==NULL){
		fprintf(stderr, "MFullPropagation::FadingInit, malloc error (amp)\n");
		exit(1);
	}
	//incr_=(double *)malloc((N0_+1)*sizeof(double));
	incr_ = new double[N0_+1];
	if(incr_==NULL){
		fprintf(stderr, "MFullPropagation::FadingInit, malloc error (incr)\n");
		exit(1);  
	}
	for(i=0;i<=N0_;i++){ 
		//amp_[i]=(double *)malloc(2*sizeof(double));
		amp_[i] = new double[2];
		if(amp_[i]==NULL) {
			fprintf(stderr, "MFullPropagation::FadingInit, malloc error (amp[])\n");
			exit(1);
		}
	}
	//phases_=(double ****)malloc(n_user*sizeof(double ***));
	phases_ = new double***[n_user];
	if(phases_==NULL){
		fprintf(stderr, "MFullPropagation::FadingInit, malloc error (phases)\n");
		exit(1);
	}
	//fad_=(double **)malloc(n_user*sizeof(double *));
	fad_ = new double*[n_user];
	if(fad_==NULL) {
		fprintf(stderr, "MFullPropagation::FadingInit, malloc error (fad)\n");
		exit(1);
	}
	//A2_=(double ***)malloc(n_user*sizeof(double **));
	A2_ = new double**[n_user];
	if(A2_==NULL){
		fprintf(stderr, "MFullPropagation::FadingInit, malloc error (A2)\n");
		exit(1);
	}
	for(i=0;i<n_user;i++){
		//phases_[i]=(double ***)malloc(tot_bases*sizeof(double **));
		phases_[i] = new double**[tot_bases];
		if(phases_[i]==NULL){
			fprintf(stderr, "MFullPropagation::FadingInit, malloc error (phases[])\n");
			exit(1);
		}
		//A2_[i]=(double **)malloc(tot_bases*sizeof(double *));
		A2_[i] = new double*[tot_bases];
		if(A2_[i]==NULL){
			fprintf(stderr, "MFullPropagation::FadingInit, malloc error (A2[])\n");
			exit(1);
		}
		//fad_[i]=(double *)malloc(tot_bases*sizeof(double));
		fad_[i] = new double[tot_bases];
		if(fad_[i]==NULL) {
			fprintf(stderr, "MFullPropagation::FadingInit, malloc error (fad[])\n");
			exit(1);
		}
		for(c=0;c<tot_bases;c++){
			//phases_[i][c]=(double **)malloc(n_rays*sizeof(double *));
			phases_[i][c] = new double*[n_rays];
			if(phases_[i][c]==NULL) {
				fprintf(stderr, "MFullPropagation::FadingInit, malloc error (phases[][])\n");
				exit(1);
			}
			//A2_[i][c]=(double *)malloc(n_rays*sizeof(double));
			A2_[i][c] = new double[n_rays];
			if(A2_[i][c]==NULL) {
				fprintf(stderr, "MFullPropagation::FadingInit, malloc error (A2[][])\n");
				exit(1);
			}
			for(l=0;l<n_rays;l++){
				//phases_[i][c][l]=(double *)malloc((N0_+1)*sizeof(double));
				phases_[i][c][l] = new double[N0_+1];
				if(phases_[i][c][l]==NULL) {
					fprintf(stderr, "MFullPropagation::FadingInit, malloc error (phases[][][])\n");
					exit(1);
				}
			}
		}
	}

	// normalized doppler frequency
	d_ = maxDopplerShift_ * timeUnit_;
    
	// initializing fast fading
	initialize_common(N0_,d_,amp_,incr_);
	initialize_all_phases(N0_,phases_);
	compute_fading(N0_, phases_, incr_, amp_, fad_, A2_);
	// schedule next fading calculation
	simulationStep();
}


void MFullPropagation::Init() {

	if (nodesNum_<=0)
	{
		fprintf(stderr, "MFullPropagation::Init, nodes simulated must be greater than 0, define them with <newNode> TCL command\n");
		exit(1);
	}
	if (((xFieldWidth_<=0)||(yFieldWidth_<=0))&&(shadowSigma_==0))
	{
		fprintf(stderr, "MFullPropagation::Init, x/y field width must be greater than zero\n");
		exit(1);
	}
	if (beta_<0)
	{
		fprintf(stderr, "MFullPropagation::Init, beta below zero\n");
		exit(1);
	}
	if (refDistance_<0)
	{
		fprintf(stderr, "MFullPropagation::Init, reference distance below zero\n");
		exit(1);
	}
	if (shadowSigma_<0)
	{
		fprintf(stderr, "MFullPropagation::Init, shadow Sigma below zero\n");
		exit(1);
	}
	if ((rayleighFading_!=0)&&(rayleighFading_!=1))
	{
		fprintf(stderr, "MFullPropagation::Init, rayleighFading usage: 1 simulate fading, 0 don't simulate fading'\n");
		exit(1);
	}
	if (rayleighFading_==1)
	{
		if (timeUnit_==0)
		{
			fprintf(stderr, "MFullPropagation::Init, timeUnit must be different from 0\n");
			exit(1);
		}
		if (timeUnit_<0)
			// recalculate timeUnit_ as function of maxDopplerShift_
			timeUnit_ = 1. / (maxDopplerShift_*fabs(timeUnit_));
	}
	// Initialize shadowing matriz: each side of the simulated area is diveded
	// into SIDE_NUM parts. Each sub-area has its own shadowing level towards
	// any other cell (mapped in the matrix), this means that multiple
	// transmission between the same cells have the same shadowing level.
	if(shadowSigma_>0.0)
		ShadowInit();
	else
		shadowMat_=NULL;
	if(rayleighFading_==1) 
		FadingInit();
	else {
		amp_=NULL;
		incr_=NULL;
		phases_=NULL;
		fad_=NULL;
		A2_=NULL;
		d_=0.0;
	}
}



double MFullPropagation::PathLoss(double distance, double lambda)
{
	double lossConst, att;
	
	lossConst = pow((lambda)/(4*pi*refDistance_),2.0);
	
	if(refDistance_ > distance)
	{
		att = lossConst;
	}
	else
	{
		att = lossConst * pow((refDistance_/distance), beta_);
	}
	return (att);
}


double MFullPropagation::Shadowing(Position* node1, Position* node2)
{

	unsigned int sourceX, sourceY, destX, destY;
	if(shadowMat_==NULL)
		// shadowing matrix is not initialized -> no shadowing simulated
		return(1.0);
	else {
		sourceX=(unsigned int)(node1->getX() * SIDE_NUM/xFieldWidth_);
		sourceY=(unsigned int)(node1->getY() * SIDE_NUM/yFieldWidth_);
		destX=(unsigned int)(node2->getX() * SIDE_NUM/xFieldWidth_);
		destY=(unsigned int)(node2->getY() * SIDE_NUM/yFieldWidth_);
		assert((sourceX>=0) && (sourceX<SIDE_NUM )
		       && (sourceY>=0) && (sourceY<SIDE_NUM)
		       && (destX>=0) && (destX<SIDE_NUM) 
		       && (destY>=0) && (destY<SIDE_NUM));
		double sh = shadowMat_[sourceX*SIDE_NUM*SIDE_NUM*SIDE_NUM+sourceY*SIDE_NUM*SIDE_NUM+destX*SIDE_NUM+destY];	// dB value
		return(pow(10.0, ((double)(sh))/((double)10.0)));
	}
}


double MFullPropagation::Rayleigh(int txId, int rxId) {

	
	if(fad_==NULL)
		return(1.0);
	else
		return(fad_[txId][rxId]);
}

int MFullPropagation::getSimulatedNodeId(Position* p)
{
	for(int i=0; i<nodesNum_; i++)
	{
		if (nodesIndexArray_[i]==p) return(i);
	}
	return (-1);
}


void MFullPropagation::simulationStep()
{
	// compute a new sample of fading
	compute_fading(N0_, phases_, incr_, amp_, fad_, A2_);
	// schedule next computation
	sampleTimer_.resched(timeUnit_);
// DEBUG code (write trace into file of the gain)
// 	FILE *fdLin;
// 	FILE *fd_dB; 
// 	char filename[100];
// 	char temp[100];
// 	for(int i = 0; i<nodesNum_; i++)
// 	{
// 		strcpy(filename, "gainTraceLinNode");
// 		sprintf(temp,"%d.out",i);
// 		strcat(filename,temp);
// 		fdLin = fopen(filename,"a");
// 		strcpy(filename, "gainTracedBNode");
// 		sprintf(temp,"%d.out",i);
// 		strcat(filename,temp);
// 		fd_dB = fopen(filename,"a");
// 		fprintf(fdLin,"%.3f\t",Scheduler::instance().clock());
// 		fprintf(fd_dB,"%.3f\t",Scheduler::instance().clock());
// 		fprintf(fdLin,"%.3f\t",nodesIndexArray_[i]->getX());
// 		fprintf(fdLin,"%.3f\t",nodesIndexArray_[i]->getY());
// 		fprintf(fd_dB,"%.3f\t",nodesIndexArray_[i]->getX());
// 		fprintf(fd_dB,"%.3f\t",nodesIndexArray_[i]->getY());
// 		for(int j = 0; j<nodesNum_; j++)
// 		{
// 			if (i!=j)
// 			{
// 				double pl = PathLoss(nodesIndexArray_[i]->getDist(nodesIndexArray_[j]), 0.125);
// 				double sh = Shadowing(nodesIndexArray_[i], nodesIndexArray_[j]);
// 				double fad = Rayleigh(i, j);
// 				double gain = pl*sh*fad;
// 				fprintf(fdLin,"%e\t",gain);
// 				fprintf(fd_dB,"%.3f\t",10*log10(gain));
// 			}
// 		}
// 		fprintf(fdLin,"\n");
// 		fprintf(fd_dB,"\n");
// 		fclose(fdLin);
// 		fclose(fd_dB);
// 	}
}


double MFullPropagation::getGain(Packet* p)
{

	hdr_MPhy* h = HDR_MPHY(p);
	//lambda = (double)(((double)3e8) / p->srcSpectralMask->getFreq());
	double pl = PathLoss(h->srcPosition->getDist(h->dstPosition), (double)(((double)3e8) / h->srcSpectralMask->getFreq()));
	double sh = Shadowing(h->srcPosition, h->dstPosition);
	int id1 = getSimulatedNodeId(h->srcPosition);
	int id2 = getSimulatedNodeId(h->dstPosition);
	if ((id1<0)||(id2<0))
	{
		fprintf(stderr, "MFullPropagation::getGain, unrecognized node, (%d,%p) or (%d,%p)\n", id1, h->srcPosition, id2, h->dstPosition);
// 		for(int i=0; i<nodesNum_; i++)
// 		{
// 			printf("node %d pt %p\n",i, nodesIndexArray_[i]);
// 		}
		exit(1);
	}
	double fad = Rayleigh(id1, id2);

// 	printf("Gain %d(%.2f,%.2f) -> %d(%.2f, %.2f): PathLoss %.3f\t Shadowing %.3f\t Fading %.3f -> TOT: %.3f[dB] %e[linear]\n", id1, h->srcPosition->getX(),
// 			h->srcPosition->getY(), id2, h->dstPosition->getX(), h->dstPosition->getY(), pl, sh, fad, pl+sh+fad, gain);
	return (pl*sh*fad);
}

