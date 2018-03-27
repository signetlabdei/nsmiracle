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

#include"umts-phy.h"
#include"umts-headers.h"
#include<rng.h>
#include<iostream>
#include<math.h>
#include<ip.h>


static class UmtsPhyClass : public TclClass {
public:
  UmtsPhyClass() : TclClass("Module/MPhy/UMTS") {}
  TclObject* create(int, const char*const*) {
    return (new UmtsPhy());
  }
} class_UmtsPhy;


static class MEUmtsPhyClass : public TclClass {
public:
  MEUmtsPhyClass() : TclClass("Module/MPhy/UMTS/ME") {}
  TclObject* create(int, const char*const*) {
    return (new UmtsPhyME());
  }
} class_UmtsPhyME;


static class BSUmtsPhyClass : public TclClass {
public:
  BSUmtsPhyClass() : TclClass("Module/MPhy/UMTS/BS") {}
  TclObject* create(int, const char*const*) {
    return (new UmtsPhyBS());
  }
} class_UmtsPhyBS;



bool UmtsPhy::mod_initialized = false;
int UmtsPhy::modid = -1;

int UmtsPhy::code_id_counter = 0;



UmtsPhy::UmtsPhy()
  : dl_smask_(0), ul_smask_(0), pcTable_(NULL)
{
  bind("chip_rate_",&chip_rate_);
  bind("spreading_factor_",&spreading_factor_);
  bind("bits_per_symbol_",&bits_per_symbol_);
  bind("coding_rate_",&coding_rate_);
  bind("coding_type_",&coding_type_);
  bind("slot_duration_",&slot_duration_);
  bind("code_id_",&code_id_);
  bind("PERtarget_",&PERtarget_);
  bind("SIRtarget_",&SIRtarget_);
  bind("maxTxPower_",&maxTxPower_);
  bind("minTxPower_",&minTxPower_);
  bind("pcStep_",&pcStep_);
  bind("alpha_",&alpha_);
  bind("PowerCtrlUpdateCmdAtStartRx_",&PowerCtrlUpdateCmdAtStartRx_);
  bind("PowerCtrlReadCmdAtStartRx_",&PowerCtrlReadCmdAtStartRx_);
//   bind("",&);


  if (!mod_initialized) 
    {
      modid = MPhy::registerModulationType(UMTS_MODNAME);
      mod_initialized = true;
    }

  // Get a new code id for this BS/ME
  code_id_counter++;
  code_id_ = code_id_counter;  
}


UmtsPhyBS::UmtsPhyBS()
{
  bind("iuccorr_",&iuccorr_);  
}


