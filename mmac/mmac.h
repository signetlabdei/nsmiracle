/* -*-	Mode:C++ -*- */
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
 * @file   mmac.h
 * @author Baldo Nicola, Federico Guerra
 * @date   Mon Nov 26 15:02:18 2007
 * 
 * @brief  
 * 
 * 
 */


#ifndef MMAC_H 
#define MMAC_H

#include<module.h>
#include<queue>


class MMac : public Module
{

public:

  MMac();
  virtual ~MMac();
 
  void recv(Packet* p);
  virtual int recvSyncClMsg(ClMessage* m);

  /** 
   * TCL command parser
   * 
   * @param argc 
   * @param argv 
   * 
   * @return TCL_OK or TCL_ERROR
   */
  virtual int command(int argc, const char*const* argv);

 /** 
   * Resets all stats
   */
  virtual void resetStats() { data_pkts_tx = 0; data_pkts_rx = 0; xdata_pkts_rx = 0; xack_pkts_rx = 0;
                              up_data_pkts_rx=0; sum_wait_time = 0; backoff_times_no = 0; ack_pkts_tx = 0;
                              ack_pkts_rx = 0; dropped_pkts_tx = 0; ctrl_pkts_tx = 0; ctrl_pkts_rx = 0; 
                              xctrl_pkts_rx = 0; error_pkts_rx = 0; }

 /** 
   * Returns total data pkts transmitted
   */
  virtual int getErrorPktsRx() { return error_pkts_rx; }

 /** 
   * Returns total data pkts transmitted
   */
  virtual int getDataPktsTx() { return data_pkts_tx; }

 /** 
   * Returns total data pkts received from other nodes and intended for this mac address
   */
  virtual int getDataPktsRx() { return data_pkts_rx; }

 /** 
   * Returns total data pkts received from other nodes and intended for other mac address
   */
  virtual int getXDataPktsRx() { return xdata_pkts_rx; }

 /** 
   * Returns total ack pkts transmitted (if used)
   */
  virtual int getAckPktsTx() { return ack_pkts_tx; }

 /** 
   * Returns total ack pkts transmitted from other nodes and intended for this mac address (if used)
   */
  virtual int getAckPktsRx() { return ack_pkts_rx; }

 /** 
   * Returns total control pkts transmitted 
   */
  virtual int getCtrlPktsTx() { return ctrl_pkts_tx; }

 /** 
   * Returns total ack pkts transmitted from other nodes and intended for this mac address (if used)
   */
  virtual int getCtrlPktsRx() { return ctrl_pkts_rx; }

 /** 
   * Returns total ack pkts transmitted from other nodes and intended for other mac address (if used)
   */
  virtual int getXCtrlPktsRx() { return xctrl_pkts_rx; }

 /** 
   * Returns total backoff times (if used)
   */
  virtual int getTotalBackoffTimes() { return backoff_times_no; }

 /** 
   * Returns total dropped pkts due to max retx limit (if used)
   */
  virtual int getDroppedPktsTx() { return dropped_pkts_tx; }

 /** 
   * Returns total discarded pkts due to queue buffer overflow (if used)
   */
  virtual int getDiscardedPktsTx() { return discarded_data_pkts; }

 /** 
   * Returns MAC efficiency 
   */
  virtual double getEfficiency() { if (data_pkts_tx > 0) 
                                         return( (double) getRemainingPkts() / (double) data_pkts_tx );
                                         else return (0.0); }

/** 
   * This method returns the mean wait time 
   */
  virtual double getMeanWaitTime() { if ( getRemainingPkts() - dropped_pkts_tx > 0) 
                                  return( sum_wait_time / (double) ( getRemainingPkts() - dropped_pkts_tx ) );
                                  else return (0.0); }

 /** 
   * This method returns the mean backoff time (if used)
   */
  virtual double getMeanBackoffTime() { if (getTotalBackoffTimes() > 0) 
                                        return( sum_backoff_time / (double) getTotalBackoffTimes()); 
                                        else return (0.0); }
protected:

  /**
   * Handle the end-of-PHY-transmission event
   *  
   * This method is expected to be re- implemented by a class
   * inheriting from MMac in order to perform the necessary MAC
   * operations.
   * 
   * @param p pointer to the packet whose transmission has ended. Note
   * that the Packet is not any more under control of the MAC at the
   * time this event occurs, hence the 'const' declaration.
   */
  virtual void Phy2MacEndTx(const Packet* p);

