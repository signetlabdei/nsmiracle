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


/**
 * @file   underwater-shannon.cc
 * @author Nicola Baldo
 * @date   Tue Nov 27 16:15:10 2007
 * 
 * @brief  
 * 
 * 
 */


#include"underwater-shannon.h"
#include"underwater-mpropagation.h"
#include<node-core.h>

#include<iostream>


static class UWMPhyShannonClass : public TclClass {
public:
  UWMPhyShannonClass() : TclClass("Module/MPhy/UWShannon") {}
  TclObject* create(int, const char*const*) {
    return (new UnderwaterMPhyShannon);
  }
} class_UWMPhyShannon;


    
bool UnderwaterMPhyShannon::initialized = false;
int UnderwaterMPhyShannon::modid = -1;



UnderwaterMPhyShannon::UnderwaterMPhyShannon()
  : PktRx(0), 
    txready(false), 
    destPos_(0),   
    last_fl(0),
    last_fr(0),
    last_dist(0),
    last_txpow(0)
{
  if (!initialized) 
    {
      modid = MPhy::registerModulationType(UWMPHYSHANNON_MODNAME);
      initialized = true;
    }
  bind("debug_",&debug_);
}


int UnderwaterMPhyShannon::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
	
  if (argc==3)
    {
      if (strcasecmp(argv[1],"setDestPosition")==0)
	{
	  destPos_ = dynamic_cast<Position*>(TclObject::lookup(argv[2]));
	  if (!destPos_)
	    return TCL_ERROR;
	  return TCL_OK;
	}
		
    }
  return MPhy::command(argc, argv);
}




/**
 * Frequency resolution for calculating integrals, in Hz
 * 
 */
#define UW_FREQ_RESOLUTION 10


void UnderwaterMPhyShannon::calculateCapacity(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);
  
  /* Distance between src and destination */
  /* Note: destination position must be known */
  assert(destPos_);
  double dist = getPosition()->getDist(destPos_);

  UnderwaterMPropagation* uwmp = dynamic_cast<UnderwaterMPropagation*>(propagation_);
  assert(uwmp);

  double fc = spectralmask_->getFreq();
  double bw = spectralmask_->getBandwidth();
  double fl = fc - bw*0.5;
  double fr = fc + bw*0.5;
  double txpow = getTxPower(p);

  if ((last_fl == fl) && (last_fr == fr) && (last_dist == dist) && (last_txpow = txpow))
    {
      if (debug_)
	    cerr << NOW << " " <<  __PRETTY_FUNCTION__ 
		 << " capacity unchanged from last call" << endl;

    }
  else
    {    
      capacity = uwmp->uw.getCapacity(txpow , dist, fl, fr, UW_FREQ_RESOLUTION);

      last_fl = fl;
      last_fr = fr;
      last_dist = dist;
      last_txpow = txpow;

      // The following is just to produce a nicer tracefile... we
      // calculate SNIR at the center frequency, assuming flat (!) noise
      // SPD within the band of interest.
  
      double attdB = uwmp->uw.getAttenuation(dist, fc/1000.0);
      last_Pr = getTxPower(p) / (pow(10, attdB/10.0)) ;
      double noiseSPDdBperHz = uwmp->uw.getNoise(fc/1000.0);
      last_Pn = bw *  pow(10, noiseSPDdBperHz/10.0);
    }

  ph->Pr = last_Pr;
  ph->Pn = last_Pn;


  if (debug_)
    cerr << NOW << " " << __PRETTY_FUNCTION__ 
	 << " fc= " << fc
	 << " bw= " << bw
	 << " fl= " << fl
	 << " fr= " << fr
	 << " TxP= " << txpow
	 << " SNR= " << last_Pr / last_Pn
	 << " capacity= " << capacity
	 << endl;

  txready = true;

}


double UnderwaterMPhyShannon::getNoisePower(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);
  if (ph->Pn > 0)
    return (ph->Pn);
  else
    {
      cerr << NOW << " " << __PRETTY_FUNCTION__ 
	   << " S=" << ph->Pr << " N=" << ph->Pn << endl;
      return 1e-17;
    }
}


int UnderwaterMPhyShannon::getModulationType(Packet*)
{
  assert(initialized);
  return modid;
}


double  UnderwaterMPhyShannon::getTxDuration(Packet* p)
{
  calculateCapacity(p);
  hdr_cmn* ch = HDR_CMN(p);
  return (ch->size() * 8 / capacity);

}



void UnderwaterMPhyShannon::startTx(Packet* p)
{
  /**
   * @bug we should also handle all possible concurrencies between TX and RX!!!
   * 
   */

  assert(txready);
  sendDown(p);
  txready = false;
}


void UnderwaterMPhyShannon::endTx(Packet* p)
{
  // Notify the MAC
  Phy2MacEndTx(p);
}



void UnderwaterMPhyShannon::startRx(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);

  /* ideal synchronization */


  if (PktRx == 0)
    {
      if (debug_)
	cerr << NOW << " " <<  __PRETTY_FUNCTION__ <<" SYNC on new packet" << endl;
      // The receiver is is not synchronized on any transmission
      // so we can sync on this packet

      if (ph->modulationType == modid)
	{
	  // This is a UWSHANNON packet so we sync on it
	  PktRx = p;

	  // Notify the MAC
	  Phy2MacStartRx(p);
	}    
    }
  else
    {
      if (debug_)
	cerr << NOW << " " <<  __PRETTY_FUNCTION__ << " already SYNCed, packet will not be received" << endl;

    }

}


void UnderwaterMPhyShannon::endRx(Packet* p)
{

  if (PktRx != 0)
    {
		
      hdr_cmn* ch = HDR_CMN(p);
      hdr_cmn* rxch = HDR_CMN(PktRx);
	
	
			
      if ( ch->uid()  == rxch->uid() )
	{  
	  // We had synchronized onto this packet so we now try to see if
	  // it has been received correctly
	
	  hdr_MPhy* ph = HDR_MPHY(p);
	  		  
	  double ber, per;

	  // We assume that if interference is non-negligible,
	  // i.e., if it is not at least 3dB below noise power,
	  // the packet will be dropped because of errors

	  ch->error() = (ph->Pn < 2*ph->Pi);

	  if (ch->error())
	    {
	      drop(p,1,MPHY_DROP_ERROR_INTERFERENCE);
	    }
	  else
	    {			
	      sendUp(p);
	    }
	
	  PktRx = 0; // We can now sync onto another packet
	  if (debug_)
	    cerr << NOW << " " <<  __PRETTY_FUNCTION__ << " ready to SYNC on another packet " << endl;

	    
	}
      else
	{
	  // We did not synchronize on this packet, so we are
	  // not attempting to receive this transmission
	  
	  Packet::free(p);
	}
    }	
}



// MSpectralMask* UnderwaterMPhyShannon::getTxSpectralMask(Packet* p)
// {
// }



// MSpectralMask* UnderwaterMPhyShannon::getRxSpectralMask(Packet* p)
// {  
// }

