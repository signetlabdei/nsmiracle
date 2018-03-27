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
 * 
 * thanks to: Roberto Petroccia for the channel sense extension
 * 
 */

#include"bpsk.h"

#include<rng.h>
#include<iostream>
#include<phymac-clmsg.h> 

enum {
  BPSK_DROPPED_REASON_NOT_SET = -10, BPSK_DROPPED_REASON_WRONG_MODID = 10, BPSK_DROPPED_REASON_NOISE,
  BPSK_DROPPED_REASON_DEAFNESS, BPSK_DROPPED_REASON_TX_PENDING
};

static class MPhy_BpskClass : public TclClass {
public:
  MPhy_BpskClass() : TclClass("Module/MPhy/BPSK") {}
  TclObject* create(int, const char*const*) {
    return (new MPhy_Bpsk);
  }
} class_MPhy_Bpsk;


bool MPhy_Bpsk::initialized = false;
int MPhy_Bpsk::modid = -1;



MPhy_Bpsk::MPhy_Bpsk()
  : PktRx(0), txPending(false), droppedPktsTxPending(0)
{
  if (!initialized) 
    {
      modid = MPhy::registerModulationType(BPSK_MODNAME);
      initialized = true;
    }
//   bind("AcquisitionThreshold_dB_", &AcquisitionThreshold_dB_);
  bind("BitRate_", &BitRate_);
//   countFailBER = 0;

}


int MPhy_Bpsk::command(int argc, const char*const* argv)
{
  //printf("MPhy::command -- %s (%d)\n", argv[1], argc);
  Tcl& tcl = Tcl::instance();

  if(argc == 2)
    {
      if(strcasecmp(argv[1], "getDroppedPktsTxPending")==0)
	{
	  tcl.resultf("%d",getDroppedPktsTxPending());
	  return TCL_OK;
        }
    }
  return MPhy::command(argc, argv);
}


int MPhy_Bpsk::getModulationType(Packet*)
{
  assert(initialized);
  return modid;
}


double  MPhy_Bpsk::getTxDuration(Packet* p)
{
  hdr_cmn* ch = HDR_CMN(p);
  hdr_MPhy* ph = HDR_MPHY(p);
  
  assert(ph->srcSpectralMask);
  
  // for BPSK the bandwidth is B = 2/T, 
  // where T is the symbol duration.
  // Here B is pre-determined by the Spectral Mask,
  // so we can calculate the bitrate R = 1/T = B/2
  if ( BitRate_ <= 0 ) BitRate_ = ph->srcSpectralMask->getBandwidth() / 2.0 ;
  double txduration = (ch->size() * 8.0 / BitRate_);
  
  if (txduration <=0)
    {
      cerr << " ch->size(): " << ch->size()
	   << " bitrate: " << BitRate_ 
	   << std::endl;
    }
  assert(txduration > 0);
  
  if (debug_)
    cerr << showpoint << NOW << " " <<  __PRETTY_FUNCTION__ 
	 << " packet size: " << ch->size() 
	 << " tx duration: " << txduration 
	 << endl;
  
  return (txduration);
} 


void MPhy_Bpsk::startTx(Packet* p)
{
  // we abort any ongoing rx activity

  if (PktRx != 0) {
    hdr_MPhy* ph = HDR_MPHY(PktRx);
    double rx_time = ph->rxtime;
   
//     drp_rsn_map[ph->txtime][ph->rxtime] = BPSK_DROPPED_REASON_TX_PENDING;
  }
  PktRx = 0;
  txPending = true;
  sendDown(p);
}


void MPhy_Bpsk::endTx(Packet* p)
{
  txPending = false;
  // Notify the MAC
  Phy2MacEndTx(p);
}



