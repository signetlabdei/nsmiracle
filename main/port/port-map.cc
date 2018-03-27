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

#include <ip.h>

#include "port-map.h"

#include<iostream>

static class PortMapClass : public TclClass 
{
public:
  PortMapClass() : TclClass("Module/Port/Map") {}
  TclObject* create(int, const char*const*) {  return (new PortMap);  }
} class_port_map_module;

PortMap::PortMap() 
  : portcounter(0)
{
  bind("debug_",&debug_);
}

PortMap::~PortMap()
{
}

int PortMap::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc == 3)
    {
      if (strcasecmp(argv[1],"assignPort")==0)
	{
	  Module *m = dynamic_cast<Module*>(tcl.lookup(argv[2]));
	  if(!m)
	    return TCL_ERROR;

	  int port = assignPort(m);
	  tcl.resultf("%d", port);
	  return TCL_OK;
	  
	}


	
    }
  return Module::command(argc, argv);
}



void PortMap::recv(Packet *p)
{
  fprintf(stderr, "PortMap: a Packet is sent without source module!!\n");
  Packet::free(p);
}




void PortMap::recv(Packet *p, int idSrc)
{
  hdr_cmn *ch = HDR_CMN(p);
  hdr_ip *iph = HDR_IP(p);

  if(ch->direction() == hdr_cmn::UP)
    {
   
      map<int, int>::const_iterator iter = id_map.find(iph->dport());

      if(iter == id_map.end())
	{
	  // Unknown Port Number
	  if (debug_) 
	    std::cerr << "PortMap::recv() (dir:UP) " 
		      << " dport=" << iph->dport()
		      << " portcounter=" << portcounter
		      << std::endl;		  
	  drop(p,1,"UPN");
	  return;
	}
      
      int id = iter->second;      
      if(debug_ > 5)
	printf("dest port %d id %d\n", iph->dport(), id);
      sendUp(id, p);
    }
  else
    {
      // direction DOWN
      map<int, int>::const_iterator iter = port_map.find(idSrc);

      if (iter == port_map.end())
	{
	  fprintf(stderr, "PortMap::recv() no port assigned to id %d, dropping packet!!!\n", idSrc);
	  Packet::free(p);
	  return;
	}
      
      int sport = iter->second;
      assert(sport>0 && sport <= portcounter);
      iph->sport() = sport;
      sendDown(p);
    }
}


int PortMap::assignPort(Module* m)
{
  int id =  m->getId();

  // Check that the provided module has not been given a port before 
  if (port_map.find(id) != port_map.end())
    return TCL_ERROR;

  portcounter++;

  int newport = portcounter;

  port_map[id] = newport;
  assert(id_map.find(newport) == id_map.end());
  id_map[newport] = id;
  assert(id_map.find(newport) != id_map.end());

  if (debug_)
    std::cerr << "PortMap::assignPort() " 
	      << " id=" << id
	      << " port=" << newport
	      << " portcounter=" << portcounter
	      << std::endl;		 
  
  return newport;
}