int UmtsPhy::command(int argc, const char*const* argv)
{
  //  printf("UmtsPhy::command -- %s (%d)\n", argv[1], argc);
  Tcl& tcl = Tcl::instance();

  if(argc == 2)
    {  
      if(strcasecmp(argv[1], "getDataRate")==0)
	{
	  tcl.resultf("%f",getDataRate());
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "printPktHeaderInfo")==0)
	{
	  std::cout << "PacketHeader/MPhy "
		    << sizeof(hdr_MPhy) << " bytes at "
		    << hdr_MPhy::offset_ << std::endl;
	  std::cout << "PacketHeader/UMTSPHY "
		    << sizeof(hdr_umtsphy) << " bytes at "
		    << hdr_umtsphy::offset_ << std::endl;
	  std::cout << "PacketHeader/RLC "
		    << sizeof(hdr_rlc) << " bytes at "
		    << hdr_rlc::offset_ << std::endl;
	  std::cout << "PacketHeader/CMN "
		    << sizeof(hdr_cmn) << " bytes at "
		    << hdr_cmn::offset_ << std::endl;
	  std::cout << "PacketHeader/IP "
		    << sizeof(hdr_ip) << " bytes at "
		    << hdr_ip::offset_ << std::endl;
	  std::cout << "size of packet = "
		    << Packet::hdrlen_ << " bytes"
		    << std::endl;
	  return TCL_OK;
	}
    }

  if(argc == 3)
    {  
      if(strcasecmp(argv[1], "setDownlinkSpectralMask")==0)
	{
	  // dynamic cast returns NULL if object is not of the correct type
	  dl_smask_ = dynamic_cast<MSpectralMask*> (TclObject::lookup(argv[2]));
	  if(!dl_smask_)
	    return TCL_ERROR;
	  return TCL_OK;
	}

      if(strcasecmp(argv[1], "setUplinkSpectralMask")==0)
	{
	  // dynamic cast returns NULL if object is not of the correct type
	  ul_smask_ = dynamic_cast<MSpectralMask*> (TclObject::lookup(argv[2]));
	  if(!ul_smask_)
	    return TCL_ERROR;
	  return TCL_OK;
	}
      if(strcasecmp(argv[1], "computeSIRtarget")==0)
	{
		int payload = atoi(argv[2]);
		if (payload<=0)
		{
			printf("Error UmtsPhy::command(%s), payload must be greater than 0, not %d\n", argv[1], payload);
			return TCL_ERROR;
		}
		double sinr = 0;
		double ber = sinr2ber(sinr);
		double per = (1 - (pow(1-ber, payload * 8)));
		while(per>PERtarget_)
		{
			sinr = sinr + 0.1;
			ber = sinr2ber(sinr);
			per = (1 - (pow(1-ber, payload * 8)));
		}
		SIRtarget_ = sinr;
		if (debug_>10) 
			printf("UMTS Power Control Algorithm: PERtarget %f -> SIRtarget %f (payload %d)\n", PERtarget_, SIRtarget_, payload);
		return TCL_OK;
	}
      if(strcasecmp(argv[1], "monitor")==0)
	{
	  int rlcId = atoi(argv[2]);
	  if (rlcId<0)
	    {
	      fprintf(stderr,"UmtsPhy::command(%s), RLC identifier (%d) must be greater than zero\n", argv[1], rlcId);
	      return TCL_ERROR;
	    }
	  addPcEntry(rlcId);
	  return TCL_OK;
	}  
  
    }

  return MPhy::command(argc, argv);
}


void UmtsPhy::addPcEntry(int id)
{
  if (debug_>50)
    printf("UMTS-PHY: monitor id %d\n", id);

  pcEntry *p = new pcEntry;

  p->id = id;
  p->TxPower_ = maxTxPower_;
  p->powerUp_ = TRUE;

  p->next_ = NULL;
  if (pcTable_==NULL)
    {
      pcTable_ = p;
      return;
    }

  // insert in order
  pcEntry *q = pcTable_;
  pcEntry *last = pcTable_;
  while(q!=NULL)
    {
      if (q->id > id) 
	break;
      last = q;
      q = q->next_;
    }
  if ((last==q)&&(q==pcTable_))
    {
      // insert in head
      p->next_ = pcTable_;
      pcTable_ = p;
    }
  else
    {
      last->next_ = p;
      p->next_ = q;
    }
  q = pcTable_;
}


pcEntry* UmtsPhy::getPcEntry(int id)
{
  pcEntry *q = pcTable_;
  while(q!=NULL)
    {
      if (q->id == id)
	{
	  return q;
	}
      q = q->next_;
    }

  if (debug_)
    std::cerr << "ID " << id << " not found in PC table" << endl;
  return NULL;
}



void UmtsPhy::pcUpdatePi(Packet* p)
{
  hdr_rlc *rh = HDR_RLC(p);
  hdr_MPhy* ph = HDR_MPHY(p);

  pcEntry *q = getPcEntry(rh->src_rlc_id_);
  q->Pi = ph->Pi;
}


