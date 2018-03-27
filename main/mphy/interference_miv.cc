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

#include"interference_miv.h"

#include<iostream>
#include <iomanip>

#include"mphy.h"
//#include <values.h>


#define POWER_PRECISION_THRESHOLD (1e-14)
#define EPSILON_TIME 0.000001
#define DOUBLE_CHECK 0.000001
#define DOUBLE_CHECK_LOW 0.001


static class MInterferenceMIVClass : public TclClass {
public:
  MInterferenceMIVClass() : TclClass("MInterference/MIV") {}
  TclObject* create(int, const char*const*) {
    return (new MInterferenceMIV);
  }
} class_minterference_miv;


void EndInterferenceMIVTimer::handle(Event *e)
{
  // This does not work, Event is not polymorphic
  //  PowerEvent* pe = dynamic_cast<PowerEvent*>(e);
  PowerEvent* pe = (PowerEvent*) e;
  mimiv->addToInterference( - pe->power, NOW);
  delete pe;
  if (mimiv->debug_) mimiv->dump("EndInterferenceMIVTimer::handle");
}


MInterferenceMIV::MInterferenceMIV()
  : endinterftimer(this)
{
  bind("maxinterval_", &maxinterval_);
  bind("debug_", &debug_);
}


MInterferenceMIV::~MInterferenceMIV()
{

}


void MInterferenceMIV::addToInterference(Packet* p)
{
  hdr_MPhy *ph = HDR_MPHY(p);
  addToInterference(ph->Pr, NOW);
  PowerEvent* pe = new PowerEvent(ph->Pr);
  Scheduler::instance().schedule(&endinterftimer, pe, ph->duration - EPSILON_TIME);
  if (debug_) dump("MInterferenceMIV::addToInterference");
}

void MInterferenceMIV::addToInterference(double power, double starttime)
{
  // Delete too old items
  Function::iterator it; 
  for (it = pp.begin(); it!=pp.end();  )
    {
      if (it->time < starttime - maxinterval_)
	{	   
	  it = pp.erase(it); // Side effect: it++	   
	}
      else
	break;
    }

  // Add new item

  if (pp.empty())
    {
      Point newp(NOW, power);
      pp.push_back(newp);      
    }
  else
    {
      Point lastp(pp.back());
      Point newp(NOW, lastp.value + power);

      // Check for cancellation errors
      // which can arise when interference is subtracted
      if (newp.value < 0)
	{
	  if (newp.value < POWER_PRECISION_THRESHOLD)
	    {
	      if (debug_)
		std::cerr << "WARNING: interf=" << newp.value << " - cancellation error or bug?" << std::endl;
	    }
	  newp.value = 0;
	}

      pp.push_back(newp);
    }

}

double MInterferenceMIV::getInterferencePower(Packet* p)
{
  hdr_MPhy *ph = HDR_MPHY(p);
  return (getInterferencePower(ph->Pr, ph->rxtime, ph->duration));  
}
      
const PowerChunkList& MInterferenceMIV::getInterferencePowerChunkList(Packet* p) {
  hdr_MPhy *ph = HDR_MPHY(p);
  return (getInterferencePowerChunkList(ph->Pr, ph->rxtime, ph->duration));  
}

int MInterferenceMIV::compareDoubleValueHighPrec(double v1, double v2) {
	if (v1 - DOUBLE_CHECK <= v2 && v2 <= v1 + DOUBLE_CHECK) {
		return 0;
	}
	if (v1 < v2) {
		return -1;
	}
	return 1;
}

int MInterferenceMIV::compareDoubleValueLowPrec(double v1, double v2) {
	if (v1 - DOUBLE_CHECK_LOW <= v2 && v2 <= v1 + DOUBLE_CHECK_LOW) {
		return 0;
	}
	if (v1 < v2) {
		return -1;
	}
	return 1;
}

const PowerChunkList& MInterferenceMIV::getInterferencePowerChunkList(double power, double starttime, double duration) {
  power_chunk_list.clear();
	
	Function::reverse_iterator rit; 
  
  double integral = 0;
  double lasttime = NOW;

  assert(starttime<= NOW);
  assert(duration > 0);
  assert(maxinterval_ > duration);

  for (rit = pp.rbegin(); rit != pp.rend(); ++rit )  {
    if (starttime < rit->time) {
      double power_integral = rit->value - power;
      double time_value = (lasttime - rit->time);
      if (compareDoubleValueLowPrec(power_integral, 0.0) <= 0 || compareDoubleValueHighPrec(time_value, EPSILON_TIME) == 0) {
        lasttime = rit->time;
        //it is the pkt I am receiving
        continue;
      }
      PowerChunk power_chunk;
      power_chunk.first = power_integral;
      power_chunk.second = (lasttime - rit->time);
      power_chunk_list.push_back(power_chunk);

      lasttime = rit->time;
    }
    else {
      //  integral += rit->value * (lasttime - starttime);
      double power_integral = rit->value - power;
      PowerChunk power_chunk;
      power_chunk.first = power_integral;
      power_chunk.second = (lasttime - starttime);
      power_chunk_list.push_back(power_chunk);
      break;
    }
  }

  return power_chunk_list;
}

double MInterferenceMIV::getInterferencePower(double power, double starttime, double duration)
{

  Function::reverse_iterator rit; 
  
  double integral = 0;
  double lasttime = NOW;

  assert(starttime<= NOW);
  assert(duration > 0);
  assert(maxinterval_ > duration);

  for (rit = pp.rbegin(); rit != pp.rend(); ++rit )
    {
      if (starttime < rit->time)
	{
	  integral += rit->value * (lasttime - rit->time);
	  lasttime = rit->time;	  
	}
      else
	{
	  integral += rit->value * (lasttime - starttime);
	  break;
	}
    }

  double interference = (integral/duration) - power;

  // Check for cancellation errors
  // which can arise when interference is subtracted
  if (interference < 0)
    {
      if (interference < POWER_PRECISION_THRESHOLD)
	{
	  // should be a cancellation error, but it exceeds the
	  // precision threshold, so we print a warning 
	  if (debug_)
	    cerr << "MInterferenceMIV::getInterferencePower() WARNING:" 
		 << " interference=" << interference 
		 << " POWER_PRECISION_THRESHOLD=" <<  POWER_PRECISION_THRESHOLD
		 << endl;
	}
      interference = 0;
    }


  if (debug_) {
    dump("MInterferenceMIV::getInterferencePower");
    std::cerr << "transmission from " << starttime 
	      << " to " << starttime + duration 
	      << " power " << power
	      << " gets interference " << interference
	      << std::endl;
  }

  return interference;
}


void MInterferenceMIV::dump(string msg)
{
  Function::iterator it; 

  std::cerr  << NOW << " "
	     << msg << std::endl;

  for (it = pp.begin(); it!=pp.end();  ++it)
    std::cerr << std::setw(7) << std::setprecision(5) <<it->value << " ";

  std::cerr << std::endl;

  for (it = pp.begin(); it!=pp.end();  ++it)
    std::cerr << std::setw(7) << std::setprecision(5) << it->time << " ";

  std::cerr << std::endl; 
}     


double MInterferenceMIV::getCurrentTotalPower()
{
  if (pp.empty())
    return 0.0;
  else
    return (pp.back().value);
}
