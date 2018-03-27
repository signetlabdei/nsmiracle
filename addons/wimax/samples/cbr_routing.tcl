# Test script to evaluate datarate in 802.16 networks.
# @author rouil
# Scenario: CBR communication between MN and Sink Node with MN attached to BS.

#
# Topology scenario:
#
#
#	        |-----|          
#	        | MN0 |                 ; 1.0.1 
#	        |-----|        
#
#
#		  (^)
#		   |
#	    |--------------|
#           | Base Station | 		; 1.0.0
#           |--------------|
#	    	   |
#	    	   |
#	     |-----------|
#            | Sink node | 		; 0.0.0
#            |-----------|
#
# Notes: 
# Traffic should not start before 25s for the following reasons:
# - Network Entry can be time consuming
#    - The time to discover the AP (i.e. receive DL_MAP) is fairly quick even
#      with channel scanning. In the order of few hundred ms.
#    - Default DCD/UCD interval is 5s. 
#    - Ranging/Registration is quick (<100ms)
# - Routing protocol used here is DSDV, with default updates interval of 15s.



source dynlibutils.tcl

dynlibload Miracle 
dynlibload miraclelink
dynlibload miraclecbr
#dynlibload miraclepong
dynlibload MiracleIp
dynlibload MiracleIpRouting
dynlibload mphy
dynlibload wimax
dynlibload mll
dynlibload MiracleWirelessCh
dynlibload MiracleBasicMovement

dynlibload Trace
dynlibload wimaxtracer
dynlibload mlltracer


#check input parameters
#if {$argc != 2} {
#	puts ""
#	puts "Wrong Number of Arguments! 2 arguments for this script"
#	puts "Usage: ns datarate.tcl profile cyclic_prefix "
#        puts "modulation: 1-7"
#        puts "cyclic_prefix: 0.25, 0.125, 0.0625, 0.03125"
#	exit 
#}



# set global variables
set output_dir .
set traffic_start 25
set traffic_stop  35
set simulation_stop 50

# Configure Wimax
#WimaxScheduler/BS set dlratio_ 0.2
#Module/802_16/MAC set debug_ 0
#Module/802_16/MAC set frame_duration_ 0.005 ;#0.020
#Module/802_16/MAC set queue_length_ 500
#Module/802_16/MAC set client_timeout_ 50 ;#to avoid BS disconnecting the SS

#define coverage area for base station: 500m coverage 
#Module/MPhy/OFDM set g_ 0.25
#Module/MPhy/OFDM set Pt_ 0.025
#Module/MPhy/OFDM set RXThresh_ 2.025e-12 ;# 500m radius
#Module/MPhy/OFDM set CSThresh_ [expr 0.9*[Phy/WirelessPhy set RXThresh_]]




Module/MPhy/OFDM set debug_   0
Module/802_16/MAC set debug_   0
Module/MPhy/OFDM set g_       0.25

Module/WirelessCh set freq_ 2115000000
Module/WirelessCh set L_ 1
Module/WirelessCh set CSThresh_ 1.0



# Set up channel
set channel [new Module/DumbWirelessCh]
$channel setTag "CHA"


set propagation [new MPropagation/MFreeSpace]

# Parameter for wireless nodes
#set opt(chan)           Channel/WirelessChannel    ;# channel type
#set opt(prop)           Propagation/TwoRayGround   ;# radio-propagation model
#set opt(netif)          Phy/WirelessPhy/OFDM       ;# network interface type
#set opt(mac)            Mac/802_16                 ;# MAC type
#set opt(ifq)            Queue/DropTail/PriQueue    ;# interface queue type
#set opt(ll)             LL                         ;# link layer type
#set opt(ant)            Antenna/OmniAntenna        ;# antenna model
#set opt(ifqlen)         50              	   ;# max packet in ifq
#set opt(adhocRouting)   DSDV                       ;# routing protocol

#set opt(x)		1100			   ;# X dimension of the topography
#set opt(y)		1100			   ;# Y dimension of the topography

#defines function for flushing and closing files
proc finish {} {
	global ns tf opt
        puts "done!"

	$ns flush-trace
	close $tf
        puts "Tracefile     : $opt(tracefile)"
}

#create the simulator
#set ns [new Simulator]
#$ns use-newtrace
set ns [new Simulator]
$ns use-Miracle

#open file for trace
set opt(tracefile)   "/tmp/${argv0}.tr"
set tf [open $opt(tracefile) w]
$ns trace-all $tf
#puts "Output file configured"


#-------?????????????????????<create the topography
#set topo [new Topography]
#$topo load_flatgrid $opt(x) $opt(y)
#puts "Topology created"



