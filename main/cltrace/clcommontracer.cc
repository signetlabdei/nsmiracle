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

#include "clcommontracer.h"
#include "clsap.h"

ClCommonTracer::ClCommonTracer() : ClMessageTracer(0)
{
}

void ClCommonTracer::format(ClMessage *m, ConnectorTrace *sap)
{
	char dir;
	switch (m->direction())
	{
		case TONODECORE:
			dir = 'N';
			break;
		case TOMODULE:
			dir = 'M';
			break;
		case UP:
			dir = 'U';
			break;
		case DOWN:
			dir = 'D';
			break;
	}
	
	
	writeTrace(sap, " [CMN: %d %d %s %d %c]",
				m->uid(),
				m->getSource(),
				m->getDestType()==UNICAST?"UNI":"BRD",
				m->getDest(),
				dir
				);
}


extern "C" int Cltrace_Init()
{
	/*
	Put here all the commands which must be execute when the library is loaded (i.e. TCL script execution)
	Remember to ruturn 0 if all is OK, otherwise return 1
	*/
	ConnectorTrace::addCommonTracer(new ClCommonTracer);
	return 0;
}
extern "C" int  Cygcltrace_Init()
{
  Cltrace_Init();
}


