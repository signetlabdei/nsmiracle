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

Module/802_11 set node ""

#Module/802_11 instproc init {} {
#	$self instvar node
#}

proc create802_11MacModule {llname ifqName macName ip 802_11MacModule qlen} {
	set module [new Module/802_11]
	set ll [new $llname]
	set mac [new $macName]
	set ifq [new $ifqName]
	set macinterface [new MacInterface]
	set llinterface [new LLInterface]
	set loginterface [new LogInterface]
	
	if {$802_11MacModule == ""} {
		set node [new Node/MobileNode]
		$node addr $ip
	} else {
		set node [$802_11MacModule set node]
	}
	
	set arptable_ [new ARPTable $node $mac]
	# FOR backward compatibility sake, hack only
	
	$arptable_ drop-target $macinterface
	
	$ll arptable $arptable_
	$ll mac $mac
	$ll down-target $ifq
	$ll up-target $llinterface
	
	$ifq target $mac
	$ifq set limit_ $qlen
	set god_ [God instance]
	
	$mac nodes [$god_ num_nodes]
	$mac up-target $ll 
	
	
	$module setMac $mac
	$module setIfq $ifq
	$module setll $ll
	$module setMacInterface $macinterface
	$module setLogInterface $loginterface
	$module setLLInterface $llinterface
	$module set node $node
	
	
	return $module
}

proc createPhyModule {phyname pmodel anttype 802_11MacModule em} {
	set module [new Module/Phy]
	set ant_ [new $anttype]
	set netif [new $phyname]
	
	$netif channel $module
	$netif up-target $module

	$netif propagation $pmodel	;# Propagation Model
	$netif antenna $ant_
	
	$module setPhy $netif
	
	if {$802_11MacModule == ""} {
		set node [new Node/MobileNode]
	} else {
		set node [$802_11MacModule set node]
		set mac [$802_11MacModule getMac]
		$mac netif $netif
	}
	$netif node $node
	if {$em != ""} {
		$node addenergymodel [new EnergyModel]
	}
	return $module
}

proc createPosition {802_11MacModule} {
	set p [new Position/MB]
	
	if {$802_11MacModule != ""} {
		set node [$802_11MacModule set node]
		$p mobileNode $node
	}
	return $p
}

proc createIPModule {} {
	set module [new Module/IP]
	return $module
}

proc createWirelessChModule {CSThres freq L} {
	set module [new Module/WirelessCh]
	# set the Carrier Sense Threshold
	if {$CSThres == ""} {
		# set to defualt value (from ns)
		$module set CSThresh_ 1.559e-11
	} else {
		$module set CSThresh_ CSThres
	}
	# set the frequency in use
	if {$freq == ""} {
		# set to defualt value (ISM band)
		$module set freq_ 2.472e9
	} else {
		$module set freq_ freq
	}
	#set the system loss factor
	if {$L == ""} {
		# set to defualt value (no system loss)
		$module set L_ 1.0
	} else {
		$module set L_ L
	}
	return $module
}

Module/WirelessCh set CSThresh_ 1.559e-11
Module/WirelessCh set freq_ 2.472e9
Module/WirelessCh set L_ 1.0
Module/WirelessCh set debug_ 0

Module/Transport set debug_ 0

Module/IP set debug_ 0

MacInterface set debug_ 0

LogInterface set debug_ 0

LLInterface set debug_ 0

Module/802_11 set debug_ 0

Module/Phy set debug_ 0
