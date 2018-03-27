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

load libMiracle.so
load libMiracleIp.so     
load libmiracleport.so
load libMiracleBasicMovement.so.0.0.0
load libTrace.so
load libClTrace.so
load libmiraclelink.so
load libmiraclecbr.so
load libMiracleRouting.so
load libMiracleAodv.so.0.0.0
load libcbrtracer.so

load libdei80211mr.so.0.0.0
load libmultiratetracer.so.0.0.0

load libMiracleMac802_11.so.0.0.0
load libMiraclePhy802_11.so
load libMiracleWirelessCh.so.0.0.0
#load ../ip/.libs/libMiracleIp.so.0.0.0
load libmiracleport.so.0.0.0
load libmphy.so
load libumts.so
load libumtstracer.so.0.0.0



set ns [new Simulator]

$ns use-Miracle
#$ns use-scheduler Heap
proc finish {} {
        global ns tf
        puts "done!"
        $ns flush-trace
        close $tf 
}       

set tf [open /tmp/out.tr w]
$ns trace-all $tf

proc createBS {nodeId} {
#	-----------------
#	| 7. Application|
#	-----------------
#	| 6. Transport	|
#	----------------
#	| 5. Port-Mux	|
#	-----------------
#	| 	4. IP	|
#	-----------------
#	| 	3. RLC	|
#	-----------------
#	| 	2. MAC	|
#	-----------------
#	| 	1. PHY	|
#	-----------------
#

	global ns channel pmodel per xFieldWidth yFieldWidth
	global ip port ipaddr LLAP apdsp mrclIpAddr_ cbrport cbrport1 cbr1_
	global node position cbr_ mux ip LL PHY ipaddr
	global umts_channel spectralmask_umts_downlink spectralmask_umts_uplink umts_propagation
	global me_phy bs_phy me_mac bs_mac me_rlc bs_rlc spreading_factor
	global availableAddress traceVer
	
	set mrclIpAddr_($nodeId) [new Address/IP "0.0.0.$availableAddress"]
	incr availableAddress

	set node($nodeId) [$ns create-M_Node]
	
	set cbr_($nodeId) [new Module/CBR]
	$cbr_($nodeId) set debug_ 10

###
	set cbr1_($nodeId) [new Module/CBR]
	$cbr1_($nodeId) set debug_ 10
###
	
	set spectralmask_umts_downlink($nodeId)  [new MSpectralMask/Rect]
	$spectralmask_umts_downlink($nodeId)  setFreq 2115000000
	$spectralmask_umts_downlink($nodeId)  setBandwidth 3840000
	set spectralmask_umts_uplink($nodeId)  [new MSpectralMask/Rect]
	$spectralmask_umts_uplink($nodeId)  setFreq   1925000000
	$spectralmask_umts_uplink($nodeId)  setBandwidth 3840000
	
#set umts_propagation($nodeId) [new MPropagation/FullPropagation]
#$umts_propagation($nodeId) set maxDopplerShift_ 6.0
#$umts_propagation($nodeId) set beta_ 3.4
#$umts_propagation($nodeId) set shadowSigma_ 0
#$umts_propagation($nodeId) set refDistance_ 1.0
#$umts_propagation($nodeId) set rayleighFading_ 1
#$umts_propagation($nodeId) set timeUnit_ -100
#$umts_propagation($nodeId) set xFieldWidth_ $xFieldWidth
#$umts_propagation($nodeId) set yFieldWidth_ $yFieldWidth
	
	set umts_propagation($nodeId)  [new MPropagation/MFreeSpace]
	
	set mux($nodeId) [new Module/Port/Map]
	set ip($nodeId) [new Module/IP]
	$ip($nodeId) set nodeId_ $nodeId
	
	set bs_phy($nodeId)     [new Module/MPhy/UMTS/BS]
	$node($nodeId) addModule 1 $bs_phy($nodeId)  0 "BS-PHY$nodeId"	
	$node($nodeId) addToChannel $umts_channel $bs_phy($nodeId)   0
	$umts_channel BSphymoduleId [$bs_phy($nodeId) id_]
#$bs_phy($nodeId) set debug_ 1
	$bs_phy($nodeId) setDownlinkSpectralMask $spectralmask_umts_downlink($nodeId)
	$bs_phy($nodeId) setUplinkSpectralMask   $spectralmask_umts_uplink($nodeId)
	$bs_phy($nodeId) setInterference         [new MInterference/MIV]
	$bs_phy($nodeId) setPropagation          $umts_propagation($nodeId)
	$bs_phy($nodeId) set nodeId_ $nodeId
#$bs_phy($nodeId) set spreading_factor_ $spreading_factor
	$bs_phy($nodeId) set spreading_factor_ 16
	
	set position($nodeId) [new "Position/BM"]
	
	$node($nodeId) addPosition $position($nodeId)
	
#$umts_propagation($nodeId) newNode $position($nodeId)

	$node($nodeId) addModule 6 $cbr_($nodeId) 10 "BS-CBR$nodeId"
#########
	$node($nodeId) addModule 6 $cbr1_($nodeId) 10 "BS-CBR$nodeId"
#########
	$node($nodeId) addModule 5 $mux($nodeId) 110 "BS-MUX$nodeId"
	$node($nodeId) addModule 4 $ip($nodeId) 10 "BS-IP$nodeId"
#$node($nodeId) addModule 2 $LL($nodeId) 10 "AP-LL$nodeId"
#$node($nodeId) addModule 1 $PHY($nodeId) 10 "AP-PHY$nodeId"
	
	
	$node($nodeId) setConnection $cbr_($nodeId) $mux($nodeId) 110
#########
	$node($nodeId) setConnection $cbr1_($nodeId) $mux($nodeId) 110
#########
	$node($nodeId) setConnection $mux($nodeId) $ip($nodeId) 1

	set cbrport($nodeId) [$mux($nodeId) assignPort $cbr_($nodeId)]
#########
	set cbrport1($nodeId) [$mux($nodeId) assignPort $cbr1_($nodeId)]
#########
	set ipaddr($nodeId) [$ip($nodeId) addr]
	puts "Created BS $nodeId:"
	puts "CBR id [$cbr_($nodeId) id_] appport $cbrport($nodeId) IPaddr $ipaddr($nodeId)"
#puts "Transport id [$transport id_]"
	puts "Port-Mux id [$mux($nodeId) id_]"
	puts "IP id [$ip($nodeId) id_]"
#puts "LL id [$LL($nodeId) id_]"
	puts "PHY id [$bs_phy($nodeId) id_]"
	puts ""
#set LLAP $LL($nodeId)
#puts "id_ ip $ip"

#return $node
}

