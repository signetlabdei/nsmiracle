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

# BPSK sample

# Two nodes, a CBR module for each one, directly plugged on a MPhy/BPSK module


#	+-----------------------+	+-----------------------+
#	| 	2. CBR  (sm)    |	| 	2. CBR	(rm)    |
#	+-----------------------+	+-----------------------+
#	| 	1. MPHY/BPSK	|	| 	1. MPHY/BPSK	|
#	+-----------------------+	+-----------------------+
#                    |                               |
#	+-------------------------------------------------------+
#       |                  DumbWirelessChannel                  |
#       +-------------------------------------------------------+

load ../nsmiracle/.libs/libMiracle.so
load ../cbr/.libs/libmiraclecbr.so
load ../wirelessch/.libs/libMiracleWirelessCh.so
load ../mphy/.libs/libmphy.so
load ../mphy/.libs/libmphytracer.so
load ../mobility/.libs/libMiracleBasicMovement.so

proc finish {} {
	global ns tf 
	puts "done!"
	$ns flush-trace
	close $tf
}



set ns [new Simulator]
$ns use-Miracle


#Module/CBR set debug_ 1


set tf [open out_test_bpsk.tr w]
$ns trace-all $tf


set channel [new Module/DumbWirelessCh]

set propagation [new MPropagation/MFreeSpace]

set smask [new MSpectralMask/Rect]
$smask setFreq 10e8
$smask setBandwidth 2000


MInterference/MIV set maxinterval_ 0.5
Module/MPhy/BPSK set bitrate_ 8000

###################
# Node 1
###################

set node1 [$ns create-M_Node] 
set cbr1 [new Module/CBR] 
$node1 addModule 2 $cbr1 0 "CBR1"

$cbr1 set packetSize_ 100
$cbr1 set period_ 0.1

set position1 [new "Position/BM"]
$node1 addPosition $position1
$position1 setX_ 0.0
$position1 setY_ 0.0

set bpsk1 [new Module/MPhy/BPSK]
$node1 addModule 1 $bpsk1 0 "BPSK1"
$bpsk1 setSpectralMask $smask
$bpsk1 setPropagation $propagation
set interf1 [new MInterference/MIV]
#$bpsk1 setInterference $interf1

$node1 setConnection $cbr1 $bpsk1 0

$node1 addToChannel $channel $bpsk1 0




###################
# Node 2
###################


set node2 [$ns create-M_Node]
set cbr2 [new Module/CBR]
$node2 addModule 2 $cbr2 0 "CBR2"

$cbr1 set packetSize_ 100
$cbr1 set period_ 0.1

set position2 [new "Position/BM"]
$node2 addPosition $position2

$position2 setX_ 0.0
$position2 setY_ 0.0

set bpsk2 [new Module/MPhy/BPSK]
$node2 addModule 1 $bpsk2 0 "BPSK2"
$bpsk2 setSpectralMask $smask
$bpsk2 setPropagation $propagation
set interf2 [new MInterference/MIV]
$bpsk2 setInterference $interf2

$interf2 set debug_ 0

$node2 setConnection $cbr2 $bpsk2 1

$node2 addToChannel $channel $bpsk2 0
 


###################
# Node 3
###################

set node3 [$ns create-M_Node]
set cbr3 [new Module/CBR]
$node3 addModule 2 $cbr3 0 "CBR3"

$cbr3 set packetSize_ 200
$cbr3 set period_ 0.3

set position3 [new "Position/BM"]
$node3 addPosition $position3

$position3 setX_ 0.0
$position3 setY_ 0.0

set bpsk3 [new Module/MPhy/BPSK]
$node3 addModule 1 $bpsk3 0 "BPSK3"
$bpsk3 setSpectralMask $smask
$bpsk3 setPropagation $propagation
set interf3 [new MInterference/MIV]
$bpsk3 setInterference $interf3


$node3 setConnection $cbr3 $bpsk3 0

$node3 addToChannel $channel $bpsk3 0







###################
# start simulation
###################


$ns at 1 "$position3 setdest 0 100 10"

puts -nonewline "Simulating..."
$ns at 0   "$cbr1 start"
$ns at 0.5 "$cbr3 start"
$ns at 195   "$cbr1 stop"
$ns at 197 "$cbr3 stop"
$ns at 200 "finish; $ns halt"
$ns run
