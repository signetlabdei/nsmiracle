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
 * @file   mmac-aloha.h
 * @author Baldo Nicola
 * @date   Mon Nov 26 16:42:41 2007
 * 
 * @brief  
 * 
 * 
 */


#ifndef MMAC_ALOHA_H 
#define MMAC_ALOHA_H

#include<module.h>
#include<mmac.h>
#include<queue>

/**
 *
 * Very very basic ALOHA implementation. 
 * No backoff, no ACK, no retransmission, infinite queue.
 *
 */
class MMacAloha : public MMac
{
public: 
  MMacAloha();
  
protected:
  
  virtual void recvFromUpperLayers(Packet* p);

  virtual void Phy2MacEndTx(const Packet* p);

  virtual void Phy2MacStartRx(const Packet* p);
  virtual void Phy2MacEndRx(Packet* p);

  std::queue<Packet*> Q;   /// MAC queue used for packet scheduling 
  bool TxActive;
  
};




#endif /* MMAC_ALOHA_H */
