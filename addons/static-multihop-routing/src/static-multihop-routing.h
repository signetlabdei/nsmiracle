/* -*- Mode:c++ -*- */

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


#ifndef STATIC_MULTIHOP_ROUTING_H
#define STATIC_MULTIHOP_ROUTING_H

#include<map>
#include<string>
#include<vector>
#include<tclcl.h>
#include<math.h>
#include<node-core.h>
#include<ip-routing.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/properties.hpp>

using namespace boost;

class SMR_Node {
public:
  SMR_Node();
  SMR_Node(Position* p, IPRoutingModule* r, nsaddr_t a);

  nsaddr_t addr;
  Position* pos;
  IPRoutingModule* ipr;
};


typedef adjacency_list < listS, vecS, undirectedS,
		       no_property, property < edge_weight_t, double >
		       > SMR_Graph;

typedef pair<nsaddr_t,nsaddr_t> SMR_Edge;
typedef graph_traits < SMR_Graph >::vertex_descriptor SMR_VertDescr;
typedef graph_traits < SMR_Graph >::edge_descriptor SMR_EdgeDescr;
typedef graph_traits < SMR_Graph >::in_edge_iterator SMR_InEdgeIter;
typedef graph_traits < SMR_Graph >::out_edge_iterator SMR_OutEdgeIter;
typedef graph_traits < SMR_Graph >::edge_iterator SMR_EdgeIter;
typedef graph_traits < SMR_Graph >::vertex_iterator SMR_VertIter;


typedef property_map<SMR_Graph, edge_weight_t>::type EdgeWeightPropMap;

typedef std::map<nsaddr_t, SMR_Node> SMR_NodePropMap;
typedef std::map<int, SMR_Node>   SMR_NodePropVect;



class StaticMultihopRouting : public TclObject {

 public:

  StaticMultihopRouting();

  /** 
   * Cost metric for communication between two nodes
   * 
   * @param m first node
   * @param n second node
   * 
   * @return the cost of communication, if communication is possible,
   * otherwise INFINITY (test for it using isinf() or isfinite(),
   * which are defined in math.h)
   */
  virtual double getCost(SMR_Node m, SMR_Node n);

  /** 
   * 
   * 
   * @param src 
   * @param dst 
   */
  virtual void setPath(int src, int dst, const char* fname = NULL);


  void  setRoutingTablesForPath(int src, int dst, std::vector<SMR_VertDescr>& p);

  void graphviz_output(const char* fname);

  void graphviz_output_path(const char* fname, int src, int dst, std::vector<SMR_VertDescr>& p);

  /** 
   * 
   * 
   * @param argc 
   * @param argv 
   * 
   * @return 
   */
  virtual int command(int argc, const char*const* argv);
  

  /** 
   * converts address to index
   * 
   * @param a address
   * 
   * @return corresponding index
   */
  int a2i(int a) { return a2im.find(a)->second;}


  /** 
   * converts index to address 
   * 
   * @param i index
   * 
   * @return corresponding address
   */
  int i2a(int i) { return i2am.find(i)->second;}



 protected:
   
  SMR_Graph graph;
  SMR_NodePropMap npm;    // stores node properties vs addr
  SMR_NodePropVect npv;   // stores node properties vs index
  int debug_;
  std::map<nsaddr_t, int>   a2im; // converts vertex index to address
  std::map<nsaddr_t, int>   i2am; // converts address to vertex index 
  
  double xmax; // max size of network, used to scale graphviz output
  double MaxHopLength_; // hard limit for the hop length

  /** 
   * Adds node to graph and calculates all costs
   * 
   * @param m
   */
  virtual void addPos(SMR_Node m);
  virtual void reset();
  
  std::string printIpAddress(nsaddr_t addr);

};





#endif  //STATIC_MULTIHOP_ROUTING_H
