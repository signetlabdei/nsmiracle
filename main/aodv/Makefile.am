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

lib_LTLIBRARIES = libMiracleAodv.la
check_PROGRAMS = 

TESTS = 

libMiracleAodv_la_SOURCES = module-aodv.h module-aodv.cc\
				aodv-rt-table.cc aodv-rt-table.h \
				aodv-tracer.h aodv-tracer.cc \
				aodv-pkt.h initlib.cc

libMiracleAodv_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libMiracleAodv_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libMiracleAodv_la_LDFLAGS = @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libMiracleAodv_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@

nodist_libMiracleAodv_la_SOURCES = InitTcl.cc

BUILT_SOURCES = InitTcl.cc

CLEANFILES = InitTcl.cc

TCL_FILES =  aodv-defaults.tcl aodv-proc.tcl

InitTcl.cc: Makefile 
		cat $(TCL_FILES) | @TCL2CPP@ AodvInitTclCode > InitTcl.cc

EXTRA_DIST = $(TCL_FILES) 
