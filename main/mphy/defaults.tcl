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


Module/MPhy set TxPower_  0.01
Module/MPhy set NoiseSPD_ [expr 1.28e-23 * 300]

PacketHeaderManager set tab_(PacketHeader/MPhy)    1

MInterference/MIV set maxinterval_ 0.1 
MInterference/MIV set debug_ 0

Module/MPhy/BPSK set debug_ 0
Module/MPhy/BPSK set AcquisitionThreshold_dB_ 0
Module/MPhy/BPSK set BitRate_ 0

MPropagation/FullPropagation set maxDopplerShift_ 6.0
MPropagation/FullPropagation set beta_ 3.4
MPropagation/FullPropagation set shadowSigma_ 0.0
MPropagation/FullPropagation set refDistance_ 1.0
MPropagation/FullPropagation set rayleighFading_ 1.0
MPropagation/FullPropagation set timeUnit_ -100.0
MPropagation/FullPropagation set debug_ 0
MPropagation/FullPropagation set xFieldWidth_ 0
MPropagation/FullPropagation set yFieldWidth_ 0



