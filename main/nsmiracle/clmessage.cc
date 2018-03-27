/*
 * Copyright (c) 2006 Regents of the SIGNET lab, University of Padova.
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
 
#include "clmessage.h"
#include <iostream>

#include"deprecated.h"

/*
ClMessage static method: it is used to register a new clMessage
*/

ClMessage_t ClMessage::nType_ = 0;

ClMessage_t ClMessage::addClMessage()
{
	return nType_++;
}

static unsigned int lastId = 0;

/*
------------------------------------------------
*/

ClMessage::ClMessage(int verbosity, ClMessage_t type) 
: verbosity_(verbosity), 
  type_(type), 
  destType_(BROADCAST), 
  source_(0), 
  dest_(CLBROADCASTADDR) 
{
	uid_ = lastId++;
}

ClMessage::ClMessage(int verbosity, ClMessage_t type, DestinationType dtype, int source, int value)
: verbosity_(verbosity), 
  type_(type), 
  source_(source), 
  destType_(dtype), 
  dest_(value) 
{
  PRINT_WARNING_DEPRECATED_FUNCTION;
  uid_ = lastId++;
}

ClMessage::ClMessage(int verbosity, ClMessage_t type, DestinationType dtype, int value)
: verbosity_(verbosity), 
  type_(type), 
  source_(-1), 
  destType_(dtype), 
  dest_(value) 
{
	uid_ = lastId++;
}


ClMessage::ClMessage(ClMessage *m)
{
	type_ = m->type_;
	destType_ = m-> destType_;
	source_ = m->source_;
	dest_ = m->dest_;
	verbosity_ = m->verbosity_;
	uid_ = m->uid_;
}

ClMessage::~ClMessage()
{
}

// return the destination type of the message
DestinationType ClMessage::getDestType()
{
	return destType_;
}

// return the destination of the message
int ClMessage::getDest()
{
	return (dest_);
}

// retrun a copy of the message
ClMessage *ClMessage::copy()
{
  cerr << "FATAL: ClMessage::copy() method was not re-implemented by some "
       << "class derived from ClMessage. This would likely result in data "
       << "loss. Please implement this method. " 
       << endl << " Exiting..." << endl;
  exit(1);
}

int ClMessage::verbosity()
{
	return verbosity_;
}

ClMessage_t ClMessage::type()
{
	return type_;
}

ClDirection ClMessage::direction()
{
	return direction_;
}

void ClMessage::direction(ClDirection dir)
{
	direction_ = dir;
}

int ClMessage::getSource()
{
	return source_;
}

void ClMessage::setSource(int src)
{
	source_ = src;
}


void ClMessage::setDest(int dst)
{
  dest_ = dst;
}


unsigned int ClMessage::uid()
{
	return uid_;
}
