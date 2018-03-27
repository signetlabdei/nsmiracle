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


#ifndef LINK_CHANNEL_H
#define LINK_CHANNEL_H


#include<channel-module.h>


#define LINKCH_DROP_REASON_ERROR "ERR"
#define LINKCH_DROP_REASON_WRONG_DIRECTION "WDIR"

/**
 * @file   link_channel.h
 * @author Baldo Nicola
 * @date   Thu Oct 26 16:38:44 2006
 * 
 * @brief  interface definition for wrapping a ns Link into miracle's Channel
 * 
 * 
 */

class ChSap;
class DropInterface;

/**
 * A wrapper for standard ns Link
 */
class LinkChannelModule : public ChannelModule {
  friend class DropInterface;
public:
  LinkChannelModule();
  virtual ~LinkChannelModule();

  /**
   * TCL command interpreter. It implements the following OTcl methods:
   * <ul>
   *  <li><b>setSimplexLink &lt;<i>SimplexLink instance</i>&gt;</b>: 
   *  	wraps the provided ns SimpleLink instance into the SimpleLinkChannelModule wrapper 
   *  <li><b>getSimplexLink &lt;<i>SimplexLink instance</i>&gt;</b>: 
   *  	returns the wrapped ns SimpleLink instance 
   * </ul>
   * 
   * 
   * @param argc number of arguments in <i>argv</i>
   * @param argv array of strings which are the comand parameters (Note that argv[0] is the name of the object)
   * 
   * @return TCL_OK or TCL_ERROR whether the command has been dispatched succesfully or no
   * 
   * @see ChannelModule, Module, PlugIn
   **/
  virtual int command(int argc, const char*const* argv);


  /** 
   * This method is invoked when a packet is passed by an upper ChSap to this module
   * 
   * @param p pointer to the packet packet
   * @param c pointer to the ChSap which passed the packet
   */
  virtual void recv(Packet* p, ChSAP* cs);


  /** 
   * This method is used by NsObjects wrapped inside the LinkChannel class
   * to pass packets to it. It should NOT be used by ChSAPs
   * 
   * @param p packet
   * @param h handler
   */
   virtual void recv(Packet* p, Handler* h);



  //  virtual void recv(Packet* p, SAP* s) { recv(p,(ChSAP*) s);}

protected:
  NsObject* linkhead_; 
  ChSAP* srcsap;
  ChSAP* dstsap;

};

#endif /*DELAY_BANDWIDTH_CHANNEL_H*/