proc createUser2Interfaces {nodeId BSid umtsdebug} {
#	----------------------------
#	|      7. Application       |
#	----------------------------
#	|      6. Transport         |
#	----------------------------
#	|      5. Port-Mux          |
#	----------------------------
#	|         4. IP             |
#	----------------------------
#	|  3. LL     |   3. RLC     |
#	---------------------------- 
#	|  2. PHY    |   2. MAC     |
#	----------------------------
#	             |   2. PHY     |
#	             ---------------
#
	global ns channel channel1 pmodel per
	global ip port ipaddr PHY LL udsp cbrport
	global node position cbr_ mux ip LL PHY LL1 PHY1 ipaddr
	global umts_channel spectralmask_umts_downlink spectralmask_umts_uplink 
	global me_phy bs_phy me_mac bs_mac me_rlc bs_rlc traceVer
	global umts_propagation packetSize xFieldWidth yFieldWidth speed s spreading_factor
	
	global broadcastAddr_ mrclIpAddr_ ipCodecUp_ ipCodecDown80211_ ipCodecDown802111_ ipCodecDownUMTS_ routing_
	
	global availableAddress traceVer
	
	set node($nodeId) [$ns create-M_Node]
	set cbr_($nodeId) [new Module/CBR]
	$cbr_($nodeId) set debug_ 110
	
#set transport [new "$transprot"]
#$transport setSTCPmanager $transmodule
#set dsp [new $application]
	set mux($nodeId) [new Module/Port/Map]
	set ipCodecUp_($nodeId) [new Module/MrclIpAddressCodec]
	$ipCodecUp_($nodeId) up
#		set ipCodecUp1_($nodeId) [new Module/MrclIpAddressCodec]
#		$ipCodecUp1_($nodeId) up
	set ipCodecDown80211_($nodeId) [new Module/MrclIpAddressCodec]
	$ipCodecDown80211_($nodeId) down
#####################
	set ipCodecDown802111_($nodeId) [new Module/MrclIpAddressCodec]
	$ipCodecDown802111_($nodeId) down
#####################
	set ipCodecDownUMTS_($nodeId) [new Module/MrclIpAddressCodec]
	$ipCodecDownUMTS_($nodeId) down
	
	set staticRoot_($nodeId) [new Routing/MrclRoutingStatic]
	$staticRoot_($nodeId) set debug_ 110
	set staticRootIpAddr_($nodeId) [new Address/IP "0.0.0.$availableAddress"]
	puts "StaticRootIpAddr_($nodeId) 0.0.0.$availableAddress"
	incr availableAddress
	$staticRoot_($nodeId) setAddr $staticRootIpAddr_($nodeId)
#$staticRoot_($nodeId) setBroadcastAddr $broadcastAddr_


###############################
#	set staticRoot1_($nodeId) [new Routing/MrclRoutingStatic]
#	$staticRoot1_($nodeId) set debug_ 110
#	set staticRootIpAddr1_($nodeId) [new Address/IP "0.0.0.$availableAddress"]
#	incr availableAddress
#	$staticRoot1_($nodeId) setAddr $staticRootIpAddr1_($nodeId)
#$staticRoot_($nodeId) setBroadcastAddr $broadcastAddr_
###############################

	
	set aodv_($nodeId) [new Routing/AODV]
	$aodv_($nodeId) set debug_ 115
	set aodvIpAddr_($nodeId) [new Address/IP "0.0.0.$availableAddress"]
	puts "aodvIpAddr_($nodeId) 0.0.0.$availableAddress"
	incr availableAddress
	$aodv_($nodeId) setAddr $aodvIpAddr_($nodeId)
	$aodv_($nodeId) setBroadcastAddr $broadcastAddr_


###########################################

	set aodv1_($nodeId) [new Routing/MrclRoutingStatic]
	$aodv1_($nodeId) set debug_ 115
	set aodvIpAddr1_($nodeId) [new Address/IP "1.0.0.$availableAddress"]
	puts "aodvIpAddr1_($nodeId) 1.0.0.$availableAddress"
	incr availableAddress
	$aodv1_($nodeId) setAddr $aodvIpAddr1_($nodeId)
	#$aodv1_($nodeId) setBroadcastAddr $broadcastAddr_



#	set aodv1_($nodeId) [new Routing/MrclRoutingStatic]
#	$aodv1_($nodeId) set debug_ 115
#	set aodvIpAddr1_($nodeId) [new Address/IP "1.1.1.$availableAddress"]
#	#incr availableAddress
#	$aodv1_($nodeId) setAddr $aodvIpAddr1_($nodeId)
#	#$aodv1_($nodeId) setBroadcastAddr $broadcastAddr_
###########################################
	
	set staticUmts_($nodeId) [new Routing/MrclRoutingStatic]
	$staticUmts_($nodeId) set debug_ 110
	set ip($nodeId) $nodeId
	set staticUmtsIpAddr_($nodeId) [new Address/IP "0.0.0.$availableAddress"]
	puts "staticUmtsIpAddr_($nodeId) 0.0.0.$availableAddress"
	incr availableAddress
	$staticUmts_($nodeId) setAddr $staticUmtsIpAddr_($nodeId)
#$staticUmts_($nodeId) setBroadcastAddr $broadcastAddr_
#$routing_($nodeId) AodvLinkLayerDetection
	
#set ip($nodeId) [new Module/IP]
	
# 802.11 Modules
	set LL($nodeId) [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $aodvIpAddr_($nodeId)] "" 100 ]
	set PHY($nodeId) [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $LL($nodeId) ""]
