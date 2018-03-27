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
 * $Id: um.cc,v 1.1 2005/04/27 14:30:10 simon Exp $
 */

// UM is basically UM with all the support for multiple flows taken out. Also,
// the credit allocation algorithm is replaced by a simple 'send-x-PDUs-per-TTI'
// algorithm.

#include <um.h>
#include <mac.h>
#include <flags.h>
#include <random.h>
#include <address.h>
#include <iostream>


static class UMRlcClass:public TclClass 
{
public:
  UMRlcClass():TclClass("UMTS/RLC/UM")   {  } 
  TclObject *create(int, const char *const *) {
    return (new UM());
  }
} class_um;



UM::UM():RLC(), temporaryPacket_(this), nextExpectedSDU_(0), nextExpectedSeqno_(0),
nextExpectedSegment_(0), errorInSDU_(0), seqno_(0), send_status_(0),
SDU_size_(0), maxseq_(-1), highest_ack_(0), maxseen_(0), next_(0),
d_address_(0), macDA_(0), tti_timer_(this, RLC_TIMER_TTI)
{
   bind("win_", &win_);
   bind_time("temp_pdu_timeout_time_", &tempPDUTimeOutTime_);
   bind("buffer_level_max_", &bufferLevelMax_);
   bind("payload_", &payloadSize_);
   bind_time("TTI_", &TTI_);
   bind("TTI_PDUs_", &TTI_PDUs_);
   bind("length_indicator_", &length_indicator_);
   bind("min_concat_data_", &min_concat_data_);
   bind("bandwidth_", &bandwidth_);
   bind("macDA_", &macDA_);

   // Calculate the size in bytes of Length Indicator including the
   // Extention bit. If a PDU consists of multiple parts, for each
   // part these two fields are added to the header. If the PDU consists
   // of only one part, these fields are omitted (9.2.1.4 + 9.2.2.5 from
   // 3GPP TS 25.322 v5.6.0).
   lengthIndicatorSize_ = ((length_indicator_ + 1) / 8);
   temporaryPacket_.p = NULL;

   tti_timer_.sched(TTI_ - 0.001);

}


int UM::command(int argc, const char *const *argv)
{
   if (argc == 2) {
      Tcl & tcl = Tcl::instance();
      if (strcmp(argv[1], "TTI") == 0) {
         tcl.resultf("%f", TTI_);
         return TCL_OK;
      } else if (strcmp(argv[1], "BW") == 0) {
         tcl.resultf("%f", bandwidth_);
         return TCL_OK;
      } // else if (strcmp(argv[1], "start_TTI") == 0) {
//          // do nothing (so ignore this methodcall, the timers are started by the
//          // constructor)
//          return TCL_OK;
//       }
   } else if (argc == 3) {
      if (strcmp(argv[1], "macDA") == 0) {
         int nodeID = Address::instance().str2addr(argv[2]);

         macDA_ = nodeID;
         return (TCL_OK);
      } else if (strcmp(argv[1], "addr") == 0) {
         address_ = Address::instance().str2addr(argv[2]);
         return (TCL_OK);
      } else if (strcmp(argv[1], "daddr") == 0) {
         int nodeID = Address::instance().str2addr(argv[2]);

         d_address_ = nodeID;
         return (TCL_OK);
      }
   }
   int returnvalue = LL::command(argc, argv);


   return returnvalue;
}

