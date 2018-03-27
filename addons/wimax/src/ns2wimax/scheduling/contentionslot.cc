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

#include "contentionslot.h"
#include <random.h>
#include <math.h>

/*
 * Creates a contention slot for the given frame
 * @param frame The frame map 
 */
ContentionSlot::ContentionSlot (FrameMap *map) 
{
  assert (map);
  map_ = map;
}

/**
 * Destructor
 */
ContentionSlot::~ContentionSlot() {}

/*
 * Set the initial contention slot window size
 * @param backoff_start the initial contention slot window size
 */
void ContentionSlot::setBackoff_start( int backoff_start ) 
{ 
  backoff_start_ = backoff_start;
}

/*
 * Set the final contention slot window size
 * @param backoff_stop the final contention slot window size 
 */
void ContentionSlot::setBackoff_stop( int backoff_stop ) 
{ 
  backoff_stop_ = backoff_stop;
}

/**
 * Resume the timers for the requests
 */
void ContentionSlot::resumeTimers () {}

/**
 * Pause the timers for the requests
 */
void ContentionSlot::pauseTimers () {}

/**** Methods for Ranging Contention slot ****/

/*
 * Creates a contention slot for the given frame
 * @param frame The frame map 
 */
RngContentionSlot::RngContentionSlot (FrameMap *map) : ContentionSlot (map)
{
  request_ = NULL;
}

/**
 * Destructor
 */
RngContentionSlot::~RngContentionSlot() 
{
  if (request_)
    delete request_;
}


/*
 * Add a ranging request
 * @param p The packet to be sent during the ranging opportunity
 */
void RngContentionSlot::addRequest (Packet *p)
{
  assert (request_ == NULL);
  request_ = new RangingRequest (this, p);
}

/*
 * Remove the pending request
 */
void RngContentionSlot::removeRequest ()
{
  //assert (request_);
  if (request_) {
    delete request_;
    request_ = NULL;
  }
}

/**
 * Resume the timers for the requests
 */
void RngContentionSlot::resumeTimers ()
{
  if (request_)
    request_->resume();
}

/**
 * Pause the timers for the requests
 */
void RngContentionSlot::pauseTimers ()
{
  if (request_)
    request_->pause();
}

/**** Methods for Bandwidth Contention slot ****/

/*
 * Creates a contention slot for the given frame
 * @param frame The frame map 
 */
BwContentionSlot::BwContentionSlot (FrameMap *map) : ContentionSlot (map)
{
  LIST_INIT (&request_list_);
}

/**
 * Destructor
 */
BwContentionSlot::~BwContentionSlot() {}

/*
 * Add a bandwidth request
 * @param p The packet to be sent during the ranging opportunity
 * @param cid The CID of the bandwidth request
 * @param len The size in bytes of the bandwidth request
 */
void BwContentionSlot::addRequest (Packet *p, int cid, int len)
{
  assert (getRequest (cid)==NULL);
  BwRequest *b = new BwRequest (this, p, cid, len);
  b->insert_entry_head (&request_list_);
}

/*
 * Remove the pending request
 */
void BwContentionSlot::removeRequest (int cid)
{
  BwRequest *b = getRequest (cid);
  if (b!=NULL) {
    b->remove_entry ();
    delete b;
  }
}

/*
 * Remove all pending reuquest
 */
void BwContentionSlot::removeRequests ()
{
  for (BwRequest *c = (BwRequest *)request_list_.lh_first; c ; c=(BwRequest *)request_list_.lh_first) {
    c->remove_entry();
    delete c;
  }
}


/*
 * Get the request for the given CID
 * @param cid The CID for the request
 */
BwRequest* BwContentionSlot::getRequest (int cid)
{
  for (BwRequest *c = (BwRequest *)request_list_.lh_first; c ; c=(BwRequest *)(c->next_entry())) {
    if (c->getCID()==cid)
      return c;
  }
  return NULL;
}

/**
 * Resume the timers for the requests
 */
void BwContentionSlot::resumeTimers ()
{
  for (BwRequest *c = (BwRequest *)request_list_.lh_first; c ; c=(BwRequest *)(c->next_entry())) {
      c->resume();
  }
}

/**
 * Pause the timers for the requests
 */
void BwContentionSlot::pauseTimers ()
{
  for (BwRequest *c = (BwRequest *)request_list_.lh_first; c ; c=(BwRequest *)(c->next_entry())) {
      c->pause();
  }
}
