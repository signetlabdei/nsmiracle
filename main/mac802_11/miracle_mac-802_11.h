/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*-
 *
 * Copyright (c) 1997 Regents of the University of California.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Header: /nfs/jade/vint/CVSROOT/ns-2/mac/mac-802_11.h,v 1.24 2004/04/02 01:00:25 xuanc Exp $
 *
 * Ported from CMU/Monarch's code, nov'98 -Padma.
 * wireless-mac-802_11.h
 */

#ifndef ns_mrclmac_80211_h
#define ns_mrclmac_80211_h

// Added by Sushmita to support event tracing (singal@nunki.usc.edu)
#include "address.h"
#include "ip.h"
#include "bi-connector.h"
#include "packet.h"
#include "route.h"
#include "ll.h"
#include "phy.h"
#include "channel.h"

#include "mrcl_mac-timers.h"
#include "marshall.h"
#include "mac-802_11.h"
#include <math.h>

class Channel;

class EventTrace;

#define ZERO	0.00000

/*
 * Medium Access Control (MAC)
 */

#define EF_COLLISION 2		// collision error flag

/* ======================================================================
   Defines / Macros used by all MACs.
   ====================================================================== */

#define ETHER_ADDR(x)	(GET4BYTE(x))

#define MAC_HDR_LEN	64

#define MAC_BROADCAST	((u_int32_t) 0xffffffff)
#define BCAST_ADDR -1

#define ETHER_ADDR_LEN	6
#define ETHER_TYPE_LEN	2
#define ETHER_FCS_LEN	4

#define ETHERTYPE_IP	0x0800
#define ETHERTYPE_ARP	0x0806


#define GET_ETHER_TYPE(x)		GET2BYTE((x))
#define SET_ETHER_TYPE(x,y)            {u_int16_t t = (y); STORE2BYTE(x,&t);}

/* ======================================================================
   Frame Formats
   ====================================================================== */

#define	MAC_ProtocolVersion	0x00

#define MAC_Type_Management	0x00
#define MAC_Type_Control	0x01
#define MAC_Type_Data		0x02
#define MAC_Type_Reserved	0x03

#define MAC_Subtype_RTS		0x0B
#define MAC_Subtype_CTS		0x0C
#define MAC_Subtype_ACK		0x0D
#define MAC_Subtype_Data	0x00

// NB: The next structs are just defined in mac-802_11.h

// enum MacState {
// 	MAC_IDLE	= 0x0000,
// 	MAC_POLLING	= 0x0001,
// 	MAC_RECV 	= 0x0010,
// 	MAC_SEND 	= 0x0100,
// 	MAC_RTS		= 0x0200,
// 	MAC_CTS		= 0x0400,
// 	MAC_ACK		= 0x0800,
// 	MAC_COLL	= 0x1000
// };
// 
// enum MacFrameType {
// 	MF_BEACON	= 0x0008, // beaconing
// 	MF_CONTROL	= 0x0010, // used as mask for control frame
// 	MF_SLOTS	= 0x001a, // announce slots open for contention
// 	MF_RTS		= 0x001b, // request to send
// 	MF_CTS		= 0x001c, // clear to send, grant
// 	MF_ACK		= 0x001d, // acknowledgement
// 	MF_CF_END	= 0x001e, // contention free period end
// 	MF_POLL		= 0x001f, // polling
// 	MF_DATA		= 0x0020, // also used as mask for data frame
// 	MF_DATA_ACK	= 0x0021  // ack for data frames
// };

// struct hdr_mrclmac {
// 	MrclMacFrameType ftype_;	// frame type
// 	int macSA_;		// source MAC address
// 	int macDA_;		// destination MAC address
// 	u_int16_t hdr_type_;     // mac_hdr type
// 
// 	double txtime_;		// transmission time
// 	double sstime_;		// slot start time
// 
// 	int padding_;
// 
// 	inline void set(MrclMacFrameType ft, int sa, int da=-1) {
// 		ftype_ = ft;
// 		macSA_ = sa;
// 		if (da != -1)  macDA_ = da;
// 	}
// 	inline MrclMacFrameType& ftype() { return ftype_; }
// 	inline int& macSA() { return macSA_; }
// 	inline int& macDA() { return macDA_; }
// 	inline u_int16_t& hdr_type() {return hdr_type_; }
// 
// 	inline double& txtime() { return txtime_; }
// 	inline double& sstime() { return sstime_; }
// 
// 	// Header access methods
// 	static int offset_;
// 	inline static int& offset() { return offset_; }
// 	inline static hdr_mrclmac* access(const Packet* p) {
// 		return (hdr_mrclmac*) p->access(offset_);
// 	}
// };

//#define HDR_MRCLMAC(p)      (hdr_mrclmac::access(p))

