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
 * $Id$
 */


#ifndef ns_umts_timers_h
#define ns_umts_timers_h

//#include "virtual_umtsmac.h"
#include "rlc.h"
#include "timer-handler.h"


#define RLC_TIMER_POLL		0
#define RLC_TIMER_RTX		1
#define RLC_TIMER_DELSND	2
#define RLC_TIMER_STPROB	3
#define RLC_TIMER_TTI		4
#define RLC_TIMER_TEMP_PDU	5 // Timeout for Temporary PDUs, in order
                             // not to get a deadlock when the last
                             // PDU is not followed by another PDU.
#define RLC_TIMER_CREDIT	6 // Timeout for sending a PDU in the
                             // Transmission Buffer.
#define RLC_TIMER_STATUS	7 // Timeout for sending Status Information
                             // in a separate PDU.
#define RLC_TIMER_MRWACK	8 // Timeout for retransmitting a STATUS MRW


#define MACHS_TIMER_FRAME      10
#define MACHS_TIMER_CREDALLOC  11
#define MACHS_TIMER_SCHEDULE   12
#define MACHS_TIMER_REORDER_STALL 13
#define MACHS_TIMER_PROCESS_CLEAR 14

class UmtsTimer:public TimerHandler {
public:
   UmtsTimer(RLC * rlc, int timeoutNumber, int flowID = -1);
   double      timeOfExpiry();
   char       *getName();
   void        cancel();        // overloaded from TimerHandler
   void        sched(double delay); // overloaded from TimerHandler
   void        resched(double delay);  // overloaded from TimerHandler
protected:
   virtual void expire(Event * e);
   RLC        *rlc_;
   int         timeoutNumber_;
};


/* class HsdpaMacTimer:public TimerHandler { */
/* public: */
/*    HsdpaMacTimer(VirtualUmtsMac * mac, int timeoutNumber, int flowID = -1); */
/*    double      timeOfExpiry(); */
/*    char       *getName(); */
/*    void        cancel();        // overloaded from TimerHandler */
/*    void        sched(double delay); // overloaded from TimerHandler */
/*    void        resched(double delay);  // overloaded from TimerHandler */
/* protected: */
/*    virtual void expire(Event * e); */
/*    VirtualUmtsMac *mac_; */
/*    int         timeoutNumber_; */
/*    int         flowID_; */
/* }; */


#endif //ns_umts_timers_h
