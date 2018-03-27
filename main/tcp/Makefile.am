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

lib_LTLIBRARIES = libmiracletcp.la libtcptracer.la 
check_PROGRAMS = 

TESTS = 

libmiracletcp_la_SOURCES = tcp-module.cc \
			tcp-module.h \
			initlib.cc 

libmiracletcp_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libmiracletcp_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@  -L../ip/ 
libmiracletcp_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lMiracleIp 


libtcptracer_la_SOURCES = tcp-tracer.cc tcp-tracer.h 
libtcptracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libtcptracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@  -L../ip/ -L./
libtcptracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lMiracleIp -lmiracletcp



nodist_libmiracletcp_la_SOURCES = TCPInitTcl.cc

BUILT_SOURCES = TCPInitTcl.cc

CLEANFILES = TCPInitTcl.cc

TCL_FILES = tcp-modules.tcl tcp-defaults.tcl

TCPInitTcl.cc: Makefile $(TCL_FILES)
		cat $(TCL_FILES) | @TCL2CPP@ MiracleTcpInitTclCode > TCPInitTcl.cc


EXTRA_DIST =  $(TCL_FILES)

