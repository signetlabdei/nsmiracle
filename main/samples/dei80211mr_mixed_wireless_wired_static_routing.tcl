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
dynlibload MiracleIp ../ip/.libs/
dynlibload MiracleIpRouting ../ip/.libs/
dynlibload miracleport ../port/.libs/
dynlibload miraclecbr ../cbr/.libs/
dynlibload cbrtracer ../cbr/.libs/
dynlibload miraclelink ../link/.libs/
dynlibload routingtracer ../ip/.libs/

dynlibload MiracleWirelessCh ../wirelessch/.libs/
dynlibload MiraclePhy802_11 ../phy802_11/.libs/
dynlibload MiracleMac802_11 ../mac802_11/.libs/


dynlibload dei80211mr

dynlibload multiratetracer ../dei80211mr/.libs/
dynlibload phytracer ../phy802_11/.libs/

dynlibload miracletcp ../tcp/.libs/

dynlibload MiracleBasicMovement ../mobility/.libs/



# Global options

# number of 802.11 nodes
set NUMWNODES       5

# number of APs, which also corresponds to 
# the number of 802.11 interfaces per node  
set NUMAPNODES       3     

# Stop time
set STOP 10

# total number of wireless devices
set NUMWD [expr $NUMWNODES * $NUMAPNODES + $NUMAPNODES]


set ns [new Simulator]
$ns use-Miracle

set tf [open out.tr w]
$ns trace-all $tf

proc finish {} {
	global ns tf 
	puts "---> SIMULATION DONE."
	$ns flush-trace
	close $tf
}


#Module/Port set debug_ 6


proc createFixedHost {} {

    # In this scenario we have only one fixed host which has index n=0
    puts "Creating fixed host (n=0)"

    #
    #	+-------------------------------------------+
    #	|  4. CBR1 | 4. CBR2  | 4. TCP1  | 4. TCP2  |
    #	+-------------------------------------------+
    #	| 	   3. Port                          |
    #	+-------------------------------------------+
    #	|          2. IP/Routing                    | 
    #	+-------------------------------------------+
    #	| 1. IP/Interface     |1. IP/Interface      |
    #	+-------------------------------------------+
    #

    global ns node fhcbr fhtcpm port ipr ipif fhcbrportnum fhtcpportnum NUMAPNODES NUMWNODES 
    
    set node(0)    [$ns create-M_Node]
    set port(0)    [new Module/Port/Map]
    set ipr(0)     [new Module/IP/Routing]


    $node(0) addModule 3 $port(0)   0 "PRT"
    $node(0) addModule 2 $ipr(0)    0 "IPR"

    $node(0) setConnection $port(0) $ipr(0)   1


    for {set n [expr $NUMAPNODES + 1]} {$n<= [expr $NUMAPNODES + $NUMWNODES]} {incr n} {

	set fhcbr($n)     [new Module/CBR]
	set fhtcpm($n)     [new Module/TCP/Reno]

	$node(0) addModule 4 $fhcbr($n)    0 "CBR$n"
	$node(0) addModule 4 $fhtcpm($n)   0 "TCP$n"

	$node(0) setConnection $fhcbr($n) $port(0) 1
	$node(0) setConnection $fhtcpm($n) $port(0) 1
	

	set fhcbrportnum($n) [$port(0) assignPort $fhcbr($n) ]
	set fhtcpportnum($n) [$port(0) assignPort $fhtcpm($n) ]

    }


    for {set ifn 1} {$ifn <= $NUMAPNODES} {incr ifn} {

	set ipif(0,$ifn)  [new Module/IP/Interface]
	$ipif(0,$ifn) addr "2.0.${ifn}.0"
	$ipif(0,$ifn) subnet "255.255.255.0"
	$ipr(0) addRoute "1.0.${ifn}.0" "255.255.255.0" "2.0.${ifn}.${ifn}"	    

	$node(0) addModule 1 $ipif(0,$ifn) 0 "IP$ifn"

	$node(0) setConnection $ipr(0) $ipif(0,$ifn)     1
	
	
    }

    set position(0) [new "Position/BM"]
    $node(0) addPosition $position(0)


    $position(0) setX_ 0.0
    $position(0) setY_ 0.0

}



