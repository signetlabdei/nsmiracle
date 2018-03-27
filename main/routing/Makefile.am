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

lib_LTLIBRARIES = libMiracleRouting.la
check_PROGRAMS = 

TESTS = 

libMiracleRouting_la_SOURCES = mrcl-address.cc mrcl-address.h \
				metric.cc metric.h \
				routing-module.cc routing-module.h \
				utility.cc utility.h \
				ip-address.cc ip-address.h \
				routing-queue.cc routing-queue.h \
				routing-tracer.h routing-tracer.cc \
				hop-count-metric.cc hop-count-metric.h \
				ip-address-codec.h ip-address-codec.cc \
				mrcl-routing-static.h mrcl-routing-static.cc \
				initlib.cc

libMiracleRouting_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@ 
libMiracleRouting_la_LDFLAGS = @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libMiracleRouting_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@

nodist_libMiracleRouting_la_SOURCES = RoutingInitTcl.cc

BUILT_SOURCES = RoutingInitTcl.cc

CLEANFILES = RoutingInitTcl.cc

TCL_FILES =  routing-proc.tcl

RoutingInitTcl.cc: Makefile 
		   cat $(TCL_FILES) | @TCL2CPP@ MrclRoutingInitTclCode > RoutingInitTcl.cc

EXTRA_DIST = $(TCL_FILES) 
