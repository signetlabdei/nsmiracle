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

#include <clsap.h>
#include<iostream>

#include "sandbox-clmsg.h"


ClMessage_t CLMSG_DISC_ASYNC = 0;		// asynchronous discovery 
ClMessage_t CLMSG_DISC_REPLY = 0;		// reply to asynchronous discovery 
ClMessage_t CLMSG_SET_SIZE   = 0;		// set size command
ClMessage_t CLMSG_DISC_SYNC = 0;		// synchronous discovery (no reply needed!)


/*-------------------------------------------------------------------------------------------------------
	methods for ClMsgDiscAsync class
---------------------------------------------------------------------------------------------------------*/ 

ClMsgDiscAsync::ClMsgDiscAsync() 
  : ClMessage(SANDBOX_CLMSG_VERBOSITY, CLMSG_DISC_ASYNC) 
{

}

ClMsgDiscAsync::ClMsgDiscAsync(ClMsgDiscAsync *m) : ClMessage(m)
{
}

// retrun a copy of the message
ClMessage *ClMsgDiscAsync::copy()
{
  //cerr <<  __PRETTY_FUNCTION__ << " Copying message" << endl;
  ClMessage *temp = new ClMsgDiscAsync(this);
  return (temp);
}


/*-------------------------------------------------------------------------------------------------------
	methods for ClMsgDiscReply class
---------------------------------------------------------------------------------------------------------*/ 

ClMsgDiscReply::ClMsgDiscReply(int dst, const char* name) 
  : ClMessage(SANDBOX_CLMSG_VERBOSITY, CLMSG_DISC_REPLY, UNICAST, dst) 
{
  strncpy(modname, name, MODNAME_STRLEN-1);
  modname[MODNAME_STRLEN-1] = '\0'; // strncpy does not null-terminate strings
}


ClMsgDiscReply::ClMsgDiscReply(ClMsgDiscReply *m) : ClMessage(m)
{
  //cerr <<  __PRETTY_FUNCTION__ << " Strncpying name " << endl;
  strncpy(modname, m->modname, MODNAME_STRLEN-1);
  //cerr <<  __PRETTY_FUNCTION__ << " Null-terminating name" << endl;
  modname[MODNAME_STRLEN-1] = '\0'; // strncpy does not null-terminate strings
}


// retrun a copy of the message
ClMessage *ClMsgDiscReply::copy()
{
  //cerr <<  __PRETTY_FUNCTION__ << " Copying message" << endl;
  ClMessage *temp = new ClMsgDiscReply(this);
  return (temp);
}



/*-------------------------------------------------------------------------------------------------------
	methods for ClMsgSet class
---------------------------------------------------------------------------------------------------------*/ 


ClMsgSet::ClMsgSet(ClMsgSet *m) 
  : ClMessage(m)
{
  newSize_ = m->newSize_;
}



ClMsgSet::ClMsgSet(int dst) 
  : ClMessage(SANDBOX_CLMSG_VERBOSITY, CLMSG_SET_SIZE, UNICAST,dst) 
{

}


// return a copy of the message
ClMessage *ClMsgSet::copy()
{
	ClMessage *temp = new ClMsgSet(this);
	return (temp);
}


int ClMsgSet::getSize()
{
	return (newSize_);
}

void ClMsgSet::setSize(int size)
{
	newSize_ = size;
}






/*-------------------------------------------------------------------------------------------------------
	methods for ClMsgDiscSync class
---------------------------------------------------------------------------------------------------------*/ 

modinfo::modinfo(int mid, const char*  mname)
{
  id = mid;
  strncpy(name, mname, MODNAME_STRLEN-1);
  name[MODNAME_STRLEN-1] = '\0'; // strncpy does not null-terminate strings
}

int modinfo::getId()
{
  return id;
}


const char* modinfo::getName()
{
  return name;
}



ClMsgDiscSync::ClMsgDiscSync() 
  : ClMessage(SANDBOX_CLMSG_VERBOSITY, CLMSG_DISC_SYNC) 
{

}

ClMsgDiscSync::ClMsgDiscSync(ClMsgDiscSync *m) 
  : ClMessage(m)
{
  //  This is designed only to be a synchronous message, it should not be copied!!!
#define TryingToCopySynchronousMessage 1
  assert(!TryingToCopySynchronousMessage);
}

// retrun a copy of the message
ClMessage *ClMsgDiscSync::copy()
{
	ClMsgDiscSync *temp = new ClMsgDiscSync(this);
	return (temp);
}


void ClMsgDiscSync::addModule(int id, const char* name)
{
  modinfo m(id, name);
  modinfo_list.push_back(m);
}


void ClMsgDiscSync::printDiscoveredModules()
{
  list<modinfo>::iterator iter = modinfo_list.begin();

  if (iter ==  modinfo_list.end())
    {
      cout << "No modules/plugin discovered" << endl;
      return;
    }

  cerr << "Discovered modules/plugins: " << endl;
  while (iter != modinfo_list.end())
    {
      cout << "ID: " << iter->getId() << " Name: " << iter->getName() << endl;
      iter++;
    }  
}

