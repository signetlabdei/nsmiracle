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
 * $Id: profile.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef PROFILE_H
#define PROFILE_H

#include "ofdm-packet.h"

class SubFrame;

class Profile;
LIST_HEAD (profile, Profile);

/**
 * This class contains information about burst such as modulation, frequency...
 */
class Profile
{
 public:
  /**
   * Creates a profile with the given frequency and encoding
   * @param f The frequency information for the profile
   * @param enc The encoding type
   */
  Profile (SubFrame *subframe, int f, Ofdm_mod_rate enc);

  /**
   * Set the IUC number for this profile
   * @param iuc The IUC number for this profile
   */
  void setIUC( int iuc );

  /**
   * Return the frequency in unit of kHz
   * @return the frequency
   */
  int getIUC();

  /**
   * Return the encoding type
   * @return the encoding type
   */
  Ofdm_mod_rate getEncoding( );

  /**
   * Return the frequency in unit of kHz
   * @return the frequency
   */
  int getFrequency( );

  /**
   * Set the encoding type
   * @param enc the encoding type
   */
  void setEncoding( Ofdm_mod_rate enc );

  /**
   * Set the frequency in unit of kHz
   * @param f the frequency
   */
  void setFrequency( int f );

  // Chain element to the list
  inline void insert_entry(struct profile *head) {
    LIST_INSERT_HEAD(head, this, link);
  }
  
  // Return next element in the chained list
  Profile* next_entry(void) const { return link.le_next; }

  // Remove the entry from the list
  inline void remove_entry() { 
    LIST_REMOVE(this, link); 
  }

 private:
  /**
   * The type of modulation used by the burst
   */
  Ofdm_mod_rate encoding_;
  
  /**
   * The downlink frequency in kHz
   */
  int frequency_;
  
  /**
   * The Interval Usage Code for the profile
   */
  int iuc_;

  /**
   * The subframe containing this profile
   * Used to inform configuration change
   */
  SubFrame *subframe_;
  
  /*
   * Pointer to next in the list
   */
  LIST_ENTRY(Profile) link;
  //LIST_ENTRY(Profile); //for magic draw
  	
};

#endif
