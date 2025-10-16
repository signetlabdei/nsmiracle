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

lib_LTLIBRARIES = libMiracleIp.la libMiracleIpRouting.la libroutingtracer.la
check_PROGRAMS = 

TESTS = 

libMiracleIp_la_SOURCES = ipmodule.cc ipmodule.h \
			ip-clmsg.h ip-clmsg.cc \
			initlib.cc

libMiracleIp_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libMiracleIp_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libMiracleIp_la_LDFLAGS = @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libMiracleIp_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@

nodist_libMiracleIp_la_SOURCES = InitTcl.cc

libMiracleIpRouting_la_SOURCES = ip-interface.h ip-interface.cc\
				ip-routing.h ip-routing.cc \
				mrinit.cc

libMiracleIpRouting_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libMiracleIpRouting_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libMiracleIpRouting_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ -L./
libMiracleIpRouting_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lMiracleIp

libroutingtracer_la_SOURCES = routing-tracer.cc

libroutingtracer_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libroutingtracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libroutingtracer_la_LDFLAGS = @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ -L./
libroutingtracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lMiracleIp

BUILT_SOURCES = InitTcl.cc

CLEANFILES = InitTcl.cc

TCL_FILES =  ipmodule-default.tcl  ipmodule-proc.tcl

InitTcl.cc: Makefile 
		cat $(TCL_FILES) | @TCL2CPP@ MiracleIpInitTclCode > InitTcl.cc

EXTRA_DIST = $(TCL_FILES) 
