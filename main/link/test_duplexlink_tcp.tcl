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

# Link sample

# This sample demonstrate how Module/Duplex link can be used.
# Two nodes are created, with only one layer and one module 
# per layer. A Module/DuplexLink connects the two nodes.
# A TCP connection is established from node 1 to node 2

load ../nsmiracle/.libs/libMiracle.so.0.0.0
load ../trace/.libs/libTrace.so.0.0.0
load ../ip/.libs/libMiracleIp.so
load .libs/libmiraclelink.so
load ../tcp/.libs/libmiracletcp.so
load ../tcp/.libs/libtcptracer.so

proc finish {} {
	global ns tf tcpTrace
	puts "---> SIMULATION DONE."
	$ns flush-trace
	close $tf
        close $tcpTrace
}

Module/Link set debug_ 0


set ns [new Simulator]
$ns use-Miracle

set tf [open out_test_duplexlink_tcp.tr w]
$ns trace-all $tf


set n(1) [$ns create-M_Node]
set tm1 [new Module/TCP/Tahoe]
$n(1) addModule 1 $tm1 0 "TCPsource"
set ftp [new Application/FTP]
set tcpa [$tm1 getTcpAgent ]
$ftp attach-agent $tcpa


set n(2) [$ns create-M_Node]
set tm2 [new Module/TCPSink]
$n(2) addModule 1 $tm2 0 "TCPsink"



set dlink [new Module/DuplexLink]
$dlink delay      0.1
$dlink bandwidth  384000
$dlink qsize      10
$dlink settags "DLCH"
$dlink connect $n(1) $tm1 1 $n(2) $tm2  1

set tcpTrace [open cwnd.tr w]
$tcpa attach $tcpTrace
$tcpa trace cwnd_
$tcpa trace nrexmit_

$ns at 0 "$ftp start"
$ns at 6 "$ftp stop"
$ns at 7 "finish; $ns halt"
$ns run