void MPhy_Bpsk::startRx(Packet* p)
{
  static int mac_addr = -1;
  
  if (debug_) {
    ClMsgPhy2MacAddr msg;
    sendSyncClMsg(&msg);
    mac_addr = msg.getAddr();
  }
  
  hdr_MPhy* ph = HDR_MPHY(p);
  double rx_time = ph->rxtime;
  double tx_time = ph->txtime;
  /* ideal synchronization */
//printf("(%12.12f Node:%d MPhy_Bpsk::startRx\n", NOW, mac_addr);
  if ( (PktRx == 0) && (txPending == false) )
    {
       // The receiver is is not synchronized on any transmission
      // so we can sync on this packet

      double snr_dB = 10*log10(ph->Pr / ph->Pn);

      if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::startRx() " << "snr_dB = " << snr_dB 
                       << "; AcquisitionThreshold_dB_ = " << getAcquisitionThreshold() << " pr " << 10*log10(ph->Pr) 
                       << " pn " << 10*log10(ph->Pn) << " end " << NOW + HDR_CMN(p)->txtime() << " src " << HDR_CMN(p)->prev_hop_
                       << " dest " << HDR_CMN(p)->next_hop()
                       << " size " << HDR_CMN(p)->size() << endl;

      if(snr_dB > getAcquisitionThreshold())
	{
// 	  std::cerr << " above threshold "<< std::endl;
	  if (ph->modulationType == modid)
	    {
	      // This is a BPSK packet so we sync on it
	      PktRx = p;

	      // Notify the MAC
	      Phy2MacStartRx(p);

             if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::startRx() " << " sync on PktRx = " << PktRx 
                              << " end " << NOW + HDR_CMN(p)->txtime() << " src " << HDR_CMN(p)->prev_hop_
                              << " dest " << HDR_CMN(p)->next_hop()
                              << " size " << HDR_CMN(p)->size() <<  endl;

	      return;
	    }    
            else {
//                drp_rsn_map[tx_time][rx_time] = BPSK_DROPPED_REASON_WRONG_MODID;

               if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::startRx() dropping pkt, wrong mod id" << endl;
            }
	}
      else
	{
//            drp_rsn_map[tx_time][rx_time] = BPSK_DROPPED_REASON_NOISE;

           if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::startRx() dropping pkt, below threshold" << endl;
//            waitForUser();
	}
    }
    else if (txPending == true) {
//        drp_rsn_map[tx_time][rx_time] = BPSK_DROPPED_REASON_TX_PENDING;

       if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::startRx() dropping pkt, tx pending" << endl;
//        waitForUser();
    }
    else {
//        drp_rsn_map[tx_time][rx_time] = BPSK_DROPPED_REASON_DEAFNESS;
//        waitForUser();

    }
}


void MPhy_Bpsk::endRx(Packet* p)
{
  static int mac_addr = -1;
  
  if (debug_) {
    ClMsgPhy2MacAddr msg;
    sendSyncClMsg(&msg);
    mac_addr = msg.getAddr();
  }
  
  hdr_cmn* ch = HDR_CMN(p);
  hdr_MPhy* ph = HDR_MPHY(p);

  if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::endRx() start rx time = " << ph->rxtime << endl;

  if (PktRx != 0)
    {

      if (PktRx == p)
	{  
	  // We had synchronized onto this packet so we now try to see if
	  // it has been received correctly
          if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::endRx() receiving sync pkt" << " dest " << HDR_CMN(p)->next_hop() << endl;

	  double per_ni; // packet error rate due to noise and/or interference
	  double per_n;  // packet error rate due to noise only 

	  int nbits = ch->size()*8;
	  double x = RNG::defaultrng()->uniform_double();
	  per_n = getPER(ph->Pr/ph->Pn, nbits);
	  bool error_n = x <= per_n;
		
// 		cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::endRx() first nbits = " << nbits 
// 		     << ";  per = " << per_n << "; x = " << x << "; error_n = " << error_n << endl;
				 
	  bool error_ni = 0;
	  
	  if (!error_n) {
			if (interference_) {	
						const PowerChunkList& power_chunk_list = interference_->getInterferencePowerChunkList(p);

						for (PowerChunkList::const_iterator itInterf = power_chunk_list.begin(); itInterf != power_chunk_list.end(); itInterf++) {
							int nbits2 = itInterf->second * BitRate_;
							per_ni = getPER(ph->Pr/(ph->Pn + itInterf->first), nbits2);		
							x = RNG::defaultrng()->uniform_double();
							error_ni = x <= per_ni;
							
// 							cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::endRx() nibts2 = " << nbits2 << "; per_ni = " << per_ni
// 							     << "; x = " << x << "; error_ni = " << error_ni << endl;
									 
							if (error_ni) {
								break;
							}
						}
			}
			else {
				per_ni = getPER(ph->Pr/(ph->Pn + ph->Pi), nbits);
				error_ni = x <= per_ni;	      
			}
	  }
	  
	  ch->error() = error_ni || error_n ;
		
	  
	  if (error_n) {
             incrErrorPktsNoise();
	  }
	  else if (error_ni) {
             incrErrorPktsInterf();
	  }
          if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::endRx() receiving pkt" << " dest " << HDR_CMN(p)->next_hop() << " error_n " << error_n << " error_ni " << error_ni << endl;

	  // always send up; the PHY is not responsible for discarding erroneous packets
	  sendUp(p);

	
	  PktRx = 0; // We can now sync onto another packet
	}
      else
	{

          if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::endRx() already sync on another pkt, dropping rx pkt" << " dest " << HDR_CMN(p)->next_hop() << endl;
//           waitForUser();
	  // We synchronized on another packet, so we are
	  // not attempting to receive this transmission
//           checkDropReason(p);
	  dropPacket(p);
	}
    }
  else
    {

      if (debug_) cout << NOW << "  MPhy_Bpsk(" << mac_addr << ")::endRx() not synced on any pkt, dropping rx pkt" << " dest " << HDR_CMN(p)->next_hop() << endl;

//       waitForUser();
//       checkDropReason(p);
      dropPacket(p);
    }
	
}

