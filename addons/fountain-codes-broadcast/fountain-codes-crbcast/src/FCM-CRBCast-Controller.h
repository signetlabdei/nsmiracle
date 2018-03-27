
#ifndef FOUNTAIN_CRBCAST_CONTROLLER_H
#define FOUNTAIN_CRBCAST_CONTROLLER_H


#include "fountain-module.h"
#include "mtrand.h"


//***************************************
// 
//
//***************************************

class FCMCRBController : public TclObject
{
  friend class FCModule;

public:

  FCMCRBController();
  virtual ~FCMCRBController();

  virtual int command(int argc, const char*const* argv);
  virtual void startPhaseOne();
  virtual void stop(int status);

  virtual double getMeanN_tx();
  virtual double getMeanP_fail();
  virtual double getMeanN_rounds();  
  virtual double getMeanResolved_Radius();
  virtual double getMeanTierResolved_Radius();
  virtual double getSourceN_tx();
  virtual double getSourceP_fail();
  virtual double getSourceN_rounds();  
  virtual double getSourceResolved_Radius();
  virtual double getMeanNodes_Covered();
  virtual double getSourceNodes_Covered();
  virtual void reset();


protected:

  /* variabili TCL*/
  double TxRadius_; // raggio cerchio di copertura;
  int TotalBlocks_;
  uint16_t genType_;
  double PbCast_;
  double Rmax_;
  /************************/

  vector<double> src_dist_vector;
  vector<FCModule*>* fcm_pointers;

  set<int>* fcm_complete; // punta ai nodi completi di FCModule
  set<int>* fcm_incomplete; // punta ai nodi incompleti di FCModule
  set<int>  border_nodes;   // nodi che devono tx
  set<int>  next_border_nodes; // nodi che dovranno tx al passo successivo
  set<int>  tier_nodes; // nodi del tier
  set<int> fcm_phase_I_senders;
  list<int>* fcm_lcomp;
//  set<double> complete_dist; // distanza e nodi completi
//  set<double> incomplete_dist;

  map<int,int> fcm_parents;
  //map<int,set<int> > resolved_area_peers; // nodi che hanno risolto
  map<int, double> tier_resolved_radius;
  map<int, double> tier_resolved_projection;
  map<int, double> net_resolved_radius;
  map<int, int> blocks_hop;

  SMap child_map;
  int fcm_source_id; // source fcm id;
  int debug_;
  
  double o_x;
  double o_y;
  double o_z;

  double resolved_radius;

  double resolved_projection;
  double mean_resolved_projection;
  double mean_tier_resolved_projection;
  double source_resolved_projection;

  double mean_Resolved_Radius;
  double mean_tier_Resolved_Radius;

  int total_hops;
  int sum_total_hops;
  double mean_total_hops;

  double mean_N_tx;
  double mean_P_fail;
  double mean_N_rounds;  
  double mean_Nodes_covered;

  double source_N_tx;
  double source_P_fail;
  double source_N_rounds;  
  double source_Resolved_Radius;
  double source_Nodes_covered;
  double total_P_fail;

  int global_rounds;
  int total_rounds;
  int current_block;
  int temp_block;
  int failed_tries;
  int phase_I_wasted_tx;
  int phase_II_wasted_adv;

  int curr_phase;

  time_t start_time;
  time_t stop_time;
  
  bool test;
  bool topology_connected;
 
  //Random* rand;
  MTRand_closed mtrand;
  MTRand_int32 mtrand_int;
  int RandSeed_;
  
  virtual void startPhaseTwo();
  virtual void setDone(const int id);
  virtual void computeDistance(int start_id, vector<double>* dist_vector);
  virtual void computeProjection(int start_id, vector<double>* proj_vector, vector<double>* dist_vector);
  virtual void computeResolveArea(int source_id , int command = 0);
  //virtual void setBorderNodes();
  virtual void computeChildNodes(const int pos_id, set<int>* incomp_nodes);
  virtual void pruneChildMap(set<int> parent_nodes);
  virtual void getStats(int source_id);
  virtual void pbCastSelection(set<int> senders); // è giusto cosi senza pointer
  virtual void startBorderNodes();
  virtual void printStats(int phase);
  virtual void waitForUser();
  virtual void initializeTopology(int phase);
  virtual void refreshCounters();
  virtual bool isConnected();
   
};

#endif /* FOUNTAIN_CRBCAST_CONTROLLER_H */

