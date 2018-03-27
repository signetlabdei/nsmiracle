/* This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * <BR>
 * We would appreciate acknowledgement if the software is used.
 * <BR>
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * </PRE></P>
 * @author  rouil
 */

#include "burst.h"


/**
 * Creates a burst
 * @param phypdu The PhyPdu where it is located
 */
Burst::Burst (PhyPdu *phypdu) : cid_(-1), duration_(0), 
				starttime_(0),iuc_(-1)
{
  assert (phypdu);
  phypdu_ = phypdu;
  queue_ = NULL;
}

/*
 * Delete the object
 */
Burst::~Burst () 
{
  //delete packets in queue
  if (queue_!=NULL) {
    for (Packet *p = dequeue(); p ; p=dequeue()) {
      Packet::free (p);
    }
    delete (queue_);
  }
} 

/**
 * Set burst CID
 * @param cid The burst CID
 */
void Burst::setCid( int cid )
{
  cid_ = cid;
}

/**
 * Return the CID for this burst
 * @return the CID for this burst
 */
int Burst::getCid( )
{
  return cid_;
}

/**
 * Return the burst duration in units of OFDM symbols
 * @return the burst duration 
 */
int Burst::getDuration( )
{
  return duration_;
}

/**
 * Set the duration of the burst in units of OFDM symbols
 * @param duration The burst duration
 */
void Burst::setDuration (int duration)
{
  duration_=duration;
}

/**
 * Set burst IUC
 * @param iuc The burst IUC
 */
void Burst::setIUC( int iuc )
{
  iuc_ = iuc;
}

/**
 * Return the Interval Usage Code
 * @return the burst start time
 */
int Burst::getIUC( )
{
  return iuc_;
}

/**
 * Set burst start time in units of symbol duration
 * @param starttime the burst start time
 */
void Burst::setStarttime( int starttime )
{
  assert (starttime >= 0);
  starttime_ = starttime;
}

/**
 * Return the burst start time in units of symbol duration
 * @return the burst start time
 */
int Burst::getStarttime( )
{
  return starttime_;
}

/**
 * Enqueue the given packet
 * @param p The packet to enqueue
 */
void Burst::enqueue (Packet * p) 
{
  if (queue_ == NULL) {
    //this is the first packet we enqueue, create queue
    queue_ = new PacketQueue();
  }
  queue_->enque (p);
}

/**
 * Dequeue a packet from the queue
 * @param p The packet to enqueue
 */
Packet * Burst::dequeue () 
{
  if (queue_==NULL) //in case there was never an enqueue
    return NULL;
  return queue_->deque ();
}

/**
 * Trigger the timer to send packets for this burst
 * @param time The time the trigger expires
 */
/*
void Burst::trigger_timer (double time)
{
  //assert (NOW < time);
  timer_.resched (time);
}
*/
