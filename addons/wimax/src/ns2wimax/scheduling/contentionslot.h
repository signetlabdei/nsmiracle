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

#ifndef CONTENTIONSLOT_H
#define CONTENTIONSLOT_H

#include "packet.h"
#include "queue.h"
#include "contentiontimer.h"
#include "contentionrequest.h"

class FrameMap;

/**
 * This class contains information about a contention slot
 */
class ContentionSlot
{
  friend class ContentionRequest;

 public:
  /*
   * Creates a contention slot for the given frame
   * @param frame The frame map 
   */
  ContentionSlot (FrameMap *map);
  
  /**
   * Destructor
   */
  virtual ~ContentionSlot();

  /*
   * Return the initial contention slot window size
   * @return the initial contention slot window size
   */
  inline int getBackoff_start( ) { return backoff_start_; }

  /*
   * Return the final contention slot window size
   * @return the final contention slot window size 
   */
  inline int getBackoff_stop( ) { return backoff_stop_; }

  /*
   * Return the opportunity size
   * @return the opportunity size
   */
  inline int getSize( ) { return size_; }

  /*
   * Set the initial contention slot window size
   * @param backoff_start the initial contention slot window size
   */
  void setBackoff_start( int backoff_start );

  /*
   * Set the final contention slot window size
   * @param backoff_stop the final contention slot window size 
   */
  void setBackoff_stop( int backoff_stop );

  /*
   * Set the opportunity size
   * @param size The opportunity size
   */
  inline void setSize( int size ) { size_ = size; }
  
  /**
   * Resume the timers for the requests
   */
  virtual void resumeTimers ();
  
  /**
   * Pause the timers for the requests
   */
  virtual void pauseTimers ();

 protected:
    /**
   * The frame map where this contention slot is located
   */
  FrameMap *map_;

  /**
   * Initial backoff window size. Must be power of 2
   */
  int backoff_start_;
  
  /**
   * Final backoff window size
   */
  int backoff_stop_;
  
  /**
   * The duration in PS of the contention slot
   */
  int size_;
 private:
  
};

/**
 * Subclass used for ranging contention slot
 */
class RngContentionSlot: public ContentionSlot {
  friend class RngContentionTimer;
 public:

  /*
   * Creates a contention slot for the given frame
   * @param frame The frame map 
   */
  RngContentionSlot (FrameMap *map);

  /**
   * Destructor
   */
  virtual ~RngContentionSlot();

  /*
   * Add a ranging request
   * @param p The packet to be sent during the ranging opportunity
   */
  void addRequest (Packet *p);

  /*
   * Remove the pending request
   */
  void removeRequest ();
  
  /**
   * Resume the timers for the requests
   */
  void resumeTimers ();

  /**
   * Pause the timers for the requests
   */
  void pauseTimers ();

 private:
  RangingRequest *request_;
};

/**
 * Subclass used for ranging contention slot
 */
class BwContentionSlot: public ContentionSlot {
 public:
  /*
   * Creates a contention slot for the given frame
   * @param frame The frame map 
   */
  BwContentionSlot (FrameMap *map);

  /**
   * Destructor
   */
  virtual ~BwContentionSlot();

  /*
   * Add a bandwidth request
   * @param p The packet to be sent during the ranging opportunity
   * @param cid The CID of the bandwidth request
   * @param len The size in bytes of the bandwidth request
   */
  void addRequest (Packet *p, int cid, int len);

  /*
   * Remove the pending request
   */
  void removeRequest (int cid);  

  /*
   * Remove all pending reuquest
   */
  void removeRequests ();

  /*
   * Get the request for the given CID
   * @param cid The CID for the request
   */
  BwRequest * getRequest (int cid);

  /**
   * Resume the timers for the requests
   */
  void resumeTimers ();

  /**
   * Pause the timers for the requests
   */
  void pauseTimers ();

 private:
  struct contentionRequest request_list_;
};

#endif
