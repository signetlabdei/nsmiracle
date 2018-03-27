#include "FCM-Multihop-Controller.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <fstream>

enum 
{
    NOT_SET = 195355, TIER = 5, SINGLE = 0, NET = 11, DISCONNECTED = 100, CONNECTED = 40, NEXT_SIMUL = 200
};

static class FCMMultiHopControlClass : public TclClass {
public:
  FCMMultiHopControlClass() : TclClass("Module/FCM/SimpleMAC/MHopCtrl") {}
  TclObject* create(int, const char*const*) {
    return ( new FCMMultihopController() );
  }
} FCM_MultiHop_Control_Class;

int FCMMultihopController::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc==3)
  {
      if(strcasecmp(argv[1], "setSourceNode") == 0)
        {
           FCModule* fcm_source =(FCModule*)TclObject::lookup(argv[2]); //get FCM Id of module
           Position* fcm_source_pos = fcm_source->getPosition();
           o_x = fcm_source_pos->getX();
           o_y = fcm_source_pos->getY();
           o_z = fcm_source_pos->getZ();
	   fcm_pointers = &(fcm_source->fcm_pointers); // mi copio i puntatori
           fcm_complete = &(fcm_source->complete_peers);
  	   fcm_incomplete = &(fcm_source->incomplete_peers);
           fcm_source_id = fcm_source->GetFCMId();
           fcm_parents[fcm_source_id]=fcm_source_id;
           initializeTopology();
           computeDistance(fcm_source_id, &src_dist_vector);
           computeInnerBorder();
           if (print_map == true) printMap();
           return TCL_OK;
        }
      else if(strcasecmp(argv[1], "setOptFile") == 0)
        {
          return TCL_OK;
        }  
  }
  else if(argc==2)
    {
      if(strcasecmp(argv[1], "startMultiHop") == 0)	// TCL command to start the packet generation and transmission
	{
          test = true;
          start_time = time(NULL);
          startMultiHop();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "testTopology") == 0)	
	{
          test = true;
          start_time = time(NULL);
          startMultiHop();
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "printMap") == 0)	
	{
          print_map = true;
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
      else if(strcasecmp(argv[1], "getMeanResolved_Radius") == 0)	
	{
          tcl.resultf("%f",getMeanResolved_Radius());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanTierResolved_Radius") == 0)	
	{
          tcl.resultf("%f",getMeanTierResolved_Radius());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getSourceN_tx") == 0)	
	{
          tcl.resultf("%f",getSourceN_tx());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getSourceP_fail") == 0)	
	{
          tcl.resultf("%f",getSourceP_fail());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getSourceN_rounds") == 0)	
	{
          tcl.resultf("%f",getSourceN_rounds());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getSourceResolved_Radius") == 0)	
	{
          tcl.resultf("%f",getSourceResolved_Radius());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getMeanNodes_Covered") == 0)	
	{
          tcl.resultf("%f",getMeanNodes_Covered());
	  return TCL_OK;
	}
      else if(strcasecmp(argv[1], "getSourceNodes_Covered") == 0)	
	{
          tcl.resultf("%f",getSourceNodes_Covered());
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

FCMMultihopController::FCMMultihopController() :
  fcm_source_id(NOT_SET),
  resolved_radius(0.0),
  total_hops(0),
  sum_total_hops(0),
  mean_total_hops(0),
  debug_(0),
  test(false),
  topology_connected(false),
  print_map(false),
  global_rounds(0),
  current_block(1),
  total_rounds(1),
  failed_tries(0),
  inner_rounds(0),
  total_connected_times(0),
  mean_N_tx(0.0),
  mean_P_fail(0.0),
  mean_N_rounds(0.0), 
  source_N_tx(0.0),
  source_P_fail(0.0),
  source_N_rounds(0.0),
  resolved_projection(0.0),
  mean_resolved_projection(0.0),
  mean_tier_resolved_projection(0.0),
  source_resolved_projection(0.0),
  mean_Resolved_Radius(0.0),
  source_Resolved_Radius(0.0),
  mean_tier_Resolved_Radius(0.0),
  mean_Nodes_covered(0.0),
  source_Nodes_covered(0.0),
  total_P_fail(0.0)
{
  bind("TxRadius_", (double*)&TxRadius_);
  bind("TotalBlocks_",(int*)&TotalBlocks_);
  bind("debug_", (int*)&debug_);
  bind("genType_", (int*)&genType_); // generator type
  bind("Rmax_", (double*)&Rmax_);
  bind("RandSeed_", (int*)&RandSeed_);
  bind("Lambda_",(int*)&Lambda_);
  border_nodes.clear();
  child_map.clear();
  next_border_nodes.clear();
  fcm_parents.clear();
  tier_nodes.clear();
  tier_resolved_radius.clear();
  net_resolved_radius.clear();
  blocks_hop.clear();
  inner_border_nodes.clear();
  //rand = new Random( genType_ );
  mtrand.seed(RandSeed_);
  //rand->setSeed( RandSeed_);
  mtrand_int.seed(RandSeed_);
}

FCMMultihopController::~FCMMultihopController()
{
 //delete rand;
}

void FCMMultihopController::computeDistance(int start_id, vector<double>* dist_vector)
{
  Position* source_pos = (fcm_pointers->at(start_id))->getPosition();

  for (int i = 0; i < fcm_pointers->size(); i++) 
  {
     Position* p_receiver = (fcm_pointers->at(i))->getPosition();
     double dist = source_pos->getDist(p_receiver);

     /*if (debug_ > 0 ) {
        cout << "FCMMultihopController::computeDistance() user id " << i << " has distance " 
             << dist << " from node id " << start_id << endl;
     }    */                  

     dist_vector->push_back(dist);
  }
  // //if ( debug_ > 0 ) waitForUser();
}

void FCMMultihopController::computeProjection(int start_id, vector<double>* proj_vector, vector<double>* dist_vector) 
{
  Position* sourcePos = (fcm_pointers->at(start_id))->getPosition();
  double s_x = sourcePos->getX();
  double s_y = sourcePos->getY();
  double s_z = sourcePos->getZ();

  //cout << " s_x " << s_x << " s_y " << s_y << " s_z " << s_z << endl;
  //waitForUser(); 

  for (int index = 0; index < fcm_pointers->size(); index++) {
 
    if ( index == start_id ) {
         proj_vector->push_back(0);
         continue;
     }

     Position* pos = (fcm_pointers->at(index))->getPosition();
     double p_x = pos->getX();
     double p_y = pos->getY();
     double p_z = pos->getZ();
     
     //cout << " p_x " << p_x << " p_y " << p_y << " p_z " << p_z << endl;
     //waitForUser(); 

     double q_x;
     double q_y;
     double q_z;

     if ( start_id == fcm_source_id) { // se il nodo di partenza è la sorgente 
         int randint = mtrand_int();
         double r_seed = (double) RandSeed_;
         if ( ( randint % 2 ) == 0 ) q_x = mtrand()*r_seed;
         else q_x = -mtrand()*r_seed;
         randint = mtrand_int();
         if ( ( randint % 2 ) == 0 ) p_y = mtrand()*r_seed;
         else q_y = -mtrand()*r_seed;
         randint = mtrand_int();
         /*if ( ( randint % 2 ) == 0 ) p_z = mtrand()*10.0;
         else q_z = -mtrand()*10.0;*/
         q_z = -1000; // simuliamo piano
     }
     else {
        int t = 2; // per avere un punto sulla retta piu avanzato di start_id
        q_x = o_x + (s_x - o_x ) * 2;
        q_y = o_y + (s_y - o_y ) * 2;
        q_z = o_z + (s_z - o_z ) * 2;
     }

     //cout << " q_x " << q_x << " q_y " << q_y << " q_z " << q_z << endl;
     //waitForUser(); 

     // calcolo il valore del prodotto scalare tra i vettori sq e sp
     double sq_x = q_x - s_x;
     double sq_y = q_y - s_y;
     double sq_z = q_z - s_z;

     //cout << " sq_x " << sq_x << " sq_y " << sq_y << " sq_z " << sq_z << endl;
     //waitForUser(); 
          
     double sp_x = p_x - s_x;
     double sp_y = p_y - s_y;
     double sp_z = p_z - s_z;

     //cout << " sp_x " << sp_x << " sp_y " << sp_y << " sp_z " << sp_z << endl;
     //waitForUser(); 

     double sp_scalar_sq = sq_x * sp_x + sq_y * sp_y + sq_z * sp_z;
     
     //cout << "sp_scalar_sq " << sp_scalar_sq << endl;
     //waitForUser();

     // calcolo modulo del vettore sq e del vettore sp
     double sq_mod = sqrt( pow( sq_x ,2) + pow( sq_y ,2) + pow( sq_z ,2) );
     double sp_mod = sqrt( pow( sp_x ,2) + pow( sp_y ,2) + pow( sp_z ,2) );

     //cout << "sq_mod " << sq_mod << endl;
     //cout << "sp_mod " << sp_mod << endl;
     //waitForUser();

     // calcolo angolo compreso e lungh proiezione
     double cos_alpha = sp_scalar_sq / ( sq_mod * sp_mod ) ;

     //cout << "cos_alpha " << cos_alpha << endl;
     //waitForUser();

     double dist = dist_vector->at(index);     

     //cout << "dist " << dist << endl;
     //waitForUser();

     double projection = dist * cos_alpha;

     //cout << "projection " << projection << endl;
     //waitForUser();

     proj_vector->push_back(projection);

   /*if (debug_ > 0 ) {
        cout << "FCMMultihopController::computeProjection() user id " << index << " has projection " 
             << projection << " from node id " << start_id << endl;
     } */                     

  }
  //if ( debug_ > 0 ) waitForUser();
}

void FCMMultihopController::computeResolveArea(int source_id, int command)
{
  static int compl_size = NOT_SET;
  set<double> complete_dist; // distanza e nodi completi
  set<double> incomplete_dist;
  set<double> complete_proj;
  set<double> incomplete_proj;
  vector<double> distance_vector; // distanza e nodi completi
  vector<double> projection_vector;

  computeDistance(source_id, &distance_vector);
  computeProjection(source_id, &projection_vector, &distance_vector);

  if (debug_ >0) cout << "************************************************************************" 
                      << endl << "FCMMultihopController::computeResolveArea() CALCULATING RES AREA" 
                      << " FOR NODE " << source_id << endl;

  // prendo le distanze dei nodi completi e le ordino
  for ( Setiter it = fcm_complete->begin(); it != fcm_complete->end(); it ++) {
       int index = (*it);
       double dist = distance_vector[index];
       double projection = projection_vector[index];

       if (debug_ >0) cout << "FCMMultihopController::computeResolveArea() dist of complete node "
                           << index << " is " << dist << " his projection on source-node vector is  " 
                           << projection << endl;

       if ( (dist <= TxRadius_) && (projection >= 0) ) complete_proj.insert(projection);

       if ( (command != NET) && (dist <= TxRadius_) ) complete_dist.insert(dist);
       else if ( command == NET ) complete_dist.insert(dist);

  }

  incomplete_proj.insert(NOT_SET);
  incomplete_dist.insert(NOT_SET);

  if ( !fcm_incomplete->empty() ) {
     // prendo le distanze dei nodi incompleti e le ordino
     for ( Setiter it = fcm_incomplete->begin(); it != fcm_incomplete->end(); it ++) {
          int index = (*it);
          double dist = distance_vector[index];
          double projection = projection_vector[index];

          if (debug_ >0) cout << "FCMMultihopController::computeResolveArea() dist of incomplete node "
                              << index << " is " << dist << " his projection on source-node vector is " 
                              << projection << endl;

          if ( (dist <= TxRadius_) && (projection > 0) ) incomplete_proj.insert(projection); 

          if ( (command != NET) && (dist <= TxRadius_) ) incomplete_dist.insert(dist);
          else if ( command == NET ) incomplete_dist.insert(dist);  

     }
  }
  //else incomplete_dist.insert(NOT_SET); 

  // prendo la dist max dei completi
  set<double>::reverse_iterator rit = complete_dist.rbegin(); 
  double compl_max_dist = *rit;

  if (debug_ >0) cout << "FCMMultihopController::computeResolveArea() max complete dist is " 
                      << compl_max_dist << endl;
    
  // prendo la dist min degli incompleti
  set<double>::iterator it = incomplete_dist.begin();
  double incompl_min_dist = *it;

  // se dist max completi < dist min incompleti => raggio di completezza = dist max completi
  if ( compl_max_dist < incompl_min_dist ) resolved_radius = compl_max_dist;

  // se no prendo il la distanza dei completi prima del massimo
  else { 
    rit = complete_dist.rbegin(); // max (iteratore in ordine inverso)
    rit++; // max - 1
    compl_max_dist = *rit;
    // itero finchè nn trovo il raggio
    while ( (compl_max_dist > incompl_min_dist) && ( rit != complete_dist.rend() ) ) {       
       compl_max_dist = *rit;
       /*cout << "FCMMultihopController::computeResolveArea() compl_max_dist is " << compl_max_dist
             << endl;*/
       rit++;
    }
    resolved_radius = compl_max_dist;
  }

 // CALCOLO PROIEZIONI ////////////////////////////////////////////////////////////////////////////

  // prendo la dist max dei completi
  rit = complete_proj.rbegin(); 
  double compl_max_proj = *rit;

  if (debug_ >0) cout << "FCMMultihopController::computeResolveArea() max complete projection on source-node"
                      << " vector is " << compl_max_proj << endl;
    
  // prendo la dist min degli incompleti

  it = incomplete_proj.begin();
  double incompl_min_proj = *it;

  if (debug_ >0) cout << "FCMMultihopController::computeResolveArea() min incomplete projection on source-node"
                      << " vector is " << incompl_min_proj << endl;

  // se dist max completi < dist min incompleti => raggio di completezza = dist max completi
  if ( compl_max_proj < incompl_min_proj ) resolved_projection = compl_max_proj;

  // se no prendo il la distanza dei completi prima del massimo
  else { 
    rit = complete_proj.rbegin(); // max (iteratore in ordine inverso)
    rit++; // max - 1
    compl_max_proj = *rit;
    // itero finchè nn trovo il raggio
    while ( (compl_max_proj > incompl_min_proj) && ( rit != complete_proj.rend() ) ) {       
       compl_max_proj = *rit;
       /*cout << "FCMMultihopController::computeResolveArea() compl_max_dist is " << compl_max_dist
             << endl;*/
       rit++;
    }
    resolved_projection = compl_max_proj;
  }

  // CALCOLO PROIEZIONI 

  if ( command == SINGLE) {
     mean_Resolved_Radius += resolved_radius;
     mean_resolved_projection += resolved_projection; // o va bene x tutti?
     if (global_rounds == 1) {
        source_Resolved_Radius += resolved_radius;
        source_resolved_projection += resolved_projection;
      /*cout << "command == single , resolved radius = " << resolved_radius << " source res = " 
             << source_Resolved_Radius << endl;*/
     }
  }
  else if ( command == TIER) {
      mean_tier_Resolved_Radius += resolved_radius;
      mean_tier_resolved_projection += resolved_projection;
    /*cout << "command == tier , resolved radius = " << resolved_radius << " mean tier res = " 
           << mean_tier_Resolved_Radius << endl; */
  }
  else {
      net_resolved_radius[total_hops]+=resolved_radius;
      /*cout << "command == net , resolved radius = " << resolved_radius << " net_resolved_radius[" 
           <<  total_hops <<"] = "<< net_resolved_radius[total_hops] << endl; */
  }

  if (debug_ > 0) {
      cout << "FCMMultihopController::computeResolveArea() calculated resolved_radius of node " 
           << source_id << " is " << resolved_radius << " calculated resolved projection is "
           << resolved_projection << endl;        
  }
  
  if ( command == SINGLE) {
     // trovo i nodi che sono dentro il cerchio di completezza
     for (Setiter it = fcm_complete->begin(); it != fcm_complete->end(); it++) {
        int index = *it;
        double dist = distance_vector[index];
     
        if ( (dist <= TxRadius_ ) && ( index != source_id ) ) {

           if (global_rounds == 1) {
               source_Nodes_covered++;

               if (debug_ > 0) {
                  cout << "FCMMultihopController::computeResolveArea() global rounds " << global_rounds 
                       << " source nodes covered called set to " << source_Nodes_covered << endl;
                   //waitForUser();
               } 
 
           }

           mean_Nodes_covered++;

           if (debug_ > 0) {
              cout << "FCMMultihopController::computeResolveArea() global rounds " << global_rounds 
                   << " mean nodes covered called set to " << mean_Nodes_covered << endl;
           } 

           if (debug_ > 0) {
              cout << "FCMMultihopController::computeResolveArea() node: " << index 
                   << " is in node " << source_id << " Resolved Area. Its parent is node is " 
                   << fcm_parents[index] << endl ;
           } 
        }
     }
  }

  else if ( (command == NET) && (debug_ > 0) ) {

     for (Setiter it = fcm_complete->begin(); it != fcm_complete->end(); it++) {
        int index = *it;
        double dist = distance_vector[index];

        if ( dist <= resolved_radius ) {
              cout << "FCMMultihopController::computeResolveArea() node: " << index 
                   << " is in node " << source_id << " Resolved Area. Its parent is node is " 
                   << fcm_parents[index] << endl ;
        }
     }
     //waitForUser();
  }
  //incomplete_dist.clear();
  //complete_dist.clear();

  if (debug_ > 0) {
     cout << "***********************************************************************" << endl;
     if (test == false) waitForUser();
  }

}

void FCMMultihopController::computeInnerBorder()
{
   inner_border_nodes.clear();
   for (int i = 0; i < src_dist_vector.size(); i++ ) {
      if ( src_dist_vector[i] <= ( Rmax_ - TxRadius_) ) {
          inner_border_nodes.insert(i);
          if (debug_ > 0) cout << "FCMMultihopController::computeInnerBorder() node " << i 
                               << " has distance from source node " << src_dist_vector[i] 
                               << " <= " << (Rmax_ - TxRadius_) << " inserting it into " 
                               << "inner border nodes. " << endl;
      }
   }
   tot_inn_border_nodes = inner_border_nodes.size();
   if (debug_ > 0) {
      cout << "FCMMultihopController::computeInnerBorder() inner_border_nodes has size " 
           << tot_inn_border_nodes << endl;
      waitForUser();
   }
}

void FCMMultihopController::computeChildNodes(const int start_pos_id, set<int>* incomp_nodes)
{
  set<int> test_nodes;
  // se nodo nn è completo lo tolgo dai nodi sul bordo  
  if ( (fcm_complete->count(start_pos_id)) == 0 ) {
     if (debug_ > 0) {
        cout << "FCMMultihopController::computeChildNodes() parent node " << start_pos_id 
             << " is not in complete set, removing from border nodes" << endl;

     }
     border_nodes.erase(start_pos_id);
     return;   
  }

  if ( start_pos_id <= fcm_pointers->size() ) { // se l'id dato è corretto

     child_map[start_pos_id].clear();

     Position* p_source = (fcm_pointers->at(start_pos_id))->getPosition();

     /*if (debug_ > 0 ) {
        cout << "FCMMultihopController::computeChildNodes() parent node " << start_pos_id 
             << " has distance " << dist_start << endl;
     }*/

     //calcolo le distanze relative e aggiorno mappa
     for ( Setiter it = incomp_nodes->begin(); it != incomp_nodes->end(); it++ ) {

         int index = *it;
         Position* p_index = (fcm_pointers->at(index))->getPosition();
         double dist = p_source->getDist(p_index);

         if (debug_ > 0 ) {
             cout << "FCMMultihopController::computeChildNodes() node id "<< index <<" has distance " 
                  << dist << " from parent node " << start_pos_id << endl;
         }

         if ( dist <= TxRadius_) {
            child_map[start_pos_id].insert(index); // inserisco nella mappa il figlio
            fcm_parents[index]=start_pos_id;
            next_border_nodes.insert(index);
            if ( test == true) {
               fcm_complete->insert(index);
               test_nodes.insert(index);
            }

            if (debug_ > 0 ) {
               cout << "FCMMultihopController::computeChildNodes() node id " << index 
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
     cout << "FCMMultihopController::computeChildNodes() ERROR start_pos_id out of bound!" << endl ;
     exit(1);
  }
}

void FCMMultihopController::pruneChildMap(set<int> parent_nodes)
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
              cout << "FCMMultihopController::pruneChildMap() childmap[" << index 
                   << "] has 0 size. node " << index <<" has been deleted from border_nodes" << endl;
           } 
           continue;
       }
       double dist_index = src_dist_vector[index];
       //Position* p_index = (fcm_pointers->at(index))->getPosition();
       //double dist = p_incompl_min->getDist(p_index);

       if (debug_ > 0 ) {
          cout << "FCMMultihopController::pruneChildMap() node " << index
               << " has size " << size << " distance from source " << dist_index << endl;
       }

       if ( (size > max_size)  ) {
          max_size = size;
          max_index = index;
          is_disconnected = false;
       }
   }
   
   if (is_disconnected == true) { // la mappa è disconnessa
      if (debug_ > 0) cout << "FCMMultihopController::pruneChildMap() all sub-trees are empty." 
                           << " The node map is not connected! " << endl;
      topology_connected = false;
      border_nodes.clear();
      return;
   }
   else topology_connected = true;
  
   if (debug_ > 0) {
       cout << "FCMMultihopController::pruneChildMap() parent node " << max_index 
            << " has been chosen: max size " << max_size << endl;
   }

   /*if (debug_ > 0) {
      cout << "FCMMultihopController::pruneChildMap() parent node " << max_index 
           << " has max no of child " << max_size << endl;
      waitForUser();
   }*/

   parent_nodes.erase(max_index); // tolgo dalla lista il nodo trovato
   
   // tolgo dagli altri set gli eventuali nodi in comune
   for ( Setiter it = child_map[max_index].begin() ; it != child_map[max_index].end() ; it++ ) {
         int value = *it;

         if (debug_ > 0) {
            cout << "FCMMultihopController::pruneChildMap() parent node " << max_index 
                 << " has child node " << value << endl;
         }

         for ( Setiter it2 = parent_nodes.begin(); it2 != parent_nodes.end(); it2++ ) {
             int index = *it2;
             child_map[index].erase(value);

             if (debug_ > 0) {
                 cout << "FCMMultihopController::pruneChildMap() deleting from parent node " << index 
                      << "'s tree the child node " << value << endl;
             }
         
             if (child_map[index].size() == 0) {
                 border_nodes.erase(index);
                 //parent_nodes.erase(index);
             
                 if (debug_ > 0) {
                    cout << "FCMMultihopController::pruneChildMap() childmap[" << index 
                         << "] has 0 size. node " << index <<" has been deleted from border_nodes" << endl;
                 }         
             }
         }
   }
 max_size = 0;
 max_index = 0;

 }
   //if (debug_ > 0) waitForUser();
}

inline void FCMMultihopController::setDone(const int id) 
{
  global_rounds++;
  border_nodes.erase(id);
  inner_border_nodes.erase(id);

  if ( src_dist_vector[id] <= (Rmax_ - TxRadius_) ) {
       if (debug_ > 0) cout << "FCMMultihopController::setDone() node " << id << " has distance " 
                            << "from source nodes " << src_dist_vector[id] << " <= " << (Rmax_ - TxRadius_)
                            << " . Calculating statistics... " << endl;
       //inner_border_nodes.erase(id);
       for ( Setiter it = fcm_complete->begin(); it != fcm_complete->end() ; it++ ) {
           int index = *it;
           inner_border_nodes.erase(index);
       }
       getStats(id);
       computeResolveArea(id, SINGLE);
       inner_rounds++;
  }

  if ( border_nodes.size() != 0 ) {
     startBorderNodes();
  }
  else {

     if (debug_ > 0) {
        cout << "********************************************************************" 
             << endl << "CALCULATING TIER RESOLVED AREA" << endl;
     }
 
     for ( Setiter it = tier_nodes.begin(); it != tier_nodes.end(); it++) {
         int index = *it; 
         computeResolveArea(index, TIER);
     }
     tier_resolved_radius[total_hops] += (mean_tier_Resolved_Radius / tier_nodes.size() ) ;
     tier_resolved_projection[total_hops] += (mean_tier_resolved_projection / tier_nodes.size() ) ;
     mean_tier_Resolved_Radius = 0;
     mean_tier_resolved_projection = 0;

     if (debug_ > 0) {
        cout << "********************************************************************" 
             << endl;
     }

     if (debug_ > 0) {
        cout << "********************************************************************" 
             << endl << "CALCULATING NETWORK RESOLVED AREA" << endl;
     }
     
     computeResolveArea(fcm_source_id, NET);

     if (debug_ > 0) {
        cout << "********************************************************************" 
             << endl;
     }
     
     border_nodes = next_border_nodes;
     next_border_nodes.clear();
     tier_nodes.clear();

     if (debug_ > 0) {
         cout << "FCMMultihopController::setDone() inner_border_nodes size is " << inner_border_nodes.size()
              << endl;
         waitForUser();
     }

     startMultiHop(); // da modificare, da fare controllo x chiudere sessione
  }
}

void FCMMultihopController::startBorderNodes() 
{
  if ( !border_nodes.empty() ) {
      Setiter it = border_nodes.begin();
      int index = *it;
     
      set<int> receivers = child_map[index];

      if (debug_ > 0 ) {
         cout << "FCMMultihopController::startBorderNodes() " 
         << "node " << index << " set int receivers size = " << receivers.size() 
         << " child_map[" << index << "] size = " << child_map[index].size() << endl ;
         //waitForUser();
      }

      if (test == false) {
         (fcm_pointers->at(index))->setInitialReceivers(&receivers);
         (fcm_pointers->at(index))->startMasterEncoder();
      }
      else {
       fcm_complete->insert(index);
       fcm_incomplete->erase(index);
       setDone(index);        
      }
  }
}

void FCMMultihopController::startMultiHop()
{
  if ( fcm_source_id == NOT_SET ) {
     cout << "FCMMultihopController::StartMultiHop() error, set source in TCL first" << endl; 
     exit(1);
  }
  if (fcm_incomplete->size() == 0) {
      stop(CONNECTED);
      return;
  } 

  if (test == true) TotalBlocks_ = 1;

  for ( Setiter it = border_nodes.begin(); it != border_nodes.end(); it++) {
     computeChildNodes( (*it) , fcm_incomplete );
  }

  pruneChildMap( border_nodes );
  
  tier_nodes = border_nodes;

  if (border_nodes.size() != 0) {
      //if (inner_border_nodes.size() != 0) total_hops++;
      total_hops++;
      blocks_hop[total_hops]++;
      startBorderNodes();
  }
  else {
   if (topology_connected == true) stop(CONNECTED);
   else stop(DISCONNECTED);
  }
}

void FCMMultihopController::refreshCounters(int status)
{
  current_block++;
 
  total_rounds += global_rounds;
  if (status == CONNECTED) {
      sum_total_hops += total_hops;
      total_connected_times++;
  }
  if (debug_ > 0) {
     cout << "FCMMultihopController::refreshCounters() "   
          << "total rounds = " << total_rounds << endl
          << "FCMMultihopController::refreshCounters() " 
          << "total hops = " << total_hops << endl
          << "FCMMultihopController::refreshCounters() " 
          << "sum total hops = " << sum_total_hops << endl;
  }
}

void FCMMultihopController::stop(int status)
{
  refreshCounters(status);


  switch (status) {

     case(CONNECTED): {
        //cout << "caso CONNECTED" << endl;
        if ( (current_block <= TotalBlocks_ ) ) {
            //topology_connected = true;
            border_nodes.clear();
            next_border_nodes.clear();
            tier_nodes.clear();
            global_rounds = 0;
            total_hops = 0;
            initializeTopology();
            startMultiHop();
        }
        else {
           if (test == false) {
               printStats();
               // qui devo schedulare nuova simulazione
           }
           else {
              cout << "FCMMultihopController::stop() Test passed, the network is connected."
                   << " Starting simulation ..."<< endl;
              //waitForUser();
              test = false;
              reset();
              initializeTopology();
              bind("TotalBlocks_",(int*)&TotalBlocks_); // PAZZESCOOOOOOOOO
              startMultiHop();
           }
         }
     }
     break;

     case(DISCONNECTED): {
         if (test == true ) {
             //topology_connected = false;
             cout << "FCMMultihopController::stop(DISCONNECTED) simulation aborted " << endl;
         }
         else {
            if ( current_block <= TotalBlocks_ ) {
               border_nodes.clear();
               next_border_nodes.clear();
               tier_nodes.clear();
               global_rounds = 0;
               total_hops = 0;
               failed_tries++;
               initializeTopology();
               startMultiHop();
            }
            else printStats();
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

void FCMMultihopController::initializeTopology()
{
   computeInnerBorder();
   fcm_complete->clear();
   fcm_complete->insert(fcm_source_id); // la sorgente è già completa
   fcm_incomplete->clear();
   child_map.clear();

   for (int i = 0; i < fcm_pointers->size(); i++) {
       fcm_incomplete->insert(i); // i nodi rimanenti sn incompleti in partenza
       //(fcm_pointers->at(i))->crb_phase = CRB_INACTIVE;
       (fcm_pointers->at(i))->setToDecoder();
       (fcm_pointers->at(i))->finished = true; // ERA FALSE!!
       (fcm_pointers->at(i))->mhop = true; 
       (fcm_pointers->at(i))->abortCurrBlock();
       (fcm_pointers->at(i))->resetStats();
   }

   fcm_incomplete->erase(fcm_source_id);
   border_nodes.insert(fcm_source_id);
   //(fcm_pointers->at(fcm_source_id))->finished = true; ERA NN COMMENTATO!!
   //computeDistance(fcm_source_id, &src_dist_vector);
}

void FCMMultihopController::getStats(int source_id)
{
 double N_tx = (fcm_pointers->at(source_id))->N_tx;
 double P_fail = (fcm_pointers->at(source_id))->P_fail;
 double N_rounds = (fcm_pointers->at(source_id))->N_rounds;
 mean_N_tx += N_tx;
 mean_P_fail += P_fail;
 mean_N_rounds += N_rounds;  

 if ( global_rounds == 1) {
    source_N_tx += N_tx;
    source_P_fail += P_fail;
    source_N_rounds += N_rounds;

    //cout << "global_rounds " << global_rounds << " source stats retrieved " << endl;
    //waitForUser();

 }

}

void FCMMultihopController::printStats()
{
  total_rounds--;
  
  mean_N_tx /= inner_rounds;
  mean_P_fail /= inner_rounds;
  mean_N_rounds /= inner_rounds;
  mean_Nodes_covered /= (fcm_pointers->size() * inner_rounds );

  source_N_tx /= TotalBlocks_;
  source_P_fail /= TotalBlocks_;
  source_N_rounds /= TotalBlocks_;
  source_Nodes_covered /= (fcm_pointers->size() * TotalBlocks_);
  
  mean_total_hops = ( (double) sum_total_hops / (double) total_connected_times ) ;
  
  mean_Resolved_Radius /= inner_rounds;
  source_Resolved_Radius /= TotalBlocks_;
  mean_resolved_projection /= inner_rounds ; 
  source_resolved_projection /= TotalBlocks_;

  stop_time = time(NULL);
  double time = difftime(stop_time,start_time);

  int hours = (int) time /(60 * 60);
  int mins = (int)(( (double)(time / (60*60) ) - (double) hours ) * 60 ); 
  int seconds = (int) (((( (double)(time / (60*60) ) - (double) hours ) * 60) - mins ) * 60 );

  total_P_fail = (double)failed_tries / (double) TotalBlocks_;

  //int mins = (int) time / 60;
  //int seconds = (int)(( (double)(time / 60) - (double)min ) * 60 ); 

  cout << "FCMMultihopController::printStats() Simulation done! elapsed time: " 
       << hours <<"h " << mins << "m " << seconds << "s " << endl
       << "inner nodes = " << tot_inn_border_nodes << endl
       << "failed tries = " << failed_tries << endl
       << "total rounds = " << total_rounds << endl
       << "inner rounds = " << inner_rounds << endl
       << "good tries = " << total_connected_times << endl
       << "broadcast P fail = " << scientific << total_P_fail << fixed << endl
       << "mean total hops number = " << fixed << mean_total_hops << endl
       << "mean resolved radius = " << mean_Resolved_Radius << " [m]" << endl
       << "source mean resolved radius = " << source_Resolved_Radius << " [m]" << endl 
       << "mean resolved projection = " << mean_resolved_projection << " [m]" << endl
       << "source resolved projection = " << source_resolved_projection << " [m]" << endl
       << "mean N_tx = " << mean_N_tx << endl
       << "mean_P_fail = " << scientific << mean_P_fail << fixed << endl
       << "mean N_rounds = " << mean_N_rounds << endl
       << "mean prob of nodes covered = " << scientific << mean_Nodes_covered << fixed << endl
       << "source mean N_tx = " << source_N_tx << endl
       << "source mean P_fail = " << scientific << source_P_fail << fixed << endl
       << "source mean N_rounds = " << source_N_rounds << endl
       << "source mean prob of nodes covered = " << scientific << source_Nodes_covered << fixed << endl;
  
  map<int,double>::iterator mit;

  for (mit = tier_resolved_radius.begin() ; mit != tier_resolved_radius.end() ; mit++) {
       int hop = (*mit).first;
       double tier_radius = (*mit).second;
       int block_no = blocks_hop[hop];
       double mean_tier_radius = tier_radius / block_no ;  
       cout << "hop no " << hop << " mean tier resolved radius = " << mean_tier_radius << " [m]"
            << " recurrence no " << block_no << endl;
  }

  for (mit = tier_resolved_projection.begin() ; mit != tier_resolved_projection.end() ; mit++) {
       int hop = (*mit).first;
       double tier_projection = (*mit).second;
       int block_no = blocks_hop[hop];
       double mean_tier_projection = tier_projection / block_no ;  
       cout << "hop no " << hop << " mean tier resolved projection = " << mean_tier_projection << " [m]" 
            << " recurrence no " << block_no << endl;
  }

  for (mit = net_resolved_radius.begin() ; mit != net_resolved_radius.end() ; mit++) {
       int hop = (*mit).first;
       double net_radius = (*mit).second;
       int block_no = blocks_hop[hop];
       double mean_net_radius = net_radius / block_no ;  
       cout << "hop no " << hop << " mean net resolved radius = " << mean_net_radius << " [m]" 
            << " recurrence no " << block_no << endl;
  }

}

void FCMMultihopController::reset()
{
 
 global_rounds = 0;
 current_block = 1;
 total_rounds = 1;

 total_hops = 0;
 sum_total_hops = 0;
 mean_total_hops = 0;

 blocks_hop.clear();
 border_nodes.clear();
 child_map.clear();
 next_border_nodes.clear();
 //complete_dist.clear();
 //incomplete_dist.clear();
 fcm_parents.clear();
 tier_nodes.clear();
 tier_resolved_radius.clear();
 tier_resolved_projection.clear();
 net_resolved_radius.clear();

 //fcm_parents[fcm_source_id]=0;

 resolved_radius = 0.0;
 mean_Resolved_Radius = 0.0;
 mean_tier_Resolved_Radius = 0.0;
 mean_resolved_projection = 0.0;
 mean_tier_resolved_projection = 0.0;
 resolved_projection = 0.0;
 source_resolved_projection = 0.0;

 mean_N_tx = 0.0;
 mean_P_fail = 0.0;
 mean_N_rounds = 0.0;  
 mean_Nodes_covered = 0.0;

 source_N_tx = 0.0;
 source_P_fail = 0.0;
 source_N_rounds = 0.0;  
 source_Resolved_Radius = 0.0;
 source_Nodes_covered = 0.0;
 total_P_fail = 0.0;
 failed_tries = 0;
 inner_rounds = 0;
 total_connected_times = 0;

}

void FCMMultihopController::printMap()
{
   stringstream str_out;

   int XI = 4;
   int blocks = TotalBlocks_;
   int max_rounds = (fcm_pointers->at(fcm_source_id))->max_rounds_;
   
   
  
   //bind("opt(XI)",(int+)&XI);

   str_out << "Map_mh_" << "lambda" << Lambda_ << "_Rmax" << Rmax_ << "_TxRad" << TxRadius_ 
           << "_Rng" << RandSeed_ ;
   
   string name = str_out.str();
   str_out.str("");
 
   string mapname = name + ".stats";
   string epsname = name + ".eps";
   string gnuname = name + ".gnu";

   str_out << "/home/fguerra/simulazioni/results/maps/" << "LAMBDA" << Lambda_ << "/" 
           << "RMAX" << Rmax_ << "/" << "RTX" << TxRadius_ << "/";
   string path = str_out.str();

   str_out.str("");

   string command = "touch " + path + mapname;
   if (system(NULL)) system(command.c_str());
   command.clear();

   command = "touch " + path + gnuname;
   if (system(NULL)) system(command.c_str());
   command.clear();

   string mappath = path + mapname;   
   string epspath = path + epsname;
   string gnupath = path + gnuname;

   ofstream file;
   file.open(mappath.c_str());
  
   file << "#FCMMultihopController::printMap() node map, rng = " << RandSeed_ << endl
        << setw(10) << "#X axis" << setw(10) << "Y axis" << setw(10) << "Z axis" << endl;

   for (int i = 0; i < fcm_pointers->size(); i++) {
       Position* curr_pos = (fcm_pointers->at(i))->getPosition();

       double x_axis = curr_pos->getX();
       double y_axis = curr_pos->getY();
       double z_axis = curr_pos->getZ();

       file << setw(10) << x_axis << setw(10) << y_axis << setw(10) << z_axis << endl;

   }     
   file << endl << endl;
   file.close();
   
   file.open(gnupath.c_str());

   file << "set term post eps enhanced \"Times-Roman\" 20 " << endl
        << "set parametric" << endl
        << "set output \'"<< epspath << "\'"<< endl
        << "set ylabel \"Y [km]\"" << endl
        << "set xlabel \"X [km]\"" << endl
        << "set grid xtics ytics" << endl
        << "#set logscale y" << endl
        << "#set logscale x" << endl
        << "set key top left Left reverse width -4" << endl
        << "set pointsize 1.4" << endl
        << "set xrange [-"<< Rmax_ + 1000 << ":" << Rmax_ + 1000 <<"]" << endl
        << "set yrange [-"<< Rmax_ + 1000 << ":" << Rmax_ + 1000 <<"]" << endl
        << "set trange [0:2*pi]" << endl
        << "set parametric" << endl
        << "plot " << "\"" << mappath << "\"" << " using 1:2 notitle with p lt 1 pt 6,\\" << endl
        << TxRadius_ <<"*sin(t),"<< TxRadius_ <<"*cos(t) notitle, "<< Rmax_ <<"*sin(t),"<< Rmax_ 
        <<"*cos(t) notitle, \\" << endl 
        << Rmax_ - TxRadius_ <<"*sin(t),"<< Rmax_ - TxRadius_ <<"*cos(t) notitle" << endl;

    file.close();

    if(system(NULL)) {
      command = "gnuplot " + gnupath;
      system(command.c_str());
      command.clear();
      command = " rm -r " + gnupath;
      system(command.c_str());
    }


}

inline void FCMMultihopController::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
}

inline double FCMMultihopController::getMeanN_tx()
{
  return mean_N_tx;
}

inline double FCMMultihopController::getMeanP_fail()
{
  return mean_P_fail;
}

inline double FCMMultihopController::getMeanN_rounds()
{
  return mean_N_rounds;  
}  

inline double FCMMultihopController::getMeanResolved_Radius()
{
  return mean_Resolved_Radius;  
}

inline double FCMMultihopController::getSourceN_tx()
{
  return source_N_tx;
}

inline double FCMMultihopController::getSourceP_fail()
{
  return source_P_fail;
}

inline double FCMMultihopController::getSourceN_rounds()
{
  return source_N_rounds; 
}

inline double FCMMultihopController::getSourceResolved_Radius()
{
  return source_Resolved_Radius;
}

inline double FCMMultihopController::getMeanTierResolved_Radius()
{
  return mean_tier_Resolved_Radius;
}

inline double FCMMultihopController::getMeanNodes_Covered()
{
  return mean_Nodes_covered;
}

inline double FCMMultihopController::getSourceNodes_Covered()
{
  return source_Nodes_covered;
}

inline bool FCMMultihopController::isConnected()
{
  return topology_connected;
}


