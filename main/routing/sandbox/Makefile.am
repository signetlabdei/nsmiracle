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

SUBDIRS = 



#AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libMiracleRoutingSandbox.la
check_PROGRAMS = 

TESTS = 

libMiracleRoutingSandbox_la_SOURCES = sandbox-rm1.h sandbox-rm1.cc\
				sandbox-address.h sandbox-address.cc\
				sandbox-metric.h sandbox-metric.cc\
				mrcl-routing-static.h mrcl-routing-static.cc\
				initlib.cc

libMiracleRoutingSandbox_la_CPPFLAGS = @NS_INCLUDES@
libMiracleRoutingSandbox_la_LDFLAGS = @NS_LDFLAGS@
libMiracleRoutingSandbox_la_LIBADD = @NS_LIBS@

nodist_libMiracleRoutingSandbox_la_SOURCES = InitTcl.cc

BUILT_SOURCES = InitTcl.cc

CLEANFILES = InitTcl.cc

TCL_FILES =  sandbox-rm1-defaults.tcl

InitTcl.cc: Makefile 
		cat $(TCL_FILES) | @TCL2CPP@ SandBoxMRoutingInitTclCode > InitTcl.cc

EXTRA_DIST = $(TCL_FILES) 