void UM::recv(Packet * p, Handler * h)
{
  if (debug_)
    std::cerr << "entering UM::recv()" << std::endl;

   hdr_cmn    *ch = HDR_CMN(p);

   // If the direction is up, we don't have to change anything, and the packet
   // can be processed.
   if (ch->direction() == hdr_cmn::UP) {
      hdr_rlc    *llh = hdr_rlc::access(p);

      if (debug_)
	std::cerr << NOW << " UM::recv() packet going UP" << std::endl;

      if (llh->dst() != address_ || ch->error() > 0) {
         // The packet was meant for another UE, or it contained errors. In
         // both cases it will be dropped.
	if (debug_)
	  std::cerr << NOW << " UM::recv() dropping packet"
		    << " llh->dst()=" << llh->dst()
		    << " address_=" << address_  
		    << " ch->error()=" << ch->error()
		    << std::endl;
	Packet::free(p);
	return;
      }
      // Check the type op packets
      if (llh->lltype() == RLC_DATA) {
         // We only pass the last PDU of an SDU. The rest of the PDUs are not
         // needed for the reassembly and can be freed.

         hdr_tcp    *tcph = hdr_tcp::access(p);
         hdr_rlc    *llh = hdr_rlc::access(p);
         int temp_sdu = tcph->seqno();
         int temp_seqno = llh->seqno();
         int temp_segment = llh->segment();


         if (temp_seqno == llh->eopno()) {
            // The packet is the last PDU of an SDU. Now, check whether the
            // complete SDU has been received. If so, pass the SDU up. In either
            // case, the next expected packet will be the first PDU of the next
            // SDU.
            if (nextExpectedSeqno_ == temp_seqno && nextExpectedSDU_ == temp_sdu
                && errorInSDU_ == 0) {
               makeSDU(p);
            } else {
               Packet::free(p);
            }
            errorInSDU_ = 0;
            nextExpectedSDU_ = temp_sdu + 1;
            nextExpectedSeqno_ = temp_seqno + 1;
         } else {
            // The packet is not the last PDU of an SDU. When the packet is the
            // expected packet, increase the next expected packet. If not, the
            // next packet is the next packet after this one. In that case,
            // set errorInSDU so that this current SDU will not be passed up,
            // unless this is the first segment of the SDU. In that case, the
            // missed PDU didn't belong to this SDU but to the previous one.
            if (nextExpectedSeqno_ == temp_seqno) {
               nextExpectedSeqno_++;
            } else {
               nextExpectedSDU_ = temp_sdu;
               nextExpectedSeqno_ = temp_seqno + 1;
               if (temp_segment != 0) {
                  errorInSDU_++;
               }
            }
            Packet::free(p);
         }

      } else {
	if (debug_)
	  std::cerr << "UM::recv() dropping packet"
		    << " llh->type=" << llh->lltype()
		    << " ch->error()=" << ch->error()
		    << std::endl;
	  

         // In case the type of the packet is something else, just drop it.
         drop(p);               // Unknown type, inclusing Positive Acknowledgements
      }
      return;
   } else {
      // We think that this should always be the case, so why set it again? For
      // testing purposes we should replace the following line by an assert
      // statement to check it.
      assert(ch->direction() == hdr_cmn::DOWN);
      sendDown(p);
   }

}

//----------------------------------------------------------------
//enquePacket
//
//if possible (i.e. buffers are not full) enqueue packet in both
// transmission and re-transmissionbuffer.
// return 1 after succesfull enqueing
// if not possible, return 0
int UM::enquePacket(Packet * p)
{

   int r_value = 0;


   r_value = enqueInBackOfTransmissionBuffer(p);


   return r_value;
}

int UM::enqueInBackOfTransmissionBuffer(Packet * p)
{
// return 1 when succesfully enqueued, 0 when buffer is full

   int r_value = 0;


   if (transmissionBuffer_.length() >= bufferLevelMax_) {
      drop(p);
   } else {
      transmissionBuffer_.enque(p);
      r_value = 1;
   }


   return r_value;
}



//---------------------------
// sendDown processes a SDU into PDUs for transmission to MAC-HS
// if possible, waiting temporary packets are filled up.
//---------------------------

