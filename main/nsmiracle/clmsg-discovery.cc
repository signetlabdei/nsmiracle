
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


/**
 * @file   clmsg-phy-interrupt.cc
 * @author Federico Guerra
 * @date   Mon Nov 26 14:59:38 2007
 * 
 * @brief  
 * 
 * 
 */



#include "clmsg-discovery.h"

#include <iostream>

ClMessage_t CLMSG_DISCOVERY;

DiscoveryData::DiscoveryData(const PlugIn* pointer, int layer_id, int stack_id, int id, const char* tcl_name, const char* miracle_tag)
{
  module_pointer = pointer;
  module_layer_id = layer_id; 
  module_stack_id = stack_id;
  module_id = id;
  module_tcl_name = tcl_name;
  module_miracle_tag = miracle_tag;
  is_valid = true;
}

DiscoveryData::DiscoveryData() 
{ 
  DiscoveryData(0, 0, 0, 0, "NULL", "NULL");
  is_valid = false;
}

DiscoveryData::DiscoveryData(const DiscoveryData& copy)
{
  module_pointer = copy.module_pointer;
  module_layer_id = copy.module_layer_id; 
  module_stack_id = copy.module_stack_id;
  module_id = copy.module_id;
  module_tcl_name = copy.module_tcl_name;
  module_miracle_tag = copy.module_miracle_tag;
  is_valid = copy.is_valid;
}

void DiscoveryData::reset()
{
  module_pointer = 0;
  module_layer_id = 0; 
  module_stack_id = 0;
  module_id = 0;
  module_tcl_name.clear();
  module_miracle_tag.clear();
  is_valid = false;
}

DiscoveryStorage::DiscoveryStorage()
{ 
  reset();
}

int DiscoveryStorage::getDataSize()
{
 return db_id.size();
}

void DiscoveryStorage::reset()
{
  db_id.clear();
  db_layer.clear();
  db_stack.clear();
  db_tag.clear();
  db_name.clear();
  db_pointer.clear();
}

void DiscoveryStorage::addEntry(const PlugIn* pointer, int layer_id, int stack_id, int id, const char* tcl_name, 
                                      						const char* miracle_tag)
{
  string name_s = tcl_name;
  string tag_s = miracle_tag;

  db_id[id] = DiscoveryData(pointer, layer_id, stack_id, id, tcl_name, miracle_tag);

  db_layer[layer_id].insert(id);
  db_stack[stack_id].insert(id);
  db_tag[tag_s].insert(id);
  db_name[name_s].insert(id);
  db_pointer[pointer] = id;
}

void DiscoveryStorage::addEntry(DiscoveryData data) 
{
  string name_s = data.getTclName();
  string tag_s = data.getMiracleTag();

  db_id[data.getId()] = data;
  db_layer[data.getLayer()].insert(data.getId());
  db_stack[data.getStackId()].insert(data.getId());
  db_tag[tag_s].insert(data.getId());
  db_name[name_s].insert(data.getId());
  db_pointer[data.getPointer()] = data.getId();
}

void DiscoveryStorage::removeEntry(int id)
{
  // erase all tables
  string name_s = db_id[id].getTclName();
  string tag_s = db_id[id].getMiracleTag();

  db_layer[db_id[id].getLayer()].erase(id);
  db_stack[db_id[id].getStackId()].erase(id);
  db_tag[tag_s].erase(id);
  db_name[name_s].erase(id);
  db_pointer.erase( db_id[id].getPointer() );
  db_id.erase(id);
}

DiscoveryStorage DiscoveryStorage::findLayer(int layer_id)
{
  DiscoveryStorage result_query;
  // if no match found we return a empty storage
  if ( db_layer.find(layer_id) != db_layer.end() ) {
    for ( SetIt iter = db_layer[layer_id].begin(); iter != db_layer[layer_id].end(); iter++ ) {
        // inserting DiscoveryData found into reply
        result_query.addEntry(db_id[*iter]);
    }
  }
  return result_query;
}

DiscoveryStorage DiscoveryStorage::findStack(int stack_id)
{
  DiscoveryStorage result_query;
  // if no match found we return a empty storage
  if ( db_stack.find(stack_id) != db_stack.end() ) {
    for ( SetIt iter = db_stack[stack_id].begin(); iter != db_stack[stack_id].end(); iter++ ) {
        // inserting DiscoveryData found into reply
        result_query.addEntry(db_id[*iter]);
    }
  }
  return result_query;
}

