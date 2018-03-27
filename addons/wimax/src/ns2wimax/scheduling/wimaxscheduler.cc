/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This software is based on the NIST NS2 WiMAX module, which can be found at
 * http://www.antd.nist.gov/seamlessandsecure/doc.html
 *
 * $Id: wimaxscheduler.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "wimaxscheduler.h"

/*
 * Create a scheduler
 * @param mac The Mac where it is located
 */
WimaxScheduler::WimaxScheduler ()
{
}

/*
 * Set the mac
 * @param mac The Mac where it is located
 */
void WimaxScheduler::setMac (Mac802_16 *mac)
{
  assert (mac!=NULL);

  mac_ = mac;
}

/**
 * Initialize the scheduler.
 */
void WimaxScheduler::init()
{

}

/**
 * This function is used to schedule bursts/packets
 */
void WimaxScheduler::schedule ()
{
  //defined by subclasses
}


/**
 * Transfer the packets from the given connection to the given burst
 * @param con The connection
 * @param b The burst
 * @param b_data Amount of data in the burst
 * @return the new burst occupation
 */
int WimaxScheduler::transfer_packets (Connection *c, Burst *b, int b_data)
{
  Packet *p;
  hdr_cmn* ch;
  hdr_mac802_16 *wimaxHdr;
  double txtime, txtime2;
  int txtime_s;
  bool pkt_transfered = false;
  Mac802_16 *phy = mac_->getPhy();

  p = c->get_queue()->head();

  int max_data;
  if (mac_->getNodeType()==STA_BS)
    max_data = phy->getMaxPktSize (b->getDuration(), mac_->getMap()->getDlSubframe()->getProfile (b->getIUC())->getEncoding())-b_data;
  else
    max_data = phy->getMaxPktSize (b->getDuration(), mac_->getMap()->getUlSubframe()->getProfile (b->getIUC())->getEncoding())-b_data;

  debug2 ("In Mac %d max data=%d (burst duration=%d, b_data=%d)\n", mac_->addr(), max_data, b->getDuration(), b_data);
  if (max_data < HDR_MAC802_16_SIZE ||
      (c->getFragmentationStatus()!=FRAG_NOFRAG && max_data < HDR_MAC802_16_SIZE+HDR_MAC802_16_FRAGSUB_SIZE))
    return b_data; //not even space for header

  while (p) {
    ch = HDR_CMN(p);
    wimaxHdr = HDR_MAC802_16(p);

    if (mac_->getNodeType()==STA_BS)
      max_data = phy->getMaxPktSize (b->getDuration(), mac_->getMap()->getDlSubframe()->getProfile (b->getIUC())->getEncoding())-b_data;
    else
      max_data = phy->getMaxPktSize (b->getDuration(), mac_->getMap()->getUlSubframe()->getProfile (b->getIUC())->getEncoding())-b_data;

    debug2 ("\tIn Mac %d max data=%d (burst duration=%d, b_data=%d)\n", mac_->addr(), max_data, b->getDuration(), b_data);
    if (max_data < HDR_MAC802_16_SIZE ||
        (c->getFragmentationStatus()!=FRAG_NOFRAG && max_data < HDR_MAC802_16_SIZE+HDR_MAC802_16_FRAGSUB_SIZE))
      return b_data; //not even space for header

    if (c->getFragmentationStatus()!=FRAG_NOFRAG) {
      if (max_data >= ch->size()-c->getFragmentBytes()+HDR_MAC802_16_FRAGSUB_SIZE) {
        //add fragmentation header
        wimaxHdr->header.type_frag = true;
        //no need to fragment again
        wimaxHdr->frag_subheader.fc = FRAG_LAST;
        wimaxHdr->frag_subheader.fsn = c->getFragmentNumber ();
        //remove packet from queue
        c->dequeue();
	ch->size() = ch->size()-c->getFragmentBytes()+HDR_MAC802_16_FRAGSUB_SIZE; //new packet size
        //update fragmentation
        debug2 ("End of fragmentation %d (max_data=%d, bytes to send=%d\n", wimaxHdr->frag_subheader.fsn, max_data, ch->size());
        c->updateFragmentation (FRAG_NOFRAG, 0, 0);
      } else {
	//need to fragment the packet again
        p = p->copy(); //copy packet to send
        ch = HDR_CMN(p);
        wimaxHdr = HDR_MAC802_16(p);
        //add fragmentation header
        wimaxHdr->header.type_frag = true;
        wimaxHdr->frag_subheader.fc = FRAG_CONT;
        wimaxHdr->frag_subheader.fsn = c->getFragmentNumber ();
        ch->size() = max_data; //new packet size
        //update fragmentation
        c->updateFragmentation (FRAG_CONT, (c->getFragmentNumber ()+1)%8, c->getFragmentBytes()+max_data-(HDR_MAC802_16_SIZE+HDR_MAC802_16_FRAGSUB_SIZE));
        debug2 ("Continue fragmentation %d\n", wimaxHdr->frag_subheader.fsn);
      }
    } else {
      if (max_data < ch->size() && c->isFragEnable()) {
        //need to fragment the packet for the first time
        p = p->copy(); //copy packet to send
        ch = HDR_CMN(p);
        wimaxHdr = HDR_MAC802_16(p);
        //add fragmentation header
        wimaxHdr->header.type_frag = true;
        wimaxHdr->frag_subheader.fc = FRAG_FIRST;
        wimaxHdr->frag_subheader.fsn = c->getFragmentNumber ();
        ch->size() = max_data; //new packet size
        //update fragmentation
        c->updateFragmentation (FRAG_FIRST, 1, c->getFragmentBytes()+max_data-(HDR_MAC802_16_SIZE+HDR_MAC802_16_FRAGSUB_SIZE));
        debug2 ("First fragmentation\n");
      } else if (max_data < ch->size() && !c->isFragEnable()) {
        //the connection does not support fragmentation
        //can't move packets anymore
        return b_data;
      } else {
        //no fragmentation necessary
        c->dequeue();
      }
    }

    if (mac_->getNodeType()==STA_BS) {
      txtime = phy->getTrxTime (ch->size(), mac_->getMap()->getDlSubframe()->getProfile (b->getIUC())->getEncoding());
      txtime2 = phy->getTrxSymbolTime (b_data+ch->size(), mac_->getMap()->getDlSubframe()->getProfile (b->getIUC())->getEncoding());
    }
    else {
      txtime = phy->getTrxTime (ch->size(), mac_->getMap()->getUlSubframe()->getProfile (b->getIUC())->getEncoding());
      txtime2 = phy->getTrxSymbolTime (b_data+ch->size(), mac_->getMap()->getUlSubframe()->getProfile (b->getIUC())->getEncoding());
    }
    ch->txtime() = txtime;
    txtime_s = (int)round (txtime2/phy->getSymbolTime ()); //in units of symbol
    //printf ("symbtime=%f\n", phy->getSymbolTime ());
    //printf ("Check packet to send: size=%d txtime=%f(%d) duration=%d(%f)\n", ch->size(),txtime, txtime_s, b->getDuration(), b->getDuration()*phy->getSymbolTime ());
    assert ( txtime2 <= b->getDuration()*phy->getSymbolTime () );
    //printf ("transfert to burst (txtime=%f, txtime2=%f, bduration=%f)\n", txtime,txtime2,b->getDuration()*phy->getSymbolTime ());
    //p = c->dequeue();   //dequeue connection queue
    b->enqueue(p);      //enqueue into burst
    b_data += ch->size(); //increment amount of data enqueued
    if (!pkt_transfered && mac_->getNodeType()!=STA_BS){ //if we transfert at least one packet, remove bw request
      pkt_transfered = true;
      mac_->getMap()->getUlSubframe()->getBw_req()->removeRequest (c->get_cid());
    }
    p = c->get_queue()->head(); //get new head
  }
  return b_data;
}