proc createAPNode {n} {
    
    puts "Creating Access Point (n=$n)"
    #
    #
    #	+------------------------------------+
    #	|          4. IP/Routing             | 
    #	+------------------------------------+
    #	| 3. IP/Interface  |3. IP/Interface  |
    #	+------------------------------------+
    #	| 2. LL            | 
    #	+-------------------  directly connected to duplex link
    #	| 1. PHY           |
    #	+-------------------
    #

    global ns node port ipr ipif LL PHY mac phy pmodel channel per peerstats
    
    set node($n)    [$ns create-M_Node]
    set ipr($n)     [new Module/IP/Routing]

    $node($n) addModule 4 $ipr($n)    0 "IPR"


    set position($n) [new "Position/BM"]
    $node($n) addPosition $position($n)

    $position($n) setX_ 0.0
    $position($n) setY_ 0.0

    # route from AP to fixed host
    $ipr($n) addRoute "2.0.${n}.0" "255.255.255.255" "2.0.${n}.0"


    # -----------------------------------
    #   This is the wireless interface
    # -----------------------------------
    set ifn 0

    set ipif($n,0)  [new Module/IP/Interface]
    $ipif($n,0) addr "1.0.${n}.${n}"
    $ipif($n,0) subnet "255.255.255.0"
    

    set LL($n,0)    [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$ipif($n,0) addr] "" 100 ]
    set PHY($n,0)   [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $LL($n,0)  ""]

    set mac($n,0)  [$LL($n,0)  getMac]
    set phy($n,0)  [$PHY($n,0)  getPhy]
    $mac($n,0)  basicMode_ Mode6Mb
    $mac($n,0)  dataMode_ Mode6Mb
    $mac($n,0)  per $per
    $mac($n,0)  PeerStatsDB $peerstats
    set pp($n,0)  [new PowerProfile]
    $mac($n,0)  powerProfile $pp($n,0) 
    $phy($n,0)  powerProfile $pp($n,0) 

    $node($n) addModule 3 $ipif($n,0) 0 "IP0"
    $node($n) addModule 2 $LL($n,0)   0 "LL0"
    $node($n) addModule 1 $PHY($n,0)  0 "PH0"

    $node($n) setConnection $ipr($n) $ipif($n,0)  1
    $node($n) setConnection $ipif($n,0) $LL($n,0) 1
    $node($n) setConnection $LL($n,0) $PHY($n,0)  1

    # create the 802.11 channel
    #set channel($n)  [createWirelessChModule "" "" ""]
    set channel($n)  [new Module/DumbWirelessCh]

    $node($n) addToChannel $channel($n) $PHY($n,0) 1
    


    # -----------------------------------
    #   This is the wired interface
    # -----------------------------------	

    set ipif($n,1)  [new Module/IP/Interface]
    $ipif($n,1) addr "2.0.${n}.${n}"
    $ipif($n,1) subnet "255.255.255.0"	    

    $node($n) addModule 3 $ipif($n,1) 0 "IP1"
    $node($n) setConnection $ipr($n) $ipif($n,1)     1

    set dlink($n) [new Module/DuplexLink]	
    $dlink($n) bandwidth 1000000
    $dlink($n) delay 0.0001
    $dlink($n) qsize 10
    $dlink($n) settags "CH$n"
    $dlink($n) connect $node($n) $ipif($n,1) 1 $node(0) $ipif(0,$n) 1

    
		
}





