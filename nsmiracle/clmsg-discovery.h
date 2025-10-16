/* -*- Mode:C++ -*- */

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
 * @file   clmsg-discovery.h
 * @author Federico Guerra
 * @date   Thu Oct 2 14:23:09 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef CLMSG_DISCOVERY_H
#define CLMSG_DISCOVERY_H

#include <clmessage.h>
#include <map>
#include <list>
#include <set>
#include <vector>
#include <packet.h>
#include <string>

#define CLMSG_DISCOVERY_VERBOSITY 3

extern ClMessage_t CLMSG_DISCOVERY;

class DiscoveryData;
class ClSAP;
class PlugIn;

// map < module_id, data_collected>
typedef map<int, DiscoveryData> DBId;
// lookup tables:
// const plugin* --> id
typedef map< const PlugIn* , int > DBPointer2Id;
// layer_id , id
typedef map< int , set<int> > DBLayer2Id;
// stack_id , id
typedef map< int , set<int> > DBStack2Id;
// miracle_tag , id
typedef map< string , set<int> > DBTag2Id;
// tcl_name , id
typedef map< string , set<int> > DBName2Id;

typedef set<int>::iterator SetIt;
typedef set<int>::reverse_iterator setRit;
typedef DBPointer2Id::iterator DBPointer2IdIt;
typedef DBPointer2Id::reverse_iterator DBPointer2IdRit;
typedef DBLayer2Id::iterator DBLayer2IdIt;
typedef DBLayer2Id::reverse_iterator DBLayer2IdRit;
typedef DBStack2Id::iterator DBStack2IdIt;
typedef DBStack2Id::reverse_iterator DBStack2IdRit;
typedef DBTag2Id::iterator DBTag2IdIt;
typedef DBTag2Id::reverse_iterator DBTag2IdRit;
typedef DBName2Id::iterator DBName2IdIt;
typedef DBName2Id::reverse_iterator DBName2IdRit;
typedef DBId::iterator DBIt;
typedef DBId::reverse_iterator DBRit;

typedef vector<DiscoveryData>::iterator VDDIt;
typedef vector<DiscoveryData>::reverse_iterator VDDRit;

class DiscoveryData {

public:

  DiscoveryData(const PlugIn* pointer, int layer_id, int stack_id, int id, const char* tcl_name, const char* miracle_tag);
  DiscoveryData();
  DiscoveryData(const DiscoveryData& DData2copy);
  void reset();

  void setPointer(const PlugIn* pointer) { module_pointer = pointer; }
  const PlugIn* getPointer() { return module_pointer; }

  void setStackId(int stack_id) { module_stack_id = stack_id; }
  int getStackId() { return module_stack_id; }

  void setLayer(int layer_id) { layer_id = module_layer_id; }
  int getLayer() { return module_layer_id; }

  void setId(int id) { module_id = id; }
  int getId() { return module_id; }
  
  void setTclName(const char* tcl_name) { module_tcl_name.clear(); module_tcl_name = tcl_name; }
  const char* getTclName() { return module_tcl_name.c_str(); }
  
  void setMiracleTag(const char* miracle_tag) { module_miracle_tag.clear(); module_miracle_tag = miracle_tag;} 
  const char* getMiracleTag() { return module_miracle_tag.c_str(); }
 
  void setValid() { is_valid = true;}
  void setNotValid() {is_valid = false;}

private:
 
  const PlugIn* module_pointer;
  int module_layer_id;
  int module_stack_id;
  int module_id;
  string module_tcl_name;
  string module_miracle_tag;
 
  bool is_valid;
};


class DiscoveryStorage 
{

public:

  DiscoveryStorage();
//   ~DiscoveryStorage();

  int getDataSize();
  void addEntry(const PlugIn* pointer, int layer_id, int stack_id, int id, const char* tcl_name, const char* miracle_tag);
  void addEntry(DiscoveryData data);
  void removeEntry(int id);
  
  void printData();
  int getSize() {return db_id.size();}

  DiscoveryStorage findLayer(int layer_id);
  DiscoveryStorage findStack(int stack_id);
  DiscoveryStorage findTag(const char* tag); 
  DiscoveryStorage findTclName(const char* tcl_name); 
  DiscoveryData findPointer(const PlugIn* pointer);
  DBIt begin() {return db_id.begin(); }
  DBIt end() {return db_id.end(); }

private:

  DBId db_id; 
  DBLayer2Id db_layer;
  DBStack2Id db_stack;
  DBTag2Id db_tag;
  DBName2Id db_name;
  DBPointer2Id db_pointer;

  void reset();
};


class ClMsgDiscovery : public ClMessage
{

public:

  ClMsgDiscovery();
  ClMessage* copy();	// copy the message

  DiscoveryStorage copyStorage();
  void addData(const PlugIn* pointer, int layer_id, int stack_id, int id, const char* tcl_name, const char* miracle_tag);
  void addSenderData(const PlugIn* pointer, int layer_id, int stack_id, int id, const char* tcl_name, const char* miracle_tag);
  void removeData(int id);  
  void removeSenderData();
  void printReplyData();

  int getDBSize();

  DiscoveryData getSenderData();

  DiscoveryStorage findLayer(int layer_id ); 
  DiscoveryStorage findStack(int stack_id );
  DiscoveryStorage findTag(const char* tag); 
  DiscoveryStorage findTclName(const char* tcl_name); 
  DiscoveryData findPointer(const PlugIn* pointer);

	
private:

  DiscoveryStorage reply_storage;
  DiscoveryData sender_data;
};


#endif /* CLMSG_DISCOVERY_H */


