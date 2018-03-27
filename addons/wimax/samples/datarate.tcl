# Test script to evaluate datarate in 802.16 networks.
# Scenario: Communication between MN and Sink Node with MN attached to BS.
#
# Topology scenario:
#
#
#	        |-----|          
#	        | MN0 |                 ; 0.0.2.2
#	        |-----|        
#
#
#		  (^)
#		   |
#	    |--------------|		; 0.0.2.1
#           | Base Station |
#           |--------------|		; 0.0.1.1
#	    	   |
#	    	   |
#	     |-----------|
#            | Sink node | 		; 0.0.1.2
#            |-----------|
#
# Notes: 
# Traffic should not start before 25s for the following reasons:
# - Network Entry can be time consuming
#    - The time to discover the AP (i.e. receive DL_MAP) is fairly quick even
#      with channel scanning. In the order of few hundred ms.
#    - Default DCD/UCD interval is 5s. 
#    - Ranging/Registration is quick (<100ms)
#
# Network stacks
#
#             Wireless node                                                                       Sink node
#       +-----------------------+                                                          +----------------------+
#       |    7. CBR             |                                                          |    7. CBR            |
#       +-----------------------+                                                          +----------------------+
#       |    6. PORT            |                       Base Station                       |    6. PORT           |
#       +-----------------------+       +-------------------------------------------+      +----------------------+
#       |    5. IP/Routing      |       |              5. IP/Routing                |      |    5. IP/Routing     |
#       +-----------------------+       +-------------------------------------------+      +----------------------+
#       |    4. IP/Interface    |       |    4. IP/Interface   |   4. IP/Interface  |      |    4. IP/Interface   |
#       +-----------------------+       +-------------------------------------------+      +----------------------+
#       |    3. Link Layer      |       |    3. Link Layer     | 3. Link Layer      |      |    3. Link Layer     |
#       +-----------------------+       +-------------------------------------------+      +----------------------+
#       |    2. 802_16 SS MAC   |       |    2. 802_16 BS MAC  | 2. MMAC/Generic    |      |    2. MMAC/Generic   |
#       +-----------------------+       +-------------------------------------------+      +----------------------+
#       |    1. MPHY/OFDM       |       |    1. MPHY/OFDM      | 1. N/A             |      |    1. N/A            |
#       +-----------------------+       +-------------------------------------------+      +----------------------+
#                    |                               |                    \                          \
#       +-------------------------------------------------------+         +--------------------------+
#       |                  DumbWirelessChannel                  |         |       Duplex link        |
#       +-------------------------------------------------------+         +--------------------------+
#
#check input parameters
if {$argc != 2} {
	puts ""
	puts "Wrong Number of Arguments! 2 arguments for this script"
	puts "Usage: ns datarate.tcl profile cyclic_prefix " 
        puts "modulation: 1-7"
        puts "cyclic_prefix: 0.25, 0.125, 0.0625, 0.03125"
	exit
}

#dynamic libs
source ../../../dynlibutils.tcl ;# Run without bash-script
#source ../../../../../samples/dynlibutils.tcl

dynlibload Miracle
dynlibload miraclelink
dynlibload miraclecbr
dynlibload miracleport
dynlibload MiracleIp
dynlibload MiracleIpRouting
dynlibload mphy
dynlibload mmac
dynlibload wimax
dynlibload mll
dynlibload MiracleWirelessCh
dynlibload MiracleBasicMovement

dynlibload Trace
dynlibload wimaxtracer
dynlibload mlltracer

# set global variables
set output_dir .
set traffic_start 25
set traffic_stop  35
set simulation_stop 50

# Configure Wimax
WimaxScheduler/BS set dlratio_ 0.2zzz
Module/802_16/MAC set debug_ 0
Module/802_16/MAC set frame_duration_ 0.005 ;#0.020
Module/802_16/MAC set queue_length_ 500
Module/802_16/MAC set client_timeout_ 50 ;#to avoid BS disconnecting the SS

#define coverage area for base station: 500m coverage 
Module/MPhy/OFDM set g_ [lindex $argv 1]
Module/MPhy/OFDM set TxPower_ 0.025
Module/MPhy/OFDM set RXThresh_ 2.025e-12 ;# 500m radius
Module/MPhy/OFDM set CSThresh_ [expr 0.9*[Module/MPhy/OFDM set RXThresh_]]

# Parameter for nodes (both wired and wireless)
set opt(chan)           Module/DumbWirelessCh      ;# channel type
set opt(prop)           MPropagation/MFreeSpace    ;# radio-propagation model
set opt(spectral)	MSpectralMask/Rect         ;# spectral mask
set opt(netif)          Module/MPhy/OFDM           ;# network interface type
set opt(mac_bs)         Module/802_16/MAC/BS       ;# BS MAC type
set opt(mac_ss)         Module/802_16/MAC/SS       ;# SS MAC type
set opt(mac_wired)	Module/MAC/Generic         ;# mac for wired nodes
set opt(ifq)            Queue/DropTail/PriQueue    ;# interface queue type
set opt(ifqlen)         50              	   ;# max packet in ifq
set opt(ll)             Module/MLL                 ;# link layer type
set opt(ipif)		Module/IP/Interface        ;# ip interface
set opt(ipr)		Module/IP/Routing          ;# ip routing

