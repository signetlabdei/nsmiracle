# WOSS - World Ocean Simulation System -
# 
# Copyright (C) 2009 Regents of Patavina Technologies 
# 
# Author: Federico Guerra - federico@guerra-tlc.com
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANATBILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses>/.

# This software has been developed by Patavina Technologies, s.r.l., 
# in collaboration with the NATO Undersea Research Centre 
# (http://www.nurc.nato.int; E-mail: pao@nurc.nato.int), 
# whose support is gratefully acknowledged.

AM_CFLAGS = -Wall -g

lib_LTLIBRARIES = libUwmStd.la libUwmStdPhyBpskTracer.la

libUwmStd_la_SOURCES =  underwater.cc underwater.h \
			underwater-channel.cc underwater-channel.h \
			underwater-mpropagation.cc underwater-mpropagation.h \
			underwater-bpsk.cc underwater-bpsk.h \
			underwater-shannon.cc underwater-shannon.h \
			uwlib.h uwlib.c \
			initlib.cc


libUwmStd_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libUwmStd_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libUwmStd_la_LDFLAGS =  @NS_LDFLAGS@  @NSMIRACLE_LDFLAGS@
libUwmStd_la_LIBADD =   @NS_LIBADD@   @NSMIRACLE_LIBADD@


libUwmStdPhyBpskTracer_la_SOURCES =  uwmphybpsk-tracer.cc

libUwmStdPhyBpskTracer_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libUwmStdPhyBpskTracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@ 
libUwmStdPhyBpskTracer_la_LDFLAGS =  @NS_LDFLAGS@  @NSMIRACLE_LDFLAGS@ -L./
libUwmStdPhyBpskTracer_la_LIBADD =   @NS_LIBADD@   @NSMIRACLE_LIBADD@ -lUwmStd


nodist_libUwmStd_la_SOURCES = uw-embeddedtcl.cc
BUILT_SOURCES = uw-embeddedtcl.cc
CLEANFILES = uw-embeddedtcl.cc

TCL_FILES =  underwater-init.tcl


uw-embeddedtcl.cc: Makefile $(TCL_FILES)
		   cat $(TCL_FILES) | @TCL2CPP@ UwStdTclCode > uw-embeddedtcl.cc


EXTRA_DIST = underwater.cc underwater.h \
	     underwater-channel.cc underwater-channel.h \
	     underwater-mpropagation.cc underwater-mpropagation.h \
	     underwater-bpsk.cc underwater-bpsk.h \
	     underwater-shannon.cc underwater-shannon.h \
	     uwlib.h uwlib.c \
	     initlib.cc \
	     $(TCL_FILES)
