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

# ****** SAMPLE 4 *******
#
#                |n|     Node 1                                              Node 2
#                |o| +-------------------+    +------------------+     +---------------+
#                |d+-+     ModuleSend    |----+ ModuleSend       |     | ModuleReceive |
#                |e| +-------------------+----+------------------+     +---------------+
#                |c+-+ ModuleMiddle | ModuleMiddle |ModuleMiddle |     | ModuleMiddle  |
# +-----------+  |o| +--------------+--------------+-------------+     +---------------+
# | Plugin    +--+r|                |              |                          |
# +-----------+  |e|    +-------------------------------------------------------+
#                       |                          Channel                      |
#                       +-------------------------------------------------------+
#
#
# In this sample the nodes have the layout represented in the picture. 
# In particular, node1 has a plugin which is connected to all modules
# by means of the node core which, in the background, acts as a
# communication bus between all modules and plugins attached to the
# node.
#
# In this sample we planned to create two nodes with tree layers with one 
# module per layer and a PlugIn connected to NodeCore. Module called SendModule 
# has to send periodically a packet, which will be received by ReceiveModule 
# in node 2. SandBoxPlugin1 has to discover the SendModule in the architecture 
# (i.e., learn its internal address in order to communicate directly) by
# means of discover cross layer message (SANDBOXPLG1MSG) sent a broadcast fashion.
# After SendModule received this packet, it has to answer in order to 
# publicize its id in the architecture. Finally SandBoxPlugin1 may now
# set the size of the packet thanks to SANDBOXPLG1MSGS cross layer message
# sent to SendModule.

source dynlibutils.tcl

# load the necessary libraries, firstly the ns-Miracle library
dynlibload Miracle ../../nsmiracle/.libs/
# then the cross layer messages trace library
dynlibload ClTrace ../../cltrace/.libs/
# the Sandbox library
dynlibload MiracleSandbox ../src/.libs/
# the mypacket tracer library
dynlibload mypkttracer ../src/.libs/

# this is the procedure which defines all the intructions to be done when
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


set ns [new Simulator]

# Command "use-Miracle" : initialize internal ns-Miracle library variables
$ns use-Miracle


# define the type of channel to use 
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
set node1 [$ns create-M_Node $opt(dtfd) $opt(cltfd)]

# instantiates all modules for node 1
set sm11 [new Module/SendModule]
set sm12 [new Module/SendModule]
set sm13 [new Module/SendModule]
set mm11 [new Module/MiddleModule]
set mm12 [new Module/MiddleModule]
set mm13 [new Module/MiddleModule]

# instantiates a plugin for node 1
set plg1 [new SandboxPlugIn1]


# inter-module trace verbosity
set imtv  1
# cross-layer bus messages trace verbosity
set cltv 1  

# Add all modules to the protocol stack


set layer 2
$node1 addModule $layer $sm11 $cltv "S11"
$node1 addModule $layer $sm12 $cltv "S12"
set layer 1
$node1 addModule $layer $mm11 $cltv "M11"
$node1 addModule $layer $mm12 $cltv "M12"
$node1 addModule $layer $mm13 $cltv "M13"

# Command "addplugin" : attaches the plug-in referenced by "$plg1"
# to the NodeCore 
$node1 addPlugin $plg1 $cltv "PLG1"

# Create interconnections among modules. 
# note that S11 is connected to M11 and M12
# and that S12 is connected to M12 and M13
$node1 setConnection $sm11 $mm11  $imtv
$node1 setConnection $sm11 $mm12  $imtv
$node1 setConnection $sm12 $mm12  $imtv
$node1 setConnection $sm12 $mm13  $imtv

# Command "addToChannel": attach the channel defined above ($channel) to the module(s) in layer 1
# and set depth at level 2 in the ChSAP(s)
$node1 addToChannel $channel $mm11 $imtv
$node1 addToChannel $channel $mm12 $imtv
$node1 addToChannel $channel $mm13 $imtv

puts "Node 1 is generated."


# node 2 has a much simpler architecture
set node2 [$ns create-M_Node $opt(dtfd) $opt(cltfd)]
set rm2 [new Module/ReceiveModule]
set mm2 [new Module/MiddleModule]
$node2 addModule 2 $rm2 $cltv
$node2 addModule 1 $mm2 $cltv
$node2 setConnection $rm2 $mm2 $imtv
$node2 addToChannel $channel $mm2 $imtv
puts "Node 2 is generated."

puts "---> BEGIN SIMULATION"







# Next step is schedule the sequance of actions to be executed during the simulation
# 1) send the "start" command to the SendModule, "$sm1", so it starts to generate and transmit packets
# each period_ secs (see def above)
#$ns at 0 "$sm11 start"
# 2) SandboxPlugin1 has to send SANDBOXPLG1MSG cross layer message in order to discover the 
# SendModule within the architecture and then it has to send to it the new packet size by 
# means of  the SANDBOXPLG1MSGS cross layer message
$ns at 0.5 "$plg1 discover"

$ns at 0.8 "$sm12 discover async down"
$ns at 1 "$sm11 discover async up"
$ns at 2 "$sm11 discover async down"
$ns at 3 "$mm11 discover async up"
$ns at 4 "$mm11 discover async down"
$ns at 5 "$sm11 discover async all"
$ns at 6 "$mm11 discover async all"

$ns at 11 "$sm11 discover sync up"
$ns at 12 "$sm11 discover sync down"
$ns at 13 "$mm11 discover sync up"
$ns at 14 "$mm11 discover sync down"
$ns at 15 "$sm11 discover sync all"
$ns at 16 "$mm11 discover sync all"


# 3) stop the packets generation
#$ns at 20 "$sm11 stop"
# 4) finish the simulation (i.e., run the procedure define above in this script)
$ns at 21 "finish; $ns halt"
# Finally, run the simulation
$ns run