proc createWirelessNode {n} {
    puts "Creating wireless node (n=$n)"
#
#	+------------------------------------+
#	|  6. CBR	   | 6. TCP          |
#	+------------------------------------+
#	| 	   5. Port                   |
#	+------------------------------------+
#	|          4. IP/Routing             | 
#	+------------------------------------+
#	| 3. IP/Interface  |3. IP/Interface  |
#	+------------------------------------+
#	| 2. LL            | 2. LL           |
#	+------------------------------------+
#	| 1. PHY           |  1. PHY         |
#	+------------------------------------+
#

	global ns node cbr tcpm port ipr ipif cbrportnum tcpportnum LL PHY mac phy pmodel channel per NUMAPNODES NUMWNODES peerstats
	
	set node($n)    [$ns create-M_Node]
	set cbr($n)     [new Module/CBR]
	set tcpm($n)    [new Module/TCP/Reno]
	set port($n)    [new Module/Port/Map]
	set ipr($n)     [new Module/IP/Routing]


	$node($n) addModule 6 $cbr($n)    0 "CBR"
	$node($n) addModule 6 $tcpm($n)   0 "TCP"
	$node($n) addModule 5 $port($n)   0 "PRT"
	$node($n) addModule 4 $ipr($n)    0 "IPR"

	$node($n) setConnection $cbr($n) $port($n) 1
	$node($n) setConnection $tcpm($n) $port($n) 1
	$node($n) setConnection $port($n) $ipr($n)   1

	set cbrportnum($n) [$port($n) assignPort $cbr($n) ]
	set tcpportnum($n) [$port($n) assignPort $tcpm($n) ]


	for {set ifn 1} {$ifn <= $NUMAPNODES} {incr ifn} {

	    set ipif($n,$ifn)  [new Module/IP/Interface]
	    $ipif($n,$ifn) addr "1.0.${ifn}.${n}"
	    $ipif($n,$ifn) subnet "255.255.255.0"

	    # route from mobile to AP
	    $ipr($n) addRoute "2.0.${ifn}.0" "255.255.255.0" "1.0.${ifn}.${ifn}"

	    # route from AP to mobile
	    $ipr($ifn) addRoute "1.0.${ifn}.${n}" "255.255.255.255" "1.0.${ifn}.${n}"

	    set LL($n,$ifn)    [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$ipif($n,$ifn) addr] "" 100 ]
	    set PHY($n,$ifn)   [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $LL($n,$ifn) ""]


	    set mac($n,$ifn)  [$LL($n,$ifn)  getMac]
	    set phy($n,$ifn)  [$PHY($n,$ifn)  getPhy]
	    $mac($n,$ifn)  basicMode_ Mode6Mb
	    $mac($n,$ifn)  dataMode_ Mode6Mb
	    $mac($n,$ifn)  per $per
	    $mac($n,$ifn)  PeerStatsDB $peerstats
	    set pp($n,$ifn)  [new PowerProfile]
	    $mac($n,$ifn)  powerProfile $pp($n,$ifn) 
	    $phy($n,$ifn)  powerProfile $pp($n,$ifn) 

	    $node($n) addModule 3 $ipif($n,$ifn) 0 "IP$ifn"
	    $node($n) addModule 2 $LL($n,$ifn)   0 "LL$ifn"
	    $node($n) addModule 1 $PHY($n,$ifn)  0 "PH$ifn"

	    $node($n) setConnection $ipr($n) $ipif($n,$ifn)     1
	    $node($n) setConnection $ipif($n,$ifn) $LL($n,$ifn) 1
	    $node($n) setConnection $LL($n,$ifn) $PHY($n,$ifn)  1

	    $node($n) addToChannel $channel($ifn) $PHY($n,$ifn) 1
	    
	}

	set position($n) [new "Position/BM"]
	$node($n) addPosition $position($n)


	$position($n) setX_ 1.0
	$position($n) setY_ 1.0

		
}





create-god $NUMWD


set per [new PER]
$per loadDefaultPERTable 
$per set noise_ 7e-11

set peerstats [new PeerStatsDB/Static]
$peerstats numpeers $NUMWD

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


#set pmodel [new Propagation/MrclTwoRayGround]
set pmodel [new Propagation/MrclFreeSpace]


createFixedHost

for {set n 1} {$n <= $NUMAPNODES } {incr n} {
    createAPNode $n
}


for {set n [expr $NUMAPNODES + 1]} {$n <= [expr $NUMWNODES + $NUMAPNODES]  } {incr n} {
    createWirelessNode $n
}

#puts "[array names fhcbrportnum]"

# Now we setup connections. 
# Warning: dont'use strings here! 
# Many applications accept only a raw int32 as destAddr_
#$cbr(3) set destAddr_ [$ipif(0,1) addr]
#$cbr(3) set destPort_ $fhcbrportnum(3)
#$ns at 1 "$cbr(3) start"
#$ns at $STOP "$cbr(3) stop"


#puts "Connecting to IP [$ipif(4,1) addr] port $cbrportnum(4) "

set rvstart [new RandomVariable/Uniform]

for {set n [expr $NUMAPNODES + 1]} {$n <= [expr $NUMWNODES + $NUMAPNODES]  } {incr n} {
    #users are statically assigned to APs in a round-robin fashion
    set ap [expr ($n % $NUMAPNODES) + 1 ]
    $fhcbr($n) set destAddr_ [$ipif($n,$ap) addr]
    $fhcbr($n) set destPort_ $cbrportnum($n)
    $ns at [$rvstart value] "$fhcbr($n) start"
    $ns at $STOP            "$fhcbr($n) stop"

}

puts "[array names mac]"
for {set i 1} {$i<=$NUMAPNODES} {incr i} {
    for {set j [expr $NUMAPNODES + 1]} {$j <= [expr $NUMWNODES + $NUMAPNODES]} {incr j} {
	#$ns at 5 "puts \"MEASURED SNR $i -> $j [$mac($j,$i) getSnr [expr $i - 1]] dB\""
	$ns at 5 "puts \"MEASURED SNR $i -> $j : \[$mac($j,$i) getSnr \[expr $i - 1\]\]\""
    }
}


$ns at [expr $STOP +1] "finish; $ns halt"
$ns run


