/*
 * Copyright (c) 2003 Ericsson Telecommunicatie B.V.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the
 *     distribution.
 * 3. Neither the name of Ericsson Telecommunicatie B.V. may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY ERICSSON TELECOMMUNICATIE B.V. AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ERICSSON TELECOMMUNICATIE B.V., THE AUTHOR OR HIS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 * Contact for feedback on EURANE: eurane@ti-wmc.nl
 * EURANE = Enhanced UMTS Radio Access Network Extensions
 * website: http://www.ti-wmc.nl/eurane/
 */

/*
 * $Id: am.h,v 1.14 2004/01/21 15:55:12 wtdavid Exp $
 */


#ifndef ns_am_h
#define ns_am_h

#include <rlc.h>
#include <um.h>
#include <umts-timers.h>
#include <umts-queue.h>
#include <umts-packet.h>
#include <umts-headers.h>

// max window size 
#define MWS 4096             

// The "win_" parameter, representing the RLC window,
// should be less than equal to MWM.
#define MWM (MWS-1)          

#define BITMAP_LENGTH (MAX_BITMAP_OCTETS*8)


#define SEND_STATUS_NONE     0x0000
#define SEND_STATUS_BITMAP   0x0001
#define SEND_STATUS_MRW      0x0002
#define SEND_STATUS_MRWACK   0x0004

class AM;

class AM:   public RLC {

public:
   AM();
   int         buff_size();
   void        timeout(int tno, int flowID = -1);
   void        CSwitch(double, double);
   int        &addr() {
      return (address_);
   } int      &daddr() {
      return (d_address_);
   }
   virtual void recv(Packet *, Handler *);
   int         getTotPDUs()  {return tot_PDUs_; }
   int         getErrPDUs()  {return err_PDUs_; }
   int         getAckPDUs()  {return ack_PDUs_; }
   int         getAckSDUs()  {return ack_SDUs_; }
   int         getDropPDUs() {return drop_PDUs_;}
   int         getDropSDUs() {return drop_SDUs_;}

 protected:
   int         bRxtSeq(int);
   void        ack(Packet *);
   Packet*     make_status(Packet *);
   Packet     *make_bitmap_SUFI(Packet *);
   Packet     *make_mrw_SUFI(Packet *);
   Packet     *make_mrwack_SUFI(Packet *);
   Packet     *make_positive_ack(Packet *);
   void        newack(Packet *);
   void        makeSDU(int);
   void        send_much(int);
   void        newback(Packet *);
   Packet     *allocpkt(int);
   void        sendDown(Packet *);
   int         update(int, int);
   int         command(int, const char *const *);

   bool        chk_size(Packet *);
   Packet     *makePDU(int);
   int         PB_S_PDU();
   int         PB_PA_PDU();
   bool        PA_Piggybackable(Packet *);
   int         AckPDUSize();
   bool        S_Piggybackable(Packet *);
   int         StatusPDUSize();
   int         SUFI_bitmap_size();
   int         SUFI_mrw_size();
   double      send_time(int);

   void        set_rtx_timer();
   void        set_poll_timer();
   void        reset_rtx_timer();
   void        cancel_rtx_timer();
   void        cancel_poll_timer();
   void        set_status_prohibit_timer();
   virtual  void        ackSDU(int oldFSN, int newFSN);
   void        discardSDU(int dsn);

   int         win_;            // RLC window size VT(WS)
   double      maxRBSize_;      // max length of the receive buffer 
   int         ack_mode_;       // ack option. Selective repeat (==1) or Bitmap (==2)
   int         poll_PDU_;       // number of PDUs before a bitmap poll should be sent
   double      overhead_;       // Time that is needed to contruct SDUs
   double      rtx_timeout_;
   double      poll_timeout_;   // rtt value to be used for timeout to recv status PDU
   double      stprob_timeout_; // value for the status prohibit timer
   int         noFastRetrans_;  // No Fast Retransmit option
   int         numdupacks_;     // dup ACKs before fast retransmit

   int         payload_;        // user data per DATA PDU
   double      bandwidth_;
   double      TTI_;
   double      next_TTI_;

   int         ack_pdu_header_;
   int         status_pdu_header_;
   int         length_indicator_;
   int         min_concat_data_;
   double      max_status_delay_;
   double      max_ack_delay_;

