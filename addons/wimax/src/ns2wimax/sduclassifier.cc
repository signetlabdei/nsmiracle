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

#include "sduclassifier.h"
  
/*
 * Create a classifier in the given mac
 */
SDUClassifier::SDUClassifier () 
{
  //set default priority
  priority_ = 0; 
}

/*
 * Interface with the TCL script
 * @param argc The number of parameter
 * @param argv The list of parameters
 */
int SDUClassifier::command(int argc, const char*const* argv)
{
  if (argc == 3) {
    if (strcmp(argv[1], "set-priority") == 0) {
      priority_ = atoi(argv[2]);
      return TCL_OK;
    }
  }
  return TCL_ERROR;
}

/**
 * Classify a packet and return the CID to use (or -1 if unknown)
 * @param p The packet to classify
 * @return The CID or -1
 */
int SDUClassifier::classify (Packet * p) {
  return -1;
}
