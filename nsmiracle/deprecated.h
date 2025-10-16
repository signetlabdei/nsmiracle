/*
 * Copyright (c) 2006 Regents of the SIGNET lab, University of Padova.
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
 */


/**
 * @file   deprecated.h
 * @author Baldo Nicola
 * @date   Thu Nov 22 18:11:42 2007
 * 
 * @brief  some macro definitions used to detect use of obsolete
 * functions and  print some warning
 * 
 * 
 */


#ifndef NSMIRACLE_DEPRECATED_H
#define NSMIRACLE_DEPRECATED_H

#include<iostream>


#define RETVAL_NOT_IMPLEMENTED 5432


#define PRINT_WARNING_DEPRECATED_FUNCTION {				\
    static bool print_warning_deprecated_function = true;		\
      if (print_warning_deprecated_function)	{			\
	cerr << "WARNING: method " << __PRETTY_FUNCTION__			\
	     << "() is deprecated " << endl;				\
	print_warning_deprecated_function = false;			\
      }									\
  }




#ifdef __STRING
#define WARNING_MSG_DEPRECATED_VIRTUAL_METHOD(X,Y) "WARNING: deprecated virtual method " __STRING(X) " is being used instead of " __STRING(Y)
#else //  __STRING is undefined !!!
#define WARNING_MSG_DEPRECATED_VIRTUAL_METHOD "WARNING: a deprecated virtual method is being used instead of a new one "
#endif // __STRING



#define RUN_DEPRECATED_OR_NEW_VIRTUAL_METHOD(X,Y) {	\
    static bool print_warning_deprecated_virtual_method = true;		\
    int retval=(Y);							\
    if ( retval ==  RETVAL_NOT_IMPLEMENTED ) {				\
	retval = (X); 							\
	if (( retval !=  RETVAL_NOT_IMPLEMENTED )                     \
             && print_warning_deprecated_virtual_method)	{	\
	   cerr << WARNING_MSG_DEPRECATED_VIRTUAL_METHOD(X,Y) << endl;   \
	   print_warning_deprecated_virtual_method = false;		\
	}								\
      }									\
  }







#endif // NSMIRACLE_DEPRECATED_H
