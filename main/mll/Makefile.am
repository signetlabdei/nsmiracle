# $Id: Makefile.am 93 2008-03-25 19:51:13Z emil $

SUBDIRS = 

lib_LTLIBRARIES = libmll.la libmlltracer.la
check_PROGRAMS = 

TESTS =

libmll_la_SOURCES = mll-module.cc mll-module.h \
		mll-clmsg.h mll-clmsg.cc \
		marptable.cc marptable.h \
		initlib.cc

libmll_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libmll_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@ 
libmll_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ 
libmll_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ 

nodist_libmll_la_SOURCES = MllInitTcl.cc

libmlltracer_la_SOURCES = mll-tracer.cc mll-tracer.h

libmlltracer_la_CXXFLAGS = @NSMIRACLE_CXXFLAGS@
libmlltracer_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@
libmlltracer_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ -L./
libmlltracer_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ -lmll

BUILT_SOURCES = MllInitTcl.cc

CLEANFILES = MllInitTcl.cc

TCL_FILES = mll-default.tcl

MllInitTcl.cc: Makefile $(TCL_FILES)
		cat $(TCL_FILES) | @TCL2CPP@ MllInitTclCode > MllInitTcl.cc

EXTRA_DIST = $(TCL_FILES)
