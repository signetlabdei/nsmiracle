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
 * @file clmsg-stats.h
 * @author Federico Guerra, Filippo Campagnaro
 * @date 18-9-15
 * @version 1.0.0
 * 
 * \brief Provides the description of Stats class
 * Class container of the stats metrics of each layer.
 * This class has to be extended in order to specialize its use.
 */

#ifndef _CLMSG_STATS_
#define _CLMSG_STATS_

#include <clmessage.h>


#define CLMSG_BASE_STATS_VERBOSITY (3)

#define STATS_TYPE_ID_UNKNOWN_TYPE (-1)  /**< The stats type is unknown */


extern ClMessage_t CLMSG_STATS;


class Stats {
public:

  /**
    * Constructor of the Stats class
  **/
  Stats();
 
  /**
    * Constructor of the Stats class
  **/
  Stats(int mod_id, int stk_id);
 
  /**
    * Destructor of the Stats class
  **/
  virtual ~Stats();
   
  /**
    * Virtual method used by the Module class in order to copy its stats an a generic fashion,
    * without the need to know the derived stats implementation.
    * @return the copy of a module the stats.
  **/

  virtual Stats* clone() const;
  
  
  virtual bool isValid() const;

  
  int module_id; /**< Identifier of the module which this stats is related */

  int stack_id; /**< Stack identifier. This is useful in the case of multistack networks */
 
  int type_id; /**< Type of module identifier. Miracle DOES NOT define the enum, letting the user 
                 * (i.e. DESERT code) decide the meaning of this in its implementation. 
                 * Example 0 = PHY, 1 = MAC, 2 = IP_ROUTING etc.. The only defined value 
                 * is -1 ⇒ unknown type 
               **/

}; 

class ClMsgStats : public ClMessage
{

public:

  /**
  * Constructor of the ClMsgStats class
  **/
  ClMsgStats();
  
  /**
  * Constructor of the ClMsgStats class
  * @param destId destination ID, if unicast = module id, if broadcast = stack position of the layer
  * @param dtype define the type of the destination (UNICAST or BROADCAST)
  **/
  ClMsgStats(int destId, DestinationType dtype = UNICAST);

  /**
    * Destructor of the ClMsgStats class
  **/
  virtual ~ClMsgStats();
  
  /**
    * Copy method of the ClMessage class
  **/
  virtual ClMessage* copy();	
  
  /**
    * Virtual method used by the Plugin class in order to copy its stats an a generic fashion,
    * without the need to know the derived stats implementation. 
    * It automatically calls clone() on the input Stat object in order to perform the copy. 
    * The underlying pointer is freed first if != NULL
    * @param in_stats constant pointer to a constant a Stats object
  **/
  void setStats(const Stats* const in_stats);

  /**
    * Virtual method used by the Plugin class in order to copy its stats an a generic fashion,
    * without the need to know the derived stats implementation.
    * @return constant reference to a Stats object. The object will fail the isValid() test if the 
    * underlying pointer is NULL.
  **/
  const Stats* const getStats() const;
  
	
private:

  /**
  * Stats pointer, dynamically allocated by method setStats
  **/
  Stats* stats_ptr;
  
};



#endif // _CLMSG_STATS_