#set LL($nodeId) [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Miracle" [$ip($nodeId) addr] "" 100]
#set PHY($nodeId) [createPhyModule "Phy/WirelessPhy/Miracle" $pmodel "Antenna/OmniAntenna" $LL($nodeId) ""]
	
	$LL($nodeId) set nodeId_ $nodeId
	
# new settings (for dei802.11mr)
	set mac [$LL($nodeId) getMac]
	set phy [$PHY($nodeId) getPhy]
	$mac basicMode_ Mode6Mb
	$mac dataMode_ Mode6Mb
	$mac per $per
	set pp [new PowerProfile]
	$mac powerProfile $pp
	$phy powerProfile $pp

#set nse [new NSE/ARF]
#$nse Max_Mode Mode54Mb
#$nse Min_Mode Mode6Mb
#$nse mac $mac
#$mac nse $nse
	set ra [new RateAdapter/ARF]
	$ra attach2mac $mac
	$ra use80211g
	$ra setmodeatindex 0
# end new settings

##############################################################

# 802.11 Modules
	set LL1($nodeId) [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $aodvIpAddr1_($nodeId)] "" 100 ]
	set PHY1($nodeId) [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $LL1($nodeId) ""]
#set LL($nodeId) [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Miracle" [$ip($nodeId) addr] "" 100]
#set PHY($nodeId) [createPhyModule "Phy/WirelessPhy/Miracle" $pmodel "Antenna/OmniAntenna" $LL($nodeId) ""]
	puts "-ATTENZIONE- CHIAMO LL con [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $aodvIpAddr_($nodeId)] e LL1 con [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $aodvIpAddr1_($nodeId)]"
	$LL1($nodeId) set nodeId_ $nodeId
	
