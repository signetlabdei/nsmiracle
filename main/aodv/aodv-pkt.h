/*
Copyright (c) 1997, 1998 Carnegie Mellon University.  All Rights
Reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The AODV code developed by the CMU/MONARCH group was optimized and tuned by Samir Das and Mahesh Marina, University of Cincinnati. The work was partially done in Sun Microsystems.
*/

/**
 * Ported to NS-Miracle by Marco Miozzo, 2007-2008
 * 
 */


#ifndef _MRCL_AODV_PKT_
#define _MRCL_AODV_PKT_

#include <mrcl-address.h>

/* =====================================================================
   Packet Formats...
   ===================================================================== */
#define AODVTYPE_HELLO  	0x01
#define AODVTYPE_RREQ   	0x02
#define AODVTYPE_RREP   	0x04
#define AODVTYPE_RERR   	0x08
#define AODVTYPE_RREP_ACK  	0x10

/*
 * AODV Routing Protocol Header Macros
 */
#define HDR_MRCL_AODV(p)		((struct hdr_mrcl_aodv*)hdr_mrcl_aodv::access(p))
#define HDR_MRCL_AODV_REQUEST(p)  	((struct hdr_mrcl_aodv_request*)hdr_mrcl_aodv::access(p))
#define HDR_MRCL_AODV_REPLY(p)	((struct hdr_mrcl_aodv_reply*)hdr_mrcl_aodv::access(p))
#define HDR_MRCL_AODV_ERROR(p)	((struct hdr_mrcl_aodv_error*)hdr_mrcl_aodv::access(p))
#define HDR_MRCL_AODV_RREP_ACK(p)	((struct hdr_mrcl_aodv_rrep_ack*)hdr_mrcl_aodv::access(p))


/*
 * AODV Header
 */

#define AODV_MAX_ERRORS 20
#define AODV_MAX_DST_ERRORS (AODV_MAX_ERRORS * MRCL_ADDRESS_MAX_LEN)

#define AODV_MAX_PKT_LENGTH (AODV_MAX_DST_ERRORS+AODV_MAX_ERRORS*4+10)

struct hdr_mrcl_aodv {
        char        ah_type;
// 	char		body[AODV_MAX_PKT_LENGTH];
	// Header access methods
	static int offset_; // required by PacketHeaderManager
	inline static int& offset() { return offset_; }
	inline static hdr_mrcl_aodv* access(const Packet* p) {
		return (hdr_mrcl_aodv*) p->access(offset_);
	}
};

struct hdr_mrcl_aodv_request {
        char        rq_type;	// Packet Type
        char        reserved[2];
        char        rq_hop_count;   // Hop Count
        int       rq_bcast_id;    // Broadcast ID

        char     	rq_dst[MRCL_ADDRESS_MAX_LEN];         // Destination Address
        int	   	rq_dst_seqno;   // Destination Sequence Number
        char     	rq_src[MRCL_ADDRESS_MAX_LEN];         // Source Address
        int		rq_src_seqno;   // Source Sequence Number

        double          rq_timestamp;   // when REQUEST sent;
					// used to compute route discovery latency
	inline char *rq_dest() {return rq_dst;};
	inline int size() {
        int rq_dst_int;
        int rq_src_int;
        memcpy(&rq_dst_int, rq_dst, sizeof(int));
        memcpy(&rq_src_int, rq_src, sizeof(int));  
        return (3*sizeof(char)+3*sizeof(int)+sizeof(double)+ rq_dst_int + rq_src_int);
	}
};

struct hdr_mrcl_aodv_reply {
        char        rp_type;        // Packet Type
        char        reserved[2];
        char        rp_hop_count;           // Hop Count
        char        rp_dst[MRCL_ADDRESS_MAX_LEN];                 // Destination IP Address
        int         rp_dst_seqno;           // Destination Sequence Number
        char        rp_src[MRCL_ADDRESS_MAX_LEN];                 // Source IP Address
        double	    rp_lifetime;            // Lifetime

        double      rp_timestamp;           // when corresponding REQ sent;
					    // used to compute route discovery latency
						
	inline int size() { 
		int rp_dst_int;
        int rp_src_int;
         memcpy(&rp_dst_int, rp_dst, sizeof(int));
         memcpy(&rp_src_int, rp_src, sizeof(int));  
        return (3*sizeof(char)+3*sizeof(int)+sizeof(double)+ rp_dst_int + rp_src_int);
	}

};

struct hdr_mrcl_aodv_error {
        char        re_type;                // Type
        char        reserved[2];            // Reserved
        char        DestCount;                 // DestCount
        // List of Unreachable destination IP addresses and sequence numbers
        char        unreachable_dst[AODV_MAX_DST_ERRORS];   
        int         unreachable_dst_seqno[AODV_MAX_ERRORS];   

	inline int size() {
	    int unreachable_dst_int;
        memcpy(&unreachable_dst_int, unreachable_dst, sizeof(int));
  		int sz = DestCount*2*(unreachable_dst_int) + sizeof(int);
		assert(sz > 0);
        	return sz;
	}

};

struct hdr_mrcl_aodv_rrep_ack {
	char	rpack_type;
	char	reserved;
};

// for size calculation of header-space reservation
union hdr_all_mrcl_aodv {
  hdr_mrcl_aodv          ah;
  hdr_mrcl_aodv_request  rreq;
  hdr_mrcl_aodv_reply    rrep;
  hdr_mrcl_aodv_error    rerr;
  hdr_mrcl_aodv_rrep_ack rrep_ack;
};


#endif