set opt(position)	Position/BM		   ;# position
set opt(x)		1100			   ;# X dimension of the topography
set opt(y)		1100			   ;# Y dimension of the topography

#defines function for flushing and closing files
proc finish {} {
        global ns tf output_dir nb_mn
        $ns flush-trace
        close $tf
#exit 0
}

#create the simulator
set ns [new Simulator]
$ns use-Miracle

#open file for trace
set tf [open $output_dir/out.res w]
$ns trace-all $tf
#puts "Output file configured"

# Set up channel
set channel [new $opt(chan)]
$channel setTag "CHA"
set propagation [new $opt(prop)]

##################### creates the sink node in the first address space.####################
set sinkNode		[$ns create-M_Node]
set sinkNode_cbr	[new Module/CBR]
set sinkNode_port	[new Module/Port/Map]
set sinkNode_ipr	[new $opt(ipr)]
set sinkNode_ipif	[new $opt(ipif)]
set sinkNode_ll		[new $opt(ll)]
set sinkNode_mac	[new $opt(mac_wired)]
set sinkNode_ifq	[new $opt(ifq)]

$sinkNode_ipif addr "0.0.1.2"
$sinkNode_ipif subnet "255.255.255.0"
$sinkNode_ipr addRoute "0.0.1.0" "255.255.255.0" "0.0.1.2"
$sinkNode_ipr defaultGateway "0.0.1.1"

$sinkNode_ifq set limit_ $opt(ifqlen)
$sinkNode_mac setIfq $sinkNode_ifq

# Add modules
$sinkNode addModule 6 $sinkNode_cbr	0 "SNK_CBR  "
$sinkNode addModule 5 $sinkNode_port	0 "SNK_PORT "
$sinkNode addModule 4 $sinkNode_ipr	0 "SNK_IPR  "
$sinkNode addModule 3 $sinkNode_ipif	0 "SNK_IPIF "
$sinkNode addModule 2 $sinkNode_ll	0 "SNK_LL   "
$sinkNode addModule 1 $sinkNode_mac	0 "SNK_MAC  "


set sinkNode_cbr_port [$sinkNode_port assignPort $sinkNode_cbr]

# Create stack
$sinkNode setConnection $sinkNode_cbr $sinkNode_port	1
$sinkNode setConnection $sinkNode_port $sinkNode_ipr	1
$sinkNode setConnection $sinkNode_ipr $sinkNode_ipif	1
$sinkNode setConnection $sinkNode_ipif $sinkNode_ll	1
$sinkNode setConnection $sinkNode_ll $sinkNode_mac	1

##################### creates the basestation in the first & second address space.####################
set bstation		[$ns create-M_Node]
set bstation_ipr	[new $opt(ipr)]
set bstation_ipif_wl	[new $opt(ipif)]
set bstation_ipif_wired	[new $opt(ipif)]

set bstation_ll_wl	[new $opt(ll)]
set bstation_ll_wired	[new $opt(ll)]

set bstation_mac_wl	[new $opt(mac_bs)]
set bstation_ifq_wl	[new $opt(ifq)]

set bstation_mac_wired	[new $opt(mac_wired)]
set bstation_ifq_wired	[new $opt(ifq)]

set bstation_phy_wl	[new $opt(netif)]

set bstation_spectralmask [new $opt(spectral)]

$bstation_ipif_wl  addr   "0.0.2.1"
$bstation_ipif_wl  subnet "255.255.255.0"
$bstation_ipr addRoute "0.0.2.0" "255.255.255.0" "0.0.2.2"

$bstation_ipif_wired  addr   "0.0.1.1"
$bstation_ipif_wired  subnet "255.255.255.0"
$bstation_ipr addRoute "0.0.1.0" "255.255.255.0" "0.0.1.2"

$bstation_ifq_wl set limit_ $opt(ifqlen)
$bstation_mac_wl setIfq $bstation_ifq_wl

$bstation_ifq_wired set limit_ $opt(ifqlen)
$bstation_mac_wired setIfq $bstation_ifq_wired

$bstation_spectralmask setFreq [Module/MPhy/OFDM set freq_]
$bstation_spectralmask setBandwidth [Module/802_16/MAC set fbandwidth_]
$bstation_phy_wl setSpectralMask $bstation_spectralmask
$bstation_phy_wl setPropagation $propagation

# Add modules
$bstation addModule 5 $bstation_ipr		0 "BS_IPR    "
$bstation addModule 4 $bstation_ipif_wl		0 "BS_IPIF_WL"
$bstation addModule 4 $bstation_ipif_wired	0 "BS_IPIF_WR"
$bstation addModule 3 $bstation_ll_wl		0 "BS_LL_WL  "
$bstation addModule 3 $bstation_ll_wired	0 "BS_LL_WR  "
$bstation addModule 2 $bstation_mac_wl		0 "BS_MAC_WL "
$bstation addModule 2 $bstation_mac_wired	0 "BS_MAC_WR "
$bstation addModule 1 $bstation_phy_wl		0 "BS_PHY_WL "

