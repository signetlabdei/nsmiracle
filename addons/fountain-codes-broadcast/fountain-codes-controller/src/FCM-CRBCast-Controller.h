
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
  virtual double getMeanNodes_Covered();
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
  set<int> fcm_phase_I_senders;
  list<int>* fcm_lcomp;
//  set<double> complete_dist; // distanza e nodi completi
//  set<double> incomplete_dist;

  map<int, int> complete_phase_II_freq;
  map<int, double> freq_phase_I_rounds;
  map<int, double> freq_phase_II_rounds;
  map<int, double> complete_phase_II;
  map<int, double> complete_phase_I;
  map<int, double> blk_max_round;

  SMap child_map;
  int fcm_source_id; // source fcm id;
  int debug_;
  
  int phase_I_hops;
  int sum_total_hops;
  //double mean_total_hops;

  double mean_Nodes_covered;

  double mean_N_tx_phase_I;
  double mean_P_fail_phase_I;
  double mean_N_rounds_phase_I;  

  double mean_N_tx_phase_II;
  double mean_P_fail_phase_II;
  double mean_N_rounds_phase_II;  

  double total_P_fail;

  int global_rounds;
  int total_rounds;
  int phase_I_total_rounds;
  int phase_II_total_rounds;
  int current_block;
  int temp_block;
  int failed_tries;
  int phase_I_wasted_tx;
  int phase_II_wasted_adv;

  int curr_phase;
  int TCL_phase_I;
  int TCL_phase_II;

  int phase_I_rounds;
  int phase_II_rounds;
  int phase_II_sessions;

  int sum_phase_I_rounds;
  int sum_phase_II_rounds;
  int sum_phase_II_sessions;

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
  virtual void computeChildNodes(const int pos_id, set<int>* incomp_nodes);
  virtual void redoBorderNodes(const set<int>* nodes);
  virtual void pruneChildMap(set<int> parent_nodes);
  virtual void getStats(int source_id);
  virtual void pbCastSelection(set<int> senders); // è giusto cosi senza pointer
  virtual void startBorderNodes();
  virtual void getCompleteProb(int phase);
  virtual void printStats();
  virtual void waitForUser();
  virtual void initializeTopology(int phase);
  virtual void refreshCounters();
  virtual bool isConnected();
  virtual void setCRBphase(int phase);
  virtual void newRound();
   
};

#endif /* FOUNTAIN_CRBCAST_CONTROLLER_H */

