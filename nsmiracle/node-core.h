/* -*- Mode:C++; -*- */
/*
 * Copyright (c) 2006 Regents of the SIGNET lab, University of Padova.
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

 /**
 * \file node-core.h
 * \author Maguolo Federico, Miozzo Marco
 * \date 9-7-06
 */

#ifndef _NODECORE_
#define _NODECORE_

#include <object.h>
#include <clsap.h>
#include <clmessage.h>

/**
 * The POSITION class is a TclObject in which are defined the spatial coordinates of
 * the node, in terms of x-axis and y-axis [m.]
 *
 * @see NodeCore
 **/


class Position : public TclObject
{
	public:
		/**
		* Position constructor
		**/
		Position();
		/**
		* Position destructor
		**/
		virtual ~Position();
		/**
		* Get the current projection on x-axis of the node postion
		* 
		* @return the current projection on x-axis of the node postion
		* 
		**/
		virtual double getX();
		/**
		* Get the current projection on y-axis of the node postion
		* 
		* @return the current projection on y-axis of the node postion
		* 
		**/
		virtual double getY();

		/**
		* Get the current latitude in signed decimal degrees
		* 
		* @return the current latitude in signed decimal degrees
		* 
		**/
		virtual double getLatitude();
		/**
		* Get the current longitude in signed decimal degrees
		* 
		* @return the current longitude in signed decimal degrees
		* 
		**/
		virtual double getLongitude();
    
    /**
    * Get the current altitude on the sea level in signed meters
    * 
    * @return the current altitude on the sea level in signed meters
    * 
    **/
    virtual double getAltitude();

		/**
		* Get the current projection on z-axis of the node postion
		* 
		* @return the current projection on z-axis of the node postion
		* 
		**/
		virtual double getZ();

		/** 
		 * Returns the distance from the other given position object
		 * 
		 * @param p pointer to the other given position object
		 * 
		 * @return distance
		 */
		virtual double getDist(Position* p);

		/** 
		 * Returns the distance from the other given position object
		 * 
		 * @param p pointer to the other given position object
		 * 
		 * @return distance
		 */
// 		virtual double getCoordDist(Position* p);

    /** 
      * Return the relative azimuth (angle on the )
      * of *p with respect to *this 
      * 
      * @param p 
      * 
      * @return azimuth in [-pi/2, pi/2]
      */
    virtual double getRelAzimuth(Position* p);

    /** 
      * Return the relative zenith (polar angle in spherical coordinates)
      * of *p with respect to *this 
      * 
      * @param p 
      * 
      * @return zenith (polar angle) in [-pi/2, pi/2]
      */
    virtual double getRelZenith(Position* p);
    
    /** 
      * Return the relative azimuth (angle on the )
      * of *p with respect to *this 
      * 
      * @param p 
      * 
      * @return azimuth in [-pi/2, pi/2]
      */
//     virtual double getCoordBearing(Position* p);

    /** 
      * Return the relative zenith (polar angle in spherical coordinates)
      * of *p with respect to *this 
      * 
      * @param p 
      * 
      * @return zenith (polar angle) in [-pi/2, pi/2]
      */
//     virtual double getCoordZenith(Position* p);

		/**
		* Set the projection on x-axis of the node postion
		* 
		* @param x value to be set as projection on the x-axis of the node position
		* 
		**/
		virtual void setX(double x);
		/**
		* Set the projection on y-axis of the node postion
		* 
		* @param y value to be set as projection on the y-axis of the node position
		* 
		**/
		virtual void setY(double y);

    /**
    * Set the projection on z-axis of the node postion
    * 
    * @param z value to be set as projection on the z-axis of the node position
    * 
    **/
    virtual void setZ(double z);
    
		/**
		* Set the latitude
		* 
		* @param latitude has to be in signed decimal degrees
		* 
		**/
		virtual void setLatitude(double latitude);
		/**
		* Set the longitude. 
		* 
		* @param longitude has to be in signed decimal degrees
		* 
		**/
		virtual void setLongitude(double longitude);