  /** 
   * Handle the detected-start-of-PHY-reception event (e.g., PHY
   * preamble successfully detected, so we expect to receive a new
   * packet, possibly with errors)
   *  
   * This method is expected to be re- implemented by a class
   * inheriting from MMac in order to perform the necessary MAC
   * operations.
   * 
   * @param p pointer to the packet whose reception has begun. Note
   * that the Packet is not yet under control of the MAC at the
   * time this event occurs, hence the 'const' declaration.
   */
  virtual void Phy2MacStartRx(const Packet* p);

  /** 
   * Handle the end-of-PHY-reception event
   * 
   *  
   * This method is expected to be re- implemented by a class
   * inheriting from MMac in order to perform the necessary MAC
   * operations.
   * 
   * @param p pointer to the packet whose reception has ended. The
   * receiving MAC can modify it at will (changing header fields,
   * deleting it...)
   */
  virtual void Phy2MacEndRx(Packet* p);


  /** 
   * Handle a packet coming from upper layers
   * 
   * @param p pointer to the packet
   */
  virtual void recvFromUpperLayers(Packet* p);

  virtual void Phy2MacCCA(bool cca);  


  /** 
   * This method must be called by the MAC to instruct the PHY to
   * start the transmission of a packet
   * 
   * @param p the packet to be transmitted
   */
  void Mac2PhyStartTx(Packet* p);


  /** 
   * This method must be called by the MAC to instruct the id-specific PHY to
   * start the transmission of a packet
   * 
   * @param p the packet to be transmitted
   * @param moduleId module's unique id number
   */
  void Mac2PhyStartTx(int moduleId, Packet* p);

 /** 
   * This method sends a synchr. clmessage to PHY asking for the tx duration of a packet
   * 
   * @param p the packet to be transmitted
   */
  double Mac2PhyTxDuration(Packet* p);

 /** 
   * This method sends a synchr. clmessage to PHY asking for the tx duration of a packet
   * 
   * @param p the packet to be transmitted
   * @param moduleId module's unique id number
   */
  double Mac2PhyTxDuration(int moduleId, Packet* p);

 /** 
   * This method sends a synchr. clmessage to turn the PHY on
   */
  void Mac2PhyTurnOn();

 /** 
   * This method sends a synchr. clmessage to turn on the id-specific PHY 
   *
   * @param moduleId module's unique id number
   */
  void Mac2PhyTurnOn(int moduleId);

 /** 
   * This method sends a synchr. clmessage to turn the PHY off
   */
  void Mac2PhyTurnOff();
  
 /** 
   * This method sends a synchr. clmessage to turn off the id-specific PHY 
   *
   * @param moduleId module's unique id number
   */
  void Mac2PhyTurnOff(int moduleId);

 /** 
   * This method sends a synchr. clmessage to get PHY interrupt status
   * returns a bool == true if PHY is on, false otherwise.
   */
  bool Mac2PhyOnOffSwitchStatus();
 
 /** 
   * This method sends a synchr. clmessage to get the id-specific PHY interrupt status
   * returns a bool == true if PHY is on, false otherwise.
   *
   * @param moduleId module's unique id number
   */
  bool Mac2PhyOnOffSwitchStatus(int moduleId);


 /** 
   * This method returns total pkts waiting for transmission
   * THIS METHOD MUST BE REIMPLEMENTED BY CHILD CLASSES
   */
  virtual int getRemainingPkts();


 /** 
   * Incr data pkts transmitted
   */
  virtual void incrErrorPktsRx() { error_pkts_rx++; }

 /** 
   * Incr data pkts transmitted
   */
  virtual void incrDataPktsTx() { data_pkts_tx++; }

 /** 
   * Incr data pkts received from other nodes and intended for this mac address
   */
  virtual void incrDataPktsRx() { data_pkts_rx++; }

 /** 
   * Incr data pkts received from upper layers
   */
  virtual void incrUpperDataRx() {up_data_pkts_rx++;}

 /** 
   * Incr ack pkts transmitted (if used)
   */
  virtual void incrAckPktsTx() { ack_pkts_tx++; }

 /** 
   * Incr ack pkts received (if used)
   */
  virtual void incrAckPktsRx() { ack_pkts_rx++; }

