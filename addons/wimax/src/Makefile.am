#
# Copyright (c) 2008 Regents of the 
# Centre Tecnològic de Telecomunicacions de Catalunya (CTTC)
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
# 3. Neither the name of the CTTC nor the names of its contributors may 
#    be used to endorse or promote products derived from this software 
#    without specific prior written permission.
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

lib_LTLIBRARIES = libwimax.la libwimaxtracer.la

libwimax_la_SOURCES =  ofdm-phy.cc ofdm-phy.h \
		mac802_16.cc mac802_16.h \
		mac802_16SS.cc mac802_16SS.h \
		mac802_16BS.cc mac802_16BS.h \
		mac802_16pkt.cc mac802_16pkt.h \
		wimax-clmsg.cc wimax-clmsg.h \
		ofdm-packet.cc ofdm-packet.h \
		initlib.cc \
		ns2wimax/destclassifier.cc \
		ns2wimax/connection.cc \
		ns2wimax/wimaxneighborentry.cc \
		ns2wimax/scheduling/framemap.cc \
		ns2wimax/scheduling/bsscheduler.h \
		ns2wimax/scheduling/dlburst.cc \
		ns2wimax/scheduling/wimaxscheduler.cc \
		ns2wimax/scheduling/ulsubframetimer.cc \
		ns2wimax/scheduling/burst.cc \
		ns2wimax/scheduling/ulburst.h \
		ns2wimax/scheduling/ssscheduler.h \
		ns2wimax/scheduling/contentiontimer.cc \
		ns2wimax/scheduling/profile.h \
		ns2wimax/scheduling/ulburst.cc \
		ns2wimax/scheduling/phypdu.h \
		ns2wimax/scheduling/dlburst.h \
		ns2wimax/scheduling/contentionrequest.cc \
		ns2wimax/scheduling/profile.cc \
		ns2wimax/scheduling/wimaxctrlagent.h \
		ns2wimax/scheduling/contentiontimer.h \
		ns2wimax/scheduling/wimaxctrlagent.cc \
		ns2wimax/scheduling/framemap.h \
		ns2wimax/scheduling/subframe.h \
		ns2wimax/scheduling/bsscheduler.cc \
		ns2wimax/scheduling/ulsubframetimer.h \
		ns2wimax/scheduling/contentionslot.cc \
		ns2wimax/scheduling/subframe.cc \
		ns2wimax/scheduling/scanningstation.cc \
		ns2wimax/scheduling/phypdu.cc \
		ns2wimax/scheduling/burst.h \
		ns2wimax/scheduling/dlsubframetimer.h \
		ns2wimax/scheduling/wimaxscheduler.h \
		ns2wimax/scheduling/scanningstation.h \
		ns2wimax/scheduling/contentionrequest.h \
		ns2wimax/scheduling/dlsubframetimer.cc \
		ns2wimax/scheduling/contentionslot.h \
		ns2wimax/scheduling/ssscheduler.cc \
		ns2wimax/serviceflowhandler.cc \
		ns2wimax/neighbordb.cc \
		ns2wimax/wimaxneighborentry.h \
		ns2wimax/neighbordb.h \
		ns2wimax/serviceflow.cc \
		ns2wimax/serviceflowqos.h \
		ns2wimax/mac802_16timer.h \
		ns2wimax/sduclassifier.cc \
		ns2wimax/mac802_16timer.cc \
		ns2wimax/serviceflowhandler.h \
		ns2wimax/peernode.h \
		ns2wimax/peernode.cc \
		ns2wimax/serviceflow.h \
		ns2wimax/sduclassifier.h \
		ns2wimax/connectionmanager.h \
		ns2wimax/connection.h \
		ns2wimax/mac-stats.h \
		ns2wimax/serviceflowqos.cc \
		ns2wimax/destclassifier.h \
		ns2wimax/connectionmanager.cc


libwimax_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libwimax_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@
libwimax_la_LIBADD =   @NS_LIBADD@  @NSMIRACLE_LIBADD@

libwimaxtracer_la_SOURCES = wimax-tracer.cc wimax-tracer.h

libwimaxtracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libwimaxtracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ -L./
libwimaxtracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lwimax


nodist_libwimax_la_SOURCES = InitTcl.cc

BUILT_SOURCES = InitTcl.cc

CLEANFILES = InitTcl.cc

TCL_FILES = ofdm-packet.tcl ofdm-phy.tcl wimax-default.tcl

InitTcl.cc: Makefile $(TCL_FILES)
		cat $(TCL_FILES) | @TCL2CPP@ InitTclCode > InitTcl.cc

EXTRA_DIST = $(TCL_FILES)