#------------------???????????????????+ set up for hierarchical routing (needed for routing over a basestation)
#$ns node-config -addressType hierarchical
#AddrParams set domain_num_ 2          			;# domain number
#lappend cluster_num 1 1            			;# cluster number for each domain 
#AddrParams set cluster_num_ $cluster_num
#lappend eilastlevel 1 2 		;# number of nodes for each cluster (1 for sink and one for mobile node + base station
#AddrParams set nodes_num_ $eilastlevel
#puts "Configuration of hierarchical addressing done"

# Create God
#----------------------???????????????????????create-god 2	

#creates the sink node in first address space.
#set sinkNode [$ns node 0.0.0]
#puts "sink node created"

#creates the Access Point (Base station)
#$ns node-config -adhocRouting $opt(adhocRouting) \
#                 -llType $opt(ll) \
#                 -macType Mac/802_16/BS \
#                 -ifqType $opt(ifq) \
#                 -ifqLen $opt(ifqlen) \
#                 -antType $opt(ant) \
#                 -propType $opt(prop)    \
#                 -phyType $opt(netif) \
#                 -channel [new $opt(chan)] \
#                 -topoInstance $topo \
#                 -wiredRouting ON \
#                 -agentTrace ON \
#                 -routerTrace ON \
#                 -macTrace ON  \
#                 -movementTrace OFF
#puts "Configuration of base station"

################## Create basestation 1 #######################
set bs1 [$ns create-M_Node]
set bs1_cbr [new Module/CBR]
set bs1_ipr     [new Module/IP/Routing]
set bs1_ipif_1	[new Module/IP/Interface]
set bs1_ipif_2	[new Module/IP/Interface]

set bs1_ll_2	[new Module/MLL]
set bs1_gmac	[new Module/MAC/Generic]
$bs1_gmac setIfq [new Queue/DropTail/PriQueue]

$bs1_ipif_1  addr   "0.0.1.1"
$bs1_ipif_1  subnet "255.255.255.0"
$bs1_ipr addRoute "0.0.1.0" "255.255.255.0" "0.0.1.1"

$bs1_ipif_2  addr   "0.0.2.1"
$bs1_ipif_2  subnet "255.255.255.0"
$bs1_ipr addRoute "0.0.2.0" "255.255.255.0" "0.0.2.1"
$bs1_ipr addRoute "0.0.3.0" "255.255.255.0" "0.0.2.2"

set bs1_ll [new Module/MLL]
set bs1_mac [new Module/802_16/MAC/BS]
$bs1_mac setIfq [new Queue/DropTail/PriQueue]
set bs1_phy [new Module/MPhy/OFDM]

# Create spectral masks
set bs1_spectralmask [new MSpectralMask/Rect]
$bs1_spectralmask setFreq 2115000000
$bs1_spectralmask setBandwidth 5000000
$bs1_phy setSpectralMask $bs1_spectralmask
$bs1_phy setPropagation $propagation

# Add modules
$bs1 addModule 6 $bs1_cbr	0 "BS1_CBR"
$bs1 addModule 5 $bs1_ipr	0 "BS1_IPR"
$bs1 addModule 4 $bs1_ipif_1	0 "BS1_IP1"
$bs1 addModule 4 $bs1_ipif_2	0 "BS1_IP2"
$bs1 addModule 3 $bs1_ll	0 "BS1_LL1"
$bs1 addModule 3 $bs1_ll_2	0 "BS1_LL2"
$bs1 addModule 2 $bs1_mac	0 "BS1_MAC"
$bs1 addModule 2 $bs1_gmac	0 "BS1_MAC2"
$bs1 addModule 1 $bs1_phy	0 "BS1_PHY"

# Create stack
$bs1 setConnection $bs1_cbr	$bs1_ipr	1
$bs1 setConnection $bs1_ipr	$bs1_ipif_1	1
$bs1 setConnection $bs1_ipr	$bs1_ipif_2	1
$bs1 setConnection $bs1_ipif_1	$bs1_ll		1
$bs1 setConnection $bs1_ipif_2	$bs1_ll_2	1
$bs1 setConnection $bs1_ll_2	$bs1_gmac	1
$bs1 setConnection $bs1_ll	$bs1_mac	1
$bs1 setConnection $bs1_mac	$bs1_phy	1

# Add to channel (media, the air)
$bs1 addToChannel $channel $bs1_phy    0

#$bs1_mac set-channel 0