void UmtsPhy::pcUpdateCmd(Packet* p)
{
  hdr_rlc *rh = HDR_RLC(p);
  hdr_MPhy* ph = HDR_MPHY(p);

  pcEntry *q = getPcEntry(rh->src_rlc_id_);
  if (q==NULL)
    {
      if (debug_>2)
	fprintf(stderr,"UmtsPhy::pcUpdateCmd() ignoring NULL pcEntry, rlc_id=%d, this_code_id_=%d\n",
		rh->src_rlc_id_,
		code_id_);
      return;
    }


  double sinr;

  if (PowerCtrlUpdateCmdAtStartRx_)
    { /// we must use interference power measured for previous RX      
      /// since at this point it is not determined for current RX
      /// moreover, Pr does still not include the effect of
      /// de-spreading at this point 
      sinr =  getPrAfterDeSpreading(p) / (ph->Pn + (q->Pi));
    }
  else
    { /// we can use the interference power for current RX   
      /// the effect of de-spreading at this point has
      /// already been taken into account
      sinr =  ph->Pr / (ph->Pn + (ph->Pi));
    }

  if (sinr < SIRtarget_ * pcStep_)
    q->powerUp_ = true;
  else
    q->powerUp_ = false;

  if (debug_>20)
    {
      printf("UMTS-PHY connection %2d -> %2d has sinr %6.2f SIRtarget %6.2f (Ptx %5.2e) -> %s\n",
	     rh->src_rlc_id_, 
	     rh->dst_rlc_id_, 
	     sinr, 
	     SIRtarget_, 
	     ph->Pt,
	     q->powerUp_ ? "UP" : "DOWN");
    }

}


void UmtsPhy::pcReadCmd(Packet* p)
{
  hdr_rlc *rh = HDR_RLC(p);
  hdr_MPhy* ph = HDR_MPHY(p);
  hdr_umtsphy* uh = HDR_UMTSPHY(p);

  pcEntry *q = getPcEntry(rh->src_rlc_id_);
  if (q == NULL)
    return;  /// unknown terminal, ignoring

  double oldTxPower = q->TxPower_;

  // get power control command 
  // (before error check since we suppose control channel is more reliable than the data one)
  if ((uh->powerUp==TRUE)&&((q->TxPower_*pcStep_)<maxTxPower_))
    q->TxPower_ = q->TxPower_ * pcStep_;
  if ((uh->powerUp==FALSE)&&((q->TxPower_/pcStep_)>minTxPower_))
    q->TxPower_ = q->TxPower_ / pcStep_;


  if (debug_==-1)
    {
      printf("UMTS-MAC %d rx pkt at %.6f: PC command %s %s (%e->%e))", 
	     rh->src_rlc_id_, 
	     Scheduler::instance().clock(),
	     uh->data == true ? "DATA" : "CTRL" ,
	     uh->powerUp == true ? "UP" : "DOWN",
	     oldTxPower,
	     q->TxPower_);
    }

}


int UmtsPhyME::command(int argc, const char*const* argv)
{
  //  printf("UmtsPhy::command -- %s (%d)\n", argv[1], argc);
  Tcl& tcl = Tcl::instance();

  if(argc == 2)
    {  
    //   if(strcasecmp(argv[1], "getSNR")==0)
// 	{
// 	  tcl.resultf("%f",getSNR());
// 	  return TCL_OK;
// 	}
//       if(strcasecmp(argv[1], "getSINR")==0)
// 	{
// 	  tcl.resultf("%f",getSINR());
// 	  return TCL_OK;
// 	}
    }

  return UmtsPhy::command(argc, argv);
}



int UmtsPhy::getModulationType(Packet*)
{
  assert(mod_initialized);
  return modid;
}


double  UmtsPhy::getTxDuration(Packet* p)
{
  return (slot_duration_);
}




