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

PacketHeaderManager set tab_(PacketHeader/Multirate) 1

proc initPer {per} {
	source "per_table.tcl"
}

Node/MobileNode instproc initMultirateWifi { param0 } {
	$self instvar mac_
	set god [God instance]
	$mac_($param0) nodes [$god num_nodes]
	$mac_($param0) basicMode_ Mode1Mb
	$mac_($param0) dataMode_ Mode1Mb
}

Node/MobileNode instproc setDataMode { param0 mode} {
	$self instvar mac_
	$mac_($param0)  dataMode_ $mode
}


Node/MobileNode instproc setBasicMode { param0 mode} {
	$self instvar mac_
	$mac_($param0) basicMode_ $mode
}

Node/MobileNode instproc setPowerProfile { param0 pp} {
	$self instvar mac_ netif_
	$mac_($param0) powerProfile $pp
	$netif_($param0) powerProfile $pp
}

Node/MobileNode instproc setPER { param0 per} {
	$self instvar mac_
	$mac_($param0) per $per
}

Node/MobileNode instproc setNSE { param0 nse} {
	$self instvar mac_
	$mac_($param0) nse $nse
	$nse mac $mac_($param0)
}

Node/MobileNode instproc setOptimizer { param0 opt} {
	$self instvar mac_
	$opt mac $mac_($param0)
}

PER set debug_ 0

# default noise for 22 MHz channel at 300K = 9.108e-14
PER set noise_ [ expr {1.38E-23 * 300 * 22E6} ]


PowerProfile set debug_ 0
Mac/802_11/Multirate set useShortPreamble_ false
Mac/802_11/Multirate set gSyncInterval_ 0
Mac/802_11/Multirate set bSyncInterval_ 0
#Mac/802_11/Multirate set basicMode_ Mode1Mb
#Mac/802_11/Multirate set dataMode_ Mode1Mb


NSE/TimeWindow set tw_ 1e-3
NSE/TimeWindow set wSize_ 1.0
NSE/TimeWindow set pktLen_ 234

NSE/TimeWindow/EstimateIdleSlots set collisionTime_ 300e-6

Optimizer/Simple set OptInterval 10e-3