# new settings (for dei802.11mr)
	set mac1 [$LL1($nodeId) getMac]
	set phy1 [$PHY1($nodeId) getPhy]
	$mac1 basicMode_ Mode6Mb
	$mac1 dataMode_ Mode6Mb
	$mac1 per $per
	set pp1 [new PowerProfile]
	$mac1 powerProfile $pp1
	$phy1 powerProfile $pp1

#set nse [new NSE/ARF]
#$nse Max_Mode Mode54Mb
#$nse Min_Mode Mode6Mb
#$nse mac $mac
#$mac nse $nse
	set ra1 [new RateAdapter/ARF]
	$ra1 attach2mac $mac1
	$ra1 use80211g
	$ra1 setmodeatindex 0
# end new settings

##############################################################
	
# UMTS Modules
	set me_rlc($nodeId)    [new Module/UMTS/RLC/AM]
	set me_mac($nodeId)    [new Module/UMTS/MAC/ME]
	set me_phy($nodeId)    [new Module/MPhy/UMTS/ME]
	
	$me_phy($nodeId) set nodeId_ $nodeId
	$me_phy($nodeId) set BSaddr_ 2
	
#set position($nodeId) [new "Position"]
	set position($nodeId) [new "Position/BM"]
#$position($nodeId) set xFieldWidth_ $xFieldWidth
#	$position($nodeId) set yFieldWidth_ $yFieldWidth
#	$position($nodeId) speedMean $speed($s)
#	$position($nodeId) set updateTime_ 0.1
#	$position($nodeId) set alpha_ 0.8
#	$position($nodeId) bound REBOUNCE
	$position($nodeId) set debug_ 0
	$node($nodeId) addPosition $position($nodeId)
#$umts_propagation($BSid) newNode $position($nodeId)

	$node($nodeId) addModule 6 $cbr_($nodeId) 10 "MT2-CBR$nodeId"
	$node($nodeId) addModule 5 $mux($nodeId) 10 "MT2-MUX$nodeId"
#$node($nodeId) addModule 4 $ip($nodeId) 10 "MT2-IP$nodeId"
	$node($nodeId) addModule 4 $ipCodecUp_($nodeId) $traceVer "IPcodecUP"
#		$node($nodeId) addModule 4 $ipCodecUp1_($nodeId) $traceVer "IPcodecUP1"
	$node($nodeId) addModule 4 $staticRoot_($nodeId) $traceVer "STr"
	
		#$node($nodeId) addModule 4 $staticRoot1_($nodeId) $traceVer "STr1"

	$node($nodeId) addModule 4 $aodv_($nodeId) $traceVer "AODV"

	$node($nodeId) addModule 4 $aodv1_($nodeId) $traceVer "AODV1"

	$node($nodeId) addModule 4 $staticUmts_($nodeId) $traceVer "STu"
	$node($nodeId) addModule 4 $ipCodecDown80211_($nodeId) $traceVer "IPcDW80211"
#############
	$node($nodeId) addModule 4 $ipCodecDown802111_($nodeId) $traceVer "IPcDW802_1"
#############
	$node($nodeId) addModule 4 $ipCodecDownUMTS_($nodeId) $traceVer "IPcDWUMTS"
	$node($nodeId) addModule 3 $LL($nodeId) 110 "MT2-LL$nodeId"
	$node($nodeId) addModule 2 $PHY($nodeId) 10 "MT2-PHY$nodeId"

###########
	$node($nodeId) addModule 3 $LL1($nodeId) 110 "MT2-LL1$nodeId"
	$node($nodeId) addModule 2 $PHY1($nodeId) 10 "MT2-PHY1$nodeId"
###########

	$node($nodeId) addModule 3 $me_rlc($nodeId)  110 "MT2-RLC$nodeId"
	$node($nodeId) addModule 2 $me_mac($nodeId)  110 "MT2-MAC$nodeId"
	$node($nodeId) addModule 1 $me_phy($nodeId)  110 "MT2-PHY$nodeId"
	
	$node($nodeId) setConnection $cbr_($nodeId) $mux($nodeId) 110;#$umtsdebug
	$node($nodeId) setConnection $mux($nodeId) $ipCodecUp_($nodeId) $umtsdebug
	$node($nodeId) setConnection $ipCodecUp_($nodeId) $staticRoot_($nodeId) $traceVer
		#$node($nodeId) setConnection $ipCodecUp_($nodeId) $staticRoot1_($nodeId) $traceVer
