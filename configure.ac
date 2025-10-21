#
# Copyright (c) 2007 Regents of the SIGNET lab, University of Padova.
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

AC_INIT(nsmiracle,2.0-beta)
AM_INIT_AUTOMAKE([-Wall -Werror])
AM_PROG_AR
AC_PROG_CXX
AC_PROG_MAKE_SET


AC_DISABLE_STATIC 

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

AC_PATH_NS_ALLINONE

AC_CONFIG_MACRO_DIR([m4])

NSMIRACLE_CXXFLAGS='-Wall -Werror -Wno-overloaded-virtual'
AC_SUBST(NSMIRACLE_CXXFLAGS)

NSMIRACLE_CPPFLAGS='-I$(top_srcdir)/nsmiracle'
NSMIRACLE_CPPFLAGS="$NSMIRACLE_CPPFLAGS "'-I$(top_srcdir)/ip'
NSMIRACLE_CPPFLAGS="$NSMIRACLE_CPPFLAGS "'-I$(top_srcdir)/mphy'
NSMIRACLE_CPPFLAGS="$NSMIRACLE_CPPFLAGS "'-I$(top_srcdir)/mmac'
NSMIRACLE_CPPFLAGS="$NSMIRACLE_CPPFLAGS "'-I$(top_srcdir)/mobility'
AC_SUBST(NSMIRACLE_CPPFLAGS)

NSMIRACLE_LDFLAGS='-L$(top_srcdir)/nsmiracle'
#NSMIRACLE_LDFLAGS="$NSMIRACLE_CPPFLAGS "'-L$(top_srcdir)/ip'
#NSMIRACLE_LDFLAGS="$NSMIRACLE_CPPFLAGS "'-L$(top_srcdir)/mphy'
AC_SUBST(NSMIRACLE_LDFLAGS)


NSMIRACLE_LIBADD='-lMiracle'
#NSMIRACLE_LIBADD="$NSMIRACLE_CPPFLAGS "'-l'
#NSMIRACLE_LIBADD="$NSMIRACLE_CPPFLAGS "'-L$(top_srcdir)/mphy'
AC_SUBST(NSMIRACLE_LIBADD)


dnl Activate all compile warnings & debug symbols
CFLAGS="${CFLAGS=} -Wall -g -Wno-write-strings"


AC_DEFINE(CPP_NAMESPACE,std)


AC_CONFIG_FILES([
		Makefile
		nsmiracle/Makefile
		cbr/Makefile
		ip/Makefile
		mobility/Makefile
		mphy/Makefile
		mmac/Makefile	
		uwm/Makefile
		m4/Makefile
		])

AC_OUTPUT
