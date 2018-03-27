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

PacketHeaderManager set tab_(PacketHeader/FCM) 1

Module/FCM/SimpleMAC set packetSize_      500
Module/FCM/SimpleMAC set period_          0.2
Module/FCM/SimpleMAC set destPort_        0
Module/FCM/SimpleMAC set destAddr_        0
Module/FCM/SimpleMAC set debug_           0
Module/FCM/SimpleMAC set alpha_           0.9 
Module/FCM/SimpleMAC set blockSize_       32
Module/FCM/SimpleMAC set symSize_         1
Module/FCM/SimpleMAC set totBlock_        2
Module/FCM/SimpleMAC set genType_         0
Module/FCM/SimpleMAC set seed_            100
Module/FCM/SimpleMAC set max_rounds_      5
Module/FCM/SimpleMAC set propSpeed_       1500
Module/FCM/SimpleMAC set n_class          1
Module/FCM/SimpleMAC set d_max            20000
Module/FCM/SimpleMAC set d_lower          1000
Module/FCM/SimpleMAC set d_upper          6000
Module/FCM/SimpleMAC set TxRadius_        5000.0

Module/FCM/SimpleMAC/MHopCtrl set debug_       0
Module/FCM/SimpleMAC/MHopCtrl set TxRadius_    5000.0
Module/FCM/SimpleMAC/MHopCtrl set TotalBlocks_ 1
Module/FCM/SimpleMAC/MHopCtrl set Rmax_        15000.0
Module/FCM/SimpleMAC/MHopCtrl set genType_     0
Module/FCM/SimpleMAC/MHopCtrl set RandSeed_    10
Module/FCM/SimpleMAC/MHopCtrl set Lambda_      5


Module/FCM/SimpleMAC/CRBCastCtrl set debug_       0
Module/FCM/SimpleMAC/CRBCastCtrl set TxRadius_    6000.0
Module/FCM/SimpleMAC/CRBCastCtrl set TotalBlocks_ 1
Module/FCM/SimpleMAC/CRBCastCtrl set genType_     0
Module/FCM/SimpleMAC/CRBCastCtrl set PbCast_      0.5
Module/FCM/SimpleMAC/CRBCastCtrl set RandSeed_    10
Module/FCM/SimpleMAC/CRBCastCtrl set Rmax_        15000.0
#Module/FCM/SimpleMAC instproc init {args} {
#    $self next $args
#    $self settag "FcSMAC"
#}