#		$node($nodeId) setConnection $ipCodecUp1_($nodeId) $staticRoot_($nodeId) $traceVer
	$node($nodeId) setConnection $staticRoot_($nodeId) $aodv_($nodeId) $traceVer
		#$node($nodeId) setConnection $staticRoot1_($nodeId) $aodv_($nodeId) $traceVer

	$node($nodeId) setConnection $staticRoot_($nodeId) $aodv1_($nodeId) $traceVer
		#$node($nodeId) setConnection $staticRoot1_($nodeId) $aodv1_($nodeId) $traceVer

	$node($nodeId) setConnection $staticRoot_($nodeId) $staticUmts_($nodeId) $traceVer
	#$node($nodeId) setConnection $ipCodecUp_($nodeId) $staticUmts_($nodeId) $traceVer
		#$node($nodeId) setConnection $staticRoot1_($nodeId) $staticUmts_($nodeId) $traceVer
		#$node($nodeId) setConnection $aodv1_($nodeId) $staticUmts_($nodeId) $traceVer
# 802.11 connections
	$node($nodeId) setConnection $aodv_($nodeId) $ipCodecDown80211_($nodeId) 110
##################
	$node($nodeId) setConnection $aodv1_($nodeId) $ipCodecDown802111_($nodeId) 110
##################
	$node($nodeId) setConnection $ipCodecDown80211_($nodeId) $LL($nodeId) 110
	$node($nodeId) setConnection $LL($nodeId) $PHY($nodeId) 110
	$node($nodeId) addToChannel $channel $PHY($nodeId) 0

##################
	$node($nodeId) setConnection $ipCodecDown802111_($nodeId) $LL1($nodeId) 110
	$node($nodeId) setConnection $LL1($nodeId) $PHY1($nodeId) 110
	$node($nodeId) addToChannel $channel1 $PHY1($nodeId) 0
##################


# UMTS connections
	$node($nodeId) setConnection $staticUmts_($nodeId) $ipCodecDownUMTS_($nodeId) 0
	$node($nodeId) setConnection $ipCodecDownUMTS_($nodeId) $me_rlc($nodeId)   $umtsdebug
	$node($nodeId) setConnection $me_rlc($nodeId) $me_mac($nodeId)   $umtsdebug
	$node($nodeId) setConnection $me_mac($nodeId) $me_phy($nodeId)   $umtsdebug
	$node($nodeId) addToChannel  $umts_channel $me_phy($nodeId)   $umtsdebug
	


###########
#$node($nodeId) setConnection $ipCodecUp_($nodeId) $staticRoot_($nodeId) $traceVer
#$node($nodeId) setConnection $staticRoot_($nodeId) $aodv_($nodeId) $traceVer
#$node($nodeId) setConnection $staticRoot_($nodeId) $staticUmts_($nodeId) $traceVer
###########

#add UMTS static route to BS
#$staticUmts_($nodeId) addRoute 0.0.0.0 0.0.0.0 [$ipCodecDownUMTS_($nodeId) id_] $mrclIpAddr_($BSid)
	set net [new Address/IP "0.0.0.0"]
	set mask [new Address/IP "255.255.255.0"]
	set mask2 [new Address/IP "255.255.255.255"]
	set net2 [new Address/IP "1.0.0.8"]
	$staticUmts_($nodeId) addRoute $net $mask $mrclIpAddr_($BSid) $ipCodecDownUMTS_($nodeId)
	$aodv1_($nodeId) addRoute $net2 $mask2 $net2 $ipCodecDown802111_($nodeId)
	
	
	$me_phy($nodeId) setDownlinkSpectralMask $spectralmask_umts_downlink($BSid)
	$me_phy($nodeId) setUplinkSpectralMask   $spectralmask_umts_uplink($BSid)

	$me_phy($nodeId) setInterference         [new MInterference/MIV/UMTS]
	$me_phy($nodeId) setPropagation          $umts_propagation($BSid)
#$me_phy($nodeId) set spreading_factor_ $spreading_factor
	$me_phy($nodeId) set spreading_factor_ 8
	
	finalizeUmtsStack $me_phy($nodeId) $me_mac($nodeId) $me_rlc($nodeId) $ip($nodeId)
	
# We need a MAC and a RLC module at the BS for each ME
	set bs_rlc($BSid,$nodeId)        [new Module/UMTS/RLC/AM]
	set bs_mac($BSid,$nodeId)        [new Module/UMTS/MAC/BS]
	