 /** 
   * Incr data pkts received from other nodes and intended for other mac address
   */
  virtual void incrXDataPktsRx() { xdata_pkts_rx++; }

 /** 
   * Incr ack pkts received from other nodes and intended for other mac address (if used)
   */
  virtual void incrXAckPktsRx() { xack_pkts_rx++; }

 /** 
   * Incr control pkts transmitted (if used)
   */
  virtual void incrCtrlPktsTx() { ctrl_pkts_tx++; }

 /** 
   * Incr control pkts received from other nodes and intended for this mac address (if used)
   */
  virtual void incrCtrlPktsRx() { ctrl_pkts_rx++; }

 /** 
   * Incr control pkts received from other nodes and intended for other mac address (if used)
   */
  virtual void incrXCtrlPktsRx() { xctrl_pkts_rx++; }

 /** 
   * Incr dropped data pkts due to max retx limit (if used)
   */
  virtual void incrDroppedPktsTx() { dropped_pkts_tx++; }

 /** 
   * Incr discarded data pkts due to queue buffer overflow (if used)
   */
  virtual void incrDiscardedPktsTx() { discarded_data_pkts++; }

 /** 
   * Incr total backoff times (if used)
   */
  virtual void incrTotalBackoffTimes() { backoff_times_no++; }

 /** 
   * This method traces the queue wait start time of a pkt received from upper layers
   */
  virtual void waitStartTime() { queue_wait_time.push(NOW); }

 /** 
   * This method traces the exit from the queue of a pkts
   * @param flag 
     if flag is true the wait time is counted for the mean wait time purposes, else it should simply 
   * discard current wait time
   */
  virtual void waitEndTime(bool flag, Packet* p = NULL );

 /** 
   * This method actually computes the exit from the queue of a pkts
   */
//   virtual void computeQueueWaitTime();

 /** 
   * This method calculates the sum of all backoff times (if used)
   */
  virtual void backoffSumDuration(double duration) { sum_backoff_time += duration; }

   /**
   * Delay when sending packets from MAC to PHY. 
   * Remember that setting this to zero might give strange results due
   * to the possible generation of simultaneous events and how these
   * might be scheduled by the simulator. If you want to
   * avoid this effect, set the value to a very small value if you
   * want to simulate 'zero' delay.
   */
  double mac2phy_delay_;

  /**
   * MAC address of this MAC instance. Automatically generated.
   * 
   */
  int addr;

  // STATS //

  /**
   * Data pkts received from upper layers
   * 
   */
  int up_data_pkts_rx;

  /**
   * Data pkts transmitted
   * 
   */
  int data_pkts_tx;

  /**
   * Data pkts received sent from other nodes to this MAC address
   * 
   */
  int data_pkts_rx;

  /**
   * ACK pkts transmitted (if used)
   * 
   */
  int ack_pkts_tx;

  /**
   * ACK pkts received (if used) sent from other nodes to this MAC address
   * 
   */
  int ack_pkts_rx;

  /**
   * Data pkts received sent from other nodes to another MAC address
   * 
   */
  int xdata_pkts_rx;

  /**
   * ACK pkts received (if used) sent from other nodes to this MAC address
   * 
   */
  int xack_pkts_rx;

  /**
   * Other control pkts transmitted (if used)
   * 
   */
  int ctrl_pkts_tx;

   /**
   * Other control pkts received (if used) sent from other nodes to this MAC address
   * 
   */
  int ctrl_pkts_rx;

   /**
   * Other control pkts received (if used) sent from other nodes to other MAC address
   * 
   */
  int xctrl_pkts_rx;

  /**
   * Total backoff times (if used)
   * 
   */
  int backoff_times_no;

  /**
   * Total number of dropped data pkts due to buffer overflow (if used)
   * 
   */
  int dropped_pkts_tx;

  /**
   * Total number of dropped data pkts due to ch->error()
   * 
   */
  int error_pkts_rx;

  /**
   * Total number of discarded data pkts due to max tx tries limit (if used)
   * 
   */
  int discarded_data_pkts;


  /**
   * Total queue wait time 
   * 
   */
  double sum_wait_time;

  /**
   * Total backoff time
   * 
   */
  double sum_backoff_time;

  /**
   * Data pkts wait time queue
   * 
   */
  queue<double> queue_wait_time; 
  /////

};






#endif /* MMAC_H */
