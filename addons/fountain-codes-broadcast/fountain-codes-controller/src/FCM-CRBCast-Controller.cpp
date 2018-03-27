#include "FCM-CRBCast-Controller.h"
#include <cmath>
#include <iomanip>

enum 
{
    NOT_SET = 1111, TIER = 5, SINGLE = 22, NET = 11, DISCONNECTED = 100, CONNECTED = 40, NEXT_SIMUL = 200,
    PHASE_ONE = 300, PHASE_TWO = 1400,
    CRB_PHASE_I = 30, CRB_PHASE_II = 31 , CRB_PHASE_I_SGT = 32 , CRB_PHASE_II_SGT = 33, CRB_INACTIVE = 88,
    INCOMPLETE = 101, COMPLETE = 102 ,
};

static class FCMCRBControllerClass : public TclClass {
public:
  FCMCRBControllerClass() : TclClass("Module/FCM/SimpleMAC/CRBCastCtrl") {}
  TclObject* create(int, const char*const*) {
    return ( new FCMCRBController() );
  }
} FCM_CRBCast_Control_Class;

int FCMCRBController::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc==3)
  {
      if(strcasecmp(argv[1], "setSourceNode") == 0)
        {
           FCModule* fcm_source =(FCModule*)TclObject::lookup(argv[2]); //get FCM Id of module
           Position* fcm_source_pos = fcm_source->getPosition();
	   fcm_pointers = &(fcm_source->fcm_pointers); // mi copio i puntatori
           fcm_complete = &(fcm_source->complete_peers);
  	   fcm_incomplete = &(fcm_source->incomplete_peers);
           fcm_lcomp = &(fcm_source->fcm_lcomp);
           fcm_source_id = fcm_source->GetFCMId();
           fcm_lcomp->push_back(fcm_source_id);
           initializeTopology(TCL_phase_I);
           computeDistance(fcm_source_id, &src_dist_vector);
           return TCL_OK;
        }
      else if(strcasecmp(argv[1], "setOptFile") == 0)
        {
          return TCL_OK;
        }  
  }
  else if(argc==2)
    {
      if(strcasecmp(argv[1], "startPhaseOne") == 0)	// TCL command to start the packet generation and transmission
	{
          test = true;
          curr_phase = TCL_phase_I;
          start_time = time(NULL);
          startPhaseOne();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "testTopology") == 0)	
	{
          test = true;
          start_time = time(NULL);
          //startMultiHop();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setPhaseOneCRB") == 0)	
	{
          TCL_phase_I = CRB_PHASE_I;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setPhaseOneSIGNET") == 0)	
	{
          TCL_phase_I = CRB_PHASE_I_SGT;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setPhaseTwoCRB") == 0)	
	{
          TCL_phase_II = CRB_PHASE_II;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "setPhaseTwoSIGNET") == 0)	
	{
          TCL_phase_II = CRB_PHASE_II_SGT;
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanN_tx") == 0)	
	{
          tcl.resultf("%f",getMeanN_tx());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanP_fail") == 0)
	{
          tcl.resultf("%f",getMeanP_fail());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanN_rounds") == 0 )
	{
          tcl.resultf("%f",getMeanN_rounds());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanNodes_Covered") == 0)	
	{
          tcl.resultf("%f",getMeanNodes_Covered());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "isConnected") == 0)	
	{
          if ( isConnected() == true ) tcl.resultf("true");
          else tcl.resultf("false");
	  return TCL_OK;
	}
     else if(strcasecmp(argv[1], "reset") == 0)	
	{
          reset();
	  return TCL_OK;
	}
    }  
  return TclObject::command(argc, argv);
}

FCMCRBController::FCMCRBController() :
  fcm_source_id(NOT_SET),
  phase_I_hops(0),
  sum_total_hops(0),
  //mean_total_hops(0),
  debug_(0),
  test(false),
  topology_connected(false),
  global_rounds(0),
  current_block(1),
  total_rounds(0),
  failed_tries(0),
  phase_I_wasted_tx(0),
  phase_II_wasted_adv(0),
  mean_N_tx_phase_I(0.0),
  mean_P_fail_phase_I(0.0),
  mean_N_rounds_phase_I(0.0), 
  mean_N_tx_phase_II(0.0),
  mean_P_fail_phase_II(0.0),
  mean_N_rounds_phase_II(0.0), 
  mean_Nodes_covered(0.0),
  total_P_fail(0.0),
  curr_phase(CRB_PHASE_I),
  phase_I_rounds(0),
  phase_II_rounds(0),
  sum_phase_I_rounds(0),
  sum_phase_II_rounds(0),
  phase_I_total_rounds(0),
  phase_II_total_rounds(0),
  sum_phase_II_sessions(0),
  phase_II_sessions(0),
  TCL_phase_I(CRB_PHASE_I),
  TCL_phase_II(CRB_PHASE_II)
{
  bind("TxRadius_", (double*)&TxRadius_);
  bind("TotalBlocks_",(int*)&TotalBlocks_);
  bind("debug_", (int*)&debug_);
  bind("genType_", (int*)&genType_); // generator type
  bind("PbCast_", (double*)&PbCast_);
  bind("RandSeed_", (int*)&RandSeed_);
  bind("Rmax_", (double*)&Rmax_);
  border_nodes.clear();
  child_map.clear();
  next_border_nodes.clear();
  blk_max_round.clear();
  fcm_phase_I_senders.clear();
  mtrand.seed(RandSeed_);
  mtrand_int.seed(RandSeed_);
  freq_phase_I_rounds.clear();
  freq_phase_II_rounds.clear();
  complete_phase_II.clear();
  complete_phase_I.clear();
  complete_phase_II_freq.clear();

}

FCMCRBController::~FCMCRBController()
{
  //delete rand;
}

void FCMCRBController::computeDistance(int start_id, vector<double>* dist_vector)
{
  Position* source_pos = (fcm_pointers->at(start_id))->getPosition();

  for (int i = 0; i < fcm_pointers->size(); i++) 
  {
     Position* p_receiver = (fcm_pointers->at(i))->getPosition();
     double dist = source_pos->getDist(p_receiver);

     /*if (debug_ > 0 ) {
        cout << "FCMCRBController::computeDistance() user id " << i << " has distance " 
             << dist << " from node id " << start_id << endl;
     }    */                  

     dist_vector->push_back(dist);
  }
  // //if ( debug_ > 0 ) waitForUser();
}

void FCMCRBController::getCompleteProb(int phase)
{
   if ( (phase == TCL_phase_I) ) {
      double compl_nodes = fcm_complete->size();
      complete_phase_I[compl_nodes]++;
      
   }
   else if ( (phase == TCL_phase_II) ) {
      double compl_nodes = fcm_complete->size();
      complete_phase_II[phase_II_rounds] += compl_nodes;
      complete_phase_II_freq[phase_II_rounds]++;
   }
}

void FCMCRBController::computeChildNodes(const int start_pos_id, set<int>* incomp_nodes)
{
  set<int> test_nodes;
  // se nodo nn è completo lo tolgo dai nodi sul bordo  
  if ( ( (fcm_complete->count(start_pos_id)) == 0 ) && ( curr_phase != CRB_PHASE_I ) ) { // giusto così
     if (debug_ > 0) {
        cout << "FCMCRBController::computeChildNodes() parent node " << start_pos_id 
             << " is not in complete set, removing from border nodes" << endl;

     }
     border_nodes.erase(start_pos_id);
     return;   
  }

  if ( start_pos_id <= fcm_pointers->size() ) { // se l'id dato è corretto

     child_map[start_pos_id].clear();

     Position* p_source = (fcm_pointers->at(start_pos_id))->getPosition();

     /*if (debug_ > 0 ) {
        cout << "FCMCRBController::computeChildNodes() parent node " << start_pos_id 
             << " has distance " << dist_start << endl;
     }*/

     //calcolo le distanze relative e aggiorno mappa
     for ( Setiter it = incomp_nodes->begin(); it != incomp_nodes->end(); it++ ) {

         int index = *it;

         if (index == start_pos_id) continue;

         Position* p_index = (fcm_pointers->at(index))->getPosition();
         double dist = p_source->getDist(p_index);

         if (debug_ > 0 ) {
             cout << "FCMCRBController::computeChildNodes() node id "<< index <<" has distance " 
                  << dist << " from parent node " << start_pos_id << endl;
         }

         if ( (dist <= TxRadius_) && (dist > 0) ) {
            child_map[start_pos_id].insert(index); // inserisco nella mappa il figlio
            next_border_nodes.insert(index);
            if ( test == true) {
               fcm_complete->insert(index);
               test_nodes.insert(index);
            }

            if (debug_ > 0 ) {
               cout << "FCMCRBController::computeChildNodes() node id " << index 
                    << " has been inserted into child_map[" << start_pos_id <<"] and into next_border_nodes " 
                    << endl;
            }
         }
     }  

     if (test == true) {
        for ( Setiter it = test_nodes.begin(); it != test_nodes.end(); it++ ) {
            int index = *it;
            incomp_nodes->erase(index);
        }
     }
   
  }
  else { 
     cout << "FCMCRBController::computeChildNodes() ERROR start_pos_id out of bound!" << endl ;
     exit(1);
  }
}

void FCMCRBController::pruneChildMap(set<int> parent_nodes)
{
   int max_size = 0;
   int max_index = 0;
   bool is_disconnected = true;
   //double dist_min_curr = NOT_SET;

   // finchè nn svuoto 
   while ( !parent_nodes.empty() ) {
   // trovo il nodo che ha max num di figli
   for ( Setiter it = parent_nodes.begin() ; it != parent_nodes.end() ; it++ ) {
       int index = *it;
       int size = child_map[index].size();
       if (size == 0) {
           parent_nodes.erase(index);
           border_nodes.erase(index);
           if (debug_ > 0) {
              cout << "FCMCRBController::pruneChildMap() childmap[" << index 
                   << "] has 0 size. node " << index <<" has been deleted from border_nodes" << endl;
           } 
           continue;
       }
       double dist_index = src_dist_vector[index];
       //Position* p_index = (fcm_pointers->at(index))->getPosition();
       //double dist = p_incompl_min->getDist(p_index);

       if (debug_ > 0 ) {
          cout << "FCMCRBController::pruneChildMap() node " << index
               << " has size " << size << " distance from source " << dist_index << endl;
       }

       if ( (size > max_size)  ) {
          max_size = size;
          max_index = index;
          is_disconnected = false;
       }
   }
   
   if (is_disconnected == true) { // la mappa è disconnessa
      cout << "FCMCRBController::pruneChildMap() all sub-trees are empty." 
           << " The node map is not connected! " << endl;
      topology_connected = false;
      border_nodes.clear();
      return;
   }
   else topology_connected = true;
  
   if (debug_ > 0) {
       cout << "FCMCRBController::pruneChildMap() parent node " << max_index 
            << " has been chosen: max size " << max_size << endl;
   }

   /*if (debug_ > 0) {
      cout << "FCMCRBController::pruneChildMap() parent node " << max_index 
           << " has max no of child " << max_size << endl;
      waitForUser();
   }*/

   parent_nodes.erase(max_index); // tolgo dalla lista il nodo trovato
   
   // tolgo dagli altri set gli eventuali nodi in comune
   for ( Setiter it = child_map[max_index].begin() ; it != child_map[max_index].end() ; it++ ) {
         int value = *it;

         if (debug_ > 0) {
            cout << "FCMCRBController::pruneChildMap() parent node " << max_index 
                 << " has child node " << value << endl;
         }

         for ( Setiter it2 = parent_nodes.begin(); it2 != parent_nodes.end(); it2++ ) {
             int index = *it2;
             child_map[index].erase(value);

             if (debug_ > 0) {
                 cout << "FCMCRBController::pruneChildMap() deleting from parent node " << index 
                      << "'s tree the child node " << value << endl;
             }
         
             if (child_map[index].size() == 0) {
                 border_nodes.erase(index);
                 parent_nodes.erase(index);
             
                 if (debug_ > 0) {
                    cout << "FCMCRBController::pruneChildMap() childmap[" << index 
                         << "] has 0 size. node " << index <<" has been deleted from border_nodes" << endl;
                 }         
             }
         }
   }
 max_size = 0;
 max_index = 0;

 }
   if (debug_ > 0) waitForUser();
}

inline void FCMCRBController::newRound()
{
  
  if (curr_phase == TCL_phase_II) {
     global_rounds++;
     phase_II_rounds++;
     //blk_max_round[current_block]=phase_II_rounds;
     //freq_phase_II_rounds[phase_II_rounds]++;
     getCompleteProb(TCL_phase_II);
  }
}


inline void FCMCRBController::setDone(const int id) 
{
  global_rounds++; 
  
  border_nodes.erase(id);

  if (id == fcm_source_id) fcm_lcomp->push_back(id);

  if (curr_phase == TCL_phase_I){
      fcm_phase_I_senders.insert(id);
      //phase_I_total_rounds++;
  }
  if (curr_phase == TCL_phase_II) {
      fcm_lcomp->push_back(id);
      phase_II_sessions++;
      //phase_II_total_rounds++;
  }

  getStats(id);
  if ((curr_phase == TCL_phase_I)) phase_I_rounds++;
  else phase_II_rounds++;
  //computeResolveArea(id, SINGLE);
  

  if ( border_nodes.size() != 0 ) {
     startBorderNodes();
  }
  else {

     if (debug_ > 0) {
        cout << "********************************************************************" 
             << endl;
     }
     
     if (curr_phase == TCL_phase_I) border_nodes = next_border_nodes;
     next_border_nodes.clear();
     if (curr_phase == TCL_phase_I) startPhaseOne(); 
     else if (curr_phase == TCL_phase_II) startPhaseTwo();
  }
}

void FCMCRBController::startBorderNodes() 
{
  if ( !border_nodes.empty() ) {
      Setiter it = border_nodes.begin();
      int index = *it;
      set<int> receivers = child_map[index];

      if (debug_ > 0 ) {
         cout << "FCMCRBController::startBorderNodes() " 
              << "node " << index << " set int receivers size = " << receivers.size() 
              << " child_map[index] size = " << child_map[index].size() << endl ;
         //waitForUser();
      }

      //child_map[index].clear();

      if (test == false) {
          //if (fcm_phase_I_senders
          (fcm_pointers->at(index))->setInitialReceivers(&receivers);
      }
      else {
         fcm_complete->insert(index);
         fcm_incomplete->erase(index);
         setDone(index);        
      }
  }
}

void FCMCRBController::startPhaseOne()
{
  if ( fcm_source_id == NOT_SET ) {
     cout << "FCMCRBController::startPhaseOne() error, set source in TCL first" << endl; 
     exit(1);
  }

  if ( fcm_phase_I_senders.size() == fcm_pointers->size() ) {
      stop(CONNECTED);
      return;
  } 

  if (test == true) TotalBlocks_ = 1;

  for ( Setiter it = border_nodes.begin(); it != border_nodes.end(); it++) {
     computeChildNodes( (*it) , fcm_incomplete );
  }

  // PBcast
  pbCastSelection(border_nodes);
  redoBorderNodes(&border_nodes);
  

  if (border_nodes.size() != 0) {
      phase_I_hops++;
      startBorderNodes();
  }
  else {
     if (topology_connected == true) stop(CONNECTED);
     else stop(DISCONNECTED);
  }
}

void FCMCRBController::startPhaseTwo()
{
  
  if (debug_ > 0 ) {
      cout << "FCMCRBController::startPhaseTwo() starting phase two" << endl;
      waitForUser();
  }  

  if (fcm_incomplete->size() == 0) {
      getCompleteProb(curr_phase);
      stop(CONNECTED);
      return;
  } 
  else {
   
     getCompleteProb(curr_phase);   

     border_nodes.clear();
     child_map.clear();

     int index = fcm_lcomp->front();
     computeChildNodes( index , fcm_incomplete );
     fcm_lcomp->pop_front(); 
 
     while ( child_map[index].empty() && (!(fcm_lcomp->empty())) ) {
         if (debug_ > 0) cout << "FCMCRBController::startPhaseTwo() node " << index << " has zero size child map"
                              << ", trying another complete node..." << endl;
         phase_II_wasted_adv++;
         fcm_lcomp->push_back(index);
         index = fcm_lcomp->front();
         computeChildNodes( index , fcm_incomplete );
         fcm_lcomp->pop_front();
     }
  
     if (debug_ > 0) cout << "FCMCRBController::startPhaseTwo() curr wasted adv " << phase_II_wasted_adv << endl
                          << "FCMCRBController::startPhaseTwo() node " << index << " has nonzero size " 
                          << child_map[index].size() << endl;
     border_nodes.insert(index);

     if (border_nodes.size() != 0) {
         //phase_I_hops++;
         startBorderNodes();
     }
     else {
        if (topology_connected == true) {
           stop(CONNECTED);
           return;
        }
        else {
             stop(DISCONNECTED);
             return;
        }
     }
  }
}

void FCMCRBController::pbCastSelection(set<int> senders) 
{
   int max_size = 0;

   for (Setiter it = senders.begin(); it != senders.end(); it++) {
       int index = *it;
       int size = child_map[index].size();
       if (size > max_size) max_size = size;
       if (size == 0) {
          if (debug_ > 0) cout << "FCMCRBController::pbCastSelection() node " << index
                               << " has no child nodes. It has been erased from border nodes" << endl;
          border_nodes.erase(index);
          if (curr_phase == TCL_phase_I) {
             fcm_phase_I_senders.insert(index);
             phase_I_wasted_tx++;
          }
          continue;
       }
       if ( (test == false) && (curr_phase == TCL_phase_I) ) {
          double p = mtrand();  
          /*if (debug_ > 0) {
              cout << "FCMCRBController::pbCastSelection() prob = " << p << ", PbCast_ = " 
                   << PbCast_ << " curr_phase " << curr_phase << endl; 
              //waitForUser();
          }*/
          if ( (p > PbCast_) && (index != fcm_source_id) ) {
              border_nodes.erase(index);
              fcm_phase_I_senders.insert(index);
              for (Setiter it = child_map[index].begin(); it != child_map[index].end(); it++ ) {
                   int next_index = (*it);
                   next_border_nodes.erase(next_index);
              }
              if (debug_ > 0) {
                  cout << "FCMCRBController::pbCastSelection() prob " << p << " is > than PbCast_ " 
                       << PbCast_ << " index " << index << " has been erased from border nodes" << endl;
                  //waitForUser();
             }
          }
          else if ( (p <= PbCast_) && (index != fcm_source_id) && (curr_phase == TCL_phase_I) && ((fcm_phase_I_senders.count(index)) != 0) ) {
              border_nodes.erase(index);
              for (Setiter it = child_map[index].begin(); it != child_map[index].end(); it++ ) {
                   int next_index = (*it);
                   next_border_nodes.erase(next_index);
              }
              if (debug_ > 0) {
                  cout << "FCMCRBController::pbCastSelection() prob " << p << " is <= than PbCast_ " 
                       << PbCast_ << " but index " << index << " has already forwarded." << endl;
                  //waitForUser();
             }
          }
          else if ( (debug_ > 0)  ) {
                  cout << "FCMCRBController::pbCastSelection() prob " << p << " is <= than PbCast_ " 
                       << PbCast_ << " index " << index << " will forward. " << endl;
                  //waitForUser();
             }
       }
   }
   if ( (max_size == 0) && (fcm_incomplete->size() != 0 ) ) topology_connected = false;
   else topology_connected = true;
   if (debug_ > 0) waitForUser();
}

void FCMCRBController::redoBorderNodes(const set<int>* nodes)
{
    for (Setiter it = nodes->begin(); it != nodes->end(); it++ ) {
        int index = (*it);

        if (debug_ >0) cout << "FCMCRBController::redoBorderNodes() node " << setw(3) << index 
                            << setw(13) << " has childs: ";

        for (Setiter b = child_map[index].begin(); b != child_map[index].end(); b++ ) {
             int next_index = (*b);

             if (debug_ >0) cout << setw(3) << next_index;

             next_border_nodes.insert(next_index);
        }
 
        if (debug_ >0) cout << endl;

   }
   if (debug_ >0) waitForUser();
}

void FCMCRBController::refreshCounters()
{

  total_rounds += global_rounds;
  
  sum_total_hops += phase_I_hops;

  if ( (curr_phase == TCL_phase_I) ) {
      sum_phase_I_rounds += phase_I_rounds;
      freq_phase_I_rounds[phase_I_rounds]++;
  }
  else {
     sum_phase_II_rounds += phase_II_rounds;
     sum_phase_II_sessions += phase_II_sessions;
     blk_max_round[current_block]=phase_II_rounds;
     freq_phase_II_rounds[phase_II_rounds]++;
  }

  global_rounds = 0;
  phase_I_hops = 0;
  phase_I_rounds = 0;
  phase_II_rounds = 0;
  phase_II_sessions = 0;

  if (debug_ > 0) {
     cout << "FCMCRBController::refreshCounters() "   
          << "total rounds = " << total_rounds << endl
          << "FCMCRBController::refreshCounters() " 
          << "phase_I hops = " << phase_I_hops << endl
          << "FCMCRBController::refreshCounters() " 
          << "sum total hops = " << sum_total_hops << endl;
  }
}

void FCMCRBController::stop(int status)
{
  refreshCounters();

  switch (status) {

     case(CONNECTED): {
        if (test == true) {
           cout << "FCMCRBController::stop(CONNECTED) Test passed, the network is connected."
                << " Starting simulation ..."<< endl;
           //waitForUser();
           test = false;
           reset();
           initializeTopology(TCL_phase_I);
           bind("TotalBlocks_",(int*)&TotalBlocks_); // PAZZESCOOOOOOOOO
           startPhaseOne();
        }
        else if (curr_phase == TCL_phase_I) {
            if (debug_ > 0) cout << "FCMCRBController::stop(CONNECTED) Phase One Completed, "
                                 << "starting Phase Two..." << endl;
            getCompleteProb(TCL_phase_I);
            curr_phase = TCL_phase_II;
            setCRBphase(TCL_phase_II);
            // devo resettare current_block e quello che mi serve per la fase 2
            startPhaseTwo();
        }
        else if ( ( curr_phase == TCL_phase_II ) && (current_block < TotalBlocks_ ) ) {
            if (debug_ > 0) cout << "FCMCRBController::stop(CONNECTED) Phase Two Completed, "
                                 << "starting a new block try" << endl;
            //topology_connected = true;
            curr_phase = TCL_phase_I;
            border_nodes.clear();
            next_border_nodes.clear();
            //fcm_lcomp->clear();
            //global_rounds = 0;
            //total_hops = 0;
            initializeTopology(TCL_phase_I);
            current_block++;
            startPhaseOne(); //devo riconciare nuovo blocco con fase 1
            
        }
        else {
           if (debug_ > 0) cout << "FCMCRBController::stop(CONNECTED) Phase Two Completed, "
                                << "all tries completed, printing stats..." << endl;
           printStats();
               // qui devo schedulare nuova simulazione
         }
     }
     break;

     case(DISCONNECTED): {
         if (test == true ) {
             //topology_connected = false;
             cout << "FCMCRBController::stop(DISCONNECTED) simulation aborted " << endl;
         }
         else {
            if ( curr_phase == TCL_phase_I ) {
               if (debug_ > 0) cout << "FCMCRBController::stop(DISCONNECTED) Phase One Completed, "
                                    << "starting Phase Two" << endl;
               getCompleteProb(TCL_phase_I);
               curr_phase = TCL_phase_II;
               setCRBphase(TCL_phase_II);
               startPhaseTwo();
            }
            else {
                if (debug_ > 0) cout << "FCMCRBController::stop(DISCONNECTED) Phase Two Completed, "
                                     << "starting a new block try" << endl;
            } 
        }
     }
     break;

     case(NEXT_SIMUL): {
         exit(1);
         //cout << "caso DISCONNECTED" << endl;
         // schedulo la prox topologia
     }
     break;
  }
}

void FCMCRBController::initializeTopology(int phase)
{
   fcm_complete->clear();
   fcm_complete->insert(fcm_source_id); // la sorgente è già completa
   fcm_incomplete->clear();
   fcm_phase_I_senders.clear();
   fcm_phase_I_senders.insert(fcm_source_id);
   child_map.clear();

   for (int i = 0; i < fcm_pointers->size(); i++) {
       fcm_incomplete->insert(i); // i nodi rimanenti sn incompleti in partenza
       (fcm_pointers->at(i))->setToDecoder();
       (fcm_pointers->at(i))->finished = true; // ERA FALSE!!
       (fcm_pointers->at(i))->mhop = true; 
       (fcm_pointers->at(i))->abortCurrBlock();
       (fcm_pointers->at(i))->resetStats();
   }
   if (phase == TCL_phase_I) {
       setCRBphase(TCL_phase_I);
       fcm_lcomp->clear();
       //fcm_lcomp->push_back(fcm_source_id);
   }
   else if (phase == TCL_phase_II) setCRBphase(TCL_phase_II);
   fcm_incomplete->erase(fcm_source_id);
   border_nodes.insert(fcm_source_id);
}

void FCMCRBController::setCRBphase(int phase)
{

  for (int i = 0; i < fcm_pointers->size(); i++) {
       if (phase == TCL_phase_I) {
           (fcm_pointers->at(i))->crb_phase = TCL_phase_I;
            (fcm_pointers->at(i))->crb_phase_I_ack_done = false;
       }
       else if ( phase == TCL_phase_II ) (fcm_pointers->at(i))->crb_phase = TCL_phase_II;
       if ((phase != CRB_PHASE_II_SGT) && (phase != CRB_PHASE_II)) (fcm_pointers->at(i))->crb_phase_I_index.clear();
       (fcm_pointers->at(i))->crb_phase_I_seed = NOT_SET;
  }

}

void FCMCRBController::getStats(int source_id)
{
  double N_tx = (fcm_pointers->at(source_id))->N_tx;
  double P_fail = (fcm_pointers->at(source_id))->P_fail;
  double N_rounds = (fcm_pointers->at(source_id))->N_rounds;
  if ( (curr_phase == TCL_phase_I) ) {
     mean_N_tx_phase_I += N_tx;
     mean_P_fail_phase_I += P_fail;
     mean_N_rounds_phase_I += N_rounds;  
  }
  else {
     mean_N_tx_phase_II += N_tx;
     mean_P_fail_phase_II += P_fail;
     mean_N_rounds_phase_II += N_rounds;  
  }
}

void FCMCRBController::printStats()
{

  mean_N_tx_phase_I /= sum_phase_I_rounds;
  mean_P_fail_phase_I /= sum_phase_I_rounds;
  mean_N_rounds_phase_I /= sum_phase_I_rounds;
 
  mean_Nodes_covered /= (fcm_pointers->size() * total_rounds );

  if ( (sum_phase_II_rounds != 0) && (sum_phase_II_sessions != 0)) {
     mean_N_tx_phase_II /= sum_phase_II_rounds;
     mean_P_fail_phase_II /= sum_phase_II_rounds;
     mean_N_rounds_phase_II /= sum_phase_II_sessions;
  }
  //mean_total_hops = ( (double) sum_total_hops / (double) total_rounds ) ;

  stop_time = time(NULL);
  double time = difftime(stop_time,start_time);

  int hours = (int) time /(60 * 60);
  int mins = (int)(( (double)(time / (60*60) ) - (double) hours ) * 60 ); 
  int seconds = (int) (((( (double)(time / (60*60) ) - (double) hours ) * 60) - mins ) * 60 );

  total_P_fail = (double)failed_tries / (double) TotalBlocks_;

  //int mins = (int) time / 60;
  //int seconds = (int)(( (double)(time / 60) - (double)min ) * 60 ); 

  cout << "FCMCRBController::printStats() Simulation done! elapsed time: " 
       << hours <<"h " << mins << "m " << seconds << "s " << endl
       << "failed tries = " << failed_tries << endl
       << "good tries = " << TotalBlocks_ - failed_tries << endl
       << "total rounds = " << total_rounds << endl
       << "phase I rounds = " << sum_phase_I_rounds << endl
       << "phase II rounds = " << sum_phase_II_rounds << endl
       << "phase II sessions = " << sum_phase_II_sessions << endl
       << "phase I hops = " << sum_total_hops << endl
       << "phase I wasted forwarding sessions = " << phase_I_wasted_tx << endl
       << "phase II wasted advertise pkts = " << phase_II_wasted_adv << endl
       << "broadcast P fail = " << scientific << total_P_fail << fixed << endl
       << "mean phase I N_tx = " << mean_N_tx_phase_I << endl
       << "mean phase I P_fail = " << scientific << mean_P_fail_phase_I << fixed << endl
       << "mean phase II N_tx = " << mean_N_tx_phase_II << endl
       << "mean phase II P_fail = " << scientific << mean_P_fail_phase_II << fixed << endl
       << "mean phase II N_rounds = " << mean_N_rounds_phase_II << endl;
  
  map<int,double>::iterator mit;
  int sum_freq = 0;
  int sum_freq_II = 0;
  double max_round = 0;

  for (mit = freq_phase_I_rounds.begin() ; mit != freq_phase_I_rounds.end() ; mit++) {
       int round = (*mit).first;
       int freq = (*mit).second;
       sum_freq += freq;
  }

  for (mit = freq_phase_I_rounds.begin() ; mit != freq_phase_I_rounds.end() ; mit++) {
       int round = (*mit).first;
       int freq = (*mit).second;
       freq_phase_I_rounds[round] /= sum_freq;
       /*cout << "phase I round's pdf: round = " << round << " prob = " << scientific 
            << freq_phase_I_rounds[round] << fixed << endl;*/
  }

  for (mit = freq_phase_II_rounds.begin() ; mit != freq_phase_II_rounds.end() ; mit++) {
       int round = (*mit).first;
       int freq = (*mit).second;
       sum_freq_II += freq;
  }

 sum_freq = 0;
 for (mit = complete_phase_I.begin() ; mit != complete_phase_I.end() ; mit++) {
       int nodes = (*mit).first;
       int freq = (*mit).second;
       sum_freq += freq;
  }
   
  for (mit = complete_phase_I.begin() ; mit != complete_phase_I.end() ; mit++) {
       int nodes = (*mit).first;
       int freq = (*mit).second;
       double percentage = (double) nodes / (double) ( fcm_pointers->size() );
       complete_phase_I[nodes] /= sum_freq;
       cout << "phase I netwrk coverage's pdf: % = " << percentage << " prob = " << scientific
            << complete_phase_I[nodes] << fixed << endl;
  }
   
  map<int,double>::reverse_iterator rit = complete_phase_II.rend();
  //rit++;
  max_round = (*rit).first;
  max_round--;

  //cout << "max round è " << max_round << endl;
  //waitForUser();

  for (mit = blk_max_round.begin() ; mit != blk_max_round.end() ; mit++) {
       int block = (*mit).first;
       double round = (*mit).second;
       int difference = abs(max_round - round);
       //if (difference == 0) cout << "diff = 0!! , rounds " << round << " block = " << block << endl;
       if ( difference > 0) {
          /*cout << "phase II round " << round << " at block " << block << ", max round is " 
               << max_round << " writing indexes: " ;*/
          for ( int i = 1; i <= difference; i++) {
              int index = round + i;
              //cout << setw(3) << index ;
              int all_nodes = fcm_pointers->size();
              complete_phase_II[index] += all_nodes;
              complete_phase_II_freq[index]++;
          }
          //cout << endl;
       }
  }

  for (mit = freq_phase_II_rounds.begin() ; mit != freq_phase_II_rounds.end() ; mit++) {
       int round = (*mit).first;
       double freq = (*mit).second;
       freq_phase_II_rounds[round] /= sum_freq_II;
       cout << "phase II round's pdf: round = " << round << " prob = " << scientific 
            << freq_phase_II_rounds[round] << fixed << endl;
  }

  for (mit = complete_phase_II.begin() ; mit != complete_phase_II.end() ; mit++) {
       int round = (*mit).first;
       double nodes = (*mit).second;
       double mean_nodes = (double) nodes / (double) complete_phase_II_freq[round];
       double mean_percentage = mean_nodes / (double) (fcm_pointers->size());
       cout << "phase II netwrk coverage progress: round = " << round << " mean % = " << mean_percentage << endl;
  }

}

void FCMCRBController::reset()
{
 
 global_rounds = 0;
 phase_I_rounds = 0;
 phase_II_rounds = 0;
 current_block = 1;
 total_rounds = 0;

 phase_I_hops = 0;
 sum_total_hops = 0;
 //mean_total_hops = 0;

 border_nodes.clear();
 child_map.clear();
 next_border_nodes.clear();
 blk_max_round.clear();
 freq_phase_I_rounds.clear();
 freq_phase_II_rounds.clear();
 complete_phase_II.clear();
 fcm_phase_I_senders.clear();
 complete_phase_I.clear();
 complete_phase_II_freq.clear();

 mean_N_tx_phase_I = 0.0;
 mean_P_fail_phase_I = 0.0;
 mean_N_rounds_phase_I = 0.0;  
 mean_N_tx_phase_II = 0.0;
 mean_P_fail_phase_II = 0.0;
 mean_N_rounds_phase_II = 0.0;  
 mean_Nodes_covered = 0.0;

 total_P_fail = 0.0;
 failed_tries = 0;
 
 curr_phase = TCL_phase_I;
 phase_I_wasted_tx = 0;
 phase_II_wasted_adv = 0;
 sum_phase_I_rounds = 0;
 sum_phase_II_rounds = 0;
 phase_I_total_rounds = 0;
 phase_II_total_rounds = 0;
 sum_phase_II_sessions= 0;
 phase_II_sessions = 0;

 //fcm_lcomp->clear(); 

}

inline void FCMCRBController::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
}

inline double FCMCRBController::getMeanN_tx()
{
  //return mean_N_tx;
}

inline double FCMCRBController::getMeanP_fail()
{
  //return mean_P_fail;
}

inline double FCMCRBController::getMeanN_rounds()
{
  //return mean_N_rounds;  
}  

inline double FCMCRBController::getMeanNodes_Covered()
{
  return mean_Nodes_covered;
}

inline bool FCMCRBController::isConnected()
{
  return topology_connected;
}


