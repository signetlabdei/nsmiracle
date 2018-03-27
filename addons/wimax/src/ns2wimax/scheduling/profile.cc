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

#include "profile.h"
#include "subframe.h"

/**
 * Creates a profile with the given frequency and encoding
 * @param f The frequency information for the profile
 * @param enc The encoding type
 */
Profile::Profile (SubFrame *subframe, int f, Ofdm_mod_rate enc) : iuc_(0)
{
  assert (subframe);
  subframe_ = subframe;
  frequency_ = f;
  encoding_ = enc;
}

/**
 * Return the encoding type
 * @return the encoding type
 */
Ofdm_mod_rate Profile::getEncoding( ) 
{ 
  return encoding_; 
}

/**
 * Set the encoding type
 * @param enc the encoding type
 */
void Profile::setEncoding( Ofdm_mod_rate enc ) 
{
  if (encoding_ != enc)
    subframe_->incrCCC();
  encoding_ = enc; 
}

/**
 * Return the frequency in unit of kHz
 * @return the frequency
 */
int Profile::getFrequency( ) 
{ 
  return frequency_; 
}

/**
 * Set the frequency in unit of kHz
 * @param f the frequency
 */
void Profile::setFrequency( int f ) 
{ 
  if (frequency_ != f)
    subframe_->incrCCC();
  frequency_ = f; 
}

/**
 * Return the frequency in unit of kHz
 * @return the frequency
 */
int Profile::getIUC( ) 
{ 
  return iuc_; 
}

/**
 * Set the IUC number for this profile
 * @param iuc The IUC number for this profile
 */
void Profile::setIUC( int iuc ) 
{ 
  if (iuc_!=0 && iuc_!= iuc)
    subframe_->incrCCC();
  iuc_ = iuc; 
}


