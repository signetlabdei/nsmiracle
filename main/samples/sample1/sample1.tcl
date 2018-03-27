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

# ****** SAMPLE 1 *******

# This sample counts to create two nodes with only one layer and one module 
# per layer. SendModule of node 1 has to send periodically a packet which 
# is received from the ReceiveModule of node 2

# load the necessary library, firstly the ns-Miracle library
load ../../.libs/libMiracle.so.0.0.0
# then the trace library
load ../../trace/.libs/libTrace.so.0.0.0
# and finally the Sandbox library
load ../../sandbox/.libs/libMiracleSandbox.so.0.0.0

# this is the procedure which contains all the intructions to be done when
# the simulation is finished
proc finish {} {
	global ns tf 
	puts "---> SIMULATION DONE."
	$ns flush-trace
	close $tf
}

# Initialization of ambient variables
# period_ : is the period between each two packet transmissions
Module/SendModule set period_ 0.1
# destPort_: is the destination port value to be set in the packet header
Module/SendModule set destPort_ 0
# destAddr_: is the destination address value to be set in the packet header
Module/SendModule set destAddr_ 0
# debug_ (for all the bottom definitions): are the intenal c++ code variables that have to 
# manage the sections of code to be processed for debugging purposes
ChannelTest set debug_ 0
Module/SendModule set debug_ 0
Module/ReceiveModule set debug_ 0
ConnectorTrace/ChSAP set debug_ 0
ConnectorTrace/Bin set debug_ 0


set ns [new Simulator]
# Command "use-Miracle" : initialize internal ns-Miracle library variables
$ns use-Miracle

# define the output file
set tf [open out_sample1.tr w]
$ns trace-all $tf

# define the type of channel to use ("ChannelTest" is the one within the Sandbox library)
set channel [new ChannelTest]

# Begin of the sequence of actions that allows to create a node, firstly you have to define 
# the structure you want to generate
# "create-M_Node" : initialize internal node variables 
set node1 [$ns create-M_Node]
# instantiates a new module ModuleSend
set sm [new Module/SendModule]
# Command "addModule" : adds the module referenced by "$sm" in the protocol stack in layer 1 with
# detph 0 in cross layer messaging
set i [$node1 addModule 1 $sm 0 "SENDER"]
# Command "addToChannel": attach the channel defined above ($channel) to the module(s) in layer 1
# and set depth at level 2 in the ChSAP(s)
$node1 addToChannel $channel $sm 2
puts "Node 1 is generated."

# repeat the same operations for node 2
set node2 [$ns create-M_Node]
set rm [new Module/ReceiveModule]
set i [$node2 addModule 1 $rm 0 "RECV"]
$node2 addToChannel $channel $rm 1
puts "Node 2 is generated."

# Next step is define the steps to do during the simulation
# 1) send the "start" command to the SendModule, "$sm", so it starts to generate and transmit packeets
$ns at 0 "$sm start"
# 2) stop the packets generation
$ns at 50 "$sm stop"
# 3) finish the simulation (i.e., run the procedure define above in this script)
$ns at 51 "finish; $ns halt"
# Finally, run the simulation (type in command line "ns sample1.tcl")
$ns run