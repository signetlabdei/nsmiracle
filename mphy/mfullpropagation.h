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

#ifndef MFULLPROPAGATION_H 
#define MFULLPROPAGATION_H


#include<tclcl.h>
#include<timer-handler.h>
#include"mpropagation.h"


class MFullPropagation;
/**
 * Timer used by MFullPropagation to schedule next sampling event
 * 
 */
class SampleTimer : public TimerHandler
{
public:
  SampleTimer(MFullPropagation *m) : TimerHandler() { module = m; }
	
protected:
  virtual void expire(Event *e);
  MFullPropagation* module;
};



class Packet;
class Position;


/** 
 * Class handling full propagation model (path loss, shadowing and fading) 
 *
 * It manages the propagation simulator, it counts for path loss (Friis model),
 * shadowing (Gudmonson model) and fading (Jakes).
 * It is necessary to define the number of users to be involved in the simulation
 * before to start it in order to set up all the dynamic variables needed.
 * 
 */

class MFullPropagation : public MPropagation
{
	friend class SampleTimer;
public:
	MFullPropagation();
	/**
	* Method that returns the propagation attenuation factor
	*
	*/
	virtual double getGain(Packet* p);
	virtual int command(int argc, const char*const* argv);
	
protected:
	/** 
	* Method that returns a value from a normal random Gaussian variable (zero mean, unitary viariance)
	*
	*/
	double Gaussian();
	/**
	* This methods initializes Jakes Simulator paramters (i.e., calculates doppler shift and aplitudes 
	* of the sinusoids)
	*
	*
	* @param NO numner of oscillator(s) to be used
	* @param d normalized doppler frequency
	* @param amp
	* @param inc
	*/
	void initialize_common(unsigned long int N0, double d, double **amp, double *incr);
	/**
	* This methods initializes phases of the oscillator(s) for a user
	*
	*
	* @param NO numner of oscillator(s) to be used
	* @param phases pointer of the user's phases array
	*/
	void initialize_phases(unsigned long int N0, double *phases);
	/**
	* This methods initializes phases of the oscillator(s) for all users
	*
	*
	* @param NO numner of oscillator(s) to be used
	* @param phases pointer of the users' phases array
	*/
	void initialize_all_phases(unsigned long int N0,double ****phases);
	/**
	* This methods computes fading quadrature components for each ray of each user,
	* and updates phases of the corresponding oscillators
	*
	*
	* @param NO numner of oscillator(s) to be used
	* @param phases pointer of the user's phases array
	* @param incr
	* @param amp pointer to the fading coefficient matrix
	* @param x
	*/
	void oscillators(unsigned long int N0,double *phases,double *incr, double **amp,double *x);
	/** 
	* Method invoked before start the channel simulation to initialize fading paramenters
	*
	*/
	void FadingInit();
	/** 
	* Method invoked before start the channel simulation to initialize shadowing paramenters
	*
	*/
	void ShadowInit();
	/** 
	* Method invoked before start the channel simulation to initialize variables
	*
	*/
	void Init();
	/**
	* This methos returns the shadowing attenuation factor (according to the lognormal sigma factor)
	*
	*
	* @param id1 identifier of the source node
	* @param id2 identifier of the destination node
	*/
	double Shadowing(Position* node1, Position* node2);
	/**
	* This methods returns the Rayleigh fading fatcor (i.e., according to the ditruibution
	* of the sum of two square Gaussian variables)
	*
	*
	* @param node1 pointer to the Position instance of source node
	* @param node2 pointer to the Position instance of destination node
	*/
	double  Rayleigh(int txId, int rxId);
	/**
	* This methods returns free-space attenuation factor
	*
	*
	* @param distance distance between the two nodes
	*/
	double PathLoss(double distance, double lambda);
	/**
	* This methods returns the id of the simulated node according to its Position class
	*
	*
	* @param p instance of the Position class of the node
	*/
	int getSimulatedNodeId(Position* p);
	/**
	* This methods computes fading quadrature components for all users and all rays,
	* and updates phases of the oscillators for all users
	*
	*
	* @param NO numner of oscillator(s) to be used
	* @param phases pointer of the user's phases array
	* @param incr
	* @param amp pointer to the fading coefficient matrix
	* @param fad
	* @param A2
	*/
	void compute_fading(unsigned long int N0,double ****phases,double *incr, double **amp,double **fad,double ***A2);
	/**
	* This methods launches the computation of a new step of the Jakes Simulator and
	* schedules the next one, according to timeUnit_.
	*
	*/
	void simulationStep();
	
	double** amp_; 					/// internal variable for Jakes Simulator
	double* incr_; 					/// internal variable for Jakes Simulator
	double**** phases_; 			/// internal variable for Jakes Simulator
	double** fad_; 					/// internal variable for Jakes Simulator
	double maxDopplerShift_;		/// doppler frequency (used in Fading Jakes Simulator)
	double d_;						/// normalized doppler frequency  (used in Fading Jakes Simulator)
	double*** A2_; 					/// internal variable for Jakes Simulator
	unsigned int nRays_;			/// internal variable for Jakes Simulator
	int N0_;						/// number of oscillators to be used in fading simulation
	double refDistance_;			/// reference distance in Path Loss computation (OBS)
									/// typically 1-10 m. (indoor), 10-100 m (outdoor)
	double beta_;					/// free-space attenuation factor (used in Jakes Simulator)
									/// Typical values:
									/// Urban macrocells					3.7-6.5
									/// Urban microcells					2.7-3.5
									/// Office building (same floor)		1.6-3.5
									/// Office building (multiple floors)	2-6
									/// Store								1.8-2.2
									/// Factory								1.6-3.3
									/// Home								3
	SampleTimer sampleTimer_;		/// timer to schedule Jakes Simulator schedule events
	double timeUnit_;				/// time between two adjacent Jakes Simulator fading computation
										/// NOTE that when the value acquired is less than 0 the actual time unit
										/// used is 1/(maxDopplerShift_*(abs(timeUnit_)))
	int rayleighFading_;			/// whether or not simulate fading (1: simulate, 0:don't simulate)
	double xFieldWidth_;			/// range of x-axis of the simulated area (for shadowing matrix)
	double yFieldWidth_;			/// range of y-axis of the simulated area (for shadowing matrix)
	double* shadowMat_;				/// matrix in which there are the shadowing gains
									/// the simulation field (defined by xFieldWidth_ x yFieldWidth_)
									/// is divided in SIDE_NUM (see "#define" in fullpropagation.cc)
									/// and for each couple of cells is defined the shadowing gain
									/// accoprding to a log-normal distribution
	double shadowSigma_;			/// lognormal shadowing standard deviation (plugged via tcl)
	int nodesNum_;					/// number of nods involved in the simulation (updated automatically when
									/// a new node is added, i.e., by means of "newNode" tcl command)
	Position** nodesIndexArray_;	/// array which codes the channel simulation nodes id to
									/// their position pointer (of Position Class)
	int debug_;						/// level of output tracing
};

#endif /* MFULLPROPAGATION_H */
