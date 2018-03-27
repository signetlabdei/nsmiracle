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

#include "transport-module.h"
#include "ip.h"

#define PORT_ALLOC_PER_TIMES 10

static class TransportModuleClass : public TclClass {
public:
	TransportModuleClass() : TclClass("Module/Transport") {}
	TclObject* create(int, const char*const*) {
	return (new TransportModule());

}
} class_transportmodule;

TransportModule::TransportModule() : Module(), allocatedPort_(0), allocatedId_(0)
{
}

TransportModule::~TransportModule()
{
}

int TransportModule::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	if(argc == 3)
	{
		if (strcasecmp(argv[1],"assignPort")==0)
		{
			Module *m = (Module *)tcl.lookup(argv[2]);
			if(!m)
				return TCL_ERROR;
			if(allocatedId_ < getUpLaySAPnum())
			{
				int *tmp = new int[getUpLaySAPnum()];
				for(int j = 0; j < getUpLaySAPnum(); j++)
				{
					if(j < allocatedId_)
						tmp[j] = ids_[j];
					else
						tmp[j] = -1;
				}
				if(allocatedId_)
					delete [] ids_;
				allocatedId_ = getUpLaySAPnum();
				ids_ = tmp;
			}
			for(int i = 0; i < getUpLaySAPnum(); i++)
			{
				SAP *s = getUpLaySAP(i);
				if(s->getModuleUpId() == m->getId())
				{
					if(m->getId() >= allocatedPort_)
					{
						int k = allocatedPort_ + ((m->getId() - allocatedPort_) / PORT_ALLOC_PER_TIMES + 1) * PORT_ALLOC_PER_TIMES;
						int *temp = new int[k];
						for(int j = 0; j < k; j++)
						{
							if(j < allocatedPort_)
								temp[j] = ports_[j];
							else
								temp[j] = -1;
						}
						if(allocatedPort_)
							delete [] ports_;
						ports_ = temp;
						allocatedPort_ += k;
					}
					if(debug_)
						printf("TransportModule::command module id=%d - i=%d\n", m->getId(), i);
					ports_[m->getId()] = i;
					ids_[i] = m->getId();
					tcl.resultf("%d", i);
					return TCL_OK;
				}
			}
			return TCL_ERROR;
		}
	}
	return Module::command(argc, argv);
}

void TransportModule::recv(Packet *p)
{
	fprintf(stderr, "TransportModule: a Packet is sent without source module!!\n");
	Packet::free(p);
}

void TransportModule::recv(Packet *p, int idSrc)
{
	hdr_cmn *ch = HDR_CMN(p);
	hdr_ip *iph = HDR_IP(p);
	if(ch->direction() == hdr_cmn::UP)
	{
		if(debug_ > 5)
			printf("dest port %d id %d\n", iph->dport(), ids_[iph->dport()]);
		sendUp(ids_[iph->dport()], p);
	}
	else
	{
		if(idSrc < allocatedPort_ && ports_[idSrc] >= 0)
		{
			iph->sport() = ports_[idSrc];
			sendDown(p);
		}
		else
		{
			fprintf(stderr, "TransportModule: a Packet is sent from a source module that I don't know!!\n");
			Packet::free(p);
		}
	}
}