double UmtsPhy::getTxPower(Packet* p)
{
	// read the power level from the power control table
	hdr_rlc *rh = HDR_RLC(p);
	hdr_MPhy* ph = HDR_MPHY(p);

	pcEntry *q = getPcEntry(rh->dst_rlc_id_);
	assert(q);

	if (q == NULL)
	  {
	    printf("Error UmtsPhy::getTxPower does not find any entry for RLC id %d\n", rh->src_rlc_id_);
	    exit(1);
	  }

	return(q->TxPower_);
}



void UmtsPhyBS::startTx(Packet* p)
{
  hdr_umtsphy* uh = HDR_UMTSPHY(p);

  uh->bs_code_id  = code_id_;
  uh->direction = DIRECTION_DOWNLINK;

  UmtsPhy::startTx(p);
}


void UmtsPhyME::startTx(Packet* p)
{
  hdr_umtsphy* uh = HDR_UMTSPHY(p);

  uh->me_code_id  = code_id_;
  uh->bs_code_id  = bs_code_id_;
  uh->direction = DIRECTION_UPLINK;

  UmtsPhy::startTx(p);
}


void UmtsPhy::startTx(Packet* p)
{

  hdr_umtsphy* uh = HDR_UMTSPHY(p);
  uh->coding_type  = coding_type_;
  uh->coding_rate = coding_rate_;
  uh->spreading_factor  = spreading_factor_;
  uh->bits_per_symbol = bits_per_symbol_;
  // put the power control command
  hdr_rlc *rh = HDR_RLC(p);
  
  pcEntry *q = getPcEntry(rh->dst_rlc_id_);
  assert(q);
  uh->powerUp = q->powerUp_;

  sendDown(p);
}

void UmtsPhy::endTx(Packet* p)
{

}


// void UmtsPhyBS::startRx(Packet* p)
// {  
//   /// BS tries to receive packet from all users

//   hdr_MPhy* ph = HDR_MPHY(p);
//   hdr_umtsphy* uh = HDR_UMTSPHY(p);

//   assert(uh->direction == DIRECTION_UPLINK);

//   // need to account for inter-user spreading code correlation
//   ph->Pr = uh->spreading_factor * ph->Pr ;
 
//   UmtsPhy::startRx(p);
// }






void UmtsPhy::startRx(Packet* p)
{

  if (PowerCtrlUpdateCmdAtStartRx_)
    pcUpdateCmd(p);

  
  if (PowerCtrlReadCmdAtStartRx_)
    pcReadCmd(p);
    
}



void UmtsPhyBS::endRx(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);
  hdr_umtsphy* uh = HDR_UMTSPHY(p);

  assert(uh->direction == DIRECTION_UPLINK);

  // need to account for inter-user spreading code correlation
  ph->Pr = getPrAfterDeSpreading(p);
  ph->Pi = ph->Pi * iuccorr_;
  
  UmtsPhy::endRx(p);
}



void UmtsPhyME::endRx(Packet* p)
{ 
  hdr_umtsphy* uh = HDR_UMTSPHY(p); 
  hdr_MPhy* ph = HDR_MPHY(p);
 
  assert(uh->direction == DIRECTION_DOWNLINK);

  if (uh->me_code_id != code_id_)
    {
      // This packet is not for me
        Packet::free(p);
	return;
    }
  else
    {
      // Packet is for me
      // Non-orthogonality of intra-cell interferers and correlation
      // of inter-cell codes for Downlink Transmissions should have
      // been accounted for by the UmtsCorrelation module      
      ph->Pr = getPrAfterDeSpreading(p);

      UmtsPhy::endRx(p);
    }
}



void UmtsPhy::endRx(Packet* p)
{
  // Note: this method is to be called AT THE END of the endRx() method of each
  // derived classes (UmtsPhyBS, UmtsPhyME) 
  
  if (! PowerCtrlReadCmdAtStartRx_)
    pcReadCmd(p);
  
  if (! PowerCtrlUpdateCmdAtStartRx_)
    pcUpdateCmd(p);
  
  pcUpdatePi(p);
  
  calculateErrors(p);
  
  sendUp(p);
  
}


