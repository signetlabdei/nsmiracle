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


lib_LTLIBRARIES = libMiracleMac802_11.la libmac80211tracer.la libarptracer.la libalttracermac80211.la

check_PROGRAMS = 

TESTS = 

libMiracleMac802_11_la_SOURCES = 802.11-module.cc 802.11-module.h \
				miracle_mac-802_11.cc miracle_mac-802_11.h \
				mrcl_mac-timers.cc mrcl_mac-timers.h \
				mrcl_ll.h mrcl_ll.cc \
				mrcl_arp.h mrcl_arp.cc \
				initlib.cc \
				802_11-tracer.cc 802_11-tracer.h

libMiracleMac802_11_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libMiracleMac802_11_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libMiracleMac802_11_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libMiracleMac802_11_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@

nodist_libMiracleMac802_11_la_SOURCES = InitTcl.cc



libmac80211tracer_la_SOURCES = 	802_11-tracer.cc 802_11-tracer.h 

libmac80211tracer_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libmac80211tracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libmac80211tracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libmac80211tracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@


libarptracer_la_SOURCES = arp-tracer.h arp-tracer.cc 

libarptracer_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libarptracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libarptracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libarptracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@


libalttracermac80211_la_SOURCES = alttracermac80211.cc 

libalttracermac80211_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libalttracermac80211_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libalttracermac80211_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libalttracermac80211_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@



BUILT_SOURCES = InitTcl.cc

CLEANFILES = InitTcl.cc

TCL_FILES =  mac802_11-default.tcl  mac802_11-proc.tcl

InitTcl.cc: Makefile $(TCL_FILES)
		cat $(TCL_FILES) | @TCL2CPP@ MiracleMac80211InitTclCode > InitTcl.cc

EXTRA_DIST = $(TCL_FILES)

