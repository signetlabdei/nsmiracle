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
    echo "usage: $0 tracefile"    
}

if test $# -ne 1 ; then
    usage
    exit 1
fi
    

if test ! -f $1 ; then
    echo "$1: not a valid filename"
    exit 1
fi

datafname=/tmp/propagation_data

sed -n "s/^[r] [0-9]*\.[0-9]\{9\}.\([0-9]\)\ LL\ IP.*TX:\(-*[0-9]*.[0-9]*\)dBm\ RX:\(-*[0-9]*.[0-9]*\)dBm\ d:\([0-9]*.[0-9]*\).*$/\1 \2 \3 \4/p" $1 >  $datafname

echo " set xlabel \"distance (m)\"; set ylabel \"attenuation (dB)\";\
       plot \"$datafname\" using (\$4):(\$2-\$3) with points title \"tracefile ($1)\", \
       \"$datafname\" using (\$4):(10*log10(((4.0*pi*\$4)/(300.0/2437.0))**2.0)) with lines title \"freespace at 2.4GHz\", \
       \"$datafname\" using (\$4):(10*log10(((4.0*pi*\$4)/(300.0/2437.0))**4.0)) with lines title \"tworayground at 2.4GHz\"" \
    | gnuplot -persist

