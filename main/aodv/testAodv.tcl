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

load libMiracle.so.0.0.0
load libTrace.so.0.0.0
load libClTrace.so.0.0.0
load libMiracleBasicMovement.so.0.0.0
load libmiraclelink.so
load libmiraclecbr.so
load libmiracleport.so
load libMiracleMac802_11.so.0.0.0
load libMiraclePhy802_11.so
load libdei80211mr.so.0.0.0
load libmultiratetracer.so.0.0.0
load libMiracleWirelessCh.so.0.0.0

load libMiracleRouting.so.0.0.0
load libMiracleAodv.so.0.0.0

set ns [new Simulator]

$ns use-Miracle

proc finish {} {
        global ns tf
        puts "done!"
        $ns flush-trace
        close $tf 
}       

set tf [open /tmp/AODVout.tr w]
$ns trace-all $tf

proc create-node {i} {
	global ns node_ position_ cbr_ ip_ tr_ port_ broadcastAddr_ ipCodecUp_ ipAddr_ ipCodecDown_
	global channel pmodel per PHY
	set node_($i) [$ns create-M_Node]
	set position_($i) [new "Position/BM"]
	$node_($i) addPosition $position_($i)
	$position_($i) setX_ 0.0
	$position_($i) setY_ 0.0

	set cbr_($i) [new Module/CBR]
	set ipAddr_($i) [new Address/IP "10.0.0.$i"]
	set ipCodecUp_($i) [new Module/MrclIpAddressCodec]
	$ipCodecUp_($i) up
	
	set ipCodecDown_($i) [new Module/MrclIpAddressCodec]
	$ipCodecDown_($i) down

	$cbr_($i) set period_ 0.01
	$cbr_($i) set size_ 1024
	set ip_($i) [new Routing/AODV]
	$ip_($i) set debug_ 5
#$ip_($i) AodvLinkLayerDetection
	puts "set addr [$ipAddr_($i) addr]"
	$ip_($i) setAddr $ipAddr_($i)
	$ip_($i) setBroadcastAddr $broadcastAddr_
	$ip_($i) AodvStandardNs2
#	if {$i == 3} {
#		$ip_($i) set debug_ 100
#	}
        #$ip_($i) metric $i.$i
	set tr_($i) [new Module/Port/Map]
	
# 802.11 Modules
	set LL($i) [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$ipCodecUp_($i) mrcl_addr2ns_addr_t $ipAddr_($i)] "" 100 ]
	set PHY($i) [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $LL($i) ""]
	
	set mac [$LL($i) getMac]
	set phy [$PHY($i) getPhy]
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

	$node_($i) addModule 5 $cbr_($i) 10 "APP"
	$node_($i) addModule 4 $tr_($i) 10 "UDP"
	$node_($i) addModule 3 $ipCodecUp_($i) 10 "IPcodecUP"
	$node_($i) addModule 3 $ip_($i) 10 "IP"
	$node_($i) addModule 3 $ipCodecDown_($i) 10 "IPcodecDW"
	$node_($i) addModule 2 $LL($i) 10 "LL"
	$node_($i) addModule 1 $PHY($i) 10 "PHY"
	

	$node_($i) setConnection $cbr_($i) $tr_($i) trace
	$node_($i) setConnection $tr_($i) $ipCodecUp_($i) trace
	$node_($i) setConnection $ipCodecUp_($i) $ip_($i) trace
	$node_($i) setConnection $ip_($i) $ipCodecDown_($i) trace
	$node_($i) setConnection $ipCodecDown_($i) $LL($i) trace
	$node_($i) setConnection $LL($i) $PHY($i) trace

	set port_($i) [$tr_($i) assignPort $cbr_($i)]
	puts "Node $i has id [$ip_($i) id_] IPaddr [$ip_($i) getAddr 1]"
	puts "Node $i has IP addr [$ip_($i) getAddr 1], old format [$ipCodecUp_($i) mrcl_addr2ns_addr_t $ipAddr_($i)]"
	set oldaddr [$ipCodecUp_($i) mrcl_addr2ns_addr_t $ipAddr_($i)]
	puts "old addr $oldaddr -> new addr [$ipCodecUp_($i) ns_addr_t2mrcl_addr $oldaddr]"
}

