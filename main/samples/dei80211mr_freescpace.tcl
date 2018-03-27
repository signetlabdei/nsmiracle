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

source dynlibutils.tcl

dynlibload Miracle ../nsmiracle/.libs/
dynlibload Trace ../trace/.libs/
dynlibload MiracleIp ../ip/.libs/
dynlibload miraclecbr ../cbr/.libs/
dynlibload MiracleBasicMovement ../mobility/.libs/
dynlibload MiracleWirelessCh ../wirelessch/.libs/
dynlibload MiraclePhy802_11 ../phy802_11/.libs/
dynlibload MiracleMac802_11 ../mac802_11/.libs/
dynlibload miracleport ../port/.libs/

dynlibload dei80211mr

dynlibload multiratetracer ../dei80211mr/.libs/
dynlibload phytracer ../phy802_11/.libs/


set tracefname "/tmp/dei80211mr_freespace.tr"

Phy/WirelessPhy/PowerAware set debug_ 0

proc finish {} {
    global ns tf tracefname datafname
    puts "---> SIMULATION DONE."
    $ns flush-trace
    close $tf
    puts "Tracefile: $tracefname"
#    exec killall -q gnuplot_x11 || true 
    exec ./plot_propagation_from_trace.sh $tracefname 
}

set macmr(0) 0

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
	global ns channel pmodel per macmr
	global ip port
	
	set node [$ns create-M_Node]
	set transport [new Module/Port/Map]
	set IP [createIPModule]
	set LL [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$IP addr] "" 100 ]
	set PHY [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $LL ""]


	set mac [$LL getMac]
	set phy [$PHY getPhy]
	$mac basicMode_ Mode6Mb
	$mac dataMode_ Mode6Mb
	$mac per $per
	set pp [new PowerProfile]
	$mac powerProfile $pp
	$phy powerProfile $pp


	set ra [new RateAdapter/ARF]
	$ra attach2mac $mac
	$ra use80211g
	$ra setmodeatindex 0
	
	
	set position [new "Position/BM"]
	
	$node addPosition $position

	set a_id [$node addModule 5 $application 3 "APP"]
	set t_id [$node addModule 4 $transport 3 "TRANS"]
	set ip_id [$node addModule 3 $IP 3 "IP"]
	set ll_id [$node addModule 2 $LL 3 "LL"]
	set phy_id [$node addModule 1 $PHY 3 "PHY"]
	$node setConnection $application $transport 1
	$node setConnection $transport $IP 1
	$node setConnection $IP $LL 1
	$node setConnection $LL $PHY 1
	$node addToChannel $channel $PHY 1
	
	set port [$transport assignPort $application]
	puts "$titolo"
	puts "Application id [$application id_]"
	puts "Transport id [$transport id_]"
	puts "IP id [$IP id_] address [$IP addr]"
	puts "LL id [$LL id_]"
	puts "PHY id [$PHY id_]"
	puts ""
	set ip [$IP addr]
	set macmr([$IP addr]) $mac
	return $node
}


set per [new PER]
$per loadDefaultPERTable
$per set noise_ 7e-11

Mac/802_11 set RTSThreshold_ 2000
Mac/802_11 set ShortRetryLimit_ 4
Mac/802_11 set LongRetryLimit_ 3
Mac/802_11/Multirate set useShortPreamble_ true
Mac/802_11/Multirate set gSyncInterval_ 0.000005
Mac/802_11/Multirate set bSyncInterval_ 0.00001

Phy/WirelessPhy set Pt_ 0.01
Phy/WirelessPhy set freq_ 2437e6
Phy/WirelessPhy set L_ 1.0
Queue/DropTail/PriQueue set Prefer_Routing_Protocols    1
Queue/DropTail/PriQueue set size_ 1000

ConnectorTrace/ChSAP set debug_ 0
ConnectorTrace/Bin set debug_ 0

ConnectorTrace/Bin set depth 5

Mac/802_11 set RTSThreshold_ 2000

set ns [new Simulator]
$ns use-Miracle

set tf [open $tracefname w]
$ns trace-all $tf

#set pmodel [new Propagation/MrclTwoRayGround]
set pmodel [new Propagation/MrclFreeSpace]

#set channel [createWirelessChModule "" "" ""]
set channel [new Module/DumbWirelessCh]


create-god 2

puts "---> CREATING NODE0"

set sender   [new Module/CBR]
set node1 [createNode $sender "Sender Node"]
set position1 [$node1 position]
$position1 setX_ 0.0
$position1 setY_ 0.0

puts "---> CREATING NODE1"

set receiver [new Module/CBR]
set node2 [createNode $receiver "Receiver Node"]
set position2 [$node2 position]
$position2 setX_ 0.0
$position2 setY_ 0.0

$sender set destPort_ $port
$sender set destAddr_ $ip

puts "Noise: [expr 10 * log10([$per set noise_]) + 30] dBm" 

puts "---> BEGIN SIMULATION"


#for {set i 1} {$i < 20} {incr i} {
#    $ns at $i "puts [$macmr(0) getSnr 1]"
#}

$ns at 0 "$sender start"
$ns at 1 "$position2 setdest 0 100 10"
$ns at 12 "$sender stop"
$ns at 13 "finish; $ns halt"
$ns run
