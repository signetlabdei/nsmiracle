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


#include"static-multihop-routing.h"
#include<node-core.h>
#include<iostream>
#include <iomanip>
#include <fstream>
#include <boost/graph/graphviz.hpp>
#include <sstream>


SMR_Node::SMR_Node()
{
  // why should we use this constructor?
  //  assert(0); 
}

SMR_Node::SMR_Node(Position* p, IPRoutingModule* r, nsaddr_t a)
  : pos(p), 
    ipr(r),
    addr(a)
{
}



static class StaticMultihopRoutingClass : public TclClass {
public:
  StaticMultihopRoutingClass() : TclClass("StaticMultihopRouting") {}
	TclObject* create(int, const char*const*) {
		return (new StaticMultihopRouting());
	}
} class_StaticMultihopRouting;



StaticMultihopRouting::StaticMultihopRouting()
  : graph(0)
{
  //std::cerr << __PRETTY_FUNCTION__ << std::endl;
  bind("debug_",&debug_);
  bind("MaxHopLength_",&MaxHopLength_);
}



int StaticMultihopRouting::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();

  if(argc == 2)
    {
      if(strcasecmp(argv[1], "reset") == 0)
	{	  
	  reset();	  	
	  return TCL_OK;
	}
    }  
  else if(argc == 3)
    {
      if(strcasecmp(argv[1], "graphviz_output") == 0)
	{	  
	  graphviz_output(argv[2]);	  	
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "xmax") == 0)
	{	  
	  xmax = atof(argv[2]);
	  return TCL_OK;
	}
    }
  if(argc == 4)
    {
      if(strcasecmp(argv[1], "setpath") == 0)
	{
	  nsaddr_t addr1 = atoi(argv[2]);
	  nsaddr_t addr2 = atoi(argv[3]);
	  setPath(a2i(addr1), a2i(addr2));	  	
	  return TCL_OK;
	}
    }
  if(argc == 5)
    {
      if(strcasecmp(argv[1], "setpath") == 0)
	{
	  nsaddr_t addr1 = atoi(argv[2]);
	  nsaddr_t addr2 = atoi(argv[3]);
	  setPath(a2i(addr1), a2i(addr2), argv[4]);	  	
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "addpos") == 0)
	//std::cerr  << __PRETTY_FUNCTION__ <<  " command: " << argv[1] <<std::endl;
	{
	  nsaddr_t addr = atoi(argv[2]);	  
	  Position* p = dynamic_cast<Position*>(tcl.lookup(argv[3]));
	  assert(p);
	  IPRoutingModule* r = dynamic_cast<IPRoutingModule*>(tcl.lookup(argv[4]));
	  assert(r);
	  SMR_Node node(p,r,addr);
	  
          addPos(node);
	  //	  std::cerr << " index " << index  << " addr " << addr  << std::endl;
	
	  return TCL_OK;
	}
    }
  return TclObject::command(argc, argv);
}

double StaticMultihopRouting::getCost(SMR_Node m, SMR_Node n)
{
  assert(m.pos);
  assert(n.pos);
  double dist = m.pos->getDist(n.pos);
  if (dist < MaxHopLength_)
    return dist;
  else 
    return INFINITY;
}

void StaticMultihopRouting::addPos(SMR_Node node)
{
  int index = add_vertex(graph);
  a2im[node.addr]  = index;
  i2am[index] = node.addr;

  SMR_NodePropMap::iterator it = npm.begin();	  
  while(it != npm.end())
    {
      double cost = getCost(it->second, node);
      if (isfinite(cost))
	{
	  nsaddr_t otheraddr = it->second.addr;
	  SMR_EdgeDescr e;
	  bool inserted;
	  //std::cerr << "adding edge " << a2i(addr) << " -- " << a2i(otheraddr) << endl;
	  tie(e, inserted) = add_edge(a2i(node.addr), a2i(otheraddr), graph);
	  assert(inserted);
	  put(edge_weight, graph, e, cost);
	}
      it++;
    }

  npm[node.addr] = node;
  npv[index] = node;
  assert(a2i(node.addr) == index);
  assert(i2a(index) == node.addr);
}




void StaticMultihopRouting::setPath(int src, int dst, const char* fname)
{
  std::vector<SMR_VertDescr> p(num_vertices(graph));   // predecessor map
  std::vector<double>        d(num_vertices(graph));

  dijkstra_shortest_paths(graph, src, predecessor_map(&p[0]).distance_map(&d[0]));

  if (fname != NULL)
    graphviz_output_path(fname, src, dst, p);

  setRoutingTablesForPath(src, dst, p);
}


void StaticMultihopRouting::setRoutingTablesForPath(int src, int dst, std::vector<SMR_VertDescr>& p)
{
  std::cout << __PRETTY_FUNCTION__ << " setting path " << src << " --> " <<dst << std::endl;
  SMR_VertDescr v = dst;
  SMR_VertDescr u = p[v];
  do
    {      
      // set routing table at the predecessor u so that v is the next hop for dst
      npv[u].ipr->addRoute(i2a(dst), 0xFFFFFFFF, i2a(v));
      v = u;
      u = p[v];
    }
  while (u != v);
  
  if (u != src)
      std::cout << __PRETTY_FUNCTION__ << " : destination unreachable" << std::endl;
}




