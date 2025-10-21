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
 * 
 * thanks to: Roberto Petroccia for the PowerChunk interference model.
 * 
 */

#ifndef INTERFERENCE_MIV_H
#define INTERFERENCE_MIV_H

#include <minterference.h>
#include <list>
#include <scheduler.h>
#include <string>

class Point {
 public:
  double time;   /**< t value    */
  double value;  /**< f(t) value */

  Point(double t, double v) {value=v; time=t;}
  Point(const Point &c) {value=c.value; time=c.time;}
  Point &operator=(const Point &rhs) { this->value=rhs.value; this->time=rhs.time; return *this;}
  int operator==(const Point &rhs) const;
  int operator<(const Point &rhs) const;
};


typedef std::list<Point> Function;
typedef std::list<Point>::iterator FunctionIterator;


class MInterferenceMIV;

class PowerEvent : public Event {
public:
  PowerEvent(double pow) : power(pow) {}
  virtual ~PowerEvent() {}
  double power;
};


class EndInterferenceMIVTimer : public Handler {

public:
  EndInterferenceMIVTimer(MInterferenceMIV* ptr) : mimiv(ptr)  {	}
  virtual void handle(Event *e);  

protected:
  MInterferenceMIV*	  mimiv;

};


/**
 * Calculate interference using Mean Integral Value
 * 
 */
class MInterferenceMIV : public MInterference
{
  friend class EndInterferenceMIVTimer;
 public:
  MInterferenceMIV();
  virtual ~MInterferenceMIV();
  virtual void addToInterference(Packet* p);
  virtual void addToInterference(double power, double starttime); 
  virtual double getInterferencePower(Packet* p);
  virtual double getInterferencePower(double power, double starttime, double endtime);
  virtual const PowerChunkList& getInterferencePowerChunkList(double power, double starttime, double duration);
  virtual double getCurrentTotalPower(); 
  virtual const PowerChunkList& getInterferencePowerChunkList(Packet* p);

 protected:
	 
  void dump(string msg); /// dumps to stdout all the interference data currently stored
  Function pp;             /// Power on the channel as a function of time
  double maxinterval_;     /// Maximum time interval for which power
			   /// information is to be stored
  EndInterferenceMIVTimer endinterftimer; /// schedules end of interference for a transmission
  int debug_;              /// debug flag to be set via TCL
  PowerChunkList power_chunk_list; /// interference power chunk list
  
  int compareDoubleValueLowPrec(double v1, double v2);
  int compareDoubleValueHighPrec(double v1, double v2);

};

#endif /* INTERFERENCE_MIV_H */


