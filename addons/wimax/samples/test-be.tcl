# Test for 802.16 scheduler.
# @author rouil
# @date 03/25/2007
# Test file for wimax
# Scenario: Communication between MN and Sink Node with MN attached to BS.
#           Using grep ^r out.res | grep MAC | grep -c cbr you can see the number of
#           mac packets received at the BS.
#           Using grep ^s out.res | grep MAC | grep -c cbr you can see the number of 
#           mac packets sent (200 packets). 
#           
#
## Topology scenario:
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


#check input parameters
if {$argc != 1} {
	puts ""
	puts "Wrong Number of Arguments! No arguments in this topology"
        puts "Syntax: ns test-be.tcl nbMNs"
	puts ""
	exit (1)
}

#dynamic libs
source ../../dynlibutils.tcl ;# Run with bash-script "test-be"
#source dynlibutils.tcl ;# Run without bash-script

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
set nb_mn [lindex $argv 0]				;# max number of mobile node
set packet_size	1500			;# packet size in bytes at CBR applications 
set output_dir .
set gap_size 0.2 ;#compute gap size between packets
set traffic_start 20
set traffic_stop  120
set simulation_stop 140
set diuc 7 ;#modulation for MNs

#define debug values
Module/802_16/MAC set debug_ 0
Module/802_16/MAC set rtg_ 20
Module/802_16/MAC set ttg_ 20
Module/802_16/MAC set frame_duration_ 0.005
Module/802_16/MAC set queue_length_ 500
Module/802_16/MAC set client_timeout_ 50 ;#to avoid BS disconnecting the SS
Module/MPhy/OFDM set g_ 0.25

#define coverage area for base station: 20m coverage 
Module/MPhy/OFDM set TxPower_ 0.025
#Phy/WirelessPhy set freq_ 2412e+6
#Phy/WirelessPhy set RXThresh_ 2.90781e-09
Module/MPhy/OFDM set RXThresh_ 2.025e-12 ;#500m radius
Module/MPhy/OFDM set CSThresh_ [expr 0.9*[Module/MPhy/OFDM set RXThresh_]]

# Parameter for wireless nodes
set opt(chan)           Module/DumbWirelessCh      ;# channel type
set opt(prop)           MPropagation/MFreeSpace     ;# radio-propagation model
set opt(spectral)       MSpectralMask/Rect         ;# spectral mask
set opt(netif)          Module/MPhy/OFDM           ;# network interface type
set opt(mac_bs)         Module/802_16/MAC/BS       ;# BS MAC type
set opt(mac_ss)         Module/802_16/MAC/SS       ;# SS MAC type
set opt(mac_wired)      Module/MAC/Generic         ;# mac for wired nodes
set opt(ifq)            Queue/DropTail/PriQueue    ;# interface queue type
set opt(ll)             Module/MLL                 ;# link layer type
#set opt(ant)            Antenna/OmniAntenna        ;# antenna model
set opt(ifqlen)         500             	         ;# max packet in ifq
set opt(ipif)           Module/IP/Interface        ;# routing protocol
set opt(ipr)           Module/IP/Routing          ;# ip routing

set opt(position)	Position/BM		   ;# position
set opt(x)		1100			   ;# X dimension of the topography
set opt(y)		1100			   ;# Y dimension of the topography

#Mac/802_11 set basicRate_ 11Mb
#Mac/802_11 set dataRate_ 11Mb
#Mac/802_11 set bandwidth_ 11Mb

