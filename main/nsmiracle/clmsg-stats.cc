/*
 * Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
 * @file clmsg-stats.cc
 * @author Federico Guerra, Filippo Campagnaro
 * @date 18-9-15
 * @version 1.0.0
 * 
 * \brief Provides the description of Stats class
 * Class container of the stats metrics of each layer.
 * This class has to be extended in order to specialize its use.
 */


#include "clmsg-stats.h"
#include <cassert>

ClMessage_t CLMSG_STATS;

Stats::Stats() : 
  module_id(0), 
  stack_id(0), 
  type_id(STATS_TYPE_ID_UNKNOWN_TYPE) 
{ 

}
 
 
Stats::Stats(int mod_id, int stck_id) : 
  module_id(mod_id), 
  stack_id(stck_id), 
  type_id(STATS_TYPE_ID_UNKNOWN_TYPE) 
{ 

}


Stats::~Stats()
{

}


Stats* Stats::clone() const 
{
  return new Stats( *this ); 
}


bool Stats::isValid() const
{
  return (type_id != STATS_TYPE_ID_UNKNOWN_TYPE);
}


ClMsgStats::ClMsgStats() : 
  ClMessage(CLMSG_BASE_STATS_VERBOSITY, CLMSG_STATS),
  stats_ptr(NULL)
{

}

ClMsgStats::ClMsgStats(int destId, DestinationType dtype) : 
  ClMessage(CLMSG_BASE_STATS_VERBOSITY, CLMSG_STATS, dtype, destId),
  stats_ptr(NULL)
{

}


ClMsgStats::~ClMsgStats()
{
  if (stats_ptr != NULL)
  {
    delete stats_ptr;
    stats_ptr = NULL;
  }
}


ClMessage* ClMsgStats::copy()
{
  // Supporting only synchronous messages!!!
  assert(0);
}


void ClMsgStats::setStats(const Stats* const in_stats)
{
  
  if (stats_ptr != NULL)
  {
    delete stats_ptr;
  }
  
  if (in_stats != NULL)
  {
    stats_ptr = in_stats->clone();
  }
  else
  {
    stats_ptr = NULL;
  }
}


const Stats* const ClMsgStats::getStats() const
{
  static Stats not_valid_stats = Stats();
  
  if (stats_ptr != NULL)
  {
    return stats_ptr;
  }

  return &not_valid_stats;
}


