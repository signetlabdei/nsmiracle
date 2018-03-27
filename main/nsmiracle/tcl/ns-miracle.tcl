#
# Copyright (c) 2006 Regents of the SIGNET labs, University of Padova.
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
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#	This product includes software developed by the Computer Systems
#	Engineering Group at Lawrence Berkeley Laboratory.
# 4. Neither the name of the University of Padova (SIGNET lab) nor of the 
#    Laboratory may be used to endorse or promote products derived from 
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

ConnectorTrace set depth_ 0
ConnectorTrace set debug_ 0

#Module set debug_ 0

NodeCore set X_ 0
NodeCore set Y_ 0
NodeCore set battery_ 0


ConnectorTrace/Bin set depth 10
ConnectorTrace/Bin set depthUp_ 10
ConnectorTrace/Bin set depthDown_ 10
ConnectorTrace/Bin set depthToNC_ 10
ConnectorTrace/Bin set depthToP_ 10

ConnectorTrace/SAP set depthUp_ 0
ConnectorTrace/SAP set depthDown_ 0

ConnectorTrace/ChSAP set depthUp_ 0
ConnectorTrace/ChSAP set depthDown_ 0

ConnectorTrace/ClSAP set depthToP_ 0
ConnectorTrace/ClSAP set depthToNC_ 0

#Simulator set nLayer_ 0
#Simulator set nSap_ 0
#Simulator set nChSap_ 0


Class M_Node

M_Node set nLayer_ 0
M_Node set nSap_ 0
M_Node set nChSap_ 0

M_Node set id_ 0
M_Node set nPlugin_ 0

M_Node instproc init args {
	$self instvar id_ 
	$self instvar nPlugin_
	
	$self set id_ 0
	$self set nPlugin_ 0
	$self set position_ ""
}


M_Node instproc addModule {layer module param1 {param2 ""} {param3 ""}} {
	$self instvar nPlugin_ bin_ nodeCore_ traceCLfd_ modules_ layer_ id_
	
    if {$layer < 1} {
	puts " addModule: invalid value ($layer) for layer, must be >=1"
	exit 1
    }

	$module Id_ $nPlugin_
	$module setlayer $layer

	set clsap [new ConnectorTrace/ClSAP]
	$module addBin $bin_
	$module addclsap $clsap

	set clDepthP 0
	set clDepthNC 0
	set moduleName ""
	if {$param3 == ""} {
		set clDepthP $param1
		set clDepthNC $param1
		set moduleName $param2
	} else {
		set clDepthP $param1
		set clDepthNC $param2
		set moduleName $param3
	}
	if {$moduleName != ""} {
		$module settag "$moduleName"
	} elseif {[$module gettag] == "---"} {
		$module settag "$nPlugin_"
	}

	
	$clsap set depthToP_ $clDepthP
	$clsap set depthToNC_ $clDepthNC
	$clsap module $module
	$clsap nodecore $nodeCore_
	$clsap preambleToPlugin "$id_ NC [$module gettag]"
	$clsap preambleToNodeCore "$id_ [$module gettag] NC" 
	$clsap attach $traceCLfd_
	$nodeCore_ addclsap $clsap $layer
	
	set modules_($nPlugin_) $module
	set layer_($nPlugin_) $layer
	
	incr nPlugin_
}

M_Node instproc addPlugin {plugin param1 {param2 ""} {param3 ""}} {
	$self instvar nPlugin_ bin_ nodeCore_ traceCLfd_ modules_ layer_ id_
	
	$plugin Id_ $nPlugin_
	
	set clsap [new ConnectorTrace/ClSAP]
	$plugin addBin $bin_
	$plugin addclsap $clsap
	set clDepthP 0
	set clDepthNC 0
	set pluginName ""
	if {$param3 == ""} {
		set clDepthP $param1
		set clDepthNC $param1
		set pluginName $param2
	} else {
		set clDepthP $param1
		set clDepthNC $param2
		set pluginName $param3
	}
	if {$pluginName != ""} {
		$plugin settag "$pluginName"
	} elseif {[$plugin gettag] == "---"} {
		$plugin settag "$nPlugin_"
	}
	$clsap set depthToP_ $clDepthP
	$clsap set depthToNC_ $clDepthNC
	$clsap module $plugin
	$clsap nodecore $nodeCore_
	$clsap preambleToPlugin "$id_ NC [$plugin gettag]"
	$clsap preambleToNodeCore "$id_ [$plugin gettag] NC" 
	$clsap attach $traceCLfd_
	$nodeCore_ addclsap $clsap 0
	
	set modules_($nPlugin_) $plugin
	set layer_($nPlugin_) 0
	
	incr nPlugin_
}

M_Node instproc contains {module} {
	$self instvar modules_ layer_
	if {$modules_([$module Id_]) == $module} {
		return 1
	} else {
		return 0
	}
	#return [expr $modules_([$module Id_]) == $module]
}

M_Node instproc addPosition {position} {
	$self instvar nodeCore_
	
	#$self set position_ $position
	$nodeCore_ position $position
}

