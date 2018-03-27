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

DropInterface set debug_ 0

Class ConnectorList

ConnectorList instproc init {  } {
    $self next 
    $self instvar head_ tail_ 
    $self set head_ [new Connector]
    $self set tail_ [new Connector]
    $head_ target $tail_
}

ConnectorList instproc tail {} {
	$self instvar tail_
    return $tail_ 
}


ConnectorList instproc head {} {
	$self instvar head_
	return $head_
}

ConnectorList instproc add-to-head { connector } {
	$self instvar head_
	$connector target [$head_ target]
	$head_ target $connector
}


#Module/Link instvar ml

#Module/Link instproc init {{qtype "Queue/DropTail"}} {

set NModuleLink 0

Module/Link set debug_ 0

Module/Link instproc init {} {
    global NModuleLink
    eval $self next    
    $self instvar delaylink_ queue_ link_ bin_ di_
    $self set link_ [new ConnectorList]

    $self setLinkHead [$link_ head]
    [$link_ tail] target $self

    set delaylink_ [new DelayLink]
    $link_ add-to-head $delaylink_
    
#    set queue_ [new $qtype]
    set queue_ [new Queue/DropTail]   
    $link_ add-to-head $queue_

    $self setLinkHead $queue_
    $queue_ target $delaylink_
    $delaylink_ target $self        

    set bin_ [new ConnectorTrace/Bin]
    set fd [[Simulator instance] get-ns-traceall]
    $bin_ preamble "$NModuleLink CHBIN"
    $bin_ attach $fd
    $self addbin $bin_
    set di_ [new DropInterface]
    $di_ setChannel $self
    $queue_ drop-target $di_
    $self id_ $NModuleLink
    incr NModuleLink
}


Module/Link instproc connect {node1 mod1 verb1 node2 mod2 verb2} {
    # setup connection
    $node1 addToChannel $self $mod1 $verb1
    $node2 addToChannel $self $mod2 $verb2
}


Module/Link instproc delay {del} {
    $self instvar delaylink_
    $delaylink_ set delay_ $del
}

Module/Link instproc bandwidth {bw} {
    $self instvar delaylink_
    $delaylink_ set bandwidth_ $bw
}

Module/Link instproc qsize {qs} {
    $self instvar queue_
    $queue_ set limit_ $qs
}


Module/Link instproc addErrorModel {em} {
    $self instvar delaylink_ queue_ link_ bin_ di_
    $delaylink_ target $em
    $em target [$link_ tail]
#     $link_ add-to-head $em
    $em drop-target $di_
}


Module/Link instproc print-params {} {
    $self instvar queue_ delaylink_
    set d [$delaylink_ set delay_]
    set b [$delaylink_ set bandwidth_]
    set q [$queue_ set limit_]
    puts "delay=$d bw=$b qsize=$q"
}




Class Module/DuplexLink 


Module/DuplexLink instproc init {} {
    $self instvar uplink_ downlink_
    set uplink_ [new Module/Link]
    set downlink_ [new Module/Link]
}

Module/DuplexLink instproc delay {del} {
    $self instvar uplink_ downlink_
    $uplink_   delay $del
    $downlink_ delay $del
}

Module/DuplexLink instproc bandwidth {bw} {
    $self instvar uplink_ downlink_
    $uplink_    bandwidth $bw
    $downlink_  bandwidth $bw  
}

Module/DuplexLink instproc qsize {qs} {
    $self instvar uplink_ downlink_
    $uplink_     qsize $qs
    $downlink_   qsize $qs
}

Module/DuplexLink instproc connect {node1 mod1 verb1 node2 mod2 verb2} {
    $self instvar uplink_ downlink_
    $uplink_ connect $node1 $mod1 $verb1 $node2 $mod2 $verb2
    $downlink_ connect $node2 $mod2 $verb2 $node1 $mod1 $verb1
}

Module/DuplexLink instproc settags {tagprefix} {
    $self instvar downlink_ uplink_
    $uplink_ settag ${tagprefix}up
    $downlink_ settag ${tagprefix}dw
}

Module/DuplexLink instproc addErrorModel {em} {
    puts "Currently you cannot add an error model to a Module/DuplexLink"
    puts "Please use two instances of Module/Link and add a module to each."
    exit 0
}