#$bs_mac($BSid,$nodeId) set maxTxPower_                       0.316227
#$bs_mac($BSid,$nodeId) set maxTxPower_                       10000
#$bs_mac($BSid,$nodeId) set minTxPower_ 0.000000003
	
	$node($BSid) addModule 3 $bs_rlc($BSid,$nodeId) 10 "BS-RLC$BSid"
	$node($BSid) addModule 2 $bs_mac($BSid,$nodeId) 10 "BS-MAC$BSid"

	$node($BSid) setConnection $ip($BSid)    $bs_rlc($BSid,$nodeId)     $umtsdebug
	$node($BSid) setConnection $bs_rlc($BSid,$nodeId) $bs_mac($BSid,$nodeId)     $umtsdebug
	$node($BSid) setConnection $bs_mac($BSid,$nodeId) $bs_phy($BSid)        $umtsdebug

	finalizeUmtsStack $bs_phy($BSid) $bs_mac($BSid,$nodeId) $bs_rlc($BSid,$nodeId) [$ip($BSid) addr]

	$me_rlc($nodeId) connect2CoreNetRlc  $bs_rlc($BSid,$nodeId)
	
#$LL($nodeId) set nodeId_ $nodeId
#$LL($nodeId) setFilePrefix $prefix
	
	set cbrport($nodeId) [$mux($nodeId) assignPort $cbr_($nodeId)]
#set ipaddr($nodeId) [$ip($nodeId) addr]
	puts "Mobile Terminal $nodeId"
	puts "CBR id [$cbr_($nodeId) id_] cbrport $cbrport($nodeId)"
	puts "StaticRoot Module addr [$staticRoot_($nodeId) getAddr 1] old format [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $staticRootIpAddr_($nodeId)]"
	puts "Aodv Module addr [$aodv_($nodeId) getAddr 1] old format [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $aodvIpAddr_($nodeId)]"

#	puts "Aodv Module1 addr [$aodv1_($nodeId) getAddr 1] old format [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $aodvIpAddr1_($nodeId)]"

	puts "StaticUmts Module addr [$staticUmts_($nodeId) getAddr 1] old format [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $staticUmtsIpAddr_($nodeId)]"
#puts "Transport id [$transport id_]"
	puts "AODV [$aodv_($nodeId) id_]"
###########
	puts "AODV1 [$aodv1_($nodeId) id_]"
###########
	puts "Static [$staticRoot_($nodeId) id_]"
	puts "StaticUMTS [$staticUmts_($nodeId) id_]"
	puts "Port-Mux id [$mux($nodeId) id_]"
	puts "LL id [$LL($nodeId) id_]"
	puts "PHY id [$PHY($nodeId) id_]"
#puts "ME-RLC id [$me_rlc($nodeId) id_]"
#puts "ME-MAC id [$me_mac($nodeId) id_]"
#puts "ME-PHY id [$me_phy($nodeId) id_]"
#puts "BS-RLC id [$bs_rlc($BSid,$nodeId) id_]"
#puts "BS-MAC id [$bs_mac($BSid,$nodeId) id_]"
#puts "BS-PHY id [$bs_phy($BSid,$nodeId) id_]"
#puts "id_ ip $ip"
#return $node

}

proc createUser80211 {nodeId net} {
#	-----------------
#	| 6. Application|
#	-----------------
#	| 5. Transport	|
#	----------------
#	| 4. Port-Mux	|
#	-----------------
#	| 	3. IP	|
#	-----------------
#	| 	2. LL	|
#	-----------------
#	| 	1. PHY	|
#	-----------------
#

	global ns channel channel1 pmodel per
	global ip port ipaddr LLAP apdsp cbrport
	global node position cbr_ mux ip LL PHY ipaddr
	
	global broadcastAddr_ mrclIpAddr_ ipCodecUp_ ipCodecDown80211_ ipCodecDownUMTS_ routing_
	
	global availableAddress traceVer
	
	set mrclIpAddr_($nodeId) [new Address/IP "$net.0.0.$availableAddress"]
	puts "Indirizzo nodo: $net.0.0.$availableAddress"
	incr availableAddress

	set cbr_($nodeId) [new Module/CBR]
	$cbr_($nodeId) set debug_ 10
#set node1 [createAP $sender $fmng "Access Provider" "-AP1" 1]
	set node($nodeId) [$ns create-M_Node]
	
	set mux($nodeId) [new Module/Port/Map]
#set ip($nodeId) [new Module/IP/AN]
	set ipCodecUp_($nodeId) [new Module/MrclIpAddressCodec]
	$ipCodecUp_($nodeId) up
	set ipCodecDown80211_($nodeId) [new Module/MrclIpAddressCodec]
	$ipCodecDown80211_($nodeId) down
	set routing_($nodeId) [new Routing/AODV]
	$routing_($nodeId) set debug_ 115
#$routing_($nodeId) AodvLinkLayerDetection
	$routing_($nodeId) setAddr $mrclIpAddr_($nodeId)
	$routing_($nodeId) setBroadcastAddr $broadcastAddr_

	set LL($nodeId) [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $mrclIpAddr_($nodeId)] "" 100 ]
	set PHY($nodeId) [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $LL($nodeId) ""]
