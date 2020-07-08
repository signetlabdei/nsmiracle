/* Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 *  split clmsg functionality from mmac-module.cc 92 2008-03-25 19:35:54Z emil $
 * 
 */

#include"mmac-clmsg.h"

/**
 * Message to get the address of the mac module
 */
MacClMsgGetAddr::MacClMsgGetAddr() : ClMessage(MAC_CLMSG_VERBOSITY, MAC_CLMSG_GET_ADDR), addr_(0) {}
MacClMsgGetAddr::MacClMsgGetAddr(DestinationType dtype, int value) :
	ClMessage(MAC_CLMSG_VERBOSITY, MAC_CLMSG_GET_ADDR, dtype, value), 
	addr_(0)
{}

MacClMsgGetAddr::MacClMsgGetAddr(MacClMsgGetAddr *m) : ClMessage(m), addr_(m->getAddr()) {}

void MacClMsgGetAddr::setAddr(int addr)
{
	addr_ = addr;
}

int MacClMsgGetAddr::getAddr()
{
	return(addr_);
}

ClMessage *MacClMsgGetAddr::copy()
{
	return (new MacClMsgGetAddr(this));
}

