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

#PacketHeaderManager set tab_(PacketHeader/ARP) 1
#PacketHeaderManager set tab_(PacketHeader/Mac802_11/Mrcl) 1

#Module/802_11 set node ""

#Module/802_11 instproc init {} {
#	$self instvar node
#}

proc create802_11MacModule {llname ifqName macName ip 802_11MacModule {qlen ""} } {
	set module [new Module/802_11]
	set ll [new $llname]
	set mac [new $macName]
	set ifq [new $ifqName]
	set macinterface [new MacInterface]
	set llinterface [new LLInterface]
	set loginterface [new LogInterface]

        set god [God instance]
	
	#if {$802_11MacModule == ""} {
	#	set node [new Node/MobileNode]
	#	$node addr $ip
	#} else {
	#	set node [$802_11MacModule set node]
	#}
	
	set arptable_ [new ARPTable/Mrcl $ip $mac]
	# FOR backward compatibility sake, hack only
	
	$arptable_ drop-target $macinterface
	
	$ll arptable $arptable_
	$ll mac $mac
	$ll down-target $ifq
	$ll up-target $llinterface
	
	$ifq target $mac
	if {$qlen != "" } {
		$ifq set limit_ $qlen
	}
	#set god_ [God instance]
	
	$mac nodes [$god num_nodes]
	$mac up-target $ll 
	
	
	$module setMac $mac
	$module setIfq $ifq
	$module setll $ll
	$module setMacInterface $macinterface
	$module setLogInterface $loginterface
	$module setLLInterface $llinterface
	#$module set node $node
	
	
	return $module
}

#proc createPosition {position} {
#	set p [new $position]
	
	#if {$802_11MacModule != ""} {
	#	set node [$802_11MacModule set node]
	#	$p mobileNode $node
	#}
#	return $p
#}
