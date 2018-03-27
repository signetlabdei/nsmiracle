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


lib_LTLIBRARIES = libmphy.la libmphytracer.la libsinrtracer.la libmphymaccltracer.la
check_PROGRAMS = 

TESTS = 

libmphy_la_SOURCES = initlib.cc \
		interference_miv.cc\
		interference_miv.h\
		mantenna.h\
		mcorrelation.h \
		mfreespace.cc\
		mfreespace.h\
		minterference.h\
		mphy.cc\
		mphy.h\
		mphy_pktheader.h \
		mphy_timers.h\
		mphy-sense.h\
		mphy-sense.cc\
		mpropagation.h\
		mspectralmask.h\
		rect_spectral_mask.cc\
		rect_spectral_mask.h \
		bpsk.cc bpsk.h\
		mfullpropagation.h\
		mfullpropagation.cc \
		phymac-clmsg.cc phymac-clmsg.h \
		channel-manager.cc channel-manager.h \
		clmsg-phy-on-off-switch.h clmsg-phy-on-off-switch.cc 


libmphy_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libmphy_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libmphy_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@

nodist_libmphy_la_SOURCES = MPhyInitTcl.cc



libmphytracer_la_SOURCES = mphy-tracer.cc

libmphytracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libmphytracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ -L./
libmphytracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lmphy


libsinrtracer_la_SOURCES = sinr-tracer.cc

libsinrtracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libsinrtracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@  -L./
libsinrtracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lmphy


libmphymaccltracer_la_SOURCES = phymac-clmsg-tracer.h phymac-clmsg-tracer.cc

libmphymaccltracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libmphymaccltracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@  -L./
libmphymaccltracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lmphy


BUILT_SOURCES = MPhyInitTcl.cc

CLEANFILES = MPhyInitTcl.cc

TCL_FILES = defaults.tcl

MPhyInitTcl.cc: Makefile $(TCL_FILES)
		cat $(TCL_FILES) | @TCL2CPP@ MPhyInitTclCode > MPhyInitTcl.cc


EXTRA_DIST = $(TCL_FILES) 
