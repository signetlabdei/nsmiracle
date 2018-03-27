#
# Copyright (c) 2008 Regents of the SIGNET lab, University of Padova.
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




AC_DEFUN([AC_ARG_WITH_STATICMULTIHOPROUTING],[

STATICMULTIHOPROUTING_PATH=''
STATICMULTIHOPROUTING_CPPLAGS=''
STATICMULTIHOPROUTING_LDFLAGS=''
STATICMULTIHOPROUTING_LIBADD=''

AC_ARG_WITH([staticmultihoprouting],
	[AS_HELP_STRING([--with-staticmultihoprouting=<directory>],
			[use staticmultihoprouting installation in <directory>])],
	[
		if test "x$withval" != "xno" ; then

   		     if test -d $withval ; then

   			STATICMULTIHOPROUTING_PATH="${withval}"

			relevantheaderfile="${STATICMULTIHOPROUTING_PATH}/src/static-multihoop-routing.h"
	    		if test ! -f "${relevantheaderfile}"  ; then
			  	AC_MSG_ERROR([could not find ${relevantheaderfile}, 
  is --with-staticmultihoprouting=${withval} correct?])
			fi		


			for dir in  \ 
				src
			do

			STATICMULTIHOPROUTING_CPPFLAGS="$STATICMULTIHOPROUTING_CPPFLAGS -I${STATICMULTIHOPROUTING_PATH}/${dir}"
			STATICMULTIHOPROUTING_LDFLAGS="$STATICMULTIHOPROUTING_LDFLAGS -L${STATICMULTIHOPROUTING_PATH}/${dir}"

			done

			for lib in \
				uwm
			do
				STATICMULTIHOPROUTING_LIBADD="$STATICMULTIHOPROUTING_LIBADD -l${lib}"
			done	



			STATICMULTIHOPROUTING_DISTCHECK_CONFIGURE_FLAGS="--with-staticmultihoprouting=$withval"
			AC_SUBST(STATICMULTIHOPROUTING_DISTCHECK_CONFIGURE_FLAGS)

   		     else	

			AC_MSG_ERROR([staticmultihoprouting path $withval is not a directory])
	
		     fi

		fi

	])


AC_SUBST(STATICMULTIHOPROUTING_CPPFLAGS)
AC_SUBST(STATICMULTIHOPROUTING_LDFLAGS)
AC_SUBST(STATICMULTIHOPROUTING_LIBADD)

])



AC_DEFUN([AC_CHECK_STATICMULTIHOPROUTING],[
	
# temporarily add NS_CPPFLAGS and STATICMULTIHOPROUTING_CPPFLAGS to CPPFLAGS
BACKUP_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $NS_CPPFLAGS $STATICMULTIHOPROUTING_CPPFLAGS"


AC_LANG_PUSH(C++)

AC_MSG_CHECKING([for staticmultihoprouting headers])


AC_PREPROC_IFELSE(
	[AC_LANG_PROGRAM([[
		#include<underwater.h>
		Underwater* uwp; 
		]],[[
		]]  )],
		[
		 AC_MSG_RESULT([yes])
		 found_staticmultihoprouting=yes
		[$1]
		],
		[
		 AC_MSG_RESULT([no])
		 found_staticmultihoprouting=no
		[$2]
		#AC_MSG_ERROR([could not find staticmultihoprouting])
		])


AM_CONDITIONAL([HAVE_STATICMULTIHOPROUTING], [test x$found_staticmultihoprouting = xyes])

# Restoring to the initial value
CPPFLAGS="$BACKUP_CPPFLAGS"

AC_LANG_POP(C++)

])











