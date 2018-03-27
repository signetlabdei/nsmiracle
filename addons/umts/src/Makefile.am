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
 
AM_CFLAGS = -Wall -g

lib_LTLIBRARIES = libumts.la libumtstracer.la 
check_PROGRAMS = 

SUBDIRS = 

TESTS = 

libumts_la_SOURCES = am.cc am.h\
		um.cc um.h \
		init.cc\
		rlc.h\
		umts-headers.h\
		umts-packet.cc\
		umts-packet.h\
		umts-queue.cc\
		umts-queue.h\
		umts-timers.cc\
		umts-timers.h\
		rlc-module.h rlc-module.cc  \
		umts-phy.h umts-phy.cc \
		umts-correlation.cc umts-correlation.h \
		mac-module.cc mac-module.h \
		umts-interference_miv.h umts-interference_miv.cc \
		umts-wirelessch.h umts-wirelessch.cc
#		qa_umts-phy.cc qa_umts-phy.h 
#		umts-ip-interface.cc	umts-ip-interface.h 

# um.cc	um.h



libumts_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libumts_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libumts_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@

nodist_libumts_la_SOURCES = InitTcl.cc

libumtstracer_la_SOURCES = umts-tracer.cc


libumtstracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libumtstracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@  -L./
libumtstracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lumts


BUILT_SOURCES = InitTcl.cc

CLEANFILES = InitTcl.cc

TCL_FILES =    umts-packet.tcl umts-modules.tcl umts-default.tcl umts-wirelessch-default.tcl #qa_umts-phy-defaults.tcl umts.tcl

InitTcl.cc: Makefile $(TCL_FILES)
		cat $(TCL_FILES) | @TCL2CPP@ InitTclCode > InitTcl.cc


EXTRA_DIST = $(TCL_FILES)
