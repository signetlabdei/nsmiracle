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

# TCP over infrastructured 802.11g and wired network
#
#
#          Mobile Equipment (ME)               Base Station (BS) - a.k.a. Access Point                                        Fixed Host (FH)
#	+-----------------------+	                                                                        +-----------------+-----------------+
#	| 	6. TCP          |                                                                        	| 6. TCPSink      |   6. TCPSink    |
#	+-----------------------+	                                                                        +-----------------+-----------------+
#	| 	5. Port         |                                                                        	| 5. Port         |   5. Port       |
#	+-----------------------+	+------------------------------------------------------------+          +-----------------+-----------------+
#	| 	4. Ip Routing   |	|                        4. Ip Routing                       |          |        4. Ip Routing              |
#	+-----------------------+	+------------------------------------------------------------+	        +-----------------------------------+
#	| 	3. Ip Interface |	| 	3. Ip Interface             |    3. Ip Interface     |          |        3. Ip Interface            |
#	+-----------------------+	+-----------------+-----------------+-------------+----------+          +-----------------+-----------------+
#	| 	2. Module/802.11|	|       2. Module/802.11            |             |                                       |
#	+-----------------------+	+-----------------+-----------------+             |                                       |
#	| 	1. Module/Phy	|	| 	1. Module/Phy         	    |             |                                       | 
#	+------------+----------+	+------------+----------------------+             |                                       | 
#                    |                               |                                    |                                       | 
#	+------------+-------------------------------+----------+             	+---------+---------------------------------------+-----+   
#       |                  DumbWirelessChannel                  |               |                          Duplex Link                  |
#       +-------------------------------------------------------+             	+-------------------------------------------------------+   
#



 
########################################
# Command-line parameters
########################################


if {$argc == 0} {
    set opt(nn)  4
    set opt(run) 1
    
} elseif {$argc != 2 } {
    puts " usage: ns $argv0 numnodes replicationnumber"
    exit
} else {
    set opt(nn)     [lindex $argv 0]       
    set opt(run)    [lindex $argv 1]
}




########################################
# Scenario Configuration
########################################



set opt(xmax)   110
set opt(xmin)   30  


# duration of each transmission
set opt(duration)     30

# starting time of each transmission
set opt(startmin)     1
set opt(startmax)     1.05

set opt(resultdir) "/tmp"
set opt(tracedir) "/tmp"
set machine_name [exec uname -n]
set opt(fstr)        ${argv0}_${opt(nn)}_${opt(xmax)}.${machine_name}
set opt(resultfname) "${opt(resultdir)}/stats_${opt(fstr)}.log"
set opt(tcptrfname)  "${opt(tracedir)}/tcp_trace.tr"
set opt(tcpTrace)    [ open  $opt(tcptrfname) w]
set opt(tracefile)   "${opt(tracedir)}/tcp_80211.tr"

########################################
# Module Libraries
########################################


# The following lines must be before loading libraries
# and before instantiating the ns Simulator
#
remove-all-packet-headers


#puts "WARNING: this script requires that the needed libraries are in your LD_LIBRARY_PATH"

source dynlibutils.tcl

dynlibload Miracle ../nsmiracle/.libs/
dynlibload MiracleBasicMovement ../mobility/.libs/
dynlibload MiracleIp ../ip/.libs/
dynlibload MiracleIpRouting ../ip/.libs/
dynlibload miracletcp ../tcp/.libs/
dynlibload miracleport ../port/.libs/
dynlibload miraclelink ../link/.libs/
dynlibload MiracleWirelessCh ../wirelessch/.libs/
dynlibload MiraclePhy802_11 ../phy802_11/.libs/
dynlibload MiracleMac802_11 ../mac802_11/.libs/
dynlibload dei80211mr



########################################
# Tracers
########################################

#dynlibload tcptracer
dynlibload routingtracer ../ip/.libs/
#dynlibload ClTrace
dynlibload multiratetracer ../dei80211mr/.libs/
dynlibload phytracer ../phy802_11/.libs/


set ns [new Simulator]
$ns use-Miracle


########################################
# Random Number Generators
########################################

global defaultRNG
set startrng [new RNG]
set positionrng [new RNG]

set rvstart [new RandomVariable/Uniform]
$rvstart set min_ $opt(startmin)
$rvstart set max_ $opt(startmax)
$rvstart use-rng $startrng

