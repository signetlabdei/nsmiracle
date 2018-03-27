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

# helloInterval_ standard ns AODV value (sec)
Routing/AODV set helloInterval_ 1.0
# minHelloIntervalRatio_ standard ns AODV value (minHelloInterval = helloInterval_ * minHelloIntervalRatio_)
Routing/AODV set minHelloIntervalRatio_ 0.75
# maxHelloIntervalRatio_ standard ns AODV value (maxHelloInterval = helloInterval_ * maxHelloIntervalRatio_)
Routing/AODV set maxHelloIntervalRatio_ 1.25
# overheadLength_ standard ns AODV value (IP header length)
Routing/AODV set overheadLength_ 20
# standard ns AODV value
Routing/AODV set routeCacheFrequency_ 0.5
Routing/AODV set debug_ 0
# RREQ_GRAT_RREP standard ns AODV value
Routing/AODV set RREQ_GRAT_RREP        0x80
