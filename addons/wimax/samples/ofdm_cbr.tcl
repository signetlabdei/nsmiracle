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

# CBR sample

# Two nodes are created, with only one CBR module
# per layer. A unidirectional Module/Link connects the two nodes.
# A single CBR flow is started from one node to the other.

source dynlibutils.tcl

dynlibload Miracle
dynlibload miraclelink
dynlibload miraclecbr
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

proc finish {} {
	global ns tf opt
        puts "done!"

	$ns flush-trace
	close $tf
        puts "Tracefile     : $opt(tracefile)"
}



set ns [new Simulator]
$ns use-Miracle

set opt(tracefile)   "/tmp/${argv0}.tr"
set tf [open $opt(tracefile) w]
$ns trace-all $tf


# Set defines
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










################## Create basestation 1 #######################
set bs1 [$ns create-M_Node]

set bs1_cbr [new Module/CBR]

set bs1_ip   [new Module/IP]
$bs1_ip  addr   "10.0.0.1"
$bs1_ip  subnet "255.255.0.0"

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
$bs1 addModule 5 $bs1_cbr	0 "BS1_CBR  "
$bs1 addModule 4 $bs1_ip	0 "BS1_IP   "
$bs1 addModule 3 $bs1_ll	0 "BS1_LL   "
$bs1 addModule 2 $bs1_mac	0 "BS1_MAC  "
$bs1 addModule 1 $bs1_phy	0 "BS1_PHY  "

# Create stack
$bs1 setConnection $bs1_cbr	$bs1_ip		1
$bs1 setConnection $bs1_ip	$bs1_ll		1
$bs1 setConnection $bs1_ll	$bs1_mac	1
$bs1 setConnection $bs1_mac	$bs1_phy	1

# Add to channel (media, the air)
$bs1 addToChannel $channel $bs1_phy    0




################## Create basestation 2 #######################
set bs2 [$ns create-M_Node]

set bs2_cbr [new Module/CBR]

set bs2_ip   [new Module/IP]
$bs2_ip  addr   "10.0.0.3"
$bs2_ip  subnet "255.255.0.0"

set bs2_ll [new Module/MLL]
set bs2_mac [new Module/802_16/MAC/BS]
$bs2_mac setIfq [new Queue/DropTail/PriQueue]
set bs2_phy [new Module/MPhy/OFDM]

# Create spectral masks
set bs2_spectralmask [new MSpectralMask/Rect]
$bs2_spectralmask setFreq 2115000000
$bs2_spectralmask setBandwidth 5000000
$bs2_phy setSpectralMask $bs2_spectralmask
$bs2_phy setPropagation $propagation

# Add modules
$bs2 addModule 5 $bs2_cbr	0 "BS2_CBR  "
$bs2 addModule 4 $bs2_ip	0 "BS2_IP   "
$bs2 addModule 3 $bs2_ll	0 "BS2_LL   "
$bs2 addModule 2 $bs2_mac	0 "BS2_MAC  "
$bs2 addModule 1 $bs2_phy	0 "BS2_PHY  "

# Create stack
$bs2 setConnection $bs2_cbr	$bs2_ip		1
$bs2 setConnection $bs2_ip	$bs2_ll		1
$bs2 setConnection $bs2_ll	$bs2_mac	1
$bs2 setConnection $bs2_mac	$bs2_phy	1

# Add to channel (media, the air)
$bs2 addToChannel $channel $bs2_phy    0





################## Create mobilenode 1 #######################
set mn1 [$ns create-M_Node]

set mn1_cbr [new Module/CBR]

set mn1_ip   [new Module/IP]
$mn1_ip  addr   "10.0.0.2"
$mn1_ip  subnet "255.255.0.0"

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
$mn1 addModule 5 $mn1_cbr	0 "MN1_CBR  "
$mn1 addModule 4 $mn1_ip	0 "MN1_IP   "
$mn1 addModule 3 $mn1_ll	0 "MN1_LL   "
$mn1 addModule 2 $mn1_mac	0 "MN1_MAC  "
$mn1 addModule 1 $mn1_phy	0 "MN1_PHY  "

# Create stack
$mn1 setConnection $mn1_cbr  $mn1_ip  1
$mn1 setConnection $mn1_ip  $mn1_ll  1
$mn1 setConnection $mn1_ll  $mn1_mac  1
$mn1 setConnection $mn1_mac  $mn1_phy  1

# Add to channel (media, the air)
$mn1 addToChannel $channel $mn1_phy    0






################# Positions #################
##### Basestation 1 #####
set bs1_pos [new "Position/BM"]
$bs1_pos set debug_ 11
$bs1 addPosition $bs1_pos
$bs1_pos setX_ 270
$bs1_pos setY_ 270

##### Basestation 2 #####
set bs2_pos [new "Position/BM"]
$bs2_pos set debug_ 11
$bs2 addPosition $bs2_pos
$bs2_pos setX_ 290
$bs2_pos setY_ 290

##### Mobilestation 1 #####
set mn1_pos [new "Position/BM"]
$mn1_pos set debug_ 11
$mn1 addPosition $mn1_pos
$mn1_pos setX_ 240
$mn1_pos setY_ 240


$mn1_cbr set destAddr_ [$bs1_ip addr]
set stop 3
$ns at 0.5 "$mn1_cbr sendPkt"
$ns at 1 "finish; $ns halt"



for {set t 0} {$t < $stop} {set t [expr $t + ($stop / 40.0)  ]} {
    $ns at $t "puts -nonewline . ; flush stdout"
}


puts -nonewline "Simulating"
$ns run