set rvposition [new RandomVariable/Uniform]
$rvposition set min_ 0
$rvposition set max_ $opt(xmax)
$rvposition use-rng $positionrng

# seed random number generator according to replication number
for {set j 1} {$j < $opt(run)} {incr j} {
    $defaultRNG next-substream
    $startrng next-substream
    $positionrng next-substream
}

proc finish {} {
    global ns tf opt me_tcpa
    puts "done!"
    $ns flush-trace
    close $tf
#    close $opt(tcpTrace)
    print_stats
    puts ""
    puts "Tracefile     : $opt(tracefile)"
    puts "Results file  : $opt(resultfname)"
    puts "TCP tracefile : $opt(tcptrfname)"
}



proc print_stats  {} {
    global me_tcpa opt me_macmr me_phy peerstats me_pos

    set resultsFilePtr [open $opt(resultfname) a]

    for {set id 1} {$id <= $opt(nn) } {incr id} {


	set thrtcp [ expr [ $me_tcpa($id) set ack_  ] * [$me_tcpa($id) set packetSize_] / $opt(duration) ]

	set snr [$me_macmr($id) getAPSnr ]

	#set pduer [expr ($me_errpdus + 0.0) / $me_totpdus]

#	puts "$opt(run) $opt(nn) $opt(xmax) $opt(duration) $id $snr $thrtcp [$me_macmr($id) getMacCounters] "
#	puts $resultsFilePtr "$opt(run) $opt(nn) $opt(xmax) $opt(duration) $id $snr $thrtcp [$me_macmr($id) getMacCounters] "

#	set logstr "$opt(run) $opt(nn) $opt(xmax) $opt(duration) $id [$me_pos($id) getX_] $snr $thrtcp [$me_macmr($id) getMacCounters] \n $id <-> 0 [$peerstats getPeerStats $id 0]"
	set logstr "$opt(run) $opt(nn) $opt(xmax) $opt(duration) $id [$me_pos($id) getX_] $snr $thrtcp [$peerstats getPeerStats $id 0]"
	#puts $logstr
 	puts $resultsFilePtr $logstr

    }
#    puts ""
#    $peerstats dump
    
    close $resultsFilePtr

}



set tf [open $opt(tracefile) w]
$ns trace-all $tf


########################################
# Override Default Module Configuration
########################################

PeerStatsDB/Static set debug_ 0

Agent/TCP set packetSize_     1000

Mac/802_11 set RTSThreshold_ 2000
Mac/802_11 set ShortRetryLimit_ 3
Mac/802_11 set LongRetryLimit_ 3
Mac/802_11/Multirate set useShortPreamble_ true
Mac/802_11/Multirate set gSyncInterval_ 0.000005
Mac/802_11/Multirate set bSyncInterval_ 0.00001

Phy/WirelessPhy set Pt_ 0.01
Phy/WirelessPhy set freq_ 2437e6
Phy/WirelessPhy set L_ 1.0
Queue/DropTail/PriQueue set Prefer_Routing_Protocols    1
Queue/DropTail/PriQueue set size_ 1000

set noisePower 7e-11
Phy/WirelessPhy set CSTresh_ [expr $noisePower * 1.1]

# starting TCP window
Agent/TCP set windowInit_ 
# Receiver Advertised Window
# Agent/TCP set window_ 64
# Agent/TCP set delay_growth_ 0
# Agent/TCP set maxburst_ 32000
# Agent/TCP set slow_start_restart_ 0
Agent/TCP set ts_resetRTO_ 1
Agent/TCP set timestamps_ 1


###############################
# Global allocations
###############################

set channel [new Module/DumbWirelessCh]
$channel setTag "CHA"

set pmodel [new Propagation/MrclFreeSpace]
create-god [expr $opt(nn) + 1]

set per [new PER]
$per set noise_ $noisePower
$per loadDefaultPERTable

set peerstats [new PeerStatsDB/Static]
$peerstats numpeers [expr $opt(nn) + 1]


###############################
#  Create Base Station
###############################


set bs_node        [$ns create-M_Node] 
set bs_ipif_dot11  [new Module/IP/Interface]

# need this in advance for ARP
$bs_ipif_dot11  addr   "1.0.1.1"

set bs_mac         [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$bs_ipif_dot11 addr] "" 100 ]
set bs_phy         [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $bs_mac ""]
set bs_ipif_wan    [new Module/IP/Interface]
set bs_ipr         [new Module/IP/Routing]

