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


Module/UMTS/RLC instproc init {args} {
    eval $self next $args
}


Class Module/UMTS/RLC/AM -superclass Module/UMTS/RLC
Class Module/UMTS/RLC/UM -superclass Module/UMTS/RLC


# Class Module/Mphy/UMTS    -superclass Module/MPhy
# Class Module/MPhy/UMTS/BS -superclass Module/MPhy/UMTS
# Class Module/MPhy/UMTS/ME -superclass Module/MPhy/UMTS

Class Module/UMTS/MAC/BS -superclass Module/UMTS/MAC
Class Module/UMTS/MAC/ME -superclass Module/UMTS/MAC


Module/UMTS/RLC/AM instproc init {args} {
    eval $self next $args    
    $self setRlc [new UMTS/RLC/AM]
}


Module/UMTS/RLC/UM instproc init {args} {
    eval $self next $args
    $self setRlc [new UMTS/RLC/UM]
}

# Sets up a bidirectional connection between the ME RLC and the BS RLC
Module/UMTS/RLC instproc connect2CoreNetRlc {bs_rlc} {
    set bsid   [$bs_rlc getRlcId]
    set meid   [$self   getRlcId]
    set bsmac  [$bs_rlc set mymac_]
    set memac  [$self set mymac_]
    set bsphy  [$bs_rlc set myphy_]
    set mephy  [$self set myphy_]

    $self    setDestRlcId  $bsid
    $bs_rlc  setDestRlcId  $meid
    $bs_rlc  setMeCodeId   [$self set myphy_code_id_]
    
    #set ip addresses
    $self set destIpAddr_ [$bs_rlc set ipAddr_]
    $bs_rlc set destIpAddr_ [$self set ipAddr_]

    #this is not really needed, just to get nicer traces...
    $self  setMeCodeId   [$self set myphy_code_id_]

    $bsmac set dst_rlc_id_ $meid
    $bsmac set me_code_id_ [$self set myphy_code_id_]
    $memac set dst_rlc_id_ $bsid

    $bsmac set initslotoffset_ 1
    $memac set initslotoffset_ 0.5
    $bsmac start
    $memac start
    
    # tell to phy layer which are the rlcId to supervise for Power Control algorithm
    $bsphy monitor $meid
    $mephy monitor $bsid
}




proc finalizeUmtsStack {phymodule macmodule rlcmodule ipAddr} {

    set rlc [$rlcmodule getRlc]

    set chr      [$phymodule set chip_rate_]
    set sf       [$phymodule set spreading_factor_]
    set cdr      [$phymodule set coding_rate_ ]
    set cid      [$phymodule set code_id_]
    set TTI      [$rlc set TTI_] 
    set TTI_PDUs [$rlc set TTI_PDUs_]


    set userbw   [$phymodule getDataRate ]  
    set slotd    [expr $TTI / $TTI_PDUs ]
    set payload  [expr ceil($userbw * $slotd /8)]

    $phymodule set slot_duration_  $slotd
    $rlc set bandwidth_ $userbw
    $rlc set payload_  $payload
    $rlcmodule set ipAddr_ $ipAddr

    $phymodule setCorrelation   [new Correlation/UMTS]
    $phymodule computeSIRtarget $payload
    [$phymodule getCorrelation] set code_id_ $cid

    $macmodule set TTI_             $TTI
    $macmodule set TTI_PDUs_        $TTI_PDUs
    $macmodule set slot_duration_   $slotd
    $macmodule set src_rlc_id_      [$rlcmodule getRlcId]

    # This variables are needed in Module/UMTS/RLC::connect
    # so you'll get an error if you forget to call finalizeUmtsStack 
    # before that
    $rlcmodule set myphy_code_id_ $cid
    $rlcmodule set mymac_           $macmodule
    $rlcmodule set myphy_           $phymodule

}





