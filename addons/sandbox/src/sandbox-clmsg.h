/* -*- Mode:C++ -*- */

//
// Copyright (c) 2006 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the 
//    names of its contributors may be used to endorse or promote products 
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//


#ifndef _CLMSGSANDBOXPLG1_
#define _CLMSGSANDBOXPLG1_

#include <clmessage.h>
#include <cltracer.h>
#include<list>

#define SANDBOX_CLMSG_VERBOSITY 1

#define MODNAME_STRLEN 20

extern ClMessage_t CLMSG_DISC_ASYNC;
extern ClMessage_t CLMSG_DISC_REPLY;
extern ClMessage_t CLMSG_SET_SIZE;
extern ClMessage_t CLMSG_DISC_SYNC;

class ClSAP;



/**
 * This class provides a broadcast message which implements an
 * asynchronous module discovery mechanism
 * 
 */
class ClMsgDiscAsync : public ClMessage
{
	public:
		ClMsgDiscAsync();
		ClMsgDiscAsync(ClMsgDiscAsync *m);		
		ClMessage* copy();	// copy the message		
	
};



/**
 * This class provides  a unicast message which is to be sent in
 * reply to a ClMsgDiscAsync message
 * 
 */
class ClMsgDiscReply : public ClMessage
{
	public:
                ClMsgDiscReply(int dst, const char* name);
		ClMsgDiscReply(ClMsgDiscReply *m);		
		ClMessage* copy();	// copy the message
                const char* getModName() {return modname;}
	
	protected:
                char modname[MODNAME_STRLEN];
};



/**
 * this class implements a synchronous cross-layer command with aims
 * at changing the packet size being used by some module.
 * 
 */
class ClMsgSet : public ClMessage
{
	public:
                ClMsgSet(int dst);
		ClMsgSet(ClMsgSet *m);
		
		ClMessage* copy();	// copy the message
		
		int getSize();		// get the size
		void setSize(int size);		// set the size
		
	private:
	
		int newSize_;
};






class modinfo  
{
public: 
  modinfo(int mid, const char*  mname);
  int getId();
  const char* getName();

protected:
  int id;
  char name[MODNAME_STRLEN];

};


/**
 * This class provides a broadcast message which implements a
 * synchronous module discovery mechanism.
 * 
 */
class ClMsgDiscSync : public ClMessage
{
public:
  ClMsgDiscSync();
  ClMsgDiscSync(ClMsgDiscSync *m);		
  ClMessage* copy();	// copy the message
	
  void addModule(int id, const char* name);
  void printDiscoveredModules();

protected:

  list<modinfo> modinfo_list;
	
};



#endif
