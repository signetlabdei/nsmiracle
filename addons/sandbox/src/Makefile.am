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



lib_LTLIBRARIES = libMiracleSandbox.la libmypkttracer.la



libMiracleSandbox_la_SOURCES = module-send.cc  module-send.h \
			module-receive.cc  module-receive.h \
			channel-test.cc channel-test.h \
			module-middle.cc module-middle.h \
			sandbox-plugin1.cc sandbox-plugin1.h \
			sandbox-clmsg.cc sandbox-clmsg.h\
			sandbox-clmsg-tracer.cc sandbox-clmsg-tracer.h\
			mypacket.h mypacket.cc \
			initlib.cc

libMiracleSandbox_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libMiracleSandbox_la_LDFLAGS = @NS_LDFLAGS@  @NSMIRACLE_LDFLAGS@
libMiracleSandbox_la_LIBADD = @NS_LIBADD@  @NSMIRACLE_LIBADD@

nodist_libMiracleSandbox_la_SOURCES = embeddedtcl.cc
BUILT_SOURCES = embeddedtcl.cc
CLEANFILES = embeddedtcl.cc

TCL_FILES =  sandbox-defaults.tcl

embeddedtcl.cc: Makefile $(TCL_FILES)
		cat $(TCL_FILES) | @TCL2CPP@ SandboxTclCode > embeddedtcl.cc

EXTRA_DIST = $(TCL_FILES)


libmypkttracer_la_SOURCES = mypacket-tracer.cc 

libmypkttracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libmypkttracer_la_LDFLAGS = @NS_LDFLAGS@  @NSMIRACLE_LDFLAGS@
libmypkttracer_la_LIBADD = @NS_LIBADD@  @NSMIRACLE_LIBADD@

