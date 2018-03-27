#
# Copyright (c) 2007 Regents of the SIGNET lab, University of Padova.
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

TCL_MACRO = ns-mobilenode.tcl

EXTRA_DIST = $(TCL_MACRO)

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libMobileNode.la
check_PROGRAMS = 

TESTS = 

libMobileNode_la_SOURCES = 	802.11-module.h 802.11-module.cc	\
				wirelessphy-module.h wirelessphy-module.cc	\
				mbposition.h mbposition.cc	\
				ipmodule.h ipmodule.cc	\
				transport-module.h transport-module.cc \
				wirelessch-module.h wirelessch-module.cc \
				802_11-tracer.h 802_11-tracer.cc \
				initlib.cc

nodist_libMobileNode_la_SOURCES = suptcl.cc

BUILT_SOURCES = suptcl.cc

CLEANFILES = suptcl.cc

libMobileNode_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libMobileNode_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ 
libMobileNode_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@

suptcl.cc: 	Makefile $(TCL_MACRO)
		cat $(TCL_MACRO) | @TCL2CPP@ MobileNodetclSupp > suptcl.cc