#set LL($nodeId) [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Miracle" [$ip($nodeId) addr] "" 100]
#set PHY($nodeId) [createPhyModule "Phy/WirelessPhy/Miracle" $pmodel "Antenna/OmniAntenna" $LL($nodeId) ""]
	
	
# new settings (for dei802.11mr)
	set mac [$LL($nodeId) getMac]
	set phy [$PHY($nodeId) getPhy]
	$mac basicMode_ Mode6Mb
	$mac dataMode_ Mode6Mb
	$mac per $per
	set pp [new PowerProfile]
	$mac powerProfile $pp
	$phy powerProfile $pp

#set nse [new NSE/ARF]
#$nse Max_Mode Mode54Mb
#$nse Min_Mode Mode6Mb
#$nse mac $mac
#$mac nse $nse
	set ra [new RateAdapter/ARF]
	$ra attach2mac $mac
	$ra use80211g
	$ra setmodeatindex 0
# end new settings
	
	set position($nodeId) [new "Position/BM"]
	
	$node($nodeId) addPosition $position($nodeId)

	$node($nodeId) addModule 6 $cbr_($nodeId) 10 "AP-FMNG$nodeId"
	$node($nodeId) addModule 5 $mux($nodeId) 10 "AP-MUX$nodeId"
	$node($nodeId) addModule 4 $ipCodecUp_($nodeId) $traceVer "IPcodecUP"
	$node($nodeId) addModule 4 $routing_($nodeId) $traceVer "AODV"
	$node($nodeId) addModule 4 $ipCodecDown80211_($nodeId) $traceVer "IPcDW80211"
	$node($nodeId) addModule 3 $LL($nodeId) 110 "AP-LL$nodeId"
	$node($nodeId) addModule 2 $PHY($nodeId) 110 "AP-PHY$nodeId"
	
	$LL($nodeId) set nodeId_ $nodeId
	
	$node($nodeId) setConnection $cbr_($nodeId) $mux($nodeId) 110
	$node($nodeId) setConnection $mux($nodeId) $ipCodecUp_($nodeId) 110
	$node($nodeId) setConnection $ipCodecUp_($nodeId) $routing_($nodeId)
	$node($nodeId) setConnection $routing_($nodeId) $ipCodecDown80211_($nodeId) 10
	$node($nodeId) setConnection $ipCodecDown80211_($nodeId) $LL($nodeId) 110
	$node($nodeId) setConnection $LL($nodeId) $PHY($nodeId) 110
	
	if {$net == 0} {
		puts "Net 0"
		$node($nodeId) addToChannel $channel $PHY($nodeId) 0
	} elseif {$net == 1} {
		puts "Net 1 e address [expr $availableAddress-1]"
		$node($nodeId) addToChannel $channel1 $PHY($nodeId) 0
	} else {
		puts "Unknown NET!"
	}
	
	set cbrport($nodeId) [$mux($nodeId) assignPort $cbr_($nodeId)]
	set ipaddr($nodeId) [$ipCodecUp_($nodeId) mrcl_addr2ns_addr_t $mrclIpAddr_($nodeId)]
	puts "Created AP $nodeId:"
	puts "CBR id [$cbr_($nodeId) id_] cbrport $cbrport($nodeId) IPaddr $ipaddr($nodeId)"
#puts "Transport id [$transport id_]"
	puts "AODV [$routing_($nodeId) id_]"
	puts "Port-Mux id [$mux($nodeId) id_]"
	puts "LL id [$LL($nodeId) id_]"
	puts "PHY id [$PHY($nodeId) id_]"
	puts ""
#set LLAP $LL($nodeId)
#puts "id_ ip $ip"
	
#set position [$node($nodeId) position]

#return $node
}


########################################

########################################

set traceVer 1

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

#umts NEW (power control)
UMTS/RLC/AM set maxRBSize_                            1000000000000000kbytes
Module/MPhy/UMTS set PowerCtrlUpdateCmdAtStartRx_   1
Module/MPhy/UMTS set PowerCtrlReadCmdAtStartRx_     1
Module/MPhy/UMTS/BS set minTxPower_      2.5e-10