double UmtsPhyME::getPrAfterDeSpreading(Packet* p)
{
  hdr_umtsphy* uh = HDR_UMTSPHY(p); 
  hdr_MPhy* ph = HDR_MPHY(p);

  return (uh->spreading_factor * ph->Pr * (1.0/(1.0+alpha_)));
}



double UmtsPhyBS::getPrAfterDeSpreading(Packet* p)
{
  hdr_umtsphy* uh = HDR_UMTSPHY(p); 
  hdr_MPhy* ph = HDR_MPHY(p);

  return (uh->spreading_factor * ph->Pr);
}




void UmtsPhy::calculateErrors(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);
  hdr_cmn* ch = HDR_CMN(p);

  double sinr = ph->Pr / (ph->Pn + (ph->Pi));
  double per = getPacketErrorRate(p, sinr);
  double rnd =  RNG::defaultrng()->uniform_double();
  ch->error() = rnd <= per;
  
  if (debug_)     std::cerr << NOW 
			    << " PER=" << per 
			    << " RND=" << rnd 
			    << " ERR=" << ch->error()
			    << std::endl;
  
  
}



double UmtsPhy::getPacketErrorRate(Packet* p, double sinr)
{
  hdr_umtsphy* uh = HDR_UMTSPHY(p);
  hdr_MPhy* ph = HDR_MPHY(p);

  /* This is the only coding currently supported */
  assert(uh->coding_type = CODE_CONVOLUTIONAL);
  assert(fabs(uh->coding_rate - 0.5) < 1e-5);

  double ber = sinr2ber(sinr);
  int nbits =  getNumBits(p);
  double per = (1 - (pow(1-ber, nbits)));

  if (debug_)
    std::cerr << NOW
	      << " SINR=" << sinr 
	      << " NBITS=" << nbits 
	      << " BER=" << ber 
	      << " PER=" << per 
	      << std::endl;
  
  return per;

}


double UmtsPhy::sinr2ber(double sinr)
{
	return (0.5*erfc(sqrt(pow(sinr, ZETA_CONVOLUTIONAL_ONEHALF))));
}


int UmtsPhy::getNumBits(Packet* p)
{
  hdr_umtsphy* uh = HDR_UMTSPHY(p);
  hdr_MPhy* ph = HDR_MPHY(p);
  hdr_cmn* ch = HDR_CMN(p);
  return (int)(ch->size()*8);
//   return (int)((chip_rate_  / 
// 		(uh->bits_per_symbol * uh->spreading_factor * ph->duration))
// 	       * uh->coding_rate);
}



double UmtsPhyME::getDataRate()
{
  // 
  // Result agrees with bit rates for uplink DPDCH 
  // as reported in Holma and Toskala, "WCDMA for UMTS", p. 90
  assert(spreading_factor_ > 0);
  return (chip_rate_ * bits_per_symbol_ * coding_rate_/ spreading_factor_ );
}


double UmtsPhyBS::getDataRate()
{
  // Maximum bit rate for downlink DPDCH 
  // Source: Holma and Toskala, "WCDMA for UMTS", p. 96

  assert(chip_rate_ == 3840000);
  assert(bits_per_symbol_ == 2);

  switch (spreading_factor_)
    {
    case 4: 	return (1872000 * coding_rate_);
    case 8:	return (912000 * coding_rate_);
    case 16: 	return (432000 * coding_rate_);	
    case 32:	return (210000 * coding_rate_);	
    case 64:	return (90000 * coding_rate_);	
    case 128:	return (51000 * coding_rate_);	
    case 256:	return (24000 * coding_rate_);
    case 512:	return (6000 * coding_rate_);
    default:    assert(0);
    }
}

// double UmtsPhyME::getSNR()
// {
//   return SNR;
// }


// double UmtsPhyME::getSINR()
// {
//   return SINR;
// }