void UM::sendDown(Packet * p)
{

   // First check whether the SDU fits into the transmission buffer completely

   int numPDUs =
         (int) ceil((double) hdr_cmn::access(p)->size() /
                    (double) payloadSize_);

   if (numPDUs + transmissionBuffer_.length() >= bufferLevelMax_) {
      Packet::free(p);
      return;
   }

   int segment = 0;

   // First check whether there is still a not-finished PDU available that can
   // be used for concatenation.
   handleTemporaryPacket(p);

   if (hdr_rlc::access(p)->segment() == 0) {
      // There was something that could be concatenated, and thus the first
      // segment of the packet was already sent, so continue with the next.
      segment++;
   }

   while (p != NULL) {
      // Each loop the front of the SDU is removed and send in a PDU. This
      // results in a smaller and smaller SDU, until at some point in time
      // it is completely gone. In that case p equals to NULL and the
      // complete SDU is segmented.

      int concat_data = 0;
      bool close_PDU = false;
      int padding = 0;
      bool force_length_indicator = false;
      Packet     *c;

      int p_size = hdr_cmn::access(p)->size();

      close_PDU = p_size <= payloadSize_;
      force_length_indicator = p_size < payloadSize_;

      if (p_size <= (payloadSize_ - lengthIndicatorSize_)) {
         // decide whether the remaining data is too big too keep
         // in which case we send it on with padding
         concat_data = payloadSize_ - p_size - 2 * lengthIndicatorSize_;
         if (concat_data < min_concat_data_) {
            concat_data = 0;
            padding = payloadSize_ - p_size - lengthIndicatorSize_;
         } else {
            padding = 0;
         }
      }

      if (close_PDU) {
         // reuse packet p
         c = p;
         p = NULL;
      } else {
         // copy p's data into c and force concat_data and padding to 0
         concat_data = 0;
         padding = 0;
         c = p->copy();
      }


      //initialise shortcuts to c's header info
      hdr_cmn    *c_ch = HDR_CMN(c);
      hdr_rlc    *c_llh = hdr_rlc::access(c);
      hdr_cmn    *p_ch = NULL;
      hdr_ip     *p_iph = NULL;

      if (p != NULL) {
         p_ch = HDR_CMN(p);
         p_iph = hdr_ip::access(p);
      }

      c_llh->lptype() = c_ch->ptype();
      c_llh->lerror_ = c_ch->error();
      c_llh->lts_ = c_ch->timestamp();
      c_llh->segment() = segment;
      segment++;

      if (close_PDU) {

         if (SDU_size_ > 0) {
            c_llh->lsize_ = SDU_size_;
         } else {
            c_llh->lsize_ = c_ch->size();
         }

         SDU_size_ = -1;

      } else if (SDU_size_ < 0) {
         assert(p != NULL);
         SDU_size_ = hdr_cmn::access(p)->size();
      }

      c_llh->lltype() = RLC_DATA;
      c_llh->seqno() = seqno_;
      seqno_++;
      c_llh->dst() = d_address_;
      c_llh->src() = address_;
      c_llh->lengthInd_ = 0;

      // [Nicola] This is totally crazy. Don't you see that
      // c_llh->seqno_ has been set a few lines above???
//       if (close_PDU) {
//          // c_llh->eopno_ needs to be set to c_llh->seqno_. However, at this
//          // moment the seqno is not known yet. So, set it to EOPNO_TO_SEQNO
//          // and check when seqno is set whether eopno is EOPNO_TO_SEQNO. If this
//          // is the case eopno can be replaced by seqno.
//          c_llh->eopno_ = EOPNO_TO_SEQNO;
//       } else {
//          c_llh->eopno_ = EOPNO_NOT_EOP;
//       }

      // [Nicola] This is much better:
      if (close_PDU) {        
	c_llh->eopno_ = c_llh->seqno();
      } else {
	c_llh->eopno_ = EOPNO_NOT_EOP;
      }
      if (concat_data || padding || force_length_indicator) {
         c_llh->lengthInd_++;
      }

      if (close_PDU) {
         c_llh->payload_[PAYLOAD_FLD1] = c_ch->size();
      } else {
	if (force_length_indicator) {
	  c_llh->payload_[PAYLOAD_FLD1] = payloadSize_ - lengthIndicatorSize_;
	} else {
	  c_llh->payload_[PAYLOAD_FLD1] = payloadSize_;
	}
      }
      c_llh->payload_[PAYLOAD_FLD2] = 0;
      c_llh->payload_[PAYLOAD_FLD3] = 0;

      c_llh->padding_ = padding; //padding will always have the correct value here

      // WTF? this is really ugly, bad things will happen here!
      char       *mh = (char *) c->access(hdr_mac::offset_);
      struct hdr_mac *c_dh = (struct hdr_mac *) mh;

      c_ch->ptype() = PT_UM;
      c_dh->macDA_ = macDA_;
      c_dh->macSA_ = -1;
      c_dh->hdr_type() = ETHERTYPE_RLC;
      c_ch->timestamp() = Scheduler::instance().clock();
      c_ch->direction() = hdr_cmn::DOWN;

      // Do we have space left over for concatenation?
      if (close_PDU) {
         if (concat_data) {
            // Store the current packet and do not send it down.
            // Because this is also the last part of the SDU we can
            // stop this method.
            c_llh->lengthInd_++;

            StoreTemporaryPacket(c, concat_data);
            break;              //exit while loop
         }
      } else {
         // reduce the size of the SDU by an amount of the
         // payload that will be sent in this PDU.
         p_ch->size() = p_ch->size() - c_llh->payload_[PAYLOAD_FLD1];
      }

      assert(c_ch->error() == 0);

      c_ch->size() = payloadSize_;


      //----------
      if (enquePacket(c) == 0) {
      } else {
      }

   }


}

// This method is called when a PDU is created that is not full, and just stores
// the PDU into a vector that will be inspected

void UM::StoreTemporaryPacket(Packet * p, int concat_data)
{

   temporaryPacket_.p = p;
   temporaryPacket_.concat_data = concat_data;
   // We can't wait forever, because then we have a problem at the end of a
   // burst of traffic: then the last PDU won't be send, causing timeouts and
   // retransmissions. So, we do want to send the incomplete packet anyway
   // at some point in time.
   temporaryPacket_.tempPDUTimer.sched(tempPDUTimeOutTime_);
}


