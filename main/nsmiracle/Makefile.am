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

TCL_MACRO = tcl/ns-miracle.tcl

EXTRA_DIST = $(TCL_MACRO)

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libMiracle.la
check_PROGRAMS = 

TESTS = 

libMiracle_la_SOURCES = bin.cc bin.h \
			clsap.cc clsap.h \
			connector-trace.cc connector-trace.h\
			plugin.cc plugin.h \
			clmessage.cc clmessage.h \
			clmsg-discovery.h clmsg-discovery.cc \
			module.cc module.h \
			node-core.cc node-core.h \
			sap.cc sap.h \
			packettracer.h packettracer.cc \
			cltracer.cc cltracer.h \
			chsap.cc	chsap.h \
			channel-module.cc channel-module.h \
			deprecated.h \
			clmsg-stats.cc clmsg-stats.h \
			logger.cpp logger.h \
			initlib.cc 

nodist_libMiracle_la_SOURCES = Miraclesuptcl.cc

BUILT_SOURCES = Miraclesuptcl.cc

CLEANFILES = Miraclesuptcl.cc

libMiracle_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libMiracle_la_CPPFLAGS = @NS_CPPFLAGS@ 
libMiracle_la_LDFLAGS = @NS_LDFLAGS@ 
libMiracle_la_LIBADD = @NS_LIBADD@


Miraclesuptcl.cc: 	Makefile $(TCL_MACRO)
			cat $(TCL_MACRO) | @TCL2CPP@ MiracletclSupp > Miraclesuptcl.cc
