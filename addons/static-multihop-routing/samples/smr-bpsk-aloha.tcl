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

# STATIC-MULTIHOP-ROUTING + BPSK + ALOHA sample


#	+-------------------------+	+-------------------------+
#	| 	6. CBR            |	| 	6. CBR            |
#	+-------------------------+	+-------------------------+
#	| 	5. Port           |	| 	5. Port           |
#	+-------------------------+	+-------------------------+
#	| 	4. IP Routing     |	| 	4. IP Routing     |
#	+-------------------------+	+-------------------------+
#	| 	3. IP Interface   |	| 	3.  IP Interface  |
#	+-------------------------+	+-------------------------+
#	| 	2. MMAC/ALOHA     |	| 	2. MMAC/ALOHA     |
#	+-------------------------+	+-------------------------+
#	|       1. MPHY/BPSK      |	|       1. MPHY/BPSK      |
#	+-------------------------+	+-------------------------+
#                    |                               |
#	+-------------------------------------------------------+
#       |                  UnderwaterChannel                    |
#       +-------------------------------------------------------+


# Module libraries

load libMiracle.so
load libmiraclecbr.so
load libMiracleWirelessCh.so
load libmphy.so
load libmmac.so
load libMiracleBasicMovement.so
load libMiracleIp.so
load libMiracleIpRouting.so
load libmiracleport.so
load libmll.so

load libmlltracer.so
load libroutingtracer.so
#load libmphytracer.so
load libcbrtracer.so
#load libsinrtracer.so
#load libmphymaccltracer.so
#load libClTrace.so
#load libverboseclcmntracer.so

load ../src/.libs/libsmhr.so

 
########################################
# Command-line parameters
########################################

if {$argc == 0} {
    
    set opt(nn)  10
    set opt(nf)  1
    set opt(maxhoplen) 20
    set opt(run) 1
    
} elseif {$argc != 4 } {
    puts " usage: ns $argv0 nn nflows maxhoplength replicationnumber"
    exit
} else {    
    set opt(nn)             [lindex $argv 0]
    set opt(nf)             [lindex $argv 1]
    set opt(maxhoplen)      [lindex $argv 2]
    set opt(run)            [lindex $argv 3]
}

set ns [new Simulator]
$ns use-Miracle


#Module/CBR set debug_ 1


######################################
# global allocations & misc options
######################################

set opt(starttime)     0.0
set opt(stoptime)    10000
set opt(fracstartmoving) 0.01
set opt(txduration) [expr $opt(stoptime) - $opt(starttime)]
set opt(xmin)        0.0
set opt(xmax)        50
set opt(ymin)        0.0
set opt(ymax)        $opt(xmax)
set opt(zmin)        0.0
set opt(zmax)        0.0
set opt(scheme)      shortestpath

set opt(paramstr)    " $opt(nn) $opt(nf) $opt(maxhoplen) $opt(run) "
set opt(scenariostr) "xmax$opt(xmax)_nn$opt(nn)_nf$opt(nf)"


#set opt(tracefilename) "/tmp/${argv0}.tr"
set opt(tracefilename) "/dev/null"
set opt(tracefile) [open $opt(tracefilename) w]

#set opt(cltracefilename) "/tmp/${argv0}.cltr"
set opt(cltracefilename) "/dev/null"
set opt(cltracefile) [open $opt(cltracefilename) w]

#$ns trace-all $opt(tracefile)

exec mkdir -p graphs/dot

set opt(machine_name) [exec uname -n]
set opt(resultdir) "results_$opt(scenariostr)"
exec mkdir -p $opt(resultdir)
set opt(resfile) $opt(resultdir)/results_${opt(scheme)}.$opt(machine_name)
set opt(pfresfile) $opt(resultdir)/pfresults_${opt(scheme)}.$opt(machine_name)



set channel [new Module/DumbWirelessCh]

set propagation [new MPropagation/MFreeSpace]

set smask [new MSpectralMask/Rect]
$smask setFreq 2.4e9
$smask setBandwidth 20e6


StaticMultihopRouting set debug_ 0

# this limits the maximum hop length. We just set it to some customizable
# value here, but in real scenarios you really want to calculate this
# value based on the PHY layer specs (TX power, error probability) and
# on the propagation model

StaticMultihopRouting set MaxHopLength_ $opt(maxhoplen)


MInterference/MIV set maxinterval_ 0.5
Module/MPhy/BPSK set bitrate_ 80000


