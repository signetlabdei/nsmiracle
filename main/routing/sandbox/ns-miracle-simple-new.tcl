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

load libMiracle.so
load libMiracleIp.so     
load libmiracleport.so
load libTrace.so
load libClTrace.so
load libmiraclelink.so
load libmiraclecbr.so
#load libStaticRoute.so
load libMiracleRouting.so
load libMiracleRoutingSandbox.so
load libcbrtracer.so


set ns [new Simulator]

$ns use-Miracle

proc finish {} {
        global ns tf
        puts "done!"
        $ns flush-trace
        close $tf 
}       

set tf [open out.tr w]
$ns trace-all $tf

proc create-node {i} {
        global ns node_ cbr_ ip_ tr_ port_
        set node_($i) [$ns create-M_Node]
        set cbr_($i) [new Module/CBR]
        $cbr_($i) set period_ 0.01
	$cbr_($i) set size_ 1024
        set ip_($i) [new Module/SandboxRM1]
#	if {$i == 3} {
#		$ip_($i) set debug_ 100
#	}

        $ip_($i) set debug_ 1
        $ip_($i) metric $i.$i
        set tr_($i) [new Module/Port]
        
        $node_($i) addModule 3 $cbr_($i) 10 "APP"
        $node_($i) addModule 2 $tr_($i) 10 "UDP"
        $node_($i) addModule 1 $ip_($i) 10 "IP"
        
	$node_($i) setConnection $tr_($i) $ip_($i) 10
        $node_($i) setConnection $cbr_($i) $tr_($i) 10
        
	set port_($i) [$tr_($i) assignPort $cbr_($i)]
	puts "IPModule node $i has id [$ip_($i) id_] IPaddr [$ip_($i) addr]"
}


proc create-router {i} {
        global ns node_ cbr_ ipM_ ipC1_ ipC2_ tr_ port_
        set node_($i) [$ns create-M_Node]
        set cbr_($i) [new Module/CBR]
        $cbr_($i) set period_ 0.01
	$cbr_($i) set size_ 1024
        set ipF_($i) [new Module/SandboxRM1]
        $ipF_($i) set debug_ 1
        #$ipF_($i) metric $i.$i
        set ipC1_($i) [new Module/SandboxRM1]
        $ipC1_($i) set debug_ 1
        $ipC1_($i) metric 2
        set ipC2_($i) [new Module/SandboxRM1]
        $ipC2_($i) set debug_ 1
        $ipC2_($i) metric 3
        set tr_($i) [new Module/Port]
        
        $node_($i) addModule 4 $cbr_($i) 1 "APP"
        $node_($i) addModule 3 $tr_($i) 1 "UDP"
        $node_($i) addModule 2 $ipF_($i) 1 "IP"
        $node_($i) addModule 1 $ipC1_($i) 1 "IPC1"
        $node_($i) addModule 1 $ipC2_($i) 1 "IPC2"
        
	$node_($i) setConnection $cbr_($i) $tr_($i) trace
	$node_($i) setConnection $tr_($i) $ipF_($i) trace
	$node_($i) setConnection $ipF_($i) $ipC1_($i) trace
	$node_($i) setConnection $ipF_($i) $ipC2_($i) trace
	

        
	set port_($i) [$tr_($i) assignPort $cbr_($i)]
	puts "IPModule Father node $i has id [$ipF_($i) id_] IPaddr [$ipF_($i) addr]"
	puts "IPModule Child node $i has id [$ipC1_($i) id_] IPaddr [$ipC1_($i) addr]"
	puts "IPModule Child node $i has id [$ipC2_($i) id_] IPaddr [$ipC2_($i) addr]"
}

create-node 1
create-router 2
create-node 3

#ConnectorTrace/ChSAP set debug_ 100
#Module/Link set debug_ 100
set link(1) [new Module/DuplexLink]
$link(1) delay      10m
$link(1) bandwidth  2mb
$link(1) qsize      5
$link(1) connect $node_(3) $ip_(3) trace $node_(2) $ipC1_(2) trace

#Module/Link set debug_ 0

set link(2) [new Module/DuplexLink]
$link(2) delay      10m
$link(2) bandwidth  2mb
$link(2) qsize      5
$link(2) connect $node_(2) $ipC1_(2) trace $node_(1) $ip_(1) trace


set link(3) [new Module/DuplexLink]
$link(3) delay      20m
$link(3) bandwidth  1.7mb
$link(3) qsize      5
$link(3) connect $node_(2) $ipC2_(2) trace $node_(1) $ip_(1) trace


#$ns at 0.09 "puts \"AAAAAAAAA [[$link(3) set downlink_] id_]\""
$ip_(3) addRoute 0.0.0.0 0.0.0.0 [[$link(1) set uplink_] id_] 0.0.0.2
$ipC1_(2) addRoute 0.0.0.0 0.0.0.0 [[$link(2) set uplink_] id_] 0.0.0.1
$ipC2_(2) addRoute 0.0.0.0 0.0.0.0 [[$link(3) set uplink_] id_] 0.0.0.1
#$ip_F(3) addRoute 0.0.0.0 0.0.0.0 [[$link(3) set uplink_] id_] 0.0.0.4
$ip_(1) addRoute 0.0.0.0 0.0.0.0 [[$link(3) set downlink_] id_] 0.0.0.2
#$ip_(2) addRoute 0.0.0.0 0.0.0.0 [[$link(2) set uplink_] id_] 0.0.0.3
#$ip_(3) addRoute 0.0.0.1 255.255.255.255 [[$link(1) set downlink_] id_] 0.0.0.1
#$ip_(3) addRoute 0.0.0.2 255.255.255.255 [[$link(2) set downlink_] id_] 0.0.0.2
#$ip_(3) addRoute 0.0.0.0 0.0.0.0 [[$link(3) set uplink_] id_] 0.0.0.4


$cbr_(3) set destAddr_ [$ip_(1) addr]
$cbr_(3) set destPort_ $port_(1)
$cbr_(3) set debug_ 0
$cbr_(3) set debug_ 0

$ns at 0.1 "$cbr_(3) sendPkt"
#$ns at 1 "$cbr_(2) start"
#$ns at 4 "$cbr_(2) stop"
#$ns at 1 "$cbr_(3) stop"

$ns at 5 "finish; $ns halt"

$ns run