   int         sent_TTI_PDUs_;
   int         TTI_PDUs_;
   double      earliest_status_send_;
   double      earliest_ack_send_;
   int         set_poll_;
   int         send_ack_;       // indicates if a positive ack is to be transmitted
   int         send_status_;    // indicates if a status PDU is to be transmitted;
                                // which SUFIs are to be included is indicated 
                                // by bitwise-OR of SEND_STATUS_* values
   int         SDU_size_;       // stores the original size of the SDU when a part already
                                // was concatenated


   int         dupacks_;        // number of (pos) duplicate acks
   int         rtt_seq_;

/*    int         poll_seq_;       // stores the highest sequence number of a packet including */
/*                                 // a poll bit */

   int         rtt_active_;
   int         prohibited_;

   int         FSN_;            // First Sequence Number of the window
				// of the transmitter, a.k.a. VT(A)

   int         MRW_delta;       // the difference between the sender
				// window and the receiver window. It
				// is set when sender window is moved and reset
				// when a MRW_ACK is received. Used to
				// limit the window size of the sender
				// until the receiver has moved the window.

   int         b_bal_;          // number of 0s in bitmap (number of nacks in bitmap)
   int         length_;         // length of bitmap
   int         bitmap_[BITMAP_LENGTH];

   /* NOTE: for bitmap acknowledgement t_seqno_ and maxseq_ are
      redundant since they are always offset by 1 */
   int         maxseq_;         // highest seqno transmitted so far,
				// a.k.a. VT(S) - 1
   int         t_seqno_;        // Seqno of next PDU to be
				// transmitted, a.k.a VT(S)

   int         highest_ack_;    // highest ack received by sender,

   int         maxseen_;        // max PDU (seqno)number seen by
				// receiver
				// a.k.a. VR(H) - 1
   int        seen_[MWS];      ///< array of PDUs seen by receiver
                                ///  for each packet i, seen_[i] is 0
                                ///  if the packet has not been
                                ///  received, > 0 if it has been
                                ///  received (1 if it is the
                                ///  end-of-SDU packet, 2 otherwise)  
   int         next_;           // next PDU expected by receiver
				// a.k.a. VR(R) 
   int         eopno_[MWS];     // if PDU n is end-of-SDU 
   				// then eopno_[n] == n 
   				// else eopno_[n] == -1
  
   u_int8_t    txcount_[MWS];    // TX counter for each AMD PDU (
				 // a.k.a. VT(DAT) )
   int         maxdat_;         // Maximum number of transmission
				// attempts for each AMD PDU

   int         tx_PDUs_before_poll;       /// Temporary counter for PDU
			       /// transmission attempts. It is
			       /// resetted each poll_PDUs_ transmissions
   

   int         tot_PDUs_;      /// Total num of PDU transmission attempts
   int         err_PDUs_;      /// Total number of PDUs reported as
			       /// erroneous in BITMAP SUFI. This
			       /// number contains both retransmitted
			       /// PDUs and moreover may count a
			       /// single error more than once if
			       /// transmission is not attempted
			       /// between subsequents STATUS PDUs.

   int         ack_PDUs_;      /// number of acknowledge PDUs, without duplicates
   int         ack_SDUs_;      /// number of acknowledge SDUs, without duplicates
   int         drop_PDUs_;     /// number of PDUs which have exceeded the retx limit (maxdat_)
   int         drop_SDUs_;     /// number of dropped SDUs due to dropped PDUs
   int         rx_PDUs_;        
   int         rx_SDUs_;


   int         address_;        // address of this RLC Entity
   int         d_address_;      // destination address of this RLC Entity

   double      TTI_time_;

   UmtsTimer   rtx_timer_;
   UmtsTimer   poll_timer_;
   UmtsTimer   delsnd_timer_;
   UmtsTimer   stprob_timer_;
   UmtsTimer   tti_timer_;
   UmtsTimer   mrwack_timer_;

   umtsQueue   rcvB_;
   umtsQueue   rxtB_;
   umtsQueue   sduB_;

   int         flowID_;         // The flow-id of the incoming
				// packets, is used to set the 
                                // flow-id of outgoing packets.


   int sender_debug_;           
   int receiver_debug_;
   

};

#endif