    /**
    * Set the signed altitude on the sea level
    * 
    * @param altitude signed altitude on the sea level in meters
    * 
    **/
    virtual void setAltitude(double altitude);
      
		/**
		* TCL command interpreter. It implements the following OTcl methods:
		* <ul>
		*  <li><b>getX_ &lt;&gt;</b>: 
		*  	get the current projection on x-axis of the node position
		*  <li><b>getY_ &lt;&gt;</b>: 
		*  	get the current projection on y-axis of the node position
		*  <li><b>setX_ &lt;<i>integer value</i>&gt;</b>: 
		*  	set the projection on x-axis value of the node position to the one requested
		*  <li><b>setY_ &lt;<i>integer value</i>&gt;</b>: 
		*  	set the projection on y-axis value of the node position to the one requested
		* </ul>
		* 
		* Moreover it inherits all the OTcl method of TclObject
		* 
		* 
		* @param argc number of arguments in <i>argv</i>
		* @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
		* 
		* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
		* 
		**/
		virtual int command(int argc, const char*const* argv);
      
      
	protected:

		/** X-axis projection value of the node position */
		double x_;
		/** Y-axis projection value of the node position */
		double y_;
		/** Z-axis projection value of the node position */
		double z_;
		/** latitude of the node position */
		double latitude_;
		/** longitude of the node position */
		double longitude_;
    /** altitude of the node position */
    double altitude_;
    
		/**
		* check if the coordinates passed are correct. 
		* 
		* @return true if latitude and longitude are in [-90,90] and [-180,180]
		**/
		bool checkCoordinates();
};

/**
 * The NODECORE class is a TclObject and Handler used to store all the common information about the node.
 * One of the most important is the cross-layer bus that connect all Modules and all PlugIn between each others.
 * It also maintain the spatial position of the node.
 *
 * @see NodeCore, PlugIn, Module, SAP, ClSAP, Position
 **/
