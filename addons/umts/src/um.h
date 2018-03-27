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
 * $Id: um.h,v 1.1 2005/04/27 14:30:10 simon Exp $
 */

// This file both defines the RLC header and UM.

#ifndef ns_um_h
#define ns_um_h

#include <vector>
#include <ll.h>
#include <queue.h>
#include <agent.h>

#include "rlc.h"
#include "umts-queue.h"
#include "umts-timers.h"
#include "umts-headers.h"
#include"umts-packet.h"
#include "timer-handler.h"


#define EOPNO_NOT_EOP -1
#define EOPNO_TO_SEQNO -2

#define PAYLOAD_FLD1 0
#define PAYLOAD_FLD2 1
#define PAYLOAD_FLD3 2


class temporaryPacket {
public:
   temporaryPacket(RLC * rlc):tempPDUTimer(rlc, RLC_TIMER_TEMP_PDU) {
   } Packet   *p;
   int         concat_data;
   UmtsTimer   tempPDUTimer;
};


class UM:   public RLC {
public:
   UM();
   virtual void recv(Packet *, Handler *);
   int         command(int, const char *const *);
   void        timeout(int tno, int flowID = -1);
   int         buff_size();
   void        CSwitch(double, double);

protected:
   int         enquePacket(Packet * p);
   int         enqueInBackOfTransmissionBuffer(Packet * p);
   void        sendDown(Packet * p);
   void        StoreTemporaryPacket(Packet * p, int concat_data);
   void        handleTemporaryPacket(Packet * p);
   void        completePDU(Packet * p);
   void        makeSDU(Packet * p);

   int         lengthIndicatorSize_;
   // The size of the Length indicator + the
   // extention bit. The size is in bytes. The
   // size is 1 or 2 bytes depending on the size
   // of the PDUs.

   // parameters of the object, set through TCL

   int         win_;
   // defines the RLC Window Size, which is the amount of packets that can be
   // sent after the last acknowledged packet.

   int         bufferLevelMax_;
   // The maximum number of PDUs for each flow-id and
   // priority pair in the Transmission Buffer.

   double      tempPDUTimeOutTime_;
   // When a PDU is constructed that has some
   // space left, the PDU is stored in the vector
   // temporaryPackets_. A timeout will be set
   // to ensure that when no further SDU arrives
   // with the same flow-id and priority, the
   // not-full PDU will be sent and will not
   // wait indefinitely.

   temporaryPacket temporaryPacket_;
   // In this vector Temporary PDUs are stored. These are PDUs that a not
   // completely full, and they can be possibly concatenated with a part of
   // a new SDU. However, this concatenation should be done quick enough,
   // otherwise timeouts and unnesessary delays may occur. So, a timer will
   // be set, when the concatenation can be done before the timer times out,
   // the PDU will be concatenated with a part of a new SDU. When the timer
   // times out, the Temporary Packet is padded and sent without concatenation.

   double      overhead_;       // Time that is needed to contruct SDUs

   int         payloadSize_;    // user data per DATA PDU
   double      TTI_;
   int         TTI_PDUs_;

   int         length_indicator_;
   int         min_concat_data_;

   int         address_;        // address of this RLC Entity

   umtsQueue   transmissionBuffer_;
   // For our Transmission Buffer we use a vector of rlcQueue, for each
   // flow-id and priority-pair one queue exists.

   int         nextExpectedSDU_;
   int         nextExpectedSeqno_;
   int         nextExpectedSegment_;
   int         errorInSDU_;

   int         seqno_;

   int         send_status_;
   int         SDU_size_;       // stores the original size of the SDU when a part already
   // was concatenated

   int         maxseq_;         // highest seqno transmitted so far
   int         highest_ack_;    // highest ack recieved by sender
   int         maxseen_;        // max PDU (seqno)number seen by receiver
//   int seen_[MWS];   // array of PDUs seen by reciever
   int         next_;           // next PDU expected by reciever

   int         d_address_;      // destination address of this RLC Entity
   int         macDA_;          // mac destination address for packets created by this RLC
   // Entity

   umtsQueue   sduB_;

   UmtsTimer   tti_timer_;

};


#endif