set bs_macmr       [$bs_mac  getMac]
set bs_wphy        [$bs_phy  getPhy]

$bs_macmr  basicMode_ Mode6Mb
$bs_macmr  dataMode_ Mode6Mb
$bs_macmr  per $per
$bs_macmr  PeerStatsDB $peerstats
set bs_pp  [new PowerProfile]
$bs_macmr  powerProfile $bs_pp
$bs_wphy   powerProfile $bs_pp


set bs_macaddr [$bs_macmr id]
$bs_macmr     bss_id $bs_macaddr

$bs_node addModule 1 $bs_phy  0 "PHY "
$bs_node addModule 2 $bs_mac  0 "MAC "
$bs_node addModule 3 $bs_ipif_dot11 0 "RIF"
$bs_node addModule 3 $bs_ipif_wan   0 "WIF"
$bs_node addModule 4 $bs_ipr  0 "IPR"

$bs_node setConnection $bs_ipr $bs_ipif_dot11  0
$bs_node setConnection $bs_ipr $bs_ipif_wan    0
$bs_node setConnection $bs_ipif_dot11 $bs_mac  0
$bs_node setConnection $bs_mac $bs_phy         0

$bs_node addToChannel $channel $bs_phy   1


set bs_pos [new "Position/BM"]
$bs_node addPosition $bs_pos
$bs_pos setX_ 0.0
$bs_pos setY_ 0.0

# IP address of bs_ipif_dot11 is already set to enable ARP
$bs_ipif_dot11  subnet "255.255.0.0"
$bs_ipif_wan   addr   "2.0.0.1"
$bs_ipif_wan   subnet "255.255.0.0"
$bs_ipr  addRoute "2.0.0.0" "255.255.0.0" "2.0.0.2"
# Route to MEs will be added later



###############################
#  Create Fixed Host
###############################

set fh_node   [$ns create-M_Node] 
set fh_ipif   [new Module/IP/Interface]
set fh_ipr    [new Module/IP/Routing]
set fh_port   [new Module/Port/Map]

$fh_node addModule 4  $fh_ipif 0 "IPF"
$fh_node addModule 5  $fh_ipr  0 "IPR"
$fh_node addModule 6  $fh_port 0 "PRT"

$fh_node setConnection $fh_port $fh_ipr   0
$fh_node setConnection $fh_ipr  $fh_ipif  0

$fh_ipif  addr   "2.0.0.2"
$fh_ipif  subnet "255.255.0.0"
$fh_ipr  addRoute "1.0.0.0" "255.255.0.0" "2.0.0.1"


###############################
#  Create... The Internet!!!
###############################

set dlink [new Module/DuplexLink]
$dlink delay      0.1
$dlink bandwidth  10000000
$dlink qsize      10
$dlink settags "INT"

$dlink connect $bs_node $bs_ipif_wan 0 $fh_node $fh_ipif 0



###############################
#  Create Mobile Equipments
###############################