// struct frame_control {
// 	u_char		fc_subtype		: 4;
// 	u_char		fc_type			: 2;
// 	u_char		fc_protocol_version	: 2;
// 
// 	u_char		fc_order		: 1;
// 	u_char		fc_wep			: 1;
// 	u_char		fc_more_data		: 1;
// 	u_char		fc_pwr_mgt		: 1;
// 	u_char		fc_retry		: 1;
// 	u_char		fc_more_frag		: 1;
// 	u_char		fc_from_ds		: 1;
// 	u_char		fc_to_ds		: 1;
// };

// struct rts_frame {
// 	struct frame_control	rf_fc;
// 	u_int16_t		rf_duration;
// 	u_char			rf_ra[ETHER_ADDR_LEN];
// 	u_char			rf_ta[ETHER_ADDR_LEN];
// 	u_char			rf_fcs[ETHER_FCS_LEN];
// };

// struct cts_frame {
// 	struct frame_control	cf_fc;
// 	u_int16_t		cf_duration;
// 	u_char			cf_ra[ETHER_ADDR_LEN];
// 	u_char			cf_fcs[ETHER_FCS_LEN];
// };

// struct ack_frame {
// 	struct frame_control	af_fc;
// 	u_int16_t		af_duration;
// 	u_char			af_ra[ETHER_ADDR_LEN];
// 	u_char			af_fcs[ETHER_FCS_LEN];
// };

/*XXX This header does not have its header access function because it shares
the same header space with hdr_mrclmac.*/
// struct hdr_mrclmac802_11 {
// 	struct frame_control	dh_fc;
// 	u_int16_t		dh_duration;
// 	u_char                  dh_ra[ETHER_ADDR_LEN];
//         u_char                  dh_ta[ETHER_ADDR_LEN];
//         u_char                  dh_3a[ETHER_ADDR_LEN];
// 	u_int16_t		dh_scontrol;
// 	u_char			dh_body[0]; // XXX Non-ANSI
// 	// Header access methods
// 	static int offset_;
// 	inline static int& offset() { return offset_; }
// 	inline static hdr_mrclmac802_11* access(const Packet* p) {
// 		return (hdr_mrclmac802_11*) p->access(offset_);
// 	}
// };

//#define HDR_MRCLMAC802_11(p)      (hdr_mrclmac802_11::access(p))

/* ======================================================================
   Definitions
   ====================================================================== */

/* Must account for propagation delays added by the channel model when
 * calculating tx timeouts (as set in tcl/lan/ns-mac.tcl).
 *   -- Gavin Holland, March 2002
 */
#define DSSS_MaxPropagationDelay        0.000002        // 2us   XXXX


class MrclMac;

class MrclTap {
public:
	virtual ~MrclTap () {}
	virtual void tap(const Packet *p) = 0;
	// tap is given all packets received by the host.
	// it must not alter or free the pkt.  If you want to frob it, copy it.
};


class MrclMacHandlerResume : public Handler {
public:
	MrclMacHandlerResume(MrclMac* m) : mac_(m) {}
	void handle(Event*);
protected:
	MrclMac* mac_;
};

class MrclMacHandlerSend : public Handler {
public:
	MrclMacHandlerSend(MrclMac* m) : mac_(m) {}
	void handle(Event*);
protected:
	MrclMac* mac_;
};


/* ==================================================================
   MAC data structure
   ================================================================*/

class MrclMac : public BiConnector {
public:
	MrclMac();
	virtual void recv(Packet* p, Handler* h);
	virtual void sendDown(Packet* p);
	virtual void sendUp(Packet *p);

	virtual void resume(Packet* p = 0);
	virtual void installTap(MrclTap *t) { tap_ = t; }
	
	inline double txtime(int bytes) {
		return (8. * bytes / bandwidth_);
	}
 	inline double txtime(Packet* p) {
		return 8. * (MAC_HDR_LEN + \
			     (HDR_CMN(p))->size()) / bandwidth_;
	}
	inline double bandwidth() const { return bandwidth_; }
	
	inline int addr() { return index_; }
	inline MacState state() { return state_; }
	inline MacState state(int m) { return state_ = (MacState) m; }
	
        //mac methods to set dst, src and hdt_type in pkt hdrs.
	// note: -1 is the broadcast mac addr.
	virtual inline int hdr_dst(char* hdr, int dst = -2) {
		struct hdr_mac *dh = (struct hdr_mac*) hdr;
		if(dst > -2)
			dh->macDA_ = dst;
		return dh->macDA();
	}
	virtual inline int hdr_src(char* hdr, int src = -2) {
		struct hdr_mac *dh = (struct hdr_mac*) hdr;
		if(src > -2)
			dh->macSA_ = src;
		return dh->macSA();
	}
	virtual inline int hdr_type(char *hdr, u_int16_t type = 0) {
		struct hdr_mac *dh = (struct hdr_mac*) hdr;
		if (type)
			dh->hdr_type_ = type;
		return dh->hdr_type();
	}

private:
        void mac_log(Packet *p) {
                logtarget_->recv(p, (Handler*) 0);
        }
        NsObject*       logtarget_;

protected:
	int command(int argc, const char*const* argv);
	virtual int initialized() { 
		return (netif_ && uptarget_ && downtarget_); 
	}
	int index_;		// MAC address
	double bandwidth_;      // channel bitrate
	double delay_;		// MAC overhead
	int abstract_;         //   MAC support for abstract LAN 
        
