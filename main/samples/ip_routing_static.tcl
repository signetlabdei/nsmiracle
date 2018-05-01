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
#dynlibload Trace ../trace/.libs/
#dynlibload ClTrace ../cltrace/.libs/
dynlibload MiracleIp ../ip/.libs/
dynlibload MiracleIpRouting ../ip/.libs/
dynlibload miracleport ../port/.libs/
dynlibload miraclecbr ../cbr/.libs/
dynlibload miraclelink ../link/.libs/
dynlibload routingtracer ../ip/.libs/

#dynlibload MiracleTcp ./tcp/.libs/
#dynlibload MiracleWirelessCh .././wirelessch/.libs/
#dynlibload MiraclePhy802_11 .././phy802_11/.libs/
#dynlibload MiracleMac802_11 .././mac802_11/.libs/


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


proc createNode {n} {
#
#	+------------------------------------+
#	|  4. CBR	   | 4. CBR          |
#	+------------------------------------+
#	| 	   3. Port                   |
#	+------------------------------------+
#	|          2. IP/Routing             | 
#	+------------------------------------+
#	| 1. IP/Iinterface | 1. IP/Interface |
#	+------------------------------------+
#

	global ns node app port ipr ipif portnum
	
	set node($n) [$ns create-M_Node]
	set app($n,0)  [new Module/CBR]
	set app($n,1)  [new Module/CBR]
	set port($n) [new Module/Port/Map]
	set ipr($n)  [new Module/IP/Routing]
	set ipif($n,0) [new Module/IP/Interface]
	set ipif($n,1) [new Module/IP/Interface]


	$node($n) addModule 4 $app($n,0)  0 "AP0"
	$node($n) addModule 4 $app($n,1)  0 "AP1"
	$node($n) addModule 3 $port($n)   0 "PRT"
	$node($n) addModule 2 $ipr($n)    0 "IPR"
	$node($n) addModule 1 $ipif($n,0) 0 "IP0"
	$node($n) addModule 1 $ipif($n,1) 0 "IP1"

	
	$node($n) setConnection $app($n,0) $port($n) 1
	$node($n) setConnection $app($n,1) $port($n) 1
	$node($n) setConnection $port($n) $ipr($n)   1
	$node($n) setConnection $ipr($n) $ipif($n,0) 1
	$node($n) setConnection $ipr($n) $ipif($n,1) 1
	
	set portnum($n,0) [$port($n) assignPort $app($n,0) ]
	set portnum($n,1) [$port($n) assignPort $app($n,1) ]
		
}

createNode 1
createNode 2
createNode 3 
createNode 4


# Now we will create the following topology:
#
#
#            +-+ 6.0.0.1           6.0.0.3  +-+
#            |1|<-------------------------->|3|
#    5.0.0.1 +-+                            +-+
#             ^                              ^ 8.0.0.3
#             |                              |
#             |                              |
#             |                              |
#             |                              |
#             v                              v
#    5.0.0.2 +-+                            +-+ 8.0.0.4
#            |2|<-------------------------->|4|
#            +-+ 7.0.0.2            7.0.0.4 +-+
#
# Note that every node has 2 interfaces, each of which has a different
# IP address and is on a different subnet.
#


# createLink (netidentificator, node1number, node1interfacenumber, node2number, node2interfacenumber)
proc createLink {net n1 n1if n2 n2if} {
    puts "creating subnet $net:"
    puts "   - interface $n1if of node $n1"
    puts "   - interface $n2if of node $n2"
    global dlink node ipif ipr
    set dlink($n1,$n2) [new Module/DuplexLink]
    $dlink($n1,$n2) bandwidth 1000000
    $dlink($n1,$n2) delay 0.0001
    $dlink($n1,$n2) qsize 10
    $dlink($n1,$n2) settags "CH$net"
    $dlink($n1,$n2) connect $node($n1) $ipif($n1,$n1if) 1 $node($n2) $ipif($n2,$n2if) 1
    
    $ipif($n1,$n1if) addr "${net}.0.0.${n1}"
    $ipif($n2,$n2if) addr "${net}.0.0.${n2}"

}


createLink 5 1 0 2 0 
createLink 6 1 1 3 0
createLink 7 2 1 4 0 
createLink 8 3 1 4 1 


# Now we force circular routing:
$ipr(1) defaultGateway "5.0.0.2"
$ipr(2) defaultGateway "7.0.0.4"
$ipr(4) defaultGateway "8.0.0.3"
$ipr(3) defaultGateway "6.0.0.1"

# print all routes
for {set i 1} {$i <5} {incr i} {
    set addr0 [$ipif($i,0) addr-string]
    set addr1 [$ipif($i,1) addr-string]
    puts "------------------------------------"
    puts "  Node $i interfaces:"
    puts "    if0 : IP $addr0"
    puts "    if1 : IP $addr1"
    puts " "
    puts "    Routing table for node $i: "
    $ipr($i) printroutes
}
puts "------------------------------------"


# Now we setup connections. 
# Warning: dont'use strings here! 
# Many applications accept only a raw int32 as destAddr_
$app(1,0) set destAddr_ [$ipif(4,0) addr]
$app(1,0) set destPort_ $portnum(4,0)
$app(4,0) set destAddr_ [$ipif(1,0) addr]
$app(4,0) set destPort_ $portnum(1,0)

$app(4,1) set destAddr_ [$ipif(1,1) addr]
$app(4,1) set destPort_ $portnum(1,1)
$app(1,1) set destAddr_ [$ipif(4,1) addr]
$app(1,1) set destPort_ $portnum(4,1)


$ns at 0 "$app(1,0) start"
$ns at 1 "$app(4,1) start"
$ns at 2 "$app(1,0) stop"
$ns at 2 "$app(4,1) stop"
$ns at 5 "finish; $ns halt"
$ns run