for {set id 1} {$id <= $opt(nn)} {incr id} {

    set me_node($id)   [$ns create-M_Node] 


    # Create Protocol Stack 

    set me_tcp($id)    [new Module/TCP/Reno]
    set me_port($id)   [new Module/Port/Map]
    set me_ipr($id)    [new Module/IP/Routing]
    set me_ipif($id)   [new Module/IP/Interface]

    # need this in advance for ARP
    $me_ipif($id) addr   "1.0.0.$id"
    
    set me_mac($id)          [create802_11MacModule "LL/Mrcl" "Queue/DropTail/PriQueue" "Mac/802_11/Multirate" [$me_ipif($id) addr] "" 100 ]
    set me_phy($id)          [createPhyModule "Phy/WirelessPhy/PowerAware" $pmodel "Antenna/OmniAntenna" $me_mac($id) ""]

    set me_macmr($id)       [$me_mac($id)  getMac]
    set me_wphy($id)       [$me_phy($id)  getPhy]

    $me_macmr($id)  basicMode_ Mode6Mb
    $me_macmr($id)  dataMode_ Mode6Mb
    $me_macmr($id)  per $per
    $me_macmr($id)  PeerStatsDB $peerstats
    set me_pp($id)  [new PowerProfile]
    $me_macmr($id)  powerProfile $me_pp($id)
    $me_wphy($id)   powerProfile $me_pp($id)

    $me_macmr($id)  bss_id $bs_macaddr

    set me_tcpa($id)   [$me_tcp($id) getTcpAgent]
    set me_ftp($id)    [new Application/FTP]
    $me_ftp($id) attach-agent $me_tcpa($id)

    $me_node($id) addModule 6 $me_tcp($id)  0 "TCP "
    $me_node($id) addModule 5 $me_port($id) 0 "PRT "
    $me_node($id) addModule 4 $me_ipr($id)  0 "IPR "
    $me_node($id) addModule 3 $me_ipif($id) 0 "IPF "
    $me_node($id) addModule 2 $me_mac($id)  0 "MAC "
    $me_node($id) addModule 1 $me_phy($id)  0 "PHY "

    $me_node($id) setConnection $me_tcp($id)  $me_port($id)  0
    $me_node($id) setConnection $me_port($id) $me_ipr($id)   0
    $me_node($id) setConnection $me_ipr($id)  $me_ipif($id)  0
    $me_node($id) setConnection $me_ipif($id) $me_mac($id)   0
    $me_node($id) setConnection $me_mac($id)  $me_phy($id)   0
    $me_node($id) addToChannel  $channel      $me_phy($id)   0

    # Protocol Stack Configuration

    # IP address of me_ipif is already set to enable ARP
    $me_ipif($id) subnet "255.255.0.0"
    $me_ipr($id) addRoute "2.0.0.2" "255.255.0.0" "1.0.1.1"
    set me_tcpportnum($id) [$me_port($id) assignPort $me_tcp($id) ]


    # Since we don't use an ad-hoc routing protocol, we need to set a
    # route to each node at the AP
    $bs_ipr  addroute "1.0.0.$id" "255.255.255.255" "1.0.0.$id"

    
    # Furthermore, for each ME we also need a TCPSink at the FH
    set fh_tcpsink($id)        [new Module/TCPSink] 
    set fh_tcpsa($id)          [$fh_tcpsink($id) getTcpAgent]
    $fh_node addModule 7   $fh_tcpsink($id) 0 "TCPS$id"
    $fh_node setConnection $fh_tcpsink($id) $fh_port  0
    set fh_tcpportnum($id) [$fh_port assignPort $fh_tcpsink($id) ]

    # setup socket connection between ME and FH
    $fh_tcpsa($id) set agent_addr_ [$fh_ipif addr]
    $fh_tcpsa($id) set dst_addr_ [$me_ipif($id) addr]
    $fh_tcpsa($id) set agent_port_ $fh_tcpportnum($id)
    $fh_tcpsa($id) set dst_port_ $me_tcpportnum($id)

    $me_tcpa($id) set agent_addr_  [$me_ipif($id) addr]
    $me_tcpa($id) set dst_addr_  [$fh_ipif addr]
    $me_tcpa($id) set agent_port_  $me_tcpportnum($id)
    $me_tcpa($id) set dst_port_  $fh_tcpportnum($id)


    # schedule data transfer
    set me_tcp_start($id) [$rvstart value]
    $ns at $me_tcp_start($id) "$me_ftp($id) start"
    $ns at [expr $me_tcp_start($id) + $opt(duration)]  "$me_ftp($id) stop"



    # setup ME position
    set me_pos($id) [new "Position/BM"]
    $me_node($id) addPosition $me_pos($id)
    $me_pos($id) setX_ [$rvposition value]
    $me_pos($id) setY_ 0.0
    #$ns at 1 "$me_pos ($id)setdest 0 100 10"    


}


###############################
#  CWND tracing
###############################

set tcptracer [new BaseTrace/Event]
$tcptracer attach $opt(tcpTrace)
$me_tcpa(1) eventtrace $tcptracer
$me_tcpa(1) attach $opt(tcpTrace)
$me_tcpa(1) trace cwnd_
$me_tcpa(1) trace nrexmit_
$me_tcpa(1) trace t_seqno_
#$me_tcpa(1) trace last_cwnd_action_
$me_tcpa(1) trace rtt_
$me_tcpa(1) trace srtt_
$me_tcpa(1) trace ssthresh_




###############################
#  Start Simulation
###############################

puts "Mac/802_11::SlotTime_  = [Mac/802_11 set SlotTime_]"


puts -nonewline "Simulating..."
$ns at [expr $opt(startmax) + $opt(duration) + 5] "finish; $ns halt"
$ns run