set pmodel [new Propagation/MrclTwoRayGround]

set channel [new Module/DumbWirelessCh]

#################
set channel1 [new Module/DumbWirelessCh]
#################

set umts_channel [new Module/UmtsWirelessCh]
set spreading_factor 4


set availableAddress 1
set broadcastAddr_ [new Address/IP "255.255.255.255"]

create-god 7

createBS 1
createUser2Interfaces 2 1 0
createUser80211 3 0
createUser80211 4 0
createUser80211 5 1


#$position(1) setX_ 0.0
#$position(1) setY_ 0.0

#$position(2) setX_ 10.0
#$position(2) setY_ 10.0

#$position(3) setX_ 30.0
#$position(3) setY_ 0.0

#$position(4) setX_ 30.0
#$position(4) setY_ 30.0

#$position(5) setX_ 0.0
#$position(5) setY_ 30.0

#$ns at 0.09 "puts \"AAAAAAAAA [[$link(3) set downlink_] id_]\""
#$ip_(3) addRoute 0.0.0.0 0.0.0.0 [[$link(1) set uplink_] id_] 0.0.0.2
#$ipC1_(2) addRoute 0.0.0.0 0.0.0.0 [[$link(2) set uplink_] id_] 0.0.0.1
#$ipC2_(2) addRoute 0.0.0.0 0.0.0.0 [[$link(3) set uplink_] id_] 0.0.0.1
#$ip_F(3) addRoute 0.0.0.0 0.0.0.0 [[$link(3) set uplink_] id_] 0.0.0.4
#$ip_(1) addRoute 0.0.0.0 0.0.0.0 [[$link(3) set downlink_] id_] 0.0.0.2
#$ip_(2) addRoute 0.0.0.0 0.0.0.0 [[$link(2) set uplink_] id_] 0.0.0.3
#$ip_(3) addRoute 0.0.0.1 255.255.255.255 [[$link(1) set downlink_] id_] 0.0.0.1
#$ip_(3) addRoute 0.0.0.2 255.255.255.255 [[$link(2) set downlink_] id_] 0.0.0.2
#$ip_(3) addRoute 0.0.0.0 0.0.0.0 [[$link(3) set uplink_] id_] 0.0.0.4


$cbr_(3) set destAddr_ [$ip(1) addr]
$cbr_(3) set destPort_ $cbrport(1)
$cbr_(3) set period_ 2
$cbr_(3) set debug_ 110

#$cbr_(3) set destAddr_ $ipaddr(5)
#$cbr_(3) set destPort_ $cbrport(5)
#$cbr_(3) set period_ 2
#$cbr_(3) set debug_ 110


#$cbr_(4) set destAddr_ [$ip(1) addr]
#$cbr_(4) set destPort_ $cbrport1(1)
#$cbr_(4) set period_ 2
#$cbr_(4) set debug_ 110

$cbr_(4) set destAddr_ $ipaddr(5)
$cbr_(4) set destPort_ $cbrport(5)
$cbr_(4) set period_ 2
$cbr_(4) set debug_ 110

$cbr_(1) set destAddr_ $ipaddr(3)
$cbr_(1) set destPort_ $cbrport(3)
$cbr_(1) set debug_ 110

#$cbr1_(1) set destAddr_ $ipaddr(4)
#$cbr1_(1) set destPort_ $cbrport(4)
#$cbr1_(1) set debug_ 110

$cbr_(5) set destAddr_ $ipaddr(4)
$cbr_(5) set destPort_ $cbrport(4)
$cbr_(5) set period_ 2
$cbr_(5) set debug_ 110

puts "Start simulation"
#$ns at 0.1 "$cbr_(3) start"
$ns at 1 "$cbr_(3) sendPkt"
#$ns at 2 "$cbr_(3) sendPkt"
#$ns at 5 "$cbr_(3) stop"
#$ns at 2 "$cbr_(4) sendPkt"
#$ns at 4 "$cbr_(3) sendPkt"
#$ns at 4 "$cbr_(4) sendPkt"
$ns at 2 "$cbr_(4) sendPkt"
#$ns at 5 "$cbr_(4) start"
$ns at 3 "$cbr_(3) sendPkt"
$ns at 4 "$cbr_(4) sendPkt"
$ns at 15 "$cbr_(4) start"
$ns at 40 "$cbr_(4) stop"
$ns at 30 "$cbr_(3) sendPkt"

$ns at 100 "finish; $ns halt"

$ns run


