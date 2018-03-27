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

#include "scanningstation.h"

/**
 * Create an data object with the given attributes
 * @param nodeid The node 
 * @param duration The scanning duration
 * @param start The frame at which the scanning start
 * @param interleaving The interleaving interval
 * @param iteration The number of iterations
 */
ScanningStation::ScanningStation (int nodeid, int duration, int start, int interleaving, int iteration)
{
  nodeid_ = nodeid;
  duration_ = duration;
  start_frame_ = start;
  interleaving_ = interleaving;
  iteration_ = iteration;
}

/**
 * Determines if the node is currently scanning
 * @param frame The current frame
 */
bool ScanningStation::isScanning (int frame)
{
  //printf ("isScanning %d frame=%d, start_frame=%d, duration=%d, interleaving=%d iteration %d\n", nodeid_, frame, start_frame_, duration_, interleaving_, iteration_);
  if ((frame < start_frame_)||(frame > (start_frame_ + (duration_+interleaving_)*iteration_)))
    return false;
  else {
    return (((frame-start_frame_)%(duration_+interleaving_))-duration_)<0;
  }
}
