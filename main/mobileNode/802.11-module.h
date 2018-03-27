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

#ifndef _80211Module_
#define _80211Module_

#include "module.h"
#include "mac.h"
#include "queue.h"

class MacModule802_11;

class MacInterface : public NsObject
{
	public:
		MacInterface();
		~MacInterface();
		virtual void recv(Packet *p, const char *why);
		virtual void recv(Packet *p, Handler *h);
		virtual void recv(Packet *p);
		void setModule(MacModule802_11 *m);
	protected:
		MacModule802_11 *module_;
};

class LogInterface : public NsObject
{
	public:
		LogInterface();
		~LogInterface();
		virtual void recv(Packet *p, const char *why);
		virtual void recv(Packet *p, Handler *h);
		virtual void recv(Packet *p);
		void setModule(MacModule802_11 *m);
	protected:
		MacModule802_11 *module_;
};


class LLInterface : public NsObject
{
	public:
		LLInterface();
		~LLInterface();
		virtual void recv(Packet *p, Handler *h);
		virtual void recv(Packet *p);
		void setModule(MacModule802_11 *m);
	protected:
		MacModule802_11 *module_;
};

class MacModule802_11 : public Module
{
	friend class MacInterface;
	friend class LogInterface;
	friend class LLInterface;
	public:
		MacModule802_11();
		virtual ~MacModule802_11();
		virtual void recv(Packet*, Handler* callback);
		virtual void recv(Packet *p);
		virtual int command(int argc, const char*const* argv);
	protected:
		Mac *mac_;
		Queue *queue_;
		LL *ll_;
		MacInterface *macif_;
		LogInterface *logif_;
		LLInterface *llif_;
};

#endif