#set bstation [$ns node 1.0.0]  
#$bstation random-motion 0
#provide some co-ord (fixed) to base station node
#$bstation set X_ 550.0
#$bstation set Y_ 550.0
#$bstation set Z_ 0.0
#[$bstation set mac_(0)] set-channel 0
#puts "Base-Station node created"

# creation of the mobile nodes
#$ns node-config -macType Mac/802_16/SS \
#                -wiredRouting OFF \
#                -macTrace ON  				;# Mobile nodes cannot do routing.

#set wl_node [$ns node 1.0.1] 	;# create the node with given @.	
#$wl_node random-motion 0			;# disable random motion
#--------------???????????????????+$wl_node base-station [AddrParams addr2id [$bstation node-addr]] ;#attach mn to basestation
#compute position of the node
#$wl_node set X_ 400.0
#$wl_node set Y_ 550.0
#$wl_node set Z_ 0.0
#puts "wireless node created ..."
#[$wl_node set mac_(0)] set-diuc [lindex $argv 0]

#[$wl_node set mac_(0)] set-channel 0


################## Create mobilenode 1 #######################
set mn1 [$ns create-M_Node]

set mn1_cbr [new Module/CBR]

set mn1_ipr     [new Module/IP/Routing]
set mn1_ipif	[new Module/IP/Interface]

$mn1_ipif addr "0.0.1.2"
$mn1_ipif subnet "255.255.255.0"
$mn1_ipr defaultGateway "0.0.1.1"

set mn1_ll [new Module/MLL]
set mn1_mac [new Module/802_16/MAC/SS]
$mn1_mac setIfq [new Queue/DropTail/PriQueue]
set mn1_phy [new Module/MPhy/OFDM]

# Create spectral masks
set mn1_spectralmask [new MSpectralMask/Rect]
$mn1_spectralmask setFreq 2115000000
$mn1_spectralmask setBandwidth 5000000
$mn1_phy setSpectralMask $mn1_spectralmask
$mn1_phy setPropagation $propagation

# Add modules
$mn1 addModule 6 $mn1_cbr	0 "MN1_CBR"
$mn1 addModule 5 $mn1_ipr	0 "MN1_IPR"
$mn1 addModule 4 $mn1_ipif	0 "MN1_IPIF"
$mn1 addModule 3 $mn1_ll	0 "MN1_LL"
$mn1 addModule 2 $mn1_mac	0 "MN1_MAC"
$mn1 addModule 1 $mn1_phy	0 "MN1_PHY"

# Create stack
$mn1 setConnection $mn1_cbr  $mn1_ipr  1
$mn1 setConnection $mn1_ipr  $mn1_ipif  1
$mn1 setConnection $mn1_ipif  $mn1_ll  1
$mn1 setConnection $mn1_ll  $mn1_mac  1
$mn1 setConnection $mn1_mac  $mn1_phy  1

# Add to channel (media, the air)
$mn1 addToChannel $channel $mn1_phy    0



#$mn1_mac set-channel 0


##################### Create the wired node
#set wn1 [$ns create-M_Node]
#set wn1_pong [new Module/Pong]

#set wn1_ip   [new Module/IP]
#$wn1_ip  addr   "10.0.0.3"
#$wn1_ip  subnet "255.255.0.0"

#set mn1_ll [new Module/MLL]

# Add modules
#$mn1 addModule 2 $mn1_pong	0 "MN1_Pong "
#$mn1 addModule 1 $mn1_ip	0 "MN1_IP   "

# Create stack
#$mn1 setConnection $mn1_pong  $mn1_ip  1
set wn1    	[$ns create-M_Node]
set wn1_cbr	[new Module/CBR]
set wn1_ipr     [new Module/IP/Routing]
set wn1_ipif	[new Module/IP/Interface]
set wn1_ll	[new Module/MLL]
set wn1_mac	[new Module/MAC/Generic]
$wn1_mac setIfq [new Queue/DropTail/PriQueue]

$wn1_ipif addr "0.0.3.2"
$wn1_ipif subnet "255.255.255.0"
$wn1_ipr addRoute "0.0.3.0" "255.255.255.0" "0.0.3.2"
$wn1_ipr defaultGateway "0.0.3.1"


$wn1 addModule 5 $wn1_cbr    0 "WN1_CBR"
$wn1 addModule 4 $wn1_ipr    0 "WN1_IPR"
$wn1 addModule 3 $wn1_ipif 0 "WN1_IPIF"
$wn1 addModule 2 $wn1_ll 0 "WN1_LL"
$wn1 addModule 1 $wn1_mac 0 "WN1_MAC"

