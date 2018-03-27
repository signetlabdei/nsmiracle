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
 * @file   mmac-aloha.cc
 * @author Baldo Nicola
 * @date   Mon Nov 26 16:50:01 2007
 * 
 * @brief  
 * 
 * 
 */


#include"mmac-aloha.h"

#include<iostream>



static class MMacAlohaClass : public TclClass {
public:
  MMacAlohaClass() : TclClass("Module/MMac/ALOHA") {}
  TclObject* create(int, const char*const*) {
    return (new MMacAloha);
  }
} class_MMacAloha;





MMacAloha::MMacAloha()
  : TxActive(false)
{
  
}

void MMacAloha::recvFromUpperLayers(Packet* p)
{  

  if (TxActive)
    {
      Q.push(p);
      if (debug_) 
	cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
	     << " TxActive => enqueueing packet" << endl;
    }
  else
    {
      Mac2PhyStartTx(p); 
      TxActive = true;
      if (debug_) 
	cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
	     << " transmitting packet" << endl;
      
    }
}

void MMacAloha::Phy2MacEndTx(const Packet* p)
{

  TxActive = false;

  // we can send something else if we have it
  if (! Q.empty())
    {
      if (debug_) 
	cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
	     << "Q.size()=" << Q.size() << ", transmitting packet"  << endl;
      
      Packet* p2;
      p2 = Q.front();
      Q.pop();
      Mac2PhyStartTx(p2);
      TxActive = true;
    }
  else
    {
      if (debug_) 
	cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
	     << "Q.size()= 0" << endl;
    }
}

void MMacAloha::Phy2MacStartRx(const Packet* p)
{
  if (debug_) 
    cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ << endl;
}


void MMacAloha::Phy2MacEndRx(Packet* p)
{
  if (debug_) 
    cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ << endl;
  hdr_cmn* ch = HDR_CMN(p);

  if (ch->error())
    drop(p, 1, "ERR");
  else
    sendUp(p);
}
