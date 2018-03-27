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
 * $Id: contentionrequest.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef CONTENTIONREQUEST_H
#define CONTENTIONREQUEST_H

#include "mac802_16.h"
#include "../mac802_16timer.h"

class ContentionSlot;
class RngContentionSlot;
class BwContentionSlot;
class ContentionTimer;
class Mac802_16;

class ContentionRequest;
/** Timer for backoff */
class WimaxBackoffTimer : public WimaxTimer {
 public:
  WimaxBackoffTimer(ContentionRequest *c, Mac802_16 *m) : WimaxTimer(m) {c_=c;}
  
  void	handle(Event *e);
  void pause(void);
  void resume(void);
 private:
  ContentionRequest *c_;
}; 


class ContentionRequest;
LIST_HEAD (contentionRequest, ContentionRequest);

/**
 * This class is used to manage contention opportunities
 * supports list
 */
class ContentionRequest
{
  friend class WimaxBackoffTimer;
 public:
  /**
   * Creates a contention slot for the given frame
   * @param s The contention slot 
   * @param p The packet to send
   */
  ContentionRequest (ContentionSlot *s, Packet *p);
  virtual ~ContentionRequest ();
  /**
   * Called when timeout expired
   */
  virtual void expire ();

  /**
   * Start the timeout timer
   */
  void starttimeout();

  /** 
   * Pause the backoff timer
   */
  void pause ();
  
  /**
   * Resume the backoff timer
   */
  void resume ();

  /// Chain element to the list
  inline void insert_entry_head(struct contentionRequest *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  /// Chain element to the list
  inline void insert_entry(ContentionRequest *elem) {
    LIST_INSERT_AFTER(elem, this, link);
  }

  /// Return next element in the chained list
  ContentionRequest* next_entry(void) const { return link.le_next; }

  /// Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }

 protected:

  /**
   * The contention slot information
   */
  ContentionSlot *s_;

  /**
   * The backoff timer
   */
  WimaxBackoffTimer *backoff_timer_;

  /**
   * The timeout timer
   */
  ContentionTimer *timeout_timer_;

  /**
   * Type of timer
   */
  timer_id type_; 

  /**
   * Value for timeout
   */
  double timeout_;

  /**
   * The current window size
   */
  int window_;

  /**
   * Number of retry
   */
  int nb_retry_;

  /** 
   * The scheduler to inform about timeout
   */
  Mac802_16 *mac_;

  /**
   * The packet to send when the backoff expires
   */
  Packet *p_;

  /**
   * Pointer to next in the list
   */
  LIST_ENTRY(ContentionRequest) link;
  //LIST_ENTRY(ContentionRequest); //for magic draw
};

/**
 * Class to handle ranging opportunities
 */
class RangingRequest: public ContentionRequest 
{
 public:

  /**
   * Creates a contention slot for the given frame
   * @param frame The frame map 
   */
  RangingRequest (ContentionSlot *s, Packet *p);

  /**
   * Called when timeout expired
   */
  void expire ();

 private:
};


/**
 * Class to handle bandwidth request opportunities
 */
class BwRequest: public ContentionRequest 
{
 public:

  /**
   * Creates a contention slot for the given frame
   * @param frame The frame map 
   */
  BwRequest (ContentionSlot *s, Packet *p, int cid, int length);

  /**
   * Called when timeout expired
   */
  void expire ();

  /**
   * Return the CID for this request
   * @return the CID for this request
   */
  inline int getCID () { return cid_; }

 private:
  /**
   * The CID for the request
   */
  int cid_;

  /**
   * The size in bytes of the bandwidth requested
   */
  int size_;
};

#endif
