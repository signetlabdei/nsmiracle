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

#ifndef SERVICEFLOWQOS_H
#define SERVICEFLOWQOS_H

/**
 * Class ServiceFlowQoS
 * Defines Qos requirements for the flows
 */ 
class ServiceFlowQoS {
  
 public:
  /**
   * Constructor
   * @param delay The maximum supported delay for the connection
   * @param datarate Average datarate
   * @param burstsize Size of each burst
   */
  ServiceFlowQoS (int delay, int datarate, int burstsize);
  
  /**
   * Return the maximum delay supported by the connection
   */
  inline double  getDelay () { return delay_; }
  
  /**
   * Return the average datarate
   */
  inline double  getDatarate () { return datarate_; }
  
  /**
   * Return the burst size
   */
  inline int  getBurstSize () { return burstsize_; }
  
  /**
   * Set the maximum delay supported by the connection
   * @param delay The new delay
   */
  inline void  setDelay (double delay) { delay_ = delay; }
  
  /**
   * Set the average datarate for the connection
   * @param datarate The average datarate
   */
  inline void  setDatarate (double datarate) { datarate_ = datarate; }
  
  /**
   * Set the burst size for the connection
   * @param size The number of byte sent for each burst
   */
  inline void  setBurstSize (int size) { burstsize_ = size; }
  
 protected:

 private:
  /**
   * The maximum delay for this connection (in sec)
   */
   double delay_;
  /**
   * The average datarate
   */
   double datarate_;
  /**
   * The number of bytes per burst
   */
   int burstsize_;
};
#endif //SERVICEFLOWQOS_H

