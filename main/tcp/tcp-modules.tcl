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

 

Module/TCP instproc init {args} {
    eval $self next $args
#     if [ $self getTcpAgent] eq NONE {
# 	$self setTcpAgent [new Agent/TCP]
#     }

}



Class Module/TCP/Tahoe -superclass Module/TCP

Module/TCP/Tahoe instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCP]

}



Class Module/TCP/Reno -superclass Module/TCP

Module/TCP/Reno instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCP/Reno]
}

Class Module/TCP/Newreno -superclass Module/TCP

Module/TCP/Newreno instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCP/Newreno]
}

Class Module/TCP/Vegas -superclass Module/TCP

Module/TCP/Vegas instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCP/Vegas]
}


Class Module/TCP/Sack1 -superclass Module/TCP

Module/TCP/Sack1  instproc init {args} {
    $self setTcpAgent [new Agent/TCP/Sack1]
    eval $self next $args
}

Class Module/TCP/FACK -superclass Module/TCP

Module/TCP/FACK instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCP/FACK]
}



Class Module/TCPSink -superclass Module/TCP

Module/TCPSink instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCPSink]
}


Class Module/TCPSink/DelAck -superclass Module/TCP

Module/TCPSink/DelAck  instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCPSink/DelAck]
}


Class Module/TCPSink/Sack1 -superclass Module/TCP

Module/TCPSink/Sack1 instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCPSink/Sack1]
}


Class Module/TCPSink/Sack1/DelAck -superclass Module/TCP

Module/TCPSink/Sack1/DelAck instproc init {args} {
    eval $self next $args
    $self setTcpAgent [new Agent/TCPSink/Sack1/DelAck]
}
