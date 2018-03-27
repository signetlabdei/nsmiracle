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

# BPSK + ALOHA sample


#	+-------------------------+	+-------------------------+
#	| 	7. CBR             |	| 	7. CBR             |
#	+-------------------------+	+-------------------------+
#	| 	6. Port            |	| 	6. Port            |
#	+-------------------------+	+-------------------------+
#	| 	5. IP Routing      |	| 	5. IP Routing      |
#	+-------------------------+	+-------------------------+
#	| 	4. IP Interface    |	| 	4.  IP Interface   |
#	+-------------------------+	+-------------------------+
#	| 	3. MLL Link Layer  |	| 	3. MLL Link Layer  |
#	+-------------------------+	+-------------------------+
#	| 	2. MMAC/ALOHA/ADV  |	| 	2. MMAC/ALOHA/ADV  |
#	+-------------------------+	+-------------------------+
#	| 1. MPHY/BPSK/Underwater |	| 1. MPHY/BPSK/Underwater |
#	+-------------------------+	+-------------------------+
#                    |                          |
#	+-----------------------------------------------------+
#      |                  UnderwaterChannel                  |
#      +-----------------------------------------------------+


# Module libraries

load libMiracle.so
#load libMiracleWirelessCh.so
load libmphy.so
load libmmac.so
load libMiracleBasicMovement.so
load libMiracleIp.so
load libMiracleIpRouting.so
load libmiracleport.so
load libmll.so

load libmiraclecbr.so

load libmlltracer.so
load libroutingtracer.so
#load libmphytracer.so
load libcbrtracer.so
#load libsinrtracer.so
#load libmphymaccltracer.so
#load libClTrace.so
#load libverboseclcmntracer.so

#load ../src/.libs/libuwm.so
#load ../src/.libs/libuwmphytracer.so

load ../src/.libs/libuwm.so



proc finish {} {
	global ns opt cbr
	puts "done!"
        puts "tracefile: $opt(tracefilename)"
	$ns flush-trace
	close $opt(tracefile)

        set per   [$cbr(2) getPer]
        set delay [$cbr(2) getftt]

        puts "PER   : $per"
        puts "delay : $delay"
}




set ns [new Simulator]
$ns use-Miracle


#Module/CBR set debug_ 1


######################################
# global allocations & misc options
######################################

set opt(starttime)       0.0
set opt(stoptime)        10000
set opt(fracstartmoving) 0.01
set opt(txduration)      [expr $opt(stoptime) - $opt(starttime)]
set opt(xmax)            10000.0

set opt(freq) 		 24000.0
set opt(bw)              4500.0

set opt(ack_mode)        "setNoAckMode"
# set opt(ack_mode)        "setAckMode"

set opt(tracefilename) "/tmp/${argv0}.tr"
set opt(tracefile) [open $opt(tracefilename) w]
set opt(cltracefilename) "/tmp/${argv0}.cltr"
set opt(cltracefile) [open $opt(cltracefilename) w]
#$ns trace-all $opt(tracefile)

set channel [new Module/UnderwaterChannel]

set propagation [new MPropagation/Underwater]


set data_mask [new MSpectralMask/Rect]
$data_mask setFreq       $opt(freq)
$data_mask setBandwidth  $opt(bw)

MInterference/MIV set maxinterval_ 500

Module/CBR set packetSize_  256
Module/CBR set period_    500

Module/MMac/ALOHA/ADV set HDR_size_             0
Module/MMac/ALOHA/ADV set ACK_size_             32
Module/MMac/ALOHA/ADV set max_tx_tries_         5
Module/MMac/ALOHA/ADV set wait_costant_         0.1
Module/MMac/ALOHA/ADV set debug_                0
Module/MMac/ALOHA/ADV set backoff_tuner_        1
Module/MMac/ALOHA/ADV set max_payload_          256
Module/MMac/ALOHA/ADV set ACK_timeout_          30.0
Module/MMac/ALOHA/ADV set alpha_                0.8
Module/MMac/ALOHA/ADV set buffer_pkts_          -1
Module/MMac/ALOHA/ADV set max_backoff_counter_  4
Module/MMac/ALOHA/ADV set listen_time_          0

###############################
# Procedure for creating nodes
###############################

