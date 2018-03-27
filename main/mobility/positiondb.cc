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


#include "positiondb.h"
#include "position-clmsg.h"
#include <node-core.h>
#include <iostream>


static class PositionDBClass : public TclClass {
public:
  PositionDBClass() : TclClass("PlugIn/PositionDB") {}
	TclObject* create(int, const char*const*) {
		return (new PositionDB());
	}
} class_positiondb;



std::map<nsaddr_t, Position*> PositionDB::posmap;

PositionDB::PositionDB()
{
  bind("debug_",&debug_);
}


int PositionDB::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc == 2)
    {
	
	
    }
  else if(argc == 4)
    {
      if(strcasecmp(argv[1], "addpos") == 0)
	{
	  int index = atoi(argv[2]);
	  Position* pp = dynamic_cast<Position*>(tcl.lookup(argv[3]));
	  assert(pp == getPosition());
	  posmap[index] = pp;
	  return TCL_OK;
	}
    }
  return PlugIn::command(argc, argv);
}

double PositionDB::getDistance(nsaddr_t srcid, nsaddr_t dstid)
{
  std::map<int,Position*>::iterator srcit, dstit;
  
  srcit = posmap.find(srcid);
  dstit = posmap.find(dstid);

  if ( ( srcit == posmap.end()) || ( dstit == posmap.end()) )
    {
      // some entry is missing
      return -1;
    }
  else
    {
      return (srcit->second)->getDist(dstit->second);
    }
}


int PositionDB::recvSyncClMsg(ClMessage* m)
{
  //std::cerr << __PRETTY_FUNCTION__ << " received msg of type " << m->type() << std::endl; 
  if (m->type() == CLMSG_POSITION_GET_DIST)
    {

      ClMsgPositionGetDist* mgd;
      mgd = dynamic_cast<ClMsgPositionGetDist*>(m);
      assert(mgd);
      //std::cerr << __PRETTY_FUNCTION__ << " received CLMSG_POSITION_GET_DIST" << std::endl; 
      mgd->returnDist(getDistance(mgd->getId1(),mgd->getId2()));
      return 0;
    }
  
  return PlugIn::recvSyncClMsg(m);
}