void UM::handleTemporaryPacket(Packet * p)
{


   if (p == NULL) {
      return;
   }
   // Fill the temporary PDU with data from the new SDU, and delete that
   // part of the SDU. Then store the temporary PDU in the Transmission
   // and Retransmission Buffers and return the remaining part of the SDU.

   // Concatenation (3GPP TS 25.301 5.3.2.2): If the contents of an RLC
   // SDU cannot be carried by one RLC PDU, the first segment of the next
   // RLC SDU may be put into the RLC PDU in concatenation with the last
   // segment of the previous RLC SDU.

   if (temporaryPacket_.p != NULL) {

      if (hdr_cmn::access(p)->size() > payloadSize_) {

         // TODO: check whether temporaryPacket exists

         hdr_rlc::access(temporaryPacket_.p)->payload_[1] =
               temporaryPacket_.concat_data;
         // this variable was previously set in makePDU
         SDU_size_ = hdr_cmn::access(p)->size();
         hdr_cmn::access(p)->size() =
               hdr_cmn::access(p)->size() - temporaryPacket_.concat_data;

         // Set the segemnt to 0, in this way it is known that segmentation
         // did occur.
         hdr_rlc::access(p)->segment() = 0;

      } else {

         hdr_rlc::access(temporaryPacket_.p)->lengthInd_--;
         hdr_rlc::access(temporaryPacket_.p)->padding_ =
               temporaryPacket_.concat_data + lengthIndicatorSize_;
      }
      enquePacket(temporaryPacket_.p);

      // Now the temporary PDU has been sent, delete the timer
      // so that the timeout will not occur anymore.
      temporaryPacket_.tempPDUTimer.cancel();
      temporaryPacket_.p = NULL;
   }
}


void UM::timeout(int tno, int flowID)
{
   vector < int >temp_vect;
   double TTI_time_;

   switch (tno) {
     case RLC_TIMER_TEMP_PDU:

        // Before we stored the temporary PDU in the
        // vector, we set the number of Length Indicators
        // and the number of bytes that could be
        // concatenated. Because we are not concatenating,
        // but padding we need one Length Indicator less,
        // thus we have to pad more.
        hdr_rlc::access(temporaryPacket_.p)->lengthInd_--;
        hdr_rlc::access(temporaryPacket_.p)->padding_ =
              temporaryPacket_.concat_data + lengthIndicatorSize_;
        enquePacket(temporaryPacket_.p);

        break;
     case RLC_TIMER_TTI:
        TTI_time_ = Scheduler::instance().clock();

	// TTI_PDUs_ now is fixed and determined via TCL
	//TTI_PDUs_ = (int) (bandwidth_ * TTI_) / (payloadSize_ * 8);

        tti_timer_.resched(TTI_);
        for (int i = 0; i < TTI_PDUs_; i++) {
           //send_packet_down
           Packet     *p = transmissionBuffer_.deque();

           if (p != NULL) {
	     hdr_cmn *ch = HDR_CMN(p);
	     hdr_rlc    *llh = hdr_rlc::access(p);
	     if (debug_)
	       {

		 std::cerr << "UM::timeout() sending packet"
			   << " seqno=" << llh->seqno()
			   << " eopno=" << llh->eopno()		   
			   << std::endl;		

	       }
	     //assert(ch->direction() == hdr_cmn::DOWN);
	     ch->direction() = hdr_cmn::DOWN;
	     downtarget_->recv(p);
           }
        }
        break;
     default:
        break;
   }

}

void UM::completePDU(Packet * p)
{


   hdr_rlc    *llh = hdr_rlc::access(p);

   // If the packet is transmitted for the first time, set the sequence number
   // and possibly the eop number.
   if (llh->seqno() == -1) {
      llh->seqno() = seqno_;
      seqno_++;
      if (llh->eopno() == EOPNO_TO_SEQNO) {
         llh->eopno() = llh->seqno();
      }
   }
   return;
}

void UM::makeSDU(Packet * p)
{
   hdr_cmn    *ch;
   hdr_rlc    *llh;

   assert(p);
   ch = HDR_CMN(p);
   llh = hdr_rlc::access(p);

   ch->ptype() = llh->lptype();
   ch->error() = llh->lerror();
   ch->timestamp() = llh->lts();
   ch->size() = llh->lsize();

   assert(ch->direction() == hdr_cmn::UP);

   uptarget_ ? sendUp(p) : Packet::free(p);
}

int UM::buff_size()
{
   return(transmissionBuffer_.size());
}

void UM::CSwitch(double bandwidth, double TTI)
{
	   bandwidth_ = bandwidth;
	      TTI_ = TTI;
}

