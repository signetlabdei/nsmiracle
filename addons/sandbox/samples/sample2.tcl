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


#
# ****** SAMPLE 2 *******
#
#     Node 1                 Node 2
# +---------------+     +---------------+
# | ModuleSend    |     | ModuleSend    |
# +---------------+     +---------------+
# | ModuleMiddle  |     | ModuleMiddle  |
# +---------------+     +---------------+
#         |                     |
#     +-----------------------------+
#     |            Channel          |
#     +-----------------------------+
#


# In this this sample we planned to create two nodes with two layers and one 
# module per layer. Module called SendModule in layer 2 of node 1 has to send 
# periodically a packet which is received from the Module called ReceiveModule 
# of node 2 (layer 2).
# In both nodes, in layer 1, we placed 2 Modules called MiddleModule which
# have only to forward to the bottom layer the packets received from the above 
# one. SendModule also wants to discover the modules which have been
# plugged above and below to it by means of a cross-layer message 

source dynlibutils.tcl

# load the necessary libraries, firstly the ns-Miracle library
dynlibload Miracle ../../nsmiracle/.libs/
# then the cross layer messages trace library
dynlibload ClTrace ../../cltrace/.libs/
# the Sandbox library
dynlibload MiracleSandbox ../src/.libs/
# the mypacket tracer library
dynlibload mypkttracer ../src/.libs/



# this is the procedure which specifies all the intructions to be done when
# the simulation is finished
proc finish {} {
    global ns opt
    puts "---> SIMULATION DONE."
    close $opt(dtfd)
    close $opt(cltfd)
    puts "Data Trace File:          $opt(dtf) "
    puts "Cross-layer Trace File:   $opt(cltf)"
}

# Initialization of ambient variables
# period_ : is the period between each two packet transmissions
Module/SendModule set period_ 0.1
# destPort_: is the destination port value to be set in the packet header
Module/SendModule set destPort_ 0
# destAddr_: is the destination address value to be set in the packet header
Module/SendModule set destAddr_ 0
# debug_ (for all the bottom definitions): are the intenal c++ Class variables used to 
# manage the sections of code to be processed for debugging purposes
ChannelTest set debug_ 0
Module/SendModule set debug_ 0
Module/MiddleModule set debug_ 0
Module/ReceiveModule set debug_ 0
ConnectorTrace/ChSAP set debug_ 0
ConnectorTrace/Bin set debug_ 0

set ns [new Simulator]
# Command "use-Miracle" : initialize internal ns-Miracle library variables
$ns use-Miracle


# define the type of channel to use ("ChannelTest" is the one within the Sandbox library)
set channel [new ChannelTest]


# define tracefile names
set opt(dtf) "/tmp/data.tr"
set opt(cltf) "/tmp/crosslayer.tr"

# open trace files and set file descriptor variables
set opt(dtfd)  [open $opt(dtf)  w ]
set opt(cltfd) [open $opt(cltf) w ]


# Begin of the sequence of actions that allows to create a node, firstly you have to define 
# the structure you want to generate

# "create-M_Node" : initialize internal node variables 
# the (optional) file descriptors indicate the trace files to be used for data and cross-layer messages
set node1 [$ns create-M_Node $opt(dtfd) $opt(cltfd) ]

# instantiates a new module ModuleSend
set sm1 [new Module/SendModule]

# instantiates a new module MiddleModule
set mm1 [new Module/MiddleModule]

# Command "addModule" : adds the module referenced by "$sm1" (SendModule) in the protocol 
# stack in layer 2 with detph 10 in packet tracing and set "SEND1" tag in trace file.
set i [$node1 addModule 2 $sm1 1 "SEND1"]

# Command "addModule" : adds the module referenced by "$mm1" (MiddleModule) in the protocol 
# stack in layer 1 with detph 10 in packet tracing and set "MIDDLE1" tag in trace file.
set j [$node1 addModule 1 $mm1 1 "MIDDLE1"]

# Command "setConnection": adds a SAP between modules $sm1 and $mm1 and sets the depth 
# of tracing to level 10
$node1 setConnection $sm1 $mm1 1

# Command "addToChannel": attach the channel defined above ($channel) to the module(s) $mm1
# and set depth at level 10 in the ChSAP(s)
$node1 addToChannel $channel $mm1 1

puts "Node 1 is generated."

# repeat the same operations for node 2
set node2 [$ns create-M_Node $opt(dtfd) $opt(cltfd)]
set rm2  [new Module/ReceiveModule]
set mm2 [new Module/MiddleModule]
set i [$node2 addModule 2 $rm2 1 "RECV2"]
set j [$node2 addModule 1 $mm2 1 "MIDDLE2"]
$node2 setConnection $rm2 $mm2 1
$node2 addToChannel $channel $mm2 1
puts "Node 2 is generated."


puts "---> BEGIN SIMULATION"
# Next step is schedule the sequance of actions to be executed during the simulation
# 1) send the "start" command to the SendModule, "$sm1", so it starts to generate and transmit packets
# each period_ secs (see def above)
$ns at 0 "$sm1 start"

$ns at 1 "$sm1 discover async up"
$ns at 2 "$sm1 discover async down"
$ns at 3 "$mm1 discover async up"
$ns at 4 "$mm1 discover async down"
$ns at 5 "$sm1 discover async all"
$ns at 6 "$mm1 discover async all"

$ns at 11 "$sm1 discover sync up"
$ns at 12 "$sm1 discover sync down"
$ns at 13 "$mm1 discover sync up"
$ns at 14 "$mm1 discover sync down"
$ns at 15 "$sm1 discover sync all"
$ns at 16 "$mm1 discover sync all"


# 7) stop the packets generation
$ns at 20 "$sm1 stop"
# 8) finish the simulation (i.e., run the procedure define above in this script)
$ns at 21 "finish; $ns halt"
# Finally, run the simulation
$ns run