$wn1 setConnection $wn1_cbr $wn1_ipr   1
$wn1 setConnection $wn1_ipr $wn1_ipif     1
$wn1 setConnection $wn1_ipif $wn1_ll     1
$wn1 setConnection $wn1_ll $wn1_mac     1



############# create wired node 2
set wn2    	[$ns create-M_Node]
set wn2_ipr     [new Module/IP/Routing]
set wn2_ipif	[new Module/IP/Interface]
set wn2_ll	[new Module/MLL]
set wn2_mac	[new Module/MAC/Generic]
$wn2_mac setIfq [new Queue/DropTail/PriQueue]

set wn2_ipif2	[new Module/IP/Interface]
set wn2_ll2	[new Module/MLL]
set wn2_mac2	[new Module/MAC/Generic]
$wn2_mac2 setIfq [new Queue/DropTail/PriQueue]


$wn2_ipif addr "0.0.2.2"
$wn2_ipif subnet "255.255.255.0"
$wn2_ipr addRoute "0.0.2.0" "255.255.255.0" "0.0.2.2"
$wn2_ipr addRoute "0.0.1.0" "255.255.255.0" "0.0.2.1"

$wn2_ipif2 addr "0.0.3.1"
$wn2_ipif2 subnet "255.255.255.0"
$wn2_ipr addRoute "0.0.3.0" "255.255.255.0" "0.0.3.1"

$wn2 addModule 4 $wn2_ipr    0 "WN2_IPR"
$wn2 addModule 3 $wn2_ipif 0 "WN2_IPIF"
$wn2 addModule 3 $wn2_ipif2 0 "WN2_IPIF2"
$wn2 addModule 2 $wn2_ll 0 "WN2_LL"
$wn2 addModule 2 $wn2_ll2 0 "WN2_LL2"
$wn2 addModule 1 $wn2_mac 0 "WN2_MAC"
$wn2 addModule 1 $wn2_mac2 0 "WN2_MAC2"

$wn2 setConnection $wn2_ipr $wn2_ipif     1
$wn2 setConnection $wn2_ipr $wn2_ipif2     1
$wn2 setConnection $wn2_ipif $wn2_ll     1
$wn2 setConnection $wn2_ipif2 $wn2_ll2     1
$wn2 setConnection $wn2_ll $wn2_mac     1
$wn2 setConnection $wn2_ll2 $wn2_mac2     1



# create the link between bs and wired node
set link [new Module/DuplexLink]	
$link bandwidth 1000000
$link delay 0.0001
$link qsize 10
$link settags "W_CH1"
$link connect $bs1 $bs1_gmac 1 $wn2 $wn2_mac 1

set link2 [new Module/DuplexLink]	
$link2 bandwidth 1000000
$link2 delay 0.0001
$link2 qsize 10
$link2 settags "W_CH2"
$link2 connect $wn2 $wn2_mac2 1 $wn1 $wn1_mac 1








#create source traffic
#Create a UDP agent and attach it to node n0
#set udp [new Agent/UDP]
#$udp set packetSize_ 1500
#$ns attach-agent $wl_node $udp

# Create a CBR traffic source and attach it to udp0
#set cbr [new Application/Traffic/CBR]
#$cbr set packetSize_ 1500
#$cbr set interval_ 0.0005
#$cbr attach-agent $udp

#create an sink into the sink node

# Create the Null agent to sink traffic
#set null [new Agent/Null] 
#$ns attach-agent $sinkNode $null

# Attach the 2 agents
#$ns connect $udp $null

# create the link between sink node and base station
#$ns duplex-link $sinkNode $bstation 100Mb 1ms DropTail


##### Basestation 1 #####
set bs1_pos [new "Position/BM"]
$bs1_pos set debug_ 11
$bs1 addPosition $bs1_pos
$bs1_pos setX_ 250
$bs1_pos setY_ 250


##### Mobilestation 1 #####
set mn1_pos [new "Position/BM"]
$mn1_pos set debug_ 11
$mn1 addPosition $mn1_pos
$mn1_pos setX_ 240
$mn1_pos setY_ 240


puts "dest addr [$wn1_ipif addr]"
$mn1_cbr set destAddr_ [$wn1_ipif addr]
$mn1_cbr set destPort_ 0
#Schedule start/stop of traffic
set stop 3
$ns at 0.5 "$mn1_cbr sendPkt"
$ns at 1 "finish; $ns halt"



for {set t 0} {$t < $stop} {set t [expr $t + ($stop / 40.0)  ]} {
    $ns at $t "puts -nonewline . ; flush stdout"
}


puts -nonewline "Simulating"
$ns run