	Phy *netif_;            // network interface
        MrclTap *tap_;              // tap agent
	LL *ll_;             	// LL this MAC is connected to
	Channel *channel_;	// channel this MAC is connected to

	Handler* callback_;	// callback for end-of-transmission
	MrclMacHandlerResume hRes_;	// resume handler
	MrclMacHandlerSend hSend_;	// handle delay send due to busy channel
	Event intr_;

	/*
	 * Internal MAC State
	 */
	MacState state_;	// MAC's current state
	Packet *pktRx_;
	Packet *pktTx_;
};



class MPHY_MIB {
public:
	MPHY_MIB(MrclMac802_11 *parent);

	inline u_int32_t getCWMin() { return(CWMin); }
        inline u_int32_t getCWMax() { return(CWMax); }
	inline double getSlotTime() { return(SlotTime); }
	inline double getSIFS() { return(SIFSTime); }
	inline double getPIFS() { return(SIFSTime + SlotTime); }
	inline double getDIFS() { return(SIFSTime + 2 * SlotTime); }
	inline double getEIFS() {
		// see (802.11-1999, 9.2.10)
		return(SIFSTime + getDIFS()
                       + (8 *  getACKlen())/PLCPDataRate);
	}
	inline u_int32_t getPreambleLength() { return(PreambleLength); }
	inline double getPLCPDataRate() { return(PLCPDataRate); }
	
	inline u_int32_t getPLCPhdrLen() {
		return((PreambleLength + PLCPHeaderLength) >> 3);
	}

	inline u_int32_t getHdrLen11() {
		return(getPLCPhdrLen() + sizeof(struct hdr_mac802_11)
                       + ETHER_FCS_LEN);
	}
	
	inline u_int32_t getRTSlen() {
		return(getPLCPhdrLen() + sizeof(struct rts_frame));
	}
	
	inline u_int32_t getCTSlen() {
		return(getPLCPhdrLen() + sizeof(struct cts_frame));
	}
	
	inline u_int32_t getACKlen() {
		return(getPLCPhdrLen() + sizeof(struct ack_frame));
	}

 private:




	u_int32_t	CWMin;
	u_int32_t	CWMax;
	double		SlotTime;
	double		SIFSTime;
	u_int32_t	PreambleLength;
	u_int32_t	PLCPHeaderLength;
	double		PLCPDataRate;
};


/*
 * IEEE 802.11 Spec, section 11.4.4.2
 *      - default values for the MAC Attributes
 */
#define MAC_FragmentationThreshold	2346		// bytes
#define MAC_MaxTransmitMSDULifetime	512		// time units
#define MAC_MaxReceiveLifetime		512		// time units

class MMAC_MIB {
public:

	MMAC_MIB(MrclMac802_11 *parent);

private:
	u_int32_t	RTSThreshold;
	u_int32_t	ShortRetryLimit;
	u_int32_t	LongRetryLimit;
public:
	u_int32_t	FailedCount;	
	u_int32_t	RTSFailureCount;
	u_int32_t	ACKFailureCount;
 public:
       inline u_int32_t getRTSThreshold() { return(RTSThreshold);}
       inline u_int32_t getShortRetryLimit() { return(ShortRetryLimit);}
       inline u_int32_t getLongRetryLimit() { return(LongRetryLimit);}
};


/* ======================================================================
   The following destination class is used for duplicate detection.
   ====================================================================== */
// class Host {
// public:
// 	LIST_ENTRY(Host) link;
// 	u_int32_t	index;
// 	u_int32_t	seqno;
// };


/* ======================================================================
   The Miracle 802.11 MAC class.
   ====================================================================== */
class MrclMac802_11 : public Mac {
	friend class MrclDeferTimer;


	friend class MrclBackoffTimer;
	friend class MrclIFTimer;
	friend class MrclNavTimer;
	friend class MrclRxTimer;
	friend class MrclTxTimer;
public:
	MrclMac802_11();
	void		recv(Packet *p, Handler *h);
	inline int	hdr_dst(char* hdr, int dst = -2);
	inline int	hdr_src(char* hdr, int src = -2);
	inline int	hdr_type(char* hdr, u_int16_t type = 0);
	
