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
#include "node-core.h"

#include<iostream>
#include<cmath>

#define COORD_NOT_SET (-500.0)
/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class PositionClass : public TclClass {
public:
	PositionClass() : TclClass("Position") {}
	TclObject* create(int, const char*const*) {
		return (new Position());
	}
} class_position;

Position::Position()
  : x_(0.0), y_(0.0), z_(0.0), latitude_(COORD_NOT_SET), longitude_(COORD_NOT_SET)
{
}

Position::~Position()
{
}

int Position::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 2)
	{
		if(strcasecmp(argv[1], "getX_") == 0)
		{
			tcl.resultf("%f",getX());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getY_") == 0)
		{
			tcl.resultf("%f",getY());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getZ_") == 0)
		{
			tcl.resultf("%f",getZ());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getLatitude_") == 0)
		{
			tcl.resultf("%f",getLatitude());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getLongitude_") == 0)
		{
			tcl.resultf("%f",getLongitude());
			return TCL_OK;
		}
    else if(strcasecmp(argv[1], "getAltitude_") == 0)
    {
      tcl.resultf("%f",getAltitude());
      return TCL_OK;
    }
	}
	else if(argc == 3)
	{
		if(strcasecmp(argv[1], "setX_") == 0)
		{
			setX(atof(argv[2]));
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setY_") == 0)
		{
			setY(atof(argv[2]));
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setZ_") == 0)
		{
			setZ(atof(argv[2]));
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setLatitude_") == 0)
		{
			setLatitude(atof(argv[2]));
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "setLongitude_") == 0)
		{
			setLongitude(atof(argv[2]));
			return TCL_OK;
		}
    else if(strcasecmp(argv[1], "setAltitude_") == 0)
    {
      setAltitude(atof(argv[2]));
      return TCL_OK;
    }
	}
	return TclObject::command(argc, argv);
}

double Position::getX()
{
	return x_;
}

double Position::getY()
{
	return y_;
}

double Position::getZ()
{
	return z_;
}

double Position::getLatitude()
{
	return latitude_;
}

double Position::getLongitude()
{
	return longitude_;
}

double Position::getAltitude()
{
  return altitude_;
}

double Position::getDist(Position* p)
{
  assert(p);
  double dz = getZ() - p->getZ();

//   if( checkCoordinates() && p->checkCoordinates() ) {

//   }
  double dx = getX() - p->getX();
  double dy = getY() - p->getY();
  return (sqrt(dx*dx + dy*dy + dz*dz));
}


double Position::getRelAzimuth(Position* p)
{
  assert(p);
//   if( checkCoordinates() && p->checkCoordinates() ) return getCoordBearing(p);
  double dx = p->getX() - getX();
  double dy = p->getY() - getY();
  return (atan2(dy, dx));
}


double Position::getRelZenith(Position* p)
{
  assert(p);
//   if( checkCoordinates() && p->checkCoordinates() ) return getCoordZenith(p);
  double dx = p->getX() - getX();
  double dy = p->getY() - getY();
  double dz = p->getZ() - getZ();
  return (atan(sqrt(dx*dx + dy*dy)/  dz));
}


// double Position::getCoordBearing(Position* p) 
// {
//   double lat1 = (double) getLatitude() * M_PI / 180.0; 
//   double lat2 = (double) p->getLatitude() * M_PI / 180.0;
//   double dLon = (double) (p->getLongitude() - getLongitude()) * M_PI / 180.0;
// 
//   double y = sin(dLon) * cos(lat2);
//   double x = cos(lat1)* sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
// 
//   // WARNING original initial bearing should be atan2(y,x). We return atan2(x,y) to have coherent results 
//   // with getRelAzimuth
//   return( atan2(x, y) );
// }


// double Position::getCoordZenith(Position* p) 
// {
//   double dz = p->getZ() - getZ();
//   double coord_dist = getCoordDist(p);
//   return (atan(coord_dist / dz));
// }


void Position::setX(double x)
{
	x_ = x;
}

void Position::setY(double y)
{
	y_ = y;
}


void Position::setZ(double z)
{
	z_ = z;
}

void Position::setLatitude(double latitude)
{
  latitude_ = latitude;
}

void Position::setLongitude(double longitude)
{
  longitude_ = longitude;
}

void Position::setAltitude(double altitude)
{
  altitude_ = altitude;
}

bool Position::checkCoordinates()
{
  return ( (latitude_ >= -90.0) && (latitude_ <= 90.0) && (longitude_ >= -180.0) && (longitude_ <= 180.0) );
}


/* ======================================================================
   TCL Hooks for the simulator
   ====================================================================== */
static class NodeCoreClass : public TclClass {
public:
	NodeCoreClass() : TclClass("NodeCore") {}
	TclObject* create(int, const char*const*) {
		return (new NodeCore());
	}
} class_node_core;

NodeCore::NodeCore() : crossLayerSAPlist_(0), nLayer_(-1), clsapPerLayer_(0), pluginLayer_(0), pluginIdInLayer_(0), pluginNum_(0), position_(0)
{
	bind("battery_", &battery_);
}

NodeCore::~NodeCore()
{
	for(int i = 0; i<nLayer_; i++)
	{
		delete [] crossLayerSAPlist_[i];
	}
	delete [] crossLayerSAPlist_;
	delete [] clsapPerLayer_;
}

// retrun the current value of the battery
void NodeCore::battery(double battery) 
{
	battery_ = battery;
}


// function to add a new ClSAP in the requested level
int NodeCore::addClSAP(ClSAP *clsap, int level)
{
	if (level<0) return 0;
//  	printf("request to add a clsap at level %d (nLayer %d)\n",level,nLayer_);
	if (nLayer_<level)
	{
		// the level requested is not yet implemented -> implement it (and, evetually
		// the intermediates ones)
		int *n = new int[level + 1];
		for(int i = 0; i <= level; i++)
		{
			if(i<=nLayer_)
				n[i] = clsapPerLayer_[i];
			else
				n[i] = 0;
		}
		if (clsapPerLayer_!=0)
			delete [] clsapPerLayer_;
		clsapPerLayer_ = n;
		ClSAP*** temp = new ClSAP**[level + 1];
		memset(temp, 0, sizeof(ClSAP**)*(level + 1));
		nLayer_ = level;
		for(int i = 0; i<=nLayer_; i++)
		{
			//if (i==level) clsapPerLayer_[i]++;
			if (clsapPerLayer_[i]!=0 || i==level)
			{
				if(i==level)
				{
					clsapPerLayer_[i]++;
					temp[i] = new ClSAP*[clsapPerLayer_[i]];
					for (int j = 0; i < clsapPerLayer_[i]-1; j++)
					{
						temp[i][j] = crossLayerSAPlist_[i][j];
					}// end for on clsapPerLayer
					temp[i][clsapPerLayer_[i]-1] = clsap;
					if(clsapPerLayer_[i]-1 != 0)
						delete [] crossLayerSAPlist_[i];
				}
				else
					temp[i] = crossLayerSAPlist_[i];
			}
			
			
		}// end for on layer
		if (crossLayerSAPlist_!=0)
			delete [] crossLayerSAPlist_;
		crossLayerSAPlist_ = temp;
	}// end if getNLayer < level
	else
	{
		ClSAP** temp = new ClSAP*[++clsapPerLayer_[level]];
		for (int j = 0; j < clsapPerLayer_[level]-1; j++)
		{
			temp[j] = crossLayerSAPlist_[level][j];
		}// end for on clsapPerLayer
		temp[clsapPerLayer_[level]-1] = clsap;
		delete [] crossLayerSAPlist_[level];
		crossLayerSAPlist_[level] = temp;
	}
	int pluginId = clsap->getPluginId();
	// update pluginId to modules in node matrix structures
	if (pluginId>=pluginNum_)
	{
		// have to resize the structure
		int* temp1 = new int[pluginId+1];
		memset(temp1, -1, sizeof(int)*(pluginId+1));
		int* temp2 = new int[pluginId+1];
		memset(temp2, -1, sizeof(int)*(pluginId+1));
		if (pluginLayer_!=0)
		{
			for(int i=0; i<=pluginId; i++)
			{
				temp1[i] = pluginLayer_[i];
				temp2[i] = pluginIdInLayer_[i];
			}
			delete [] pluginLayer_;
			delete [] pluginIdInLayer_;
		}
		pluginLayer_ = temp1;
		pluginIdInLayer_ = temp2;
		pluginNum_ = pluginId;
	}
	pluginLayer_[pluginId] = level;
	pluginIdInLayer_[pluginId] = clsapPerLayer_[level]-1;
// 	printf("pluginIdInLayer_[%d]=%d\n",pluginId, clsapPerLayer_[level]);
	return 1;
}


// TCL command interpreter
int NodeCore::command(int argc, const char*const* argv)
{
	if (argc == 2)
	{
		Tcl& tcl = Tcl::instance();
		if(strcasecmp(argv[1], "position") == 0)
		{
			if(!position_)
				return TCL_ERROR;
			tcl.result(position_->name());
			return TCL_OK;
		}
	}
	else if(argc ==3)
	{
		Tcl& tcl = Tcl::instance();
		if(strcasecmp(argv[1], "position") == 0)
		{
		        position_ = dynamic_cast<Position*>(tcl.lookup(argv[2]));
			if(!position_)
				return (TCL_ERROR);
			return TCL_OK;
		}
	}
	else if(argc==4)
	{
		if(strcasecmp(argv[1], "addclsap") == 0)
		{
			ClSAP *clsap = dynamic_cast<ClSAP*>(TclObject::lookup(argv[2]));
			int level = atoi(argv[3]);
			if(!clsap || level < 0)
				return (TCL_ERROR);
			if(addClSAP(clsap, level))
				return (TCL_OK);
			return (TCL_ERROR);
		}
	}
	return TclObject::command(argc, argv);
}


// send a cross-layer message to i-level j-module
void NodeCore::sendClSAP(int level, int j, ClMessage *m)
{
	crossLayerSAPlist_[level][j]->sendModule(m, 0);
}

// send a cross-layer message to i-level j-module
void NodeCore::sendSynchronousClSAP(int level, int j, ClMessage *m)
{
	crossLayerSAPlist_[level][j]->sendSynchronousModule(m);
}

// return the number of the layer current implemented
int NodeCore::getNLayer()
{
	return nLayer_;
}

// return the number of ClSAP installed at layer
int NodeCore::getNClSAP(int layer)
{
	return clsapPerLayer_[layer];
}

// return the j-plugin id at level indicated
int NodeCore::getPluginID(int level, int j)
{
	return crossLayerSAPlist_[level][j]->getPluginId();
}

// return the layer in which the module specified is installed
int NodeCore::getLayer(int pluginId)
{
	if ((pluginId<0)||(pluginId>pluginNum_)||(pluginLayer_[pluginId]<0))
	{
		fprintf(stderr, "Error, NodeCore.getLayer: plugin specified does not exist\n");
		exit(1);
	}
	return (pluginLayer_[pluginId]);
}

// return the id within a layer of the specified module
int NodeCore::getIdInLayer(int pluginId)
{
	if ((pluginId<0)||(pluginId>pluginNum_)||(pluginIdInLayer_[pluginId]<0))
	{
		fprintf(stderr, "Error, NodeCore.getIdInLayer: plugin specified does not exist\n");
		exit(1);
	}
	return (pluginIdInLayer_[pluginId]);
}



// cross layer message command interpreter
int NodeCore::crLayCommand(ClMessage* m)
{
	// send a clMessage to its destination
	int dest = m->getDest();
	int src = m->getSource();
	if(m->getDestType() == UNICAST)
	{
		if ((dest<0)||(dest>pluginNum_))
		{
		  	cerr << __PRETTY_FUNCTION__ 
			     << " ClMsg dest (" << dest 
			     << ") does not exist, discarding message " << endl;
			delete m;
			return 1;
		}

		if (dest == src)
		{
		  	cerr << __PRETTY_FUNCTION__ 
			     << " ClMsg dest == src (" << dest 
			     << "), discarding message " << endl;
			delete m;
			return 1;
		}

		int layer = getLayer(dest);
		int idLayer = getIdInLayer(dest);
		sendClSAP(layer, idLayer, m->copy());
	
	}
	else
	{
		/*
		It is a broadcast message
		*/
		if ((dest < 0 && dest != CLBROADCASTADDR)||(dest>nLayer_))
		{
			fprintf(stderr, "Error, NodeCore.crLayCommand: clMessage destination does not exist\n");
			exit(1);
		}
		if(dest != CLBROADCASTADDR)
		{
			/*
			We send the message to all the modules in layer dest
			*/
			for(int j = 0; j < getNClSAP(dest); j++)
				if(getPluginID(dest, j) != src)
					sendClSAP(dest, j, m->copy());
		}
		else
		{
			/*
			We send the message to all the modules
			*/
// 			printf("send a broadcast clmsg nlayer %d\n ",getNLayer());
			int i,j;
			// layer 0 is the "plugin" layer, layers > 0 are ordinary layers		
			for(i = 0; i <= getNLayer(); i++)
			{
// 				printf("nclasp %d\n",getNClSAP(i));
				for(j = 0; j < getNClSAP(i); j++)
				{
// 					printf("try with %d,%d id = %d (src=%i)\n",i,j,getPluginID(i, j), src);
					if(getPluginID(i, j) != src)
					{
						sendClSAP(i, j, m->copy());
// 						printf("send to %d\n", getPluginID(i, j));
					}
				}
			}
		}
	}
	delete m;
	return 0;
}

// cross layer message command interpreter
int NodeCore::synchronousCrLayCommand(ClMessage* m)
{
	// send a clMessage to its destination
	int dest = m->getDest();
	if(m->getDestType() == UNICAST)
	{
		if ((dest<0)||(dest>pluginNum_))
		{
			fprintf(stderr, "Error, NodeCore.crLayCommand: clMessage destination does not exist\n");
			exit(1);
		}
		int layer = getLayer(dest);
		int idLayer = getIdInLayer(dest);
		sendSynchronousClSAP(layer, idLayer, m);
		// OLD basic version: send to all the ClSAP installed on modules
	/*	for(int i = 1; i <= getNLayer(); i++)
			for(int j = 0; j < getNClSAP(i); j++)
				sendClSAP(i, j, m->copy());*/
	
	}
	else
	{
		/*
		It is a broadcast message
		*/
		if ((dest < 0 && dest != CLBROADCASTADDR)||(dest>nLayer_))
		{
			fprintf(stderr, "Error, NodeCore.crLayCommand: clMessage destination does not exist\n");
			exit(1);
		}
		int src = m->getSource();
		if(dest != CLBROADCASTADDR)
		{
			/*
			We send the message to all the modules in layer dest
			*/
			for(int j = 0; j < getNClSAP(dest); j++)
				if(getPluginID(dest, j) != src)
					sendSynchronousClSAP(dest, j, m);
		}
		else
		{
			/*
			We send the message to all the modules
			*/
		  // fprintf(stderr,"%s send a broadcast clmsg nlayer %d\n ", __PRETTY_FUNCTION__, getNLayer());
			int i,j;

			// layer 0 is the "plugin" layer, layers > 0 are ordinary layers
			for(i = 0; i <= getNLayer(); i++)
			{
// 				printf("nclasp %d\n",getNClSAP(i));
				for(j = 0; j < getNClSAP(i); j++)
				{
// 					printf("try with %d,%d id = %d (src=%i)\n",i,j,getPluginID(i, j), src);
					if(getPluginID(i, j) != src)
					{
					  //fprintf(stderr, "%s send to %d\n", __PRETTY_FUNCTION__,  getPluginID(i, j));
						sendSynchronousClSAP(i, j, m);
					}
				}
			}
		}
	}
	//delete m;
	return 1;
}

void NodeCore::handle(Event* e)
{
	crLayCommand((ClMessage*)e);
}

Position *NodeCore::getPosition()
{
	return position_;
}