Module/CBR set packetSize_  256
Module/CBR set period_      60
Module/CBR set PoissonTraffic_ 1

Module/MMac/CSMA set BaseBackoffTime_  60
Module/MMac/CSMA set AckTimeout_       60 
Module/MMac/CSMA set debug_             0
Module/MMac/CSMA set debug_states_      0
Module/MMac/CSMA set HeaderSize_       24

Module/MPhy/BPSK set AcquisitionThreshold_dB_ 0.0

set mhrouting [new StaticMultihopRouting]

# this is for plotting only
$mhrouting xmax $opt(xmax)    



 ########################################
 # Random Number Generators
 ########################################

 global defaultRNG
 set startrng [new RNG]
 set positionrng [new RNG]

 # set rvstart [new RandomVariable/Uniform]
 # $rvstart set min_ $opt(startmin)
 # $rvstart set max_ $opt(startmax)
 # $rvstart use-rng $startrng

 set rvposx [new RandomVariable/Uniform]
 $rvposx set min_ $opt(xmin)
 $rvposx set max_ $opt(xmax)
 $rvposx use-rng $positionrng

 set rvposy [new RandomVariable/Uniform]
 $rvposy set min_ $opt(ymin)
 $rvposy set max_ $opt(ymax)
 $rvposy use-rng $positionrng

 set rvposz [new RandomVariable/Uniform]
 $rvposz set min_ $opt(zmin)
 $rvposz set max_ $opt(zmax)
 $rvposz use-rng $positionrng

 # seed random number generator according to replication number
 for {set j 1} {$j < $opt(run)} {incr j} {
     $defaultRNG next-substream
     $startrng next-substream
     $positionrng next-substream
 }


 ###############################
 # Procedure for creating nodes
 ###############################

 proc createNode {id } {

     global channel propagation smask ns cbr position bpsk node port portnum ipr ipif
     global phy posdb opt rvposx rvposy rvposz mhrouting mll mac

     set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)] 

     set cbr($id) [new Module/CBR] 
     set port($id) [new Module/Port/Map]
     set ipr($id)  [new Module/IP/Routing]
     set ipif($id) [new Module/IP/Interface]
     set mll($id) [new Module/MLL] 
     set mac($id) [new Module/MMac/ALOHA] 
     set phy($id) [new Module/MPhy/BPSK]

     $node($id) addModule 7 $cbr($id)  0  "CBR"
     $node($id) addModule 6 $port($id) 0  "PRT"
     $node($id) addModule 5 $ipr($id)  0 "IPR"
     $node($id) addModule 4 $ipif($id) 0 "IPF"   
     $node($id) addModule 3 $mll($id)  0 "MLL"
     $node($id) addModule 2 $mac($id)  0 "MAC"
     $node($id) addModule 1 $phy($id)  0 "PHY"

     $node($id) setConnection $cbr($id)  $port($id)  1
     $node($id) setConnection $port($id) $ipr($id)   0
     $node($id) setConnection $ipr($id)  $ipif($id)  0
     $node($id) setConnection $ipif($id) $mll($id)   0 
     $node($id) setConnection $mll($id)  $mac($id)   0
     $node($id) setConnection $mac($id)  $phy($id)   0
     $node($id) addToChannel $channel    $phy($id)   0

     set portnum($id) [$port($id) assignPort $cbr($id) ]
     if {$id > 254} {
	 puts "hostnum > 254!!! exiting"
	 exit
     }
     $ipif($id) addr "1.0.0.${id}"
     #interface can reach directly all nodes if needed
     $ipif($id) subnet "0.0.0.0"

     set position($id) [new "Position/BM"]
     $node($id) addPosition $position($id)
     set posdb($id) [new "PlugIn/PositionDB"]
     $node($id) addPlugin $posdb($id) 20 "PDB"
     $posdb($id) addpos [$ipif($id) addr] $position($id)
     $position($id) setX_ [$rvposx value]
     $position($id) setY_ [$rvposy value]
     $position($id) setZ_ [$rvposz value]
     #puts "node $id at ([$position($id) getX_], [$position($id) getY_], [$position($id) getZ_])"

     $phy($id) set debug_ 0
     $phy($id) setPropagation $propagation
     $phy($id) setSpectralMask $smask

 
    set interf($id) [new MInterference/MIV]
    $phy($id) setInterference $interf($id)

    $mhrouting addpos [$ipif($id) addr]  $position($id) $ipr($id)

}