#defines function for flushing and closing files
proc finish {} {
        global ns tf output_dir/out.tr
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
# create one CBR module for each SS in order to simulate each flow
for {set i 0} {$i < $nb_mn} {incr i} {
	set sinkNode_cbr($i)	[new Module/CBR]
}
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
for {set i 0} {$i < $nb_mn} {incr i} {
	$sinkNode addModule 6 $sinkNode_cbr($i)	0 "SNK_CBR$i "
}
$sinkNode addModule 5 $sinkNode_port	0 "SNK_PORT "
$sinkNode addModule 4 $sinkNode_ipr	0 "SNK_IPR  "
$sinkNode addModule 3 $sinkNode_ipif	0 "SNK_IPIF "
$sinkNode addModule 2 $sinkNode_ll	0 "SNK_LL   "
$sinkNode addModule 1 $sinkNode_mac	0 "SNK_MAC  "

for {set i 0} {$i < $nb_mn} {incr i} {
	set sinkNode_cbr_port($i) [$sinkNode_port assignPort $sinkNode_cbr($i)]
}

# Create stack
for {set i 0} {$i < $nb_mn} {incr i} {
	$sinkNode setConnection $sinkNode_cbr($i) $sinkNode_port	1
}
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
$bstation_ipr addRoute "0.0.2.0" "255.255.255.0" "0.0.2.1"

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

##################### creates the wireless nodes in the second address space.####################
for {set i 0} {$i < $nb_mn} {incr i} {
	set wl_node($i)		[$ns create-M_Node]
	set wl_node_cbr($i)		[new Module/CBR]
	set wl_node_port($i)	[new Module/Port/Map]
	set wl_node_ipr($i)		[new $opt(ipr)]
	set wl_node_ipif($i)	[new $opt(ipif)]
	set wl_node_ll($i)		[new $opt(ll)]
	set wl_node_mac($i)		[new $opt(mac_ss)]
	set wl_node_ifq($i)		[new $opt(ifq)]
	set wl_node_phy($i)		[new $opt(netif)]
	set wl_node_spectralmask($i) [new $opt(spectral)]
	
	$wl_node_cbr($i) set packetSize_ 1500
	$wl_node_cbr($i) set period_ 0.0005
	
	$wl_node_ipif($i) addr "0.0.2.[expr $i + 1]"
	$wl_node_ipif($i) subnet "255.255.255.0"
	$wl_node_ipr($i) defaultGateway "0.0.2.1"
	
	$wl_node_ifq($i) set limit_ $opt(ifqlen)
	$wl_node_mac($i) setIfq $wl_node_ifq($i)
	
	$wl_node_spectralmask($i) setFreq [Module/MPhy/OFDM set freq_]
	$wl_node_spectralmask($i) setBandwidth [Module/802_16/MAC set fbandwidth_]
	$wl_node_phy($i) setSpectralMask $wl_node_spectralmask($i)
	$wl_node_phy($i) setPropagation $propagation
	
	# Add modules
	$wl_node($i) addModule 7 $wl_node_cbr($i)	0 "WL_CBR  "
	$wl_node($i) addModule 6 $wl_node_port($i)	0 "WL_PORT "
	$wl_node($i) addModule 5 $wl_node_ipr($i)	0 "WL_IPR  "
	$wl_node($i) addModule 4 $wl_node_ipif($i)	0 "WL_IPIF "
	$wl_node($i) addModule 3 $wl_node_ll($i)	0 "WL_LL   "
	$wl_node($i) addModule 2 $wl_node_mac($i)	0 "WL_MAC  "
	$wl_node($i) addModule 1 $wl_node_phy($i)	0 "WL_PHY  "
	
	set wl_node_cbr_port($i) [$wl_node_port($i) assignPort $wl_node_cbr($i)]
	
	# Create stack
	$wl_node($i) setConnection $wl_node_cbr($i)	$wl_node_port($i)	1
	$wl_node($i) setConnection $wl_node_port($i)	$wl_node_ipr($i)	1
	$wl_node($i) setConnection $wl_node_ipr($i)	$wl_node_ipif($i)	1
	$wl_node($i) setConnection $wl_node_ipif($i)	$wl_node_ll($i)	1
	$wl_node($i) setConnection $wl_node_ll($i)	$wl_node_mac($i)	1
	$wl_node($i) setConnection $wl_node_mac($i)	$wl_node_phy($i)	1
	
	# Add to channel (media, the air)
	$wl_node($i) addToChannel $channel $wl_node_phy($i)    0
	
#$wl_node_mac($i) set-diuc [lindex $argv 0]
	$wl_node_mac($i) set-diuc $diuc
	$wl_node_mac($i) set-channel 0

}


##################### Positions ####################

##### Basestation #####
set bstation_pos [new $opt(position)]

$bstation addPosition $bstation_pos
$bstation_pos setX_ 550.0
$bstation_pos setY_ 550.0

##### Wireless nodes #####

for {set i 0} {$i < $nb_mn} {incr i} {

	set wl_node_pos($i) [new $opt(position)]
	
	$wl_node($i) addPosition $wl_node_pos($i)
	$wl_node_pos($i) setX_ [expr 340.0]
	$wl_node_pos($i) setY_ 550.0
}

# create the link between sink node and base station
set duplexlink [new Module/DuplexLink]	
$duplexlink bandwidth 100Mb
$duplexlink delay 0.001
$duplexlink qsize 50
$duplexlink settags "W_CH1"
$duplexlink connect $bstation $bstation_mac_wired 1 $sinkNode $sinkNode_mac 1

# setup socket connection between wireless node and sink node
for {set i 0} {$i < $nb_mn} {incr i} {
	$wl_node_cbr($i) set destAddr_ [$sinkNode_ipif addr]
	$wl_node_cbr($i) set destPort_ $sinkNode_cbr_port($i)
	
	$wl_node_cbr($i) set packetSize_ $packet_size
	$wl_node_cbr($i) set period_ $gap_size
	
	$sinkNode_cbr($i) set destAddr_ [$wl_node_ipif($i) addr]
	$sinkNode_cbr($i) set destPort_ $wl_node_cbr_port($i)
}

set diff 0.02
for {set i 0} {$i < $nb_mn} {incr i} {
#$ns at [expr $traffic_start+$i*$diff] "$wl_node_cbr($i) sendPkt"
	$ns at [expr $traffic_start+$i*$diff] "$wl_node_cbr($i) start"
	$ns at [expr $traffic_stop+$i*$diff] "$wl_node_cbr($i) stop"
}


$ns at $simulation_stop "finish; $ns halt"
puts "Starts simulation for $nb_mn mobile nodes..."
$ns run
puts "Simulation done."


