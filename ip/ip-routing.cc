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

#include<ip.h>

#include"ip-routing.h"
#include"ipmodule.h"



#define DROP_TTL_EXPIRED_DEPHT 1
#define DROP_TTL_EXPIRED_REASON "TTL"

#define DROP_DEST_UNREACHABLE_DEPTH 1
#define DROP_DEST_UNREACHABLE_REASON "DUR"

static class IPRoutingModuleClass : public TclClass {
public:
  IPRoutingModuleClass() : TclClass("Module/IP/Routing") {}
  TclObject* create(int, const char*const*) {
    return (new IPRoutingModule);

  }
} class_ipinterface_module;


IPRoutingModule::IPRoutingModule() : nroutes(0)
{
  clearRoutes();
}

IPRoutingModule::~IPRoutingModule() 
{
}


int IPRoutingModule::recvSyncClMsg(ClMessage* m) 
{
  return Module::recvSyncClMsg(m);
}


void IPRoutingModule::clearRoutes()
{
  for (int i=0; i<IP_ROUTING_MAX_ROUTES; i++)
    {
      destination[i] = 0;
      subnetmask[i] = 0;
      nexthop[i] = 0;
    } 

  nroutes = 0;
}

void IPRoutingModule::addRoute(nsaddr_t dst, nsaddr_t mask, nsaddr_t next)
{ 
  assert(nroutes < IP_ROUTING_MAX_ROUTES);
  destination[nroutes] = dst;
  subnetmask[nroutes] = mask;
  nexthop[nroutes] = next;
  nroutes++;


}


int IPRoutingModule::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  if(argc == 2)
    {
      if (strcasecmp(argv[1],"numroutes")==0) 
	{
	  tcl.resultf("%d", nroutes);
	  return TCL_OK;
	  
	}
      if (strcasecmp(argv[1],"clearroutes")==0) 
	{
	  clearRoutes();
	  return TCL_OK;
	}
      else if (strcasecmp(argv[1],"printroutes")==0) 
	{
	  printf("Destination    Subnet Mask    Next Hop\n");
	  for (int i=0; i<nroutes; i++) {		
	    printf("%d.%d.%d.%d\t\t",
		   (destination[i] & 0xff000000)>>24,
		   (destination[i] & 0x00ff0000)>>16,
		   (destination[i] & 0x0000ff00)>>8,
		   (destination[i] & 0x000000ff));
	    printf("%d.%d.%d.%d\t\t",
		   (subnetmask[i] & 0xff000000)>>24,
		   (subnetmask[i] & 0x00ff0000)>>16,
		   (subnetmask[i] & 0x0000ff00)>>8,
		   (subnetmask[i] & 0x000000ff));
	    printf("%d.%d.%d.%d\n",
		   (nexthop[i] & 0xff000000)>>24,
		   (nexthop[i] & 0x00ff0000)>>16,
		   (nexthop[i] & 0x0000ff00)>>8,
		   (nexthop[i] & 0x000000ff));
	  }
	  return TCL_OK;
	}		   		
    }
  else if (argc == 3) {
    if (strcasecmp (argv[1], "defaultGateway") == 0) {
      addRoute( 0x00000000, 0x00000000, str2addr(argv[2]));
      return TCL_OK;
    }
	      
  }
  else if (argc == 5) {
    if (strcasecmp (argv[1], "addroute") == 0) {
      addRoute(str2addr(argv[2]) ,
	       str2addr(argv[3]),
	       str2addr(argv[4]));
      return TCL_OK;
    }
	      
  }

  /* we DON'T call IPModule::command here
   since we don't need the tcl functionality 
   of IPModule class */
  return Module::command(argc, argv);
}




void IPRoutingModule::recv(Packet *p)
{
  hdr_ip *iph = HDR_IP(p);
  hdr_cmn *ch = HDR_CMN(p);

  if(ch->direction() == hdr_cmn::UP)
    {

      /* WARNING 
       * it is required that there is an IP module below this 
       * which checks if the IP of the interface corresponds to
       * the next_hop address
       * otherwise ALL nodes receiving the last hop transmission
       * will forward the packet to upper layers!
       */ 

      if(iph->daddr() == ch->next_hop_ || iph->daddr() == (nsaddr_t)IP_BROADCAST)
	{ /* Packet is arrived at its destination */
	  ch->size() -= IP_HDR_LEN;
	  sendUp(p);
	  return;
	}
                  
      /* Check TTL */

      iph->ttl()--; 
      if (iph->ttl() == 0)
	{
	  drop(p,DROP_TTL_EXPIRED_DEPHT, DROP_TTL_EXPIRED_REASON);
	  return;
	}

      /* Forward Packet */      
      ch->direction() = hdr_cmn::DOWN; 
      ch->next_hop_ = GetNextHop(p);

      if (ch->next_hop_ == 0) 
	{
	  drop(p,DROP_DEST_UNREACHABLE_DEPTH, DROP_DEST_UNREACHABLE_REASON);
	}
      else
	{
	  sendDown(p);
	}
      
      
    }
  else
    {  /* direction DOWN */
      //assert(iph->saddr() == 0); /* who the hell has set IP source address ? */
      iph->saddr() = 0;

      iph->ttl() = IP_DEF_TTL;      
      ch->size() += IP_HDR_LEN;
      ch->addr_type()	= NS_AF_INET;
      

//       if ((u_int32_t)iph->daddr() == IP_BROADCAST)
// 	ch->next_hop()	= IP_BROADCAST;
      
      
      ch->next_hop_ = GetNextHop(p);
     
      
      if (ch->next_hop_ == 0) 
	{
	  drop(p,DROP_DEST_UNREACHABLE_DEPTH, DROP_DEST_UNREACHABLE_REASON);
	}
      else
	{
	  sendDown(p);
	}
  
    }

}  
 

nsaddr_t IPRoutingModule::GetNextHop(Packet *p)
{
  hdr_ip *iph = HDR_IP(p);
  return GetNextHop(iph->daddr());
}


nsaddr_t IPRoutingModule::GetNextHop(nsaddr_t dst)
{

  //////////////////////////////////////////////////////////////////////////////////////
  ///////////////////// HACK to make module work on 64 bit architecture ////////////////
  ///////////////////// need better coding!                             ////////////////
  //////////////////////////////////////////////////////////////////////////////////////

  static char temp_buffer[256];

  sprintf(temp_buffer, "IPRoutingModule::GetNextHop destination %x (%d.%d.%d.%d)\n",
	 dst,
	 (dst & 0xff000000)>>24,
	 (dst & 0x00ff0000)>>16,
	 (dst & 0x0000ff00)>>8,
	 (dst & 0x000000ff));

  //////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////

  for (int i=0; i<nroutes; i++) 
    {     
      if ((dst & subnetmask[i]) == (destination[i] & subnetmask[i]))
	{
	  
	  return nexthop[i];      
	}
    }

  return 0; 
}


