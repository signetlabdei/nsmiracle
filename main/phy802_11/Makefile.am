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


lib_LTLIBRARIES = libMiraclePhy802_11.la libphytracer.la
check_PROGRAMS = 

TESTS = 

libMiraclePhy802_11_la_SOURCES = wirelessphy-module.cc wirelessphy-module.h \
				mrcl_wireless-phy.cc mrcl_wireless-phy.h \
				mrcl_freespace.cc mrcl_freespace.h \
				mrcl_tworayground.cc mrcl_tworayground.h \
				mrcl_propagation.h deipropagation.h \
				simplepathloss.cc simplepathloss.h \
				initlib.cc

libMiraclePhy802_11_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libMiraclePhy802_11_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libMiraclePhy802_11_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libMiraclePhy802_11_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@

nodist_libMiraclePhy802_11_la_SOURCES = MiraclePhy80211InitTcl.cc

libphytracer_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libphytracer_la_SOURCES = phy-tracer.cc phy-tracer.h
libphytracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libphytracer_la_LDFLAGS = @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ -L./
libphytracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lMiraclePhy802_11

BUILT_SOURCES = MiraclePhy80211InitTcl.cc

CLEANFILES = MiraclePhy80211InitTcl.cc

TCL_FILES =  wirelessphy-default.tcl  wirelessphy-proc.tcl

MiraclePhy80211InitTcl.cc: Makefile $(TCL_FILES)
		           cat $(TCL_FILES) | @TCL2CPP@ MiraclePhy80211InitTclCode > MiraclePhy80211InitTcl.cc


EXTRA_DIST = $(TCL_FILES)


