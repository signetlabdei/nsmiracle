


MPropagation/Underwater set shipping_            0.5
MPropagation/Underwater set windspeed_           0.0
MPropagation/Underwater set practicalSpreading_  1.75
MPropagation/Underwater set debug_               0

Module/UnderwaterChannel set debug_  0

# 97 dB re uPa => 5e9 uPa
Module/MPhy/BPSK/Underwater set TxPower_                5e9
# 190 dB re uPa => 1e19 uPa
Module/MPhy/BPSK/Underwater set MaxTxSPL_dB_            190
Module/MPhy/BPSK/Underwater set MinTxSPL_dB_             10
Module/MPhy/BPSK/Underwater set MaxTxRange_           10000
Module/MPhy/BPSK/Underwater set PER_target_            0.01
Module/MPhy/BPSK/Underwater set RxSnrPenalty_dB_        -10
Module/MPhy/BPSK/Underwater set TxSPLMargin_dB_          10
Module/MPhy/BPSK/Underwater set ConsumedEnergy_           0
Module/MPhy/BPSK/Underwater set SPLOptimization_          0
Module/MPhy/BPSK/Underwater set CentralFreqOptimization_  0
Module/MPhy/BPSK/Underwater set BandwidthOptimization_    0
Module/MPhy/BPSK/Underwater set debug_                    0


Module/MPhy/UWShannon set TxPower_ 5e9
Module/MPhy/UWShannon set debug_     0


