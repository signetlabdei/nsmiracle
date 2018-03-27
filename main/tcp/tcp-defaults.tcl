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



Module/TCP set debug_ 0
Module/TCP/Tahoe set debug_ 0
Module/TCP/Reno set debug_ 0
Module/TCP/Newreno set debug_ 0
Module/TCP/Vegas set debug_ 0
Module/TCP/Sack1 set debug_ 0
Module/TCP/FACK set debug_ 0
Module/TCPSink set debug_ 0
Module/TCPSink/DelAck set debug_ 0
Module/TCPSink/Sack1 set debug_ 0
Module/TCPSink/Sack1/DelAck set debug_ 0

Module/TCP set agent_addr_ 0
Module/TCP set agent_port_ 0
Module/TCP set dst_addr_ 0
Module/TCP set dst_port_ 0



# Packet Headers needed by the TCP modules wrapped in Miracle modules

PacketHeaderManager set tab_(PacketHeader/Flags)      1
PacketHeaderManager set tab_(PacketHeader/Mac)        1
PacketHeaderManager set tab_(PacketHeader/IP)         1
PacketHeaderManager set tab_(PacketHeader/TCP)        1
PacketHeaderManager set tab_(PacketHeader/QS)         1