class NodeCore : public TclObject, public Handler {
public:
	/**
	* NodeCore constructor
	**/
	NodeCore();
	/**
	* NodeCore destructor
	**/
	virtual ~NodeCore();
	/**
	* Set the value of the battery
	* 
	* @param battery value to be setted requested
	* 
	**/
	virtual void battery(double battery);
	/**
	* Get the current value of the battery
	* 
	* @return battery value
	* 
	**/
	inline double battery() { return battery_; }	// return the current level of the battery
	/**
	* Asynchronous cross-layer message command demiltiplexer. It has to sent a copy of the messages to the
	* destination(s) indicated.
	* 
	* @param m instace of <i>ClMessage</i> to be dispatched
	* 
	* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	* 
	**/
	virtual int crLayCommand(ClMessage* m);
	/**
	* Synchronous cross-layer message command demiltiplexer. It has to sent a copy of the messages to the
	* destination(s) indicated.
	* 
	* @param m instace of <i>ClMessage</i> to be dispatched
	* 
	* @return zero on success, nonzero on error
	* 
	**/
	virtual int synchronousCrLayCommand(ClMessage* m);
	/**
	* TCL command interpreter. It implements the following OTcl methods:
	* <ul>
	*  <li><b>position &lt;&gt;</b>: 
	*  	get the pointer to the instance in <i>position_</i> of the Position class installed</li>
	*  <li><b>position &lt;<i>Position instance</i>&gt;</b>: 
	*  	return a pointer to the instance in <i>position_</i> of the Position class installed</li>
	*  <li><b>addclsap &lt;<i>ClSAP instance</i>&gt;<i>level value</i>&gt</b>: 
	*  	add a new ClSAP instance to the <i>crossLayerSAPlist_</i> pointers array in the level indicated</li>
	* </ul>
	* 
	* Moreover it inherits all the OTcl method of TclObject and Handler
	* 
	* 
	* @param argc number of arguments in <i>argv</i>
	* @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
	* 
	* @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
	*
	**/
	virtual int command(int argc, const char*const* argv);
	/**
	* Return the pointer to instance of the Position installed
	* 
	* @return the pointer to instance of the Position installed
	* 
	**/
	virtual Position *getPosition();
private:
	/** Pointer to the array of pointers of the ClSAP installed in all levels */
	ClSAP*** crossLayerSAPlist_;
	/** Number of layer in use (must be greater than 0)*/
	int nLayer_;
	/** Array in which are defined the number of ClSAP for each layer */
	int *clsapPerLayer_;
	/**
	* Send an asyncronous cross-layer message (i.e., it does not require a direct answer from the target)
	* to the indicated ClSAP
	* 
	* @param i define the level in which the message has to be sent
	* @param j define the index inside the level in which the message has to be sent
	* @param m an instance of <i>ClMessage</i> to be sent
	* 
	* @see ClSAP, ClMessage, ClTracer
	**/
	void sendClSAP(int i, int j, ClMessage *m);
	/**
	* Send an asyncronous cross-layer message (i.e., it always require a direct answer from the target)
	* to the indicated ClSAP
	* 
	* @param i define the level in which the message has to be sent
	* @param j define the index inside the level in which the message has to be sent
	* @param m an instance of <i>ClMessage</i> to be sent
	* 
	* @see ClSAP, ClMessage, ClTracer
	**/
	void sendSynchronousClSAP(int i, int j, ClMessage *m);
	/**
	* Method used internally to add a ClSAP in the indicated level
	* 
	* @param clsap pointer to instance of the clsap to be added
	* @param level define the level in which the clsap has to be added
	*
	* @return TCL_ERROR when it is not able to add the ClSAP, otherwise TCL_OK
	* 
	* @see ClSAP, ClMessage, ClTracer
	**/
	int addClSAP(ClSAP *clsap, int level);		// PRIVATE ???
	/**
	* Return the number of the layer in use
	* 
	* @return the number of the layer in use
	* 
	* @see ClSAP, ClMessage, ClTracer
	**/
	int getNLayer();
	/**
	* Return the number of the ClSAP installed in the layer indicated
	*
	* @param layer layer in which tests the number of ClSAP installed
	*
	* @return the number of the ClSAP installed in the layer indicated
	* 
	* @see ClSAP, ClMessage, ClTracer
	**/
	int getNClSAP(int layer);
	/**
	* Return the PlugIn id attached to the j-ClSAP at the i-level
	*
	* @param i layer in which PlugIn is installed
	* @param j index within the layer in which PlugIn is installed
	*
	* @return the id of the PlugIn indicated
	* 
	* @see ClSAP
	**/
	int getPluginID(int i, int j);
	/**
	* Return the layer in which the PlugIn indicated is installed
	*
	* @param pluginId id of the PlugIn
	*
	* @return the layer in which the PlugIn indicated is installed
	* 
	* @see PlugIn
	**/
	int getLayer(int pluginId);
	/**
	* Return the index within the layer in which the PlugIn indicated is installed
	*
	* @param pluginId id of the PlugIn
	*
	* @return the index within the layer in which the PlugIn indicated is installed
	* 
	* @see PlugIn
	**/
	int getIdInLayer(int pluginId);
	void handle(Event *e);
	/** Array in which are indicated the layer in which the Plugin indicated is installed */
	int *pluginLayer_;
	/** Array in which are indicated the index in the layer of the Plugin indicated */
	int *pluginIdInLayer_;
	/** Total number of PlugIn installed */
	int pluginNum_;
	/** Pointer to the instance of the Position class of the node */
	Position *position_;
	/** Value of the battery */
	double battery_;	// level of the battery
	// ...other node parameters
};

#endif
