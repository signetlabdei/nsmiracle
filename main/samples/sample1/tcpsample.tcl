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


set ns [new Simulator]


load ../../.libs/libMiracle.so

$ns use-Miracle

#load ../../mobileNode/
load ../../trace/.libs/libTrace.so
load ../../cltrace/.libs/libClTrace.so
load ../../sandbox/.libs/libMiracleSandbox.so
load ../../tcp/.libs/libmiracletcp.so



proc finish {} {
	global ns tf 
	puts "---> SIMULATION DONE."
	$ns flush-trace
	close $tf
}

Module/SendModule set period_ 0.1
Module/SendModule set destPort_ 0
Module/SendModule set destAddr_ 0
ChannelTest set debug_ 0
Module/SendModule set debug_ 0
Module/MiddleModule set debug_ 0
Module/ReceiveModule set debug_ 0

ConnectorTrace/ChSAP set debug_ 0
ConnectorTrace/Bin set debug_ 0

ChannelTest set delay_ 0.1

set tf [open out.tr w]
$ns trace-all $tf

set channel [new ChannelTest]

puts "---> CREATING NODE0"

set node1 [$ns create-M_Node]

set tm1 [new Module/TCP/Tahoe]
$node1 addModule 2 $tm1 0

set mm1 [new Module/MiddleModule]
$node1 addModule 1 $mm1 0

$node1 setConnection $tm1 $mm1 1
$node1 addToChannel $channel $mm1 1
set ftp [new Application/FTP]
$ftp attach-agent [$tm1 getTcpAgent]




puts "---> CREATING NODE1"

set node2 [$ns create-M_Node]
set tm2 [new Module/TCPSink]
$node2 addModule 2 $tm2 0
set mm2 [new Module/MiddleModule]
$node2 addModule 1 $mm2 0
$node2 setConnection $tm2 $mm2 1
$node2 addToChannel $channel $mm2 1



puts "---> BEGIN SIMULATION"

$ns at 0 "$ftp start"
$ns at 50 "$ftp stop"
$ns at 51 "finish; $ns halt"
$ns run