void StaticMultihopRouting::graphviz_output_path(const char* fname, int src, int dst, std::vector<SMR_VertDescr>& p)
{   
  ofstream dot_file(fname, ios_base::trunc);
  double graphviz_size = 20.0;
  double scale_factor = graphviz_size / xmax;

  EdgeWeightPropMap ewmap = get(edge_weight, graph);
   
  dot_file << "graph G {\n"
	   << "size=\"" << graphviz_size << "," << graphviz_size << "\";\n"
	   << " node [shape=\"circle\"]\n";
  

  SMR_VertIter vi, vi_end;
  for (tie(vi, vi_end) = vertices(graph); vi != vi_end; ++vi) 
    {
      SMR_VertDescr v;
      v = *vi;

      Position* pos = ((npv.find(v))->second).pos;      
      dot_file << v 
	       << " [pos=\"" << pos->getX() * scale_factor << "," 
	       << pos->getY() * scale_factor << "!\""
	//<< ", label=\"" << pos->getZ()  << "\""
	       << "];\n";
    }

  SMR_VertDescr v = dst;
  SMR_VertDescr u = p[v];


  while (u != v)
    {      
      SMR_EdgeDescr e;
      std::cout << " Node: " << v
		<< " Predecessor: " << u << std::endl;

      {
	// all this mess just because it appears not possible to just
	// make an edge out of u and v...
	SMR_InEdgeIter ei, ei_end;
	tie(ei, ei_end) = in_edges(v, graph);	

	do
	  {
	    assert(ei != ei_end);
	    e = *ei;
	    ++ei;
	  }
	while (source(e, graph) != u);
	assert(source(e,graph) == u);
	assert(target(e,graph) == v);
      }

      dot_file << u << " -- " << v 
	       << " [label=\"" 
	       << get(ewmap, e) << "\""
	       << "];\n";
      v = u;
      u = p[v];
    }

  if (u != src)
    std::cout << __PRETTY_FUNCTION__ << " : destination unreachable" << std::endl;

  dot_file << "}" << std::endl;  
  dot_file.close();
  
}

std::string StaticMultihopRouting::printIpAddress(nsaddr_t addr) {
  std::stringstream s_out;
  s_out << ((addr & 0xff000000)>>24) << "." << ((addr & 0x00ff0000)>>16) << "." << ((addr & 0x0000ff00)>>8) 
        << "." << (addr & 0x000000ff);
  return(s_out.str());   
}


void StaticMultihopRouting::graphviz_output(const char* fname)
{   
  //std::cerr  << __PRETTY_FUNCTION__ << " file: " << fname << endl;

  ofstream dot_file(fname, ios_base::trunc);

  //write_graphviz(dot_file, graph);
  //write_graphviz(dot_file, graph, node_writer(npv), hop_writer(get(edge_weight, graph)) );


  double graphviz_size = 20.0;

  dot_file << "graph G {\n"
	   << "size=\"" << graphviz_size << "," << graphviz_size << "\"\n"
	   << " node [shape=\"circle\"]\n";
  double scale_factor = graphviz_size / xmax;

  SMR_VertIter vi, vi_end;
  for (tie(vi, vi_end) = vertices(graph); vi != vi_end; ++vi) 
    {
      SMR_VertDescr v;
      v = *vi;

      Position* pos = ((npv.find(v))->second).pos;      
      dot_file << v 
	       << " [pos=\"" << pos->getX() * scale_factor << "," 
	       << pos->getY() * scale_factor << "!\""
	//	       << ", label=\"" << pos->getZ()  << "\""
	       << "]\n";
    }


 //  SMR_EdgeIter ei, ei_end;

//   for (tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei) 
//     {
//       SMR_EdgeDescr e = *ei;
//       SMR_VertDescr u,v;

//       u = source(e, graph);
//       v = target(e, graph);

//       EdgeWeightPropMap ewmap = get(edge_weight, graph);
      
//       dot_file << u << " -- " << v 
// 		<< " [label=\"" 
// 		<< get(ewmap, e) << "\""
// 		<< "]\n";
//     }


  dot_file << "}" << std::endl;  
  dot_file.close();
  
}

void StaticMultihopRouting::reset() {
  graph.clear();
  a2im.clear();
  i2am.clear();
  npm.clear();
  npv.clear();
}



class hop_writer {
public: 

  hop_writer(EdgeWeightPropMap m) { ewpm=m; }

  void operator()(std::ostream& out, SMR_EdgeDescr e) const { 

    double cost = get(ewpm, e);
    out << " [label=\"" << cost << "\""
	<< "]";
  }
  
protected:
  EdgeWeightPropMap ewpm;
 
};



class node_writer {
public: 

  node_writer(SMR_NodePropVect  v) {npv = v;}

  void operator()(std::ostream& out, const int& v) const { 
    Position* pos = ((npv.find(v))->second).pos;
    out << " [pos=\"" << pos->getX() 
	<< "," 
	<< pos->getY() << "!\","
    << " label=\"" << pos->getZ() << "\""
	<< "]";
  }
  
protected:
  SMR_NodePropVect npv;
};

