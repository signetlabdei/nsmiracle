#
# Copyright (c) 2006 Regents of the SIGNET lab, University of Padova.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University of Padova (SIGNET lab) nor the 
#    names of its contributors may be used to endorse or promote products 
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

AC_INIT(miracleport,0.1)
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL


dnl Activate all compile warnings & turn them to errors.
dnl This should have been already achieved by passing the [-Wall -Werror] 
dnl parameter to AM_INIT_AUTOMAKE, but for some weird reason it doesn't work.
dnl CFLAGS="${CFLAGS=} -Wall -Werror"
CFLAGS="${CFLAGS=} -Wall -g"

NS_ALLINONE_PATH=''
NS_PATH=''
TCL_PATH=''
OTCL_PATH=''
NS_INCLUDES=''

AC_ARG_WITH([ns-allinone],
	[AS_HELP_STRING([--with-ns-allinone=<directory>],
			[use ns-allinone installation in <directory>, where it is expected to find ns, tcl, otcl and tclcl subdirs])],
	[
	    if test ! -d $withval 
		then
		AC_MSG_ERROR([ns-allinone path $withval is not valid])
	    else	
		NS_ALLINONE_PATH=$withval

		NS_PATH=$NS_ALLINONE_PATH/`cd $NS_ALLINONE_PATH; ls -d ns-* | head -n 1`
		TCL_PATH=$NS_ALLINONE_PATH/`cd $NS_ALLINONE_PATH; ls -d * | grep -e 'tcl[0-9].*' | head -n 1`
		TCLCL_PATH=$NS_ALLINONE_PATH/`cd $NS_ALLINONE_PATH; ls -d tclcl-* | head -n 1`
		OTCL_PATH=$NS_ALLINONE_PATH/`cd $NS_ALLINONE_PATH; ls -d otcl-* | head -n 1`

		BASIC_INCLUDES="-I$NS_ALLINONE_PATH/include -I$NS_PATH -I$TCLCL_PATH -I$OTCL_PATH"

		dnl This is to allow configure to find headers when performing tests
		dnl CPPFLAGS="$CPPFLAGS $BASIC_INCLUDES"

	    fi
	])

if test x$NS_ALLINONE_PATH = x ;    then
   AC_MSG_ERROR([you must specify ns-allinone installation path using --with-ns-allinone=PATH])
fi	     

AC_ARG_WITH([ns-miracle],
	[AS_HELP_STRING([--with-ns-miracle=<directory>],
			[use ns-miracle installation in <directory>, where it is expected to find ns-miracle library source])],
	[
	    if test ! -d $withval 
		then
		AC_MSG_ERROR([ns-miracle path $withval is not valid])
	    else	
		NS_MIRACLE_PATH=$withval

		BASIC_INCLUDES="$BASIC_INCLUDES -I$NS_MIRACLE_PATH"

	    fi
	])




NS_INCLUDES=$BASIC_INCLUDES

dnl ns include paths, 
dnl uncomment what you need and comment the rest if you wish

NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/mac"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/propagation"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/mobile"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/pcap"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/tcp"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/sctp"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/common"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/link"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/queue"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/trace"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/adc"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/apps"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/routing"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/tools"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/classifier"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/mcast"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/diffusion3/lib"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/diffusion3/lib/main"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/diffusion3/lib/nr"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/diffusion3/ns"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/diffusion3/filter_core"
NS_INCLUDES="$NS_INCLUDES  -I$NS_PATH/asim"

AC_SUBST(NS_INCLUDES)

NS_LDFLAGS=""
AC_SUBST(NS_LDFLAGS)

NS_LIBS=""
AC_SUBST(NS_LIBS)


AC_DEFINE(CPP_NAMESPACE,std)


dnl ******* Check for needed header files ***********


dnl echo "CPPFLAGS = $CPPFLAGS"

dnl We need to change this only for header checking


BACKUP_INCLUDES=$CPPFLAGS
CPPFLAGS=$NS_INCLUDES

dnl echo "NS_INCLUDES = $NS_INCLUDES"

AC_CHECK_HEADERS([tcl.h],,AC_MSG_ERROR([could not find tcl.h]))
AC_CHECK_HEADERS([otcl.h],,AC_MSG_ERROR([could not find otcl.h]))
dnl AC_CHECK_HEADERS([tclcl.h],,AC_MSG_ERROR([could not find tclcl.h]), 
dnl 		 [ 
dnl 		   #if HAVE_TCL_H
dnl 		   #include <tcl.h>
dnl 		   #endif 
dnl 		 ])  	

AC_MSG_CHECKING([if programs can be compiled against NS headers])

AC_PREPROC_IFELSE(
	[AC_LANG_PROGRAM([[
		#include<tcl.h>
		#include<otcl.h>
		#include<tclcl.h>
		#include<packet.h>
		Packet* p; 
		]],[[
		p = new packet;
		delete p;
		]]  )],
        [AC_MSG_RESULT([yes])],
        [
	  AC_MSG_RESULT([no])
	  AC_MSG_ERROR([could not compile a test program against NS headers. Is --with-ns-allinone set correctly? ])
        ])

AC_MSG_CHECKING([if programs can be compiled against ns-miracle headers])
AC_PREPROC_IFELSE(
	[AC_LANG_PROGRAM([[
		#include<cltracer.h>
		ClMessageTracer* t; 
		]],[[
		]]  )],
        [AC_MSG_RESULT([yes])],
        [
	  AC_MSG_RESULT([no])
	  AC_MSG_ERROR([could not compile a test program against ns-miracle headers. Is --with-ns-miracle set correctly? ])
        ])

# AC_CHECK_HEADERS([cltracer.h],,AC_MSG_ERROR([you must specify ns-miracle installation path using --with-ns-miracle=PATH]))

dnl Restoring to the initial value
CPPFLAGS=$BACKUP_INCLUDES


TCL2CPP=
AC_PATH_PROG([TCL2CPP],[tcl2c++],[none],[$PATH:$TCLCL_PATH])
if test $TCL2CPP = none ;    then
	AC_MSG_ERROR([could not find tcl2c++])
fi


AC_CONFIG_FILES([
		Makefile
 		])


AC_OUTPUT
