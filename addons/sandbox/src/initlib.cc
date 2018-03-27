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


#include <tclcl.h>
#include <clmessage.h>
#include<clsap.h>

#include"sandbox-clmsg.h"
#include"sandbox-plugin1.h"
#include"sandbox-clmsg-tracer.h"
#include<iostream>

extern EmbeddedTcl SandboxTclCode;

extern "C" int Miraclesandbox_Init()
{
  /*
   * Put here all the commands which must be execute when the library
   * is loaded (i.e. TCL script execution)  
   * Remember to return 0 if all is OK, otherwise return 1
  */

	CLMSG_DISC_ASYNC = ClMessage::addClMessage();
	CLMSG_DISC_REPLY = ClMessage::addClMessage();
	CLMSG_SET_SIZE = ClMessage::addClMessage();
	CLMSG_DISC_SYNC = ClMessage::addClMessage();
	
	ClSAP::addTracer(new ClMsgDiscAsyncTracer);
	ClSAP::addTracer(new ClMsgDiscReplyTracer);
	ClSAP::addTracer(new ClMsgSetTracer);

	SandboxTclCode.load();

	cerr << "Executed Miraclesandbox_Init()" << endl;
  return 0;
}


extern "C" int Cygmiraclesandbox_Init()
{
  return Miraclesandbox_Init();
}