M_Node instproc position {} {
	$self instvar nodeCore_
	
	return [$nodeCore_ position]
}


M_Node instproc setConnection {moduleUp moduleDown {trace "notrace"} {traceup ""}} {
	$self instvar nodeCore_ tracefd_ layer_ id_
	#puts "add sap in layer $layer between module $nModule and $nModuleDown (depth=$depth)"
	#for {set i 0} {$i <= $nLayer_} { incr i} {
	#	set n $pluginPerLayer_($i)
	#	puts "Layer $i pluginPerLayer=$pluginPerLayer_($i)"
	#	if {$n > 0} {
	#		for {set j 0} {$j < $n} {incr j} {
	#			puts "layer $i module $j id [$layers_($i,$j) Id_]"
	#		}
	#	}
	#}
	if {$moduleUp == "" || $moduleDown == ""} {
		error "The specified module does not exist !"
	}
	if {![$self contains $moduleUp]} {
		error "The given up module does not belong to this node!!"
	}
	if {![$self contains $moduleDown]} {
		error "The given down module does not belong to this node!!"
	}
	if {$layer_([$moduleUp Id_]) < [expr $layer_([$moduleDown Id_])]} {
		error "The up module must be above or at most in the same layer of the down module!!"
	}
	set depth 0
	if {$trace == "trace"} {
		set depth 1
	}
	# verify if trace is an integer value
	if {[regexp {^[0-9]+$} $trace]} {
		set depth $trace
	}
	set depthup $depth
	if {$traceup != ""} {
		set depthup 0
		if {$traceup == "trace"} {
			set depthup 1
		}
		# verify if traceup is an integer value
		if {[regexp {^[0-9]+$} $traceup]} {
			set depth $traceup
		}
	}
	set sap [new ConnectorTrace/SAP]
	#puts "layer up $sap_($i,0), module up $sap_($i,1), module down $sap_($i,2)"
	$sap upmodule $moduleUp
	$sap downmodule $moduleDown
	
	#puts "sap in $id between $upModule and $downModule"
	$sap preambleUp "$id_ [$moduleDown gettag] [$moduleUp gettag]"
	$sap preambleDown "$id_ [$moduleUp gettag] [$moduleDown gettag]"
	$sap attach $tracefd_
	$sap set depthDown_ $depth
	$sap set depthUp_ $depthup
	$moduleUp addDownsap $sap
	$moduleDown addUpsap $sap
	return $sap
}

M_Node instproc addToChannel {channel module {trace "notrace"} {traceup ""}} {
	$self instvar nodeCore_ id_ tracefd_
	
	if {$channel == "" || $module == ""} {
		error "The specified down module does not exist!"
	}
	if {$nodeCore_ == ""} {
		error "The node must be setup first!"
	}
	set depth 0
	if {$trace == "trace" || $trace == "1"} {
		set depth 1
	}
	set depthup $depth
	if {$traceup != ""} {
		if {$traceup == "trace" || $traceup == "1"} {
			set depthup 1
		} else {
			set depthup 0
		}
	}
	set chsap [new ConnectorTrace/ChSAP]
	$chsap module $module
	$chsap channel $channel
        if {[$channel gettag] == "---"} {
	    set chtag "CH" 
        } else {
	    set chtag [$channel gettag]
	}
        $chsap preambleUp "$id_ $chtag [$module gettag]"
	$chsap preambleDown "$id_ [$module gettag] $chtag"
	$chsap attach $tracefd_
	$chsap set depthDown_ $depth
	$chsap set depthUp_ $depthup
	$chsap nodeCore $nodeCore_
	$module addDownsap $chsap
	$channel addsap $chsap
	return $chsap
}

#End class M_Node

Simulator instproc create-M_Node {{tracefd ""} {traceCLfd ""}} {
	#$self instvar nLayer_ pluginPerLayer_ layers_ nSap_ sap_ nChSap_ chSap_
	$self instvar nM_Node
	
	if {$tracefd == ""} {
	    set tracefd [$self get-ns-traceall]
	}
	if {$traceCLfd == ""} {
	    set traceCLfd $tracefd
	}
	#puts "tracefd $tracefd traceCLfd $traceCLfd"
	
	#if {$nLayer_ <= 0} {
	#	error "There are no layer defined"
	#}
	#if {$nSap_ <= 0 && $chSap_ <= 0} {
	#	error "There are no declaration of module interconection"
	#}
	
	set node [new M_Node]
	$node set tracefd_ $tracefd
	$node set traceCLfd_ $traceCLfd
	$node set id_ $nM_Node
	
	$node set nodeCore_ [new NodeCore]
	$node set bin_ [new ConnectorTrace/Bin]
	[$node set bin_] preamble "$nM_Node BIN"
	[$node set bin_] attach $tracefd
	
	incr nM_Node
	#set tracefd [$self get-ns-traceall]
	#$node setup $nLayer_ $pluginPerLayer_ $layers_ $nSap_ $sap_ $nChSap_ $chSap_ $tracefd
	return $node
}

Simulator instproc use-Miracle {} {
	$self instvar nM_Node
	$self set nM_Node 0
}
