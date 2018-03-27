
//
// Copyright (c) 2007 Regents of the SIGNET lab, University of Padova.
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

#include"sandbox-clmsg-tracer.h"

/*-------------------------------------------------------------------------------------------------------
	methods for ClMsgDiscAsyncTracer class
---------------------------------------------------------------------------------------------------------*/ 

ClMsgDiscAsyncTracer::ClMsgDiscAsyncTracer() 
  : ClMessageTracer(CLMSG_DISC_ASYNC)
{
}

void ClMsgDiscAsyncTracer::format(ClMessage *m, ConnectorTrace *sap)
{
	writeTrace(sap, " discovery ");
}





/*-------------------------------------------------------------------------------------------------------
	methods for ClMsgDiscReplyTracer class
---------------------------------------------------------------------------------------------------------*/ 

ClMsgDiscReplyTracer::ClMsgDiscReplyTracer() 
  : ClMessageTracer(CLMSG_DISC_REPLY)
{
}

void ClMsgDiscReplyTracer::format(ClMessage *m, ConnectorTrace *sap)
{
	writeTrace(sap, " discovery reply");
}





/*-------------------------------------------------------------------------------------------------------
	methods for ClMsgSandboxFe1DiscTracer class
---------------------------------------------------------------------------------------------------------*/ 

ClMsgSetTracer::ClMsgSetTracer() : ClMessageTracer(CLMSG_SET_SIZE)
{
}

void ClMsgSetTracer::format(ClMessage *m, ConnectorTrace *sap)
{
	writeTrace(sap, " [SNBXPLG1set: -newSize%d ]", ((ClMsgSet *)m)->getSize());
}

