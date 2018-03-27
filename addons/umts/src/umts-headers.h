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
 *
 * Ported spring 2007 from EURANE to NSMIRACLE 
 * by Nicola Baldo and Marco Miozzo
 * SIGNET Group, Department of Information Engineering,
 * University of Padova
 *
 *
 */

/*
 * 
 */

#ifndef ns_umts_headers_h
#define ns_umts_headers_h

// Type of Payload in MAC frame
#define ETHERTYPE_RLC 0x0900 

/* Data RLC PDU (with eventual piggybacked STATUS) */
#define    RLC_DATA     0x8000         

/* Stand-alone STATUS PDU */   
#define    RLC_STATUS   0x0000 

/* Positive ACK (like Tahoe) */        
#define    RLC_ACK      0x0001 

/* Bitmap SUFI */
#define    RLC_BITMAP   0x0002         

/* Move Receiver Window request SUFI
 * sent by sender to notify SDU discard */
#define    RLC_MRW      0X0004         

/* Move Receiver Window request acknowledged SUFI
 * sent by receiver to notify RLC_MRW reception */   
#define    RLC_MRWACK   0X0008         


// Maximum number of octets used for the BITMAP SUFI
#define MAX_BITMAP_OCTETS 16


struct hdr_rlc {
   packet_t    lptype_;         // SDU type
   int         lerror_;         // error flag of SDU
   double      lts_;            // ts value of SDU
   int         lsize_;          // payload SDU size

   int         lltype_;        // RLC frame type
   int         seqno_;          // PDU sequence number
   int         a_seqno_;        // PDU sequence number
   int         eopno_;          // end of SDU seqno
   int         segment_;        // the segment number of the PDU, starts with 0 for the first
   // PDU of a SDU.
   bool        poll_;           // poll flag

   int         payload_[3];
   int         lengthInd_;
   int         padding_;

  /** 
   * BITMAP SUperFIeld 
   * See 3GPP TS 25.322 v5.13.0 (UMTS RLC Specifications)
   * section 9.2.2.11.5 
   */
  int         FSN_;             ///< The first sequence number in the bitmap
  int         length_;          ///< length of the content of the
				///  BITMAP SUFI. Note that this in BITS
				///  and not in octets as per the standard.
  u_int8_t    bitmapsufi_[MAX_BITMAP_OCTETS];   // BITMAP SUFI using bit fields

  /* MRW SUperFIeld */
  int Nlength;          
  int SN_MRW;            // In practice, the new desired value for AM::next_

  /* MRW ACK SUperFIeld */
  int SN_MRW_ACK;        // The current value at the receiver for AM::next_, i.e., VR(R)
                         // This is sent to the transmitter so it can
                         // update FSN_, i.e., VT(A)
  
  int src_rlc_id_;           // Unique identifier of source RLC instance
  int dst_rlc_id_;           // Unique identifier of dest   RLC instance

   nsaddr_t    src_;
   nsaddr_t    dst_;

   static int  offset_;
   inline int &offset() {
      return offset_;
   } static hdr_rlc *access(const Packet * p) {
      return (hdr_rlc *) p->access(offset_);
   }

   packet_t & lptype() {
      return (lptype_);
   }
   int        &lerror() {
      return lerror_;
   }
   double     &lts() {
      return (lts_);
   }
   int        &lsize() {
      return lsize_;
   }

  int& lltype() {
      return lltype_;
   }
   int        &seqno() {
      return seqno_;
   }
   int        &a_seqno() {
      return a_seqno_;
   }
   int        &eopno() {
      return eopno_;
   }
   int        &segment() {
      return segment_;
   }
   bool       &poll() {
      return poll_;
   }

   int        &FSN() {
      return FSN_;
   }
   int        &length() {
      return length_;
   }

  /** 
   * Returns the i-th bit of the BITMAP SUFI
   * 
   * @param i position of the bit. must be >=0 and <128
   * 
   * @return 0 if bit is 0, 1 if bit is 1
   */
   int        bitmap(int i) {
     assert(i>=0);
     assert(i<128);
      return ((bitmapsufi_[i / 8] & ((0x01)<<(i%8))) >> (i%8));
   }

      /** 
       * Sets the i-th bit of the BITMAP SUFI to 1
       * 
       * @param i 
       */
   void        setbitmap(int i) {
     assert(i>=0);
     assert(i<128);
     bitmapsufi_[i / 8] |= ((0x01)<<(i%8));
   }

   int        &lengthInd() {
      return lengthInd_;
   }
   int        &padding() {
      return padding_;
   }
   int        &payload(int i) {
      return payload_[i];
   }

   nsaddr_t & src() {
      return (src_);
   }
   nsaddr_t & dst() {
      return (dst_);
   }
};






struct hdr_umtsphy {
  int bs_code_id;        // scrambling code id of the BS
  int me_code_id;        // scrambling code id of the ME
  int coding_type;       // channel coding type (convolutional, etc.)
  double coding_rate;    // rate of the channel coding type
  int spreading_factor;  // spreading factor
  int bits_per_symbol;   // yeah
  int direction;         // uplink or downlink
  bool data;		 // true when packet contains data, otherwise is a control packet
  bool powerUp;		 // true when the power control algorithm has to command to reduce the power of a step 


  static int  offset_;
  inline int &offset() { return offset_; } 
  static hdr_umtsphy *access(const Packet * p) {
    return (hdr_umtsphy *) p->access(offset_);
  }
  
};

#define HDR_RLC(p) (hdr_rlc::access(p))
#define HDR_UMTSPHY(p) (hdr_umtsphy::access(p))







#endif
