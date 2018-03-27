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

#load ./nsmiracle/.libs/libMiracle.so.0.0.0
load ./nsmiracle/.libs/libMiracle.so.0.0.0
load ./trace/.libs/libTrace.so.0.0.0
load ./cltrace/.libs/libClTrace.so.0.0.0
load ./sandbox/.libs/libMiracleSandbox.so.0.0.0
load ./mobility/.libs/libMiracleBasicMovement.so.0.0.0

load ./wirelessch/.libs/libMiracleWirelessCh.so.0.0.0
load ./phy802_11/.libs/libMiraclePhy802_11.so.0.0.0
load ./mac802_11/.libs/libMiracleMac802_11.so.0.0.0
load ./ip/.libs/libMiracleIp.so.0.0.0
load ./port/.libs/libmiracleport.so.0.0.0
#load ./tcp/.libs/libMiracleTcp.so.0.0.0


proc finish {} {
	global ns tf 
	puts "---> SIMULATION DONE."
	$ns flush-trace
	close $tf
}

proc createNode {application titolo} {
#	-----------------
#	| 5. Application|
#	-----------------
#	| 4. Transport	|
#	-----------------
#	| 	3. IP	|
#	-----------------
#	| 	2. LL	|
#	-----------------
#	| 	1. PHY	|
#	-----------------
#
	global ns channel pmodel
	global ip port
	
	set node [$ns create-M_Node]
	set transport [new Module/Port]
	set ip [createIPModule]
	set LL [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Miracle" [$ip addr] "" 100 ]
	set PHY [createPhyModule "Phy/WirelessPhy/Miracle" $pmodel "Antenna/OmniAntenna" $LL ""]
	
	set position [new "Position/BM"]
	
	$node addPosition $position

	set a_id [$node addModule 5 $application 3 "APP"]
	set t_id [$node addModule 4 $transport 3 "TRANS"]
	set ip_id [$node addModule 3 $ip 3 "IP"]
	set ll_id [$node addModule 2 $LL 3 "LL"]
	set phy_id [$node addModule 1 $PHY 3 "PHY"]
	$node setConnection $application $transport 1
	$node setConnection $transport $ip 1
	$node setConnection $ip $LL 1
	$node setConnection $LL $PHY 1
	$node addToChannel $channel $PHY 1
	
	set port [$transport assignPort $application]
	puts "$titolo"
	puts "Application id [$application id_]"
	puts "Transport id [$transport id_]"
	puts "IP id [$ip id_]"
	puts "LL id [$LL id_]"
	puts "PHY id [$PHY id_]"
	puts ""
	set ip [$ip addr]
	return $node
}

Module/SendModule set packetSize_ 160
Module/SendModule set period_ 0.1
Module/SendModule set debug_ 0
Module/SendModule set destPort_ 0
Module/SendModule set destAddr_ 0
Module/ReceiveModule set debug_ 0

ConnectorTrace/ChSAP set debug_ 0
ConnectorTrace/Bin set debug_ 0

ConnectorTrace/Bin set depth 5

Mac/802_11 set RTSThreshold_ 2000

set ns [new Simulator]
$ns use-Miracle

set tf [open out.tr w]
$ns trace-all $tf

set pmodel [new Propagation/MrclTwoRayGround]
set channel [createWirelessChModule "" "" ""]


create-god 2

puts "---> CREATING NODE0"

set sender [new Module/SendModule]
set node1 [createNode $sender "Node Sender"]
set position [$node1 position]
$position setX_ 0.0
$position setY_ 0.0

puts "---> CREATING NODE1"

set receiver [new Module/ReceiveModule]
set node2 [createNode $receiver "Node Receiver"]
set position [$node2 position]
$position setX_ 10.0
$position setY_ 0.0
$sender set destPort_ $port
$sender set destAddr_ $ip

puts "---> BEGIN SIMULATION"

$ns at 0 "$sender start"
$ns at 1 "$position setdest 20 0 1"
$ns at 20 "$sender stop"
$ns at 21 "finish; $ns halt"
$ns run
