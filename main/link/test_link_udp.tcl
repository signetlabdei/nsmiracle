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

# This sample demonstrate how Module/Link can be used.
# Two nodes are created, with only one layer and one module 
# per layer. A Module/Link connects the two nodes.
# SendModule of node 1 has to send periodically a packet which 
# is received from the ReceiveModule of node 2

load ../.libs/libMiracle.so.0.0.0
load ../trace/.libs/libTrace.so.0.0.0
load ../sandbox/.libs/libMiracleSandbox.so.0.0.0
load .libs/libmiraclelink.so

proc finish {} {
	global ns tf 
	puts "---> SIMULATION DONE."
	$ns flush-trace
	close $tf
}


Module/SendModule set period_ 0.1
Module/SendModule set destPort_ 0
Module/SendModule set destAddr_ 0
Module/Link set debug_ 0
Module/SendModule set debug_ 0
Module/ReceiveModule set debug_ 0
ConnectorTrace/ChSAP set debug_ 1
ConnectorTrace/Bin set debug_ 0


set ns [new Simulator]
$ns use-Miracle


set tf [open out_test_link_udp.tr w]
$ns trace-all $tf


set node1 [$ns create-M_Node] 
set sm [new Module/SendModule] 
$node1 addModule 1 $sm 0 "SEND" 


set node2 [$ns create-M_Node]
set rm [new Module/ReceiveModule]
$node2 addModule 1 $rm 0 "RECV"


set link [new Module/Link]
$link delay      0.01
$link bandwidth  1000000
$link qsize      5
$link connect $node1 $sm 1 $node2 $rm 1 
#$link print-params 


$ns at 0 "$sm start"
$ns at 1 "$sm stop"
$ns at 2 "finish; $ns halt"
$ns run
