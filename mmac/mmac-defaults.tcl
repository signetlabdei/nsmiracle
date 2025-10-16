#
# Copyright (c) 2008 Regents of the SIGNET lab, University of Padova.
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

Module/MMac/ALOHA set debug_ 0

Module/MAC/Generic set debug_ 0

Module/MMac/CSMA   set HeaderSize_        16
Module/MMac/CSMA   set BaseBackoffTime_   0.001
Module/MMac/CSMA   set AckTimeout_        0.001
Module/MMac/CSMA   set debug_states_      0 
Module/MMac/CSMA   set debug_             0


Module/MMac/ALOHA/ADV set HDR_size_ 		0
Module/MMac/ALOHA/ADV set ACK_size_  		32
Module/MMac/ALOHA/ADV set max_tx_tries_		5
Module/MMac/ALOHA/ADV set wait_costant_		0.1
Module/MMac/ALOHA/ADV set debug_		0
Module/MMac/ALOHA/ADV set backoff_tuner_   	1
Module/MMac/ALOHA/ADV set max_payload_		500
Module/MMac/ALOHA/ADV set ACK_timeout_		30.0
Module/MMac/ALOHA/ADV set alpha_		0.8
Module/MMac/ALOHA/ADV set buffer_pkts_		-1
Module/MMac/ALOHA/ADV set max_backoff_counter_   4
Module/MMac/ALOHA/ADV set listen_time_ 0