DiscoveryStorage DiscoveryStorage::findTag(const char* tag)
{
  string tag_s = tag;

  DiscoveryStorage result_query;
  // if no match found we return a empty storage
  if ( db_tag.find(tag_s) != db_tag.end() ) {
    for ( SetIt iter = db_tag[tag_s].begin(); iter != db_tag[tag_s].end(); iter++ ) {
        // inserting DiscoveryData found into reply
        result_query.addEntry(db_id[*iter]);
    }
  }
  return result_query;
}

DiscoveryStorage DiscoveryStorage::findTclName(const char* tcl_name)
{
  string name_s = tcl_name;

  DiscoveryStorage result_query;
  // if no match found we return a empty storage
  if ( db_name.find(name_s) != db_name.end() ) {
    for ( SetIt iter = db_name[name_s].begin(); iter != db_name[name_s].end(); iter++ ) {
        // inserting DiscoveryData found into reply
        result_query.addEntry(db_id[*iter]);
    }
  }
  return result_query;
}

DiscoveryData DiscoveryStorage::findPointer(const PlugIn* pointer)
{
  DiscoveryData result_query;
  result_query.reset();
  // if no match found we return a empty DiscoveryData
  if ( db_pointer.find(pointer) == db_pointer.end() ) return result_query;
  else return( result_query = db_id[ db_pointer[pointer] ] );
}

void DiscoveryStorage::printData()
{
  cout << "Storage size = " << getSize() << endl;

  for (DBIt it = db_id.begin(); it != db_id.end(); it++) {
 
           int id = (*it).first;
      cout << "Module id = " << id << endl 
           << "Module layer = " << db_id[id].getLayer() << endl
           << "Module stack id = " << db_id[id].getStackId() << endl
           << "Module tcl name = " << db_id[id].getTclName() << endl 
           << "Module miracle tag = " << db_id[id].getMiracleTag() << endl
           << "Module pointer = " << hex << db_id[id].getPointer() << endl << endl;
  }
}


ClMsgDiscovery::ClMsgDiscovery()
  : ClMessage(CLMSG_DISCOVERY_VERBOSITY, CLMSG_DISCOVERY)
{
  sender_data.reset();
}

ClMessage* ClMsgDiscovery::copy()
{
  // Supporting only synchronous messages!!!
  assert(0);
}

void ClMsgDiscovery::addData(const PlugIn* pointer, int layer_id, int stack_id, int id, const char* tcl_name, 
                                                                                   const char* miracle_tag)
{
  reply_storage.addEntry(pointer, layer_id, stack_id, id, tcl_name, miracle_tag);
}

void ClMsgDiscovery::addSenderData(const PlugIn* pointer, int layer_id, int stack_id, int id, const char* tcl_name, const char* miracle_tag)
{
  sender_data.setPointer(pointer);
  sender_data.setLayer(layer_id);
  sender_data.setStackId(stack_id);
  sender_data.setId(id);
  sender_data.setTclName(tcl_name);
  sender_data.setMiracleTag(miracle_tag);
  sender_data.setValid();
}

void ClMsgDiscovery::removeData(int id)
{
  reply_storage.removeEntry(id);
}

void ClMsgDiscovery::removeSenderData()
{
  sender_data.reset();
}

DiscoveryData ClMsgDiscovery::getSenderData()
{
  return(sender_data);
}

DiscoveryStorage ClMsgDiscovery::findLayer(int layer_id)
{
  return(reply_storage.findLayer(layer_id));
}

DiscoveryStorage ClMsgDiscovery::findStack(int stack_id)
{
  return(reply_storage.findStack(stack_id));
}

DiscoveryStorage ClMsgDiscovery::findTag(const char* tag)
{
  return(reply_storage.findTag(tag));
}

DiscoveryStorage ClMsgDiscovery::findTclName(const char* tcl_name)
{
  return(reply_storage.findTclName(tcl_name));
}

 
DiscoveryData ClMsgDiscovery::findPointer(const PlugIn* pointer)
{
  return(reply_storage.findPointer(pointer));
}

DiscoveryStorage ClMsgDiscovery::copyStorage()
{
   reply_storage.addEntry(sender_data.getPointer(), sender_data.getLayer(), sender_data.getStackId(), sender_data.getId(), sender_data.getTclName()
                                                                        , sender_data.getMiracleTag());
   return reply_storage;
}

void ClMsgDiscovery::printReplyData()
{
  reply_storage.printData();
}

int ClMsgDiscovery::getDBSize()
{
  return(reply_storage.getSize());
}
