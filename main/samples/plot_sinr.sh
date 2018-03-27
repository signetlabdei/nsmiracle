#!/bin/bash

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



set -o errexit

if ! which gnuplot > /dev/null 2>/dev/null ; then
    echo "to plot the results, please install gnuplot."
    exit 1
fi
    

killall -q gnuplot_x11 || true

usage () {
    echo "usage: $0 tracefile "    
}

if test $# -ne 1 ; then
    usage
    exit 1
fi
    

if test ! -f $1 ; then
    echo "$1: not a valid filename"
    exit 1
fi

datafname=/tmp/mphy_rxpower_data
tracefile=$1

# use this for PC in UPLINK
grep '^r .* 0 PHY  MAC1.*SINR'  $tracefile \
   | awk '$6~/SINR/{print $2,$6}' \
   | sed -e 's/SINR=\([^ ]*\)/\1/' \
   > $datafname

# use this for PC in DOWNLINK
# grep '^r .* 1 PHY  MAC .*SINR'  $tracefile \
#     | awk '$6~/SINR/{print $2,$6}' \
#     | sed -e 's/SINR=\([^ ]*\)/\1/' \
#     > $datafname



# spreading factor
#sf=16
# inter-user correlation
#iuc=0.2
#alpha factor = 1/(1+alpha)
#af=0.82117

gstr=" set logscale y;  set grid xtics mytics ; set mytics;  set xlabel \"time (s)\"; set ylabel \"SINR\";\
       plot [0.2:0.3][*:*] \"$datafname\" using (\$1):(\$2) with linespoints title \"tracefile ($tracefile)\", \
       (2.8) with lines title \"SIR target\" "


echo $gstr  | gnuplot -persist
echo "set term png; set output \"powerctrl.png\"; $gstr "  | gnuplot -persist