	inline int bss_id() { return bss_id_; }
	
	// Added by Sushmita to support event tracing
        void trace_event(char *, Packet *);
        EventTrace *et_;

protected:
	void	backoffHandler(void);
	void	deferHandler(void);
	void	navHandler(void);
	void	recvHandler(void);
	void	sendHandler(void);
	void	txHandler(void);

private:
	int		command(int argc, const char*const* argv);

	/*
	 * Called by the timers.
	 */
	void		recv_timer(void);
	void		send_timer(void);
	int		check_pktCTRL();
	int		check_pktRTS();
	int		check_pktTx();

	/*
	 * Packet Transmission Functions.
	 */
	void	send(Packet *p, Handler *h);
	void 	sendRTS(int dst);
	void	sendCTS(int dst, double duration);
	void	sendACK(int dst);
	void	sendDATA(Packet *p);
	void	RetransmitRTS();
	void	RetransmitDATA();

	/*
	 * Packet Reception Functions.
	 */
	void	recvRTS(Packet *p);
	void	recvCTS(Packet *p);
	void	recvACK(Packet *p);
	void	recvDATA(Packet *p);

	void		capture(Packet *p);
	void		collision(Packet *p);
	void		discard(Packet *p, const char* why);
	void		rx_resume(void);
	void		tx_resume(void);

	inline int	is_idle(void);

	/*
	 * Debugging Functions.
	 */
	void		trace_pkt(Packet *p);
	void		dump(char* fname);

	inline int initialized() {	
		return (cache_ && logtarget_
                        && Mac::initialized());
	}

	inline void mac_log(Packet *p) {
                logtarget_->recv(p, (Handler*) 0);
        }

	double txtime(Packet *p);
	double txtime(double psz, double drt);
	double txtime(int bytes) { /* clobber inherited txtime() */ abort(); return 0;}

	inline void transmit(Packet *p, double timeout);
	inline void checkBackoffTimer(void);
	inline void postBackoff(int pri);
	inline void setRxState(MacState newState);
	inline void setTxState(MacState newState);


	inline void inc_cw() {
		cw_ = (cw_ << 1) + 1;
		if(cw_ > phymib_.getCWMax())
			cw_ = phymib_.getCWMax();
	}
	inline void rst_cw() { cw_ = phymib_.getCWMin(); }

	inline double sec(double t) { return(t *= 1.0e-6); }
	inline u_int16_t usec(double t) {
		u_int16_t us = (u_int16_t)floor((t *= 1e6) + 0.5);
		return us;
	}
	inline void set_nav(u_int16_t us) {
		double now = Scheduler::instance().clock();
		double t = us * 1e-6;
		if((now + t) > nav_) {
			nav_ = now + t;
			if(mhNav_.busy())
				mhNav_.stop();
			mhNav_.start(t);
		}
	}

protected:
	MPHY_MIB         phymib_;
        MMAC_MIB         macmib_;

       /* the macaddr of my AP in BSS mode; for IBSS mode
        * this is set to a reserved value IBSS_ID - the
        * MAC_BROADCAST reserved value can be used for this
        * purpose
        */
       int     bss_id_;
       enum    {IBSS_ID=MAC_BROADCAST};


private:
	double		basicRate_;
 	double		dataRate_;
	
	/*
	 * Mac Timers
	 */
	MrclIFTimer		mhIF_;		// interface timer
	MrclNavTimer	mhNav_;		// NAV timer
	MrclRxTimer		mhRecv_;		// incoming packets
	MrclTxTimer		mhSend_;		// outgoing packets

	MrclDeferTimer	mhDefer_;	// defer timer
	MrclBackoffTimer	mhBackoff_;	// backoff timer

	/* ============================================================
	   Internal MAC State
	   ============================================================ */
	double		nav_;		// Network Allocation Vector

	MacState	rx_state_;	// incoming state (MAC_RECV or MMAC_IDLE)
	MacState	tx_state_;	// outgoint state
	int		tx_active_;	// transmitter is ACTIVE

	Packet          *eotPacket_;    // copy for eot callback

	Packet		*pktRTS_;	// outgoing RTS packet
	Packet		*pktCTRL_;	// outgoing non-RTS packet

	u_int32_t	cw_;		// Contention Window
	u_int32_t	ssrc_;		// STA Short Retry Count
	u_int32_t	slrc_;		// STA Long Retry Count

	int		min_frame_len_;

	NsObject*	logtarget_;
	NsObject*       EOTtarget_;     // given a copy of packet at TX end




	/* ============================================================
	   Duplicate Detection state
	   ============================================================ */
	u_int16_t	sta_seqno_;	// next seqno that I'll use
	int		cache_node_count_;
	Host		*cache_;
};

#endif /* __mac_80211_h__ */

