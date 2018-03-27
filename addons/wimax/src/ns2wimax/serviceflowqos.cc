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

#include "serviceflowqos.h"

/**
 * Constructor
 * @param delay The maximum delay for this connection
 * @param datarate The average datarate for the connection
 * @param burstsize The number of byte sent per burst
 */
ServiceFlowQoS::ServiceFlowQoS (int delay, int datarate, int burstsize) {
  delay_ = delay;
  datarate_ = datarate;
  burstsize_ = burstsize;
}


