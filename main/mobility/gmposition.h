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

#ifndef _GAUSSMARKOVMOBMODEL_
#define _GAUSSMARKOVMOBMODEL_

#include "node-core.h"


#define sgn(x) ( ((x)==0.0) ? 0.0 : ((x)/fabs(x)) )
#define pi (4*atan(1.0))


/**
 * \enum BoundType
 * \brief Defines the behaviour of the node when it reaches an edge of the simulation field
 *
 **/
typedef enum BoundType
{
	SPHERIC,
	THOROIDAL,
	HARDWALL,
	REBOUNCE
};

/**
 * This class implements the Gauss Markov mobility model. Movement occurs by 
 * updating the speed and the direction only when an information regarding 
 * node position is required, in this case, if is necessary, all the uncalculated
 * previous states are computed according to a finite state Markov process.
 * The model is designed to adapt to different level of randomness via one tuning
 * parameter (alpha). Initially each node is assigned a current speed and direction,
 * which will be updated during the simulation.
 *
 * @see NodeCore, Position, BMPosition
 **/
class GMPosition : public Position
{
	public:
		/**
		* Constructor
		*/
		GMPosition();
		/**
		* Destructor
		*/
		virtual ~GMPosition();
		/**
		* Method that return the current projection of the node on the x-axis.
		* If it's necessary (updating time ia expired), update the position values 
		* before returns it.
		*/
		virtual double getX();
		/**
		* Method that return the current projection of the node on the y-axis.
		* If it's necessary (updating time ia expired), update the position values 
		* before returns it.
		*/
		virtual double getY();
		
		/**
		* TCL command intepreter
		* <ul>
		*  <li><b>bound &lt;<i>BoundType value</i>&gt;</b>: 
		*  	set the behaviour ot the node when it reaches the edge of the simulation field
		*  <li><b>speedMean &lt;<i>real value</i>&gt;</b>: 
		*  	set the mean value of the speed
		* </ul>
		* 
		* Moreover it inherits all the OTcl method of Position
		* 
		* 
		* @param argc number of arguments in <i>argv</i>
		* @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
		* 
		* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
		* 
		**/
		virtual int command(int argc, const char*const* argv);
	private:
		/**
		* Method that updates both the position coordinates as function of the number 
		* of states to be evaluated.
		*/
		virtual void update(double now);
		/** 
		* Method that returns a value from a normal random Gaussian variable (zero mean, unitary viariance)
		*
		*/
		double Gaussian();
		
		
		double xFieldWidth_;					/// range of the x-axis of the field to be simulated 
		double yFieldWidth_;					/// range of the y-axis of the field to be simulated
		double alpha_;							/// Parameter to be used to vary the randomness:
                                /// 0: totally random values (Brownian motion)
                                /// 1: linear motion
		double speedMean_;					/// Defines the mean value of the speed
													/// when it is setted to zero the node moves anyway
		double directionMean_;					/// Defines the mean value of the direction
		BoundType bound_;							/// Defines the behaviour of the node when it reaches the edge:
														/// SPHERIC: return in the simulation field on the opposite side
														/// THOROIDAL: return in the centre of simulation field
														/// HARDWALL: the movement is stopped in the edge
														/// REBOUNCE: the node rebounce (i.e., the movement that should be 
														/// outside the simulation field is mirrored inside)
		double updateTime_;						/// Time between two update computation
		double nextUpdateTime_;					/// Intenal variable used to evaluate the steps to be computed
		double speed_;								/// current value of the speed
		double direction_;						/// current value of the direction
		int debug_;									/// define the level of verbosity (for debugging purposes)
};

#endif		// _GAUSSMARKOVMOBMODEL_
