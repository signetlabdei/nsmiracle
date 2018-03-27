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

#ifndef FRAMEMAP_H
#define FRAMEMAP_H

#include "subframe.h"
#include "mac802_16.h"

/**
 * This class contains the datastructure to describe a frame
 */
class FrameMap
{
 public:
  /*
   * Creates a map of the frame
   * @param mac Pointer to the mac layer
   */
  FrameMap (Mac802_16 *mac);

  /**
   * Compute and return the DCD frame
   */
  Packet* getDCD( );
  /**
   * Compute the DL_MAP packet based on the information contained in the structure
   */
  Packet* getDL_MAP( );

  /**
   * Compute and return the UCD frame
   */
  Packet* getUCD( );

  /**
   * Compute and return the UL_MAP frame
   */
  Packet* getUL_MAP( );

  /**
   * Return the attached mac
   * @return the mac
   */
  inline Mac802_16 * getMac () { return mac_; }

  /**
   * Return the DL subframe
   * @return the DL subframe
   */
  inline DlSubFrame * getDlSubframe () { return &dlsubframe_; }


  /**
   * Return the UL subframe
   * @return the UL subframe
   */
  inline UlSubFrame * getUlSubframe () { return &ulsubframe_; }


  /**
   * Parse a DL_MAP message and create the data structure
   * @param frame The DL frame information
   */
  void parseDLMAPframe (mac802_16_dl_map_frame *frame);

  /**
   * Parse a DCD message and create the data structure
   * @param frame The DL frame information
   */
  void parseDCDframe (mac802_16_dcd_frame *frame);

  /**
   * Parse a UL_MAP message and create the data structure
   * @param frame The UL frame information
   */
  void parseULMAPframe (mac802_16_ul_map_frame *frame);

  /**
   * Parse a UCD message and create the data structure
   * @param frame The DL frame information
   */
  void parseUCDframe (mac802_16_ucd_frame *frame);

  /**
   * Set the start time of the frame
   */
  inline void setStarttime (double time) { starttime_ = time; }

  /**
   * Return the time the frame started
   * @return The time the frame started
   */
  inline double getStarttime () { return starttime_; }

private:
  /**
   * The mac layer
   */
  Mac802_16 *mac_;

  /**
   * The frame duration
   */
  double duration_;
  
  /**
   * Time the frame started. Used for synchronization
   */
  double starttime_;

  /**
   * The number of PS required to switch from receiver to transmitter
   */
  int rtg_;
  
  /**
   * The number of PS required to switch from sender to receiver
   */
  int ttg_;
  
  /**
   * The downlink subframe
   */
  DlSubFrame dlsubframe_;

  /**
   * The uplink subframe
   */
  UlSubFrame ulsubframe_;
};

#endif
