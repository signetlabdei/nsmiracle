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
 * $Id: umts-timers.cc,v 1.18 2004/01/20 13:29:24 simon Exp $
 */

#include "umts-timers.h"

UmtsTimer::UmtsTimer(RLC * rlc, int timeoutNumber, int flowID):TimerHandler()
{

   if (rlc != NULL) {
      rlc_ = rlc;
      timeoutNumber_ = timeoutNumber;
   } else {
   }
}

void UmtsTimer::expire(Event *)
{
   rlc_->timeout(timeoutNumber_);
}

double UmtsTimer::timeOfExpiry()
{
   return event_.time_;
}


char       *UmtsTimer::getName()
{
   switch (timeoutNumber_) {
     case RLC_TIMER_POLL:
        return "RLC_TIMER_POLL";
        break;
     case RLC_TIMER_RTX:
        return "RLC_TIMER_RTX";
        break;
     case RLC_TIMER_DELSND:
        return "RLC_TIMER_DELSND";
        break;
     case RLC_TIMER_STPROB:
        return "RLC_TIMER_STPROB";
        break;
     case RLC_TIMER_TTI:
        return "RLC_TIMER_TTI";
        break;
     case RLC_TIMER_TEMP_PDU:
        return "RLC_TIMER_TEMP_PDU";
        break;
     case RLC_TIMER_CREDIT:
        return "RLC_TIMER_CREDIT";
        break;
     case RLC_TIMER_STATUS:
        return "RLC_TIMER_STATUS";
        break;
     case RLC_TIMER_MRWACK:
        return "RLC_TIMER_MRWACK";
        break;
     default:
        return "UNKNOWN";
        break;
   }
}

// HsdpaMacTimer::HsdpaMacTimer(VirtualUmtsMac * mac, int timeoutNumber,
//                              int flowID):TimerHandler()
// {
//    mac_ = mac;
//    timeoutNumber_ = timeoutNumber;
//    flowID_ = flowID;
// }

// void HsdpaMacTimer::expire(Event *)
// {
//    mac_->timeout(timeoutNumber_);
// }

// double HsdpaMacTimer::timeOfExpiry()
// {
//    return event_.time_;
// }

// char       *HsdpaMacTimer::getName()
// {
//    switch (timeoutNumber_) {
//      case MACHS_TIMER_FRAME:
//         return "MACHS_TIMER_FRAME";
//         break;
//      case MACHS_TIMER_CREDALLOC:
//         return "MACHS_TIMER_CREDALLOC";
//         break;
//      case MACHS_TIMER_SCHEDULE:
//         return "MACHS_TIMER_SCHEDULE";
//         break;
//      case MACHS_TIMER_REORDER_STALL:
//         return "MACHS_TIMER_REOR";
//         break;
//      case MACHS_TIMER_PROCESS_CLEAR:
//         return "MACHS_TIMER_PROCESS_CLEAR";
//         break;
//      default:
//         return "UNKNOWN HSDPAMAC TIMER";
//         break;
//    }
// }

//
void UmtsTimer::cancel()
{
   if (status_ != TIMER_PENDING) {
      return;
   } else {
      ((TimerHandler *) this)->cancel();
   }
}

void UmtsTimer::sched(double delay)
{
   if (delay >= 0) {
      ((TimerHandler *) this)->sched(delay);
   } else {
      abort();
   }
}

void UmtsTimer::resched(double delay)
{
   if (delay >= 0) {
      ((TimerHandler *) this)->resched(delay);
   } else {
      abort();
   }
}


// void HsdpaMacTimer::cancel()
// {

//    if (status_ != TIMER_PENDING) {
//       return;
//    } else {
//       ((TimerHandler *) this)->cancel();
//    }
// }

// void HsdpaMacTimer::sched(double delay)
// {
//    if (delay >= 0) {
//       ((TimerHandler *) this)->sched(delay);
//    } else {
//       abort();
//    }
// }

// void HsdpaMacTimer::resched(double delay)
// {
//    if (delay >= 0) {
//       ((TimerHandler *) this)->resched(delay);
//    } else {
//       abort();
//    }
// }
