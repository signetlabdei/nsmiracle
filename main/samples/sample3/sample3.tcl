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

# ****** SAMPLE 3 *******

load ../../.libs/libMiracle.so.0.0.0
load ../../trace/.libs/libTrace.so.0.0.0
load ../../cltrace/.libs/libClTrace.so.0.0.0
load ../../sandbox/.libs/libMiracleSandbox.so.0.0.0

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

set ns [new Simulator]
$ns use-Miracle

set tf [open out.tr w]
$ns trace-all $tf

set channel [new ChannelTest]

puts "---> CREATING NODE0"

set node1 [$ns create-M_Node]
set sm1 [new Module/SendModule]
set mm1 [new Module/MiddleModule]
set mm2 [new Module/MiddleModule]
set mmm [new Module/MiddleModule]
set i [$node1 addModule 3 $sm1 3]
set k [$node1 addModule 2 $mm1 3]
set z [$node1 addModule 2 $mm2 3]
set j [$node1 addModule 1 $mmm 3]
$node1 setConnection $sm1 $mm1 1
$node1 setConnection $sm1 $mm2 1
$node1 setConnection $mm1 $mmm 1
$node1 setConnection $mm2 $mmm 1
$node1 addToChannel $channel $mmm 1

puts "---> CREATING NODE1"

set node2 [$ns create-M_Node]
set sm2 [new Module/ReceiveModule]
set mm3 [new Module/MiddleModule]
set i [$node2 addModule 2 $sm2 0]
set j [$node2 addModule 1 $mm3 0]
$node2 setConnection $sm2 $mm3 1
$node2 addToChannel $channel $mm3 1

puts "---> BEGIN SIMULATION"

$ns at 0 "$sm1 start"
$ns at 5 "$sm1 changeiface 1 [expr rand()]"
$ns at 10 "$sm1 changeiface 2 [expr rand()]"
$ns at 20 "$sm1 changeiface 3 [expr rand()]"
$ns at 30 "$sm1 changeiface 4 [expr rand()]"
$ns at 40 "$sm1 changeiface 5 [expr rand()]"
$ns at 50 "$sm1 stop"
$ns at 53 "finish; $ns halt"
$ns run