proc connectNodes {id1 id2} {
    global ipif ipr portnum cbr
    $cbr($id1) set destAddr_ [$ipif($id2) addr]
    $cbr($id1) set destPort_ $portnum($id2)
    $cbr($id2) set destAddr_ [$ipif($id1) addr]
    $cbr($id2) set destPort_ $portnum($id1)  

# DO NOT add routes manually! that's the purpose of having a static multihop routing module
#    $ipr($id1) addRoute "0.0.1.${id2}" "255.255.255.255" "0.0.1.${id1}"
#    $ipr($id2) addRoute "0.0.1.${id1}" "255.255.255.255" "0.0.1.${id2}"
}

###############################
# create nodes
###############################

for {set id 0} {$id < $opt(nn)} {incr id}  {
    createNode $id
}



###############################
# fill ARP tables
###############################

for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
	$mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }   
}


###############################
# setup flows
###############################

set fid 0
while {$fid < $opt(nf) } {
    set id1 [expr $fid * 2]
    set id2 [expr $fid * 2 + 1]
 
    $ns at $opt(starttime)   "$cbr($id1) start"
    $ns at $opt(stoptime)    "$cbr($id1) stop"

    connectNodes $id1 $id2

    # This is the procedure that sets up the route between two nodes
    # note that we already informed the mhrouting module of the
    # position of each node by running the command "$mhrouting addpos"
    # upon node creation. The following "setpath" command calculates
    # the shortest path between nodes. 
    $mhrouting setpath [$ipif($id1) addr] [$ipif($id2) addr] "graphs/dot/path_${id1}-${id2}_${opt(scheme)}.dot"

    incr fid
}

#set speed [expr $opt(xmax) / ($opt(txduration) * (1 - $opt(fracstartmoving)))]
#puts "Node 2 moving to ($opt(xmax),0.0) at $speed m/s"
#$ns at [expr $opt(txduration) * $opt(fracstartmoving)] "$position(2) setdest $opt(xmax) 0.0 $speed "

for {set id 0} {$id < $opt(nn)} {incr id}  {
    global ipr
    if {[$ipr($id) numroutes] > 0} {
	#puts "Node $id"
	#$ipr($id) printroutes
    }
}

#$mhrouting graphviz_output "graphs/dot/${opt(scheme)}_net.dot"









proc finish {} {
    global ns opt cbr phy
    puts "done!"
    puts "tracefile: $opt(tracefilename)"
    $ns flush-trace
    close $opt(tracefile)

    #####################
    # print stats
    #####################

    set resultsFilePtr [open $opt(resfile) a]
    set pfresultsFilePtr [open $opt(pfresfile) a]
    

    # global results

    set sentpkts 0
    set recvpkts 0
    for {set id 0} {$id < $opt(nn)} {incr id}  {       
	set sentpkts    [expr $sentpkts + [$cbr($id) getsentpkts]]
	set recvpkts    [expr $recvpkts + [$cbr($id) getrecvpkts]]	
    } 

    set nrgeff     [expr $sentpkts / $recvpkts]



    # results per flow

    set sumper 0
    set sumthr 0
    set sumftt 0

    set id 1
    while {$id < [expr $opt(nf) * 2]} {	
	set per [$cbr($id) getper]
	set thr [$cbr($id) getthr]
	set ftt [$cbr($id) getftt]
	#set npkts [$cbr($id) getsentpkts]
	set sumper [expr $sumper + $per]
	set sumthr [expr $sumthr + $thr]
	set sumftt [expr $sumftt + $ftt]

	set pfresultstr "$id $thr $ftt $per"	
	puts "$opt(paramstr) $pfresultstr"
	puts $pfresultsFilePtr "$opt(paramstr) $pfresultstr" 

	set id [expr $id + 2]

    }

    set avgper [expr $sumper / $opt(nf)]
    set avgthr [expr $sumthr / $opt(nf)]
    set avgftt [expr $sumftt / $opt(nf)]

    set resultstr " $sentpkts $recvpkts $nrgeff $avgper $avgthr $avgftt"	
    puts "$opt(paramstr) $resultstr"
    puts $resultsFilePtr "$opt(paramstr) $resultstr" 
    
}




###################
# start simulation
###################

puts -nonewline "Simulating"

for {set t $opt(starttime)} {$t <= $opt(stoptime)} {set t [expr $t + $opt(txduration) / 40.0 ]} {
    $ns at $t "puts -nonewline ."
}
$ns at [expr $opt(stoptime) + 250]  "finish; $ns halt"
$ns run