set channel [new Module/DumbWirelessCh]
set pmodel [new Propagation/MrclTwoRayGround]
# dei802.11mr settings
set per [new PER]
$per loadDefaultPERTable
$per set noise_ 7e-11
Mac/802_11 set RTSThreshold_ 10000
Mac/802_11 set ShortRetryLimit_ 9
Mac/802_11 set LongRetryLimit_ 8
Mac/802_11/Multirate set useShortPreamble_ true
Mac/802_11/Multirate set gSyncInterval_ 0.000005
Mac/802_11/Multirate set bSyncInterval_ 0.00001

Phy/WirelessPhy set Pt_ 0.01
Phy/WirelessPhy set freq_ 2437e6
Phy/WirelessPhy set L_ 1.0
Queue/DropTail/PriQueue set Prefer_Routing_Protocols    1
Queue/DropTail/PriQueue set size_ 1000
# end dei802.11mr settings

set broadcastAddr_ [new Address/IP "255.255.255.255"]

create-god 3

create-node 1
create-node 2
create-node 3

puts "broadcast Addr new [$broadcastAddr_ addr], old format [$ipCodecUp_(1) mrcl_addr2ns_addr_t $broadcastAddr_]"

#ConnectorTrace/ChSAP set debug_ 100
#Module/Link set debug_ 100
#set link(1) [new Module/DuplexLink]
#$link(1) delay      10m
#$link(1) bandwidth  2mb
#$link(1) qsize      5
#$link(1) connect $node_(1) $PHY(1) trace $node_(2) $PHY(2) trace

#set link(2) [new Module/DuplexLink]
#$link(2) delay      10m
#$link(2) bandwidth  2mb
#$link(2) qsize      5
#$link(2) connect $node_(2) $ipCodecDown_(2) notrace $node_(3) $ipCodecDown_(3) notrace

set channel [new Module/DumbWirelessCh]
$node_(1) addToChannel $channel $PHY(1) trace
$node_(2) addToChannel $channel $PHY(2) trace
$node_(3) addToChannel $channel $PHY(3) trace


$cbr_(1) set destAddr_ [$ipCodecUp_(3) mrcl_addr2ns_addr_t $ipAddr_(3)]
$cbr_(1) set destPort_ $port_(3)
$cbr_(1) set debug_ 1
$cbr_(3) set destAddr_ [$ipCodecUp_(1) mrcl_addr2ns_addr_t $ipAddr_(1)]
$cbr_(3) set destPort_ $port_(1)
$cbr_(3) set debug_ 1

$ip_(1) set debug_ 8
$ip_(2) set debug_ 8
$ip_(3) set debug_ 8

#$cbr_(2) set destAddr_ [$ipCodecUp_(1) mrcl_addr2ns_addr_t $ipAddr_(1)]
#$cbr_(2) set destPort_ $port_(1)
#$cbr_(2) set debug_ 1

$cbr_(1) set period_ 1.0

$position_(2) setX_ 50.0
$position_(2) setY_ 0.0
$position_(3) setX_ 120.0
$position_(3) setY_ 0.0
puts "Send Packet, start Simulation"
#$position_(2) set debug_ 51
#$ns at 0.0 "$ip_(1) start"
#$ns at 0.0 "$ip_(2) start"
#$ns at 0.0 "$ip_(3) start"
#$ns at 10.1 "$position_(2) setdest 10 10 10"
#$ns at 10.1 "$position_(3) setdest 0 0 2"
#$ns at 3.0 "$cbr_(1) sendPkt"
$ns at 3.0 "$cbr_(1) sendPkt"
#$ns at 4.1 "$cbr_(2) sendPkt"
#$ns at 10.1 "$cbr_(1) sendPkt"
#$ns at 4.0 "$cbr_(3) sendPkt"
#$ns at 4.5 "$cbr_(2) sendPkt"
#$ns at 1 "$cbr_(2) start"
#$ns at 4 "$cbr_(2) stop"
#$ns at 1 "$cbr_(3) stop"

$ns at 100 "finish; $ns halt"

$ns run
