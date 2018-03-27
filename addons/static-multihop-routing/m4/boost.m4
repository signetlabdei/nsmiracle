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





AC_DEFUN([AC_CHECK_BOOST_GRAPH],[


AC_LANG_PUSH(C++)

AC_MSG_CHECKING([for boost graph library])


AC_PREPROC_IFELSE(
	[AC_LANG_PROGRAM([[
		#include <boost/utility.hpp>
		#include <boost/graph/adjacency_list.hpp>
		#include <boost/graph/topological_sort.hpp>
		#include <boost/graph/depth_first_search.hpp>
		#include <boost/graph/visitors.hpp>
		#include <boost/graph/max_cardinality_matching.hpp>
		#include <boost/graph/edge_list.hpp>
		#include <boost/graph/graphviz.hpp>
		adjacency_list<listS, vecS, bidirectionalS> Graph;
		]],[[
		]]  )],
		[
		 AC_MSG_RESULT([yes])
		 found_boostgraph=yes
		[$1]
		],
		[
		 AC_MSG_RESULT([no])
		 found_boostgraph=no
		[$2]
		])


AM_CONDITIONAL([HAVE_BOOST_GRAPH], [test x$found_boostgraph = xyes])


AC_LANG_POP(C++)

])


















# AC_DEFUN([AC_PATH_NSMIRACLE], [

# AC_REQUIRE(AC_PATH_NS_ALLINONE)

# ########################################################
# # checking if ns-allinone path has been setup correctly
# ########################################################

# # temporarily add NS_CPPFLAGS and NSMIRACLE_CPPFLAGS to CPPFLAGS
# BACKUP_CPPFLAGS=$CPPFLAGS
# CPPFLAGS="$CPPFLAGS $NS_CPPFLAGS NSMIRACLE_CPPFLAGS"

# AC_MSG_CHECKING([if programs can be compiled against ns-miracle headers])
# AC_PREPROC_IFELSE(
# 	[AC_LANG_PROGRAM([[
# 		#include<cltracer.h>
# 		ClMessageTracer* t; 
# 		]],[[
# 		]]  )],
#         [AC_MSG_RESULT([yes])],
#         [
# 	  AC_MSG_RESULT([no])
# 	  AC_MSG_ERROR([could not compile a test program against ns-miracle headers. Is --with-ns-miracle set correctly? ])
#         ])

# # AC_CHECK_HEADERS([cltracer.h],,AC_MSG_ERROR([you must specify ns-miracle installation path using --with-ns-miracle=PATH]))

# # Restoring to the initial value
# CPPFLAGS=$BACKUP_CPPFLAGS
# ])