# Create stack
$bstation setConnection $bstation_ipr		$bstation_ipif_wl	1
$bstation setConnection $bstation_ipr		$bstation_ipif_wired	1
$bstation setConnection $bstation_ipif_wl	$bstation_ll_wl		1
$bstation setConnection $bstation_ipif_wired	$bstation_ll_wired	1
$bstation setConnection $bstation_ll_wl		$bstation_mac_wl	1
$bstation setConnection $bstation_ll_wired	$bstation_mac_wired	1
$bstation setConnection $bstation_mac_wl	$bstation_phy_wl	1

# Add to channel (media, the air)
$bstation addToChannel $channel $bstation_phy_wl    0

$bstation_mac_wl set-channel 0

##################### creates the wireless node in the second address space.####################
set wl_node		[$ns create-M_Node]
set wl_node_cbr		[new Module/CBR]
set wl_node_port	[new Module/Port/Map]
set wl_node_ipr		[new $opt(ipr)]
set wl_node_ipif	[new $opt(ipif)]
set wl_node_ll		[new $opt(ll)]
set wl_node_mac		[new $opt(mac_ss)]
set wl_node_ifq		[new $opt(ifq)]
set wl_node_phy		[new $opt(netif)]
set wl_node_spectralmask [new $opt(spectral)]

$wl_node_cbr set packetSize_ 1500
$wl_node_cbr set period_ 0.0005

$wl_node_ipif addr "0.0.2.2"
$wl_node_ipif subnet "255.255.255.0"
$wl_node_ipr defaultGateway "0.0.2.1"

$wl_node_ifq set limit_ $opt(ifqlen)
$wl_node_mac setIfq $wl_node_ifq

$wl_node_spectralmask setFreq [Module/MPhy/OFDM set freq_]
$wl_node_spectralmask setBandwidth [Module/802_16/MAC set fbandwidth_]
$wl_node_phy setSpectralMask $wl_node_spectralmask
$wl_node_phy setPropagation $propagation

# Add modules
$wl_node addModule 7 $wl_node_cbr	0 "WL_CBR  "
$wl_node addModule 6 $wl_node_port	0 "WL_PORT "
$wl_node addModule 5 $wl_node_ipr	0 "WL_IPR  "
$wl_node addModule 4 $wl_node_ipif	0 "WL_IPIF "
$wl_node addModule 3 $wl_node_ll	0 "WL_LL   "
$wl_node addModule 2 $wl_node_mac	0 "WL_MAC  "
$wl_node addModule 1 $wl_node_phy	0 "WL_PHY  "

set wl_node_cbr_port [$wl_node_port assignPort $wl_node_cbr]

# Create stack
$wl_node setConnection $wl_node_cbr	$wl_node_port	1
$wl_node setConnection $wl_node_port	$wl_node_ipr	1
$wl_node setConnection $wl_node_ipr	$wl_node_ipif	1
$wl_node setConnection $wl_node_ipif	$wl_node_ll	1
$wl_node setConnection $wl_node_ll	$wl_node_mac	1
$wl_node setConnection $wl_node_mac	$wl_node_phy	1

# Add to channel (media, the air)
$wl_node addToChannel $channel $wl_node_phy    0

$wl_node_mac set-diuc [lindex $argv 0]
$wl_node_mac set-channel 0

##################### Positions ####################

##### Basestation #####
set bstation_pos [new $opt(position)]

$bstation addPosition $bstation_pos
$bstation_pos setX_ 550.0
$bstation_pos setY_ 550.0

##### Wireless node #####
set wl_node_pos [new $opt(position)]

$wl_node addPosition $wl_node_pos
$wl_node_pos setX_ 400.0
$wl_node_pos setY_ 550.0

# create the link between sink node and base station
set duplexlink [new Module/DuplexLink]	
$duplexlink bandwidth 100Mb
$duplexlink delay 0.001
$duplexlink qsize 50
$duplexlink settags "W_CH1"
$duplexlink connect $bstation $bstation_mac_wired 1 $sinkNode $sinkNode_mac 1

# setup socket connection between wireless node and sink node
$wl_node_cbr set destAddr_ [$sinkNode_ipif addr]
$wl_node_cbr set destPort_ $sinkNode_cbr_port

$sinkNode_cbr set destAddr_ [$wl_node_ipif addr]
$sinkNode_cbr set destPort_ $wl_node_cbr_port

#Schedule start/stop of traffic
#$ns at $traffic_start "$wl_node_cbr sendPkt"
$ns at $traffic_start "$wl_node_cbr start"
$ns at $traffic_stop "$wl_node_cbr stop"

$ns at $simulation_stop "finish; $ns halt"
puts "Starts simulation"
$ns run
puts "Simulation done."