proc createNode {id } {

    global channel propagation smask ns cbr position bpsk node port portnum ipr ipif
    global phy posdb opt mll mac data_mask
    
    set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 

    set cbr($id)  [new Module/CBR] 
    set port($id) [new Module/Port/Map]
    set ipr($id)  [new Module/IP/Routing]
    set ipif($id) [new Module/IP/Interface]
    set mll($id)  [new Module/MLL] 
    set mac($id)  [new Module/MMac/ALOHA/ADV] 
    set phy($id)  [new Module/MPhy/BPSK/Underwater]

    $node($id)  addModule 7 $cbr($id)   0  "CBR"
    $node($id)  addModule 6 $port($id)  0  "PRT"
    $node($id)  addModule 5 $ipr($id)   0  "IPR"
    $node($id)  addModule 4 $ipif($id)  0  "IPF"   
    $node($id) addModule  3 $mll($id)   0  "MLL"
    $node($id)  addModule 2 $mac($id)   1  "MAC"
    $node($id)  addModule 1 $phy($id)   1  "PHY"

    $node($id) setConnection $cbr($id)   $port($id)  0
    $node($id) setConnection $port($id)  $ipr($id)   0
    $node($id) setConnection $ipr($id)   $ipif($id)  0
    $node($id) setConnection $ipif($id)  $mll($id)   0
    $node($id) setConnection $mll($id)   $mac($id)   0
    $node($id) setConnection $mac($id)   $phy($id)   1
    $node($id) addToChannel  $channel    $phy($id)   1

    set portnum($id) [$port($id) assignPort $cbr($id) ]
    if {$id > 254} {
	puts "hostnum > 254!!! exiting"
	exit
    }
    $ipif($id) addr "1.0.0.${id}"
    $ipif($id) subnet "0.0.0.0"
    
    set position($id) [new "Position/BM"]
    $node($id) addPosition $position($id)
    set posdb($id) [new "PlugIn/PositionDB"]
    $node($id) addPlugin $posdb($id) 20 "PDB"
    $posdb($id) addpos [$mac($id) addr] $position($id)
    $position($id) setX_ 0.0
    $position($id) setY_ 0.0
    $position($id) setZ_ -10.0

    $phy($id) set debug_ 0
    $phy($id) setPropagation $propagation

    set interf($id) [new MInterference/MIV]
    $phy($id) setSpectralMask $data_mask
    $phy($id) setInterference $interf($id)

#     $mac($id) printTransitions
    $mac($id) $opt(ack_mode)
    $mac($id) initialize
}


proc connectNodes {id1 id2} {
    global ipif ipr portnum cbr
    $cbr($id1) set destAddr_ [$ipif($id2) addr]
    $cbr($id1) set destPort_ $portnum($id2)
    $cbr($id2) set destAddr_ [$ipif($id1) addr]
    $cbr($id2) set destPort_ $portnum($id1)  
    $ipr($id1) addRoute "1.0.0.${id2}" "255.255.255.255" "1.0.0.${id2}"
    $ipr($id2) addRoute "1.0.0.${id1}" "255.255.255.255" "1.0.0.${id1}"
}

###############################
# create nodes
###############################

createNode 1
$position(1) setX_ 0.0
$ns at $opt(starttime)   "$cbr(1) start"
$ns at $opt(stoptime)   "$cbr(1) stop"

createNode 2
$position(2) setX_ 0.0
#$position(2) set debug_ 543

connectNodes 1 2

for {set id1 1} {$id1 < 3} {incr id1}  {
    for {set id2 1} {$id2 < 3} {incr id2}  {
	$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }   
}

set speed [expr $opt(xmax) / ($opt(txduration) * (1 - $opt(fracstartmoving)))]
puts "Node 2 moving to ($opt(xmax),0.0) at $speed m/s"
$ns at [expr $opt(txduration) * $opt(fracstartmoving)] "$position(2) setdest $opt(xmax) 0.0 $speed "


###################
# start simulation
###################




puts -nonewline "Simulating"

for {set t $opt(starttime)} {$t <= $opt(stoptime)} {set t [expr $t + $opt(txduration) / 40.0 ]} {
    $ns at $t "puts -nonewline ."
}
$ns at [expr $opt(stoptime) + 50]  "finish; $ns halt"
$ns run
