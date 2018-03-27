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


# UMTS starts
UMTS/RLC/UM set avoidReordering_                      false
UMTS/RLC/UM set payload_                              0
UMTS/RLC/UM set TTI_PDUs_                             15
UMTS/RLC/UM set bandwidth_                            0
UMTS/RLC/UM set debug_                                false
UMTS/RLC/UM set macDA_                                -1
UMTS/RLC/UM set win_                                  1024
UMTS/RLC/UM set temp_pdu_timeout_time_                10ms
UMTS/RLC/UM set buffer_level_max_                     500
UMTS/RLC/UM set TTI_                                  10ms
UMTS/RLC/UM set length_indicator_                     7
UMTS/RLC/UM set min_concat_data_                      3
UMTS/RLC/UM set delay_                                50us

UMTS/RLC/AM set avoidReordering_                      false
UMTS/RLC/AM set ack_mode_                             2
UMTS/RLC/AM set win_                                  1024
UMTS/RLC/AM set maxRBSize_                            100kbytes
UMTS/RLC/AM set overhead_                             0
UMTS/RLC/AM set payload_                              0
UMTS/RLC/AM set TTI_PDUs_                             15
UMTS/RLC/AM set rtx_timeout_                          140ms
UMTS/RLC/AM set noFastRetrans_                        0
UMTS/RLC/AM set numdupacks_                           2
UMTS/RLC/AM set poll_PDU_                             256
UMTS/RLC/AM set poll_timeout_                         85ms
UMTS/RLC/AM set stprob_timeout_                       40ms
UMTS/RLC/AM set macDA_                                -1
UMTS/RLC/AM set maxdat_                               3
UMTS/RLC/AM set debug_                                false
UMTS/RLC/AM set bandwidth_                            0
UMTS/RLC/AM set TTI_                                  10ms
UMTS/RLC/AM set length_indicator_                     7
UMTS/RLC/AM set ack_pdu_header_                       1
UMTS/RLC/AM set status_pdu_header_                    20
UMTS/RLC/AM set min_concat_data_                      3
UMTS/RLC/AM set max_status_delay_                     10ms
UMTS/RLC/AM set max_ack_delay_                        10ms
UMTS/RLC/AM set delay_                                50us
UMTS/RLC/AM set ack_PDUs_                             0
UMTS/RLC/AM set ack_SDUs_                             0
UMTS/RLC/AM set drop_PDUs_                            0
UMTS/RLC/AM set drop_SDUs_                            0
UMTS/RLC/AM set tot_PDUs_                              0
UMTS/RLC/AM set err_PDUs_                             0
UMTS/RLC/AM set sender_debug_                         0
UMTS/RLC/AM set receiver_debug_                         0


Module/MPhy/UMTS set chip_rate_                       3840000
Module/MPhy/UMTS set coding_type_                     1        
Module/MPhy/UMTS set coding_rate_                     0.5

Module/MPhy/UMTS set slot_duration_                0
Module/MPhy/UMTS set code_id_                      0
# alpha_ is used to compute the signal power used for the data from the whole one
# which is comprehensive of data and cotrol, its value is 49/225
Module/MPhy/UMTS set alpha_                        0.2177777777
# SIRtarget_ 1.45 -> BER 10^-2
# SIRtarget_ 1.78 -> BER 10^-3
# SIRtarget_ 2.04 -> BER 10^-4
# SIRtarget_ 2.26 -> BER 10^-5
# SIRtarget_ 2.44 -> BER 10^-6

# NOTE: users should NOT modify SIRtarget_ since it is calculated upon
# finalization of the UMTS stack using PERtarget_  
Module/MPhy/UMTS set SIRtarget_                    0
Module/MPhy/UMTS set PERtarget_		     0.00001
Module/MPhy/UMTS set PowerCtrlUpdateCmdAtStartRx_   1
Module/MPhy/UMTS set PowerCtrlReadCmdAtStartRx_    1
Module/MPhy/UMTS set debug_                        0

# same as Module/MPhy/UMTS alpha_ variable
MInterference/MIV/UMTS set alpha_                  0.2177777777

# pcStep_ (power control step): 1 dB = 1.258925412
Module/MPhy/UMTS set pcStep_                           1.258925412

#ME maxTxPower_: -16 dB = 0.025119
Module/MPhy/UMTS/ME set maxTxPower_                       0.025119
#ME minTxPower_: -96 dB = 2.5e-10
Module/MPhy/UMTS/ME set minTxPower_                       2.5e-10
Module/MPhy/UMTS/ME set TxPower_                          0.025119

#BS maxTxPower_: -5 dB = 0.316227
Module/MPhy/UMTS/BS set maxTxPower_                       0.316227
#BS minTxPower_: -25 dB = 0.00316227
Module/MPhy/UMTS/BS set minTxPower_                       0.00316227
Module/MPhy/UMTS/BS set TxPower_                          0.316227


#Module/MPhy/UMTS/BS set iuccorr_                      0.18604
#Module/MPhy/UMTS/BS set iuccorr_                      0.0078279
Module/MPhy/UMTS/BS set iuccorr_                      0.2
Module/MPhy/UMTS/BS set bits_per_symbol_              2
Module/MPhy/UMTS/BS set spreading_factor_             8

Module/MPhy/UMTS/ME set bits_per_symbol_              1
Module/MPhy/UMTS/ME set spreading_factor_             4


Module/UMTS/RLC set ipAddr_									-1
Module/UMTS/RLC set destIpAddr_								-1
Module/UMTS/RLC/AM set debug_                         0

Module/UMTS/RLC/UM set debug_                         0

# Most values are set by finalizeUmtsStack and Module/UMTS/RLC::connect
Module/UMTS/MAC set debug_                            0
Module/UMTS/MAC set src_rlc_id_                       0
Module/UMTS/MAC set dst_rlc_id_                       0
Module/UMTS/MAC set me_code_id_                       0
Module/UMTS/MAC set slot_duration_                    0
Module/UMTS/MAC set TTI_PDUs_                         0
Module/UMTS/MAC set TTI_                              0
Module/UMTS/MAC set interleaving_                     0.040
Module/UMTS/MAC set initslotoffset_                   -1

#Module/IP/Interface/UMTS set debug_                   0

Correlation/UMTS set dl_orthogonality_                0.4
Correlation/UMTS set code_id_                         0
Correlation/UMTS set debug_                           0


# Mac/Umts set delay_                                   10us
# Mac/Umts set TTI_                                     10ms
# Mac/Umts set shared_delay_                            3ms
# Mac/Umts set debug_                                   false