void MPhy_Bpsk::checkDropReason(Packet* p) {
  assert(drp_rsn_map.size() > 0);

  hdr_MPhy* ph = HDR_MPHY(p);

  cout.precision(16);

  if (debug_) cout << NOW << "  MPhy_Bpsk::checkDropReason() pkt txtime = " << ph->txtime << "; rxtime = " 
                   << ph->rxtime << endl;

  map<double, map<double, int> >::iterator it = drp_rsn_map.lower_bound(ph->txtime);
  map<double,int>::iterator it2;

  if ( it != drp_rsn_map.end() ) {
      it2 = it->second.lower_bound(ph->rxtime);

      if ( it2 == it->second.end() ) {
         if (debug_) cout << NOW << "  MPhy_Bpsk::checkDropReason() ERROR, can't find rx time" << endl;
	 return; //exit(1);
      }
  }
  else {
      if(debug_) cout << NOW << "  MPhy_Bpsk::checkDropReason() ERROR, can't find tx time" << endl;
      return; //exit(1);
  }

  if (it2->second == BPSK_DROPPED_REASON_WRONG_MODID) {
      if (debug_) cout << NOW << "  MPhy_Bpsk::checkDropReason() pkt dropped due to wrong mod id" << endl;

      incrDroppedPktsWrongModId();
  }
  else if (it2->second == BPSK_DROPPED_REASON_NOISE) {
      if (debug_) cout << NOW << "  MPhy_Bpsk::checkDropReason() pkt dropped due to noise" << endl;

      incrDroppedPktsNoise();
  }
  else if (it2->second == BPSK_DROPPED_REASON_DEAFNESS) {
      if (debug_) cout << NOW << "  MPhy_Bpsk::checkDropReason() pkt dropped due to deafness conditions" << endl;

      incrDroppedPktsDeaf();
  } 
  else if (it2->second == BPSK_DROPPED_REASON_TX_PENDING) {
      if (debug_) cout << NOW << "  MPhy_Bpsk::checkDropReason() pkt dropped due to tx pending" << endl;

      incrDroppedPktsTxPending();
  }
  else {
      cout << NOW << "  MPhy_Bpsk::checkDropReason() wrong reason!" << endl;
//       exit(1);
  }
}

void MPhy_Bpsk::dropPacket(Packet* p) { 
//   hdr_MPhy* ph = HDR_MPHY(p);
// 
//   map<double, map<double, int> >::iterator it = drp_rsn_map.lower_bound(ph->txtime);
// 
//   if ( it != drp_rsn_map.end() ) {
//       it->second.erase(ph->rxtime);
//       
//   }
//   else {
//       if (debug_) cout << NOW << "  MPhy_Bpsk::dropPacket() ERROR, can't find tx time" << endl;
//       return;//exit(1);
//   }
// 
//   if ( it->second.size() == 0 ) drp_rsn_map.erase(ph->txtime);
 
  Packet::free(p); 
}

double MPhy_Bpsk::getPER(double snr, int nbits)
{	 
  double ber = 0.5*erfc(sqrt(snr));  
  double per = 1-pow(1 - ber, nbits );

// 	cout << NOW << "  MPhy_Bpsk()::getPER() snr = " << 10*log10(snr) << " db " << "; nbits = " << nbits
//        << "; ber = " << ber << "; per = " << per << endl;
			 
	return per;
}

void MPhy_Bpsk::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
} 

