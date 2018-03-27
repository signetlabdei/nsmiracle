Module/802_16/MAC set debug_                0

Module/802_16/MAC set queue_length_         50 ;#maximum number of packets

Module/802_16/MAC set frame_duration_       0.004
Module/802_16/MAC set channel_              0
Module/802_16/MAC set fbandwidth_           5e+6
Module/802_16/MAC set rtg_                  10
Module/802_16/MAC set ttg_                  10
Module/802_16/MAC set dcd_interval_         5 ;#max 10s
Module/802_16/MAC set ucd_interval_         5 ;#max 10s
Module/802_16/MAC set init_rng_interval_    1 ;#max 2s
Module/802_16/MAC set lost_dlmap_interval_  0.6
Module/802_16/MAC set lost_ulmap_interval_  0.6
Module/802_16/MAC set t1_timeout_           [expr 5* [Module/802_16/MAC set dcd_interval_]]
Module/802_16/MAC set t2_timeout_           [expr 5* [Module/802_16/MAC set init_rng_interval_]]
Module/802_16/MAC set t3_timeout_           0.2
Module/802_16/MAC set t6_timeout_           3
Module/802_16/MAC set t12_timeout_          [expr 5* [Module/802_16/MAC set ucd_interval_]]
Module/802_16/MAC set t16_timeout_          0.1 ;#qos dependant
Module/802_16/MAC set t17_timeout_          5
Module/802_16/MAC set t21_timeout_          0.02 ;#max 10s. Use 20ms to replace preamble scanning
Module/802_16/MAC set contention_rng_retry_ 16
Module/802_16/MAC set invited_rng_retry_    16 ;#16
Module/802_16/MAC set request_retry_        2 ;#16
Module/802_16/MAC set reg_req_retry_        3
Module/802_16/MAC set tproc_                0.001
Module/802_16/MAC set dsx_req_retry_        3
Module/802_16/MAC set dsx_rsp_retry_        3

Module/802_16/MAC set rng_backoff_start_    2
Module/802_16/MAC set rng_backoff_stop_     6
Module/802_16/MAC set bw_backoff_start_     2
Module/802_16/MAC set bw_backoff_stop_      6

Module/802_16/MAC set scan_duration_        50
Module/802_16/MAC set interleaving_interval_ 50
Module/802_16/MAC set scan_iteration_       2
Module/802_16/MAC set t44_timeout_          0.1
Module/802_16/MAC set max_dir_scan_time_    0.2 ;#max scan for each neighbor BSs
Module/802_16/MAC set client_timeout_       0.5
Module/802_16/MAC set nbr_adv_interval_     0.5
Module/802_16/MAC set scan_req_retry_       3

Module/802_16/MAC set lgd_factor_           1
Module/802_16/MAC set rxp_avg_alpha_        1
Module/802_16/MAC set delay_avg_alpha_      1
Module/802_16/MAC set jitter_avg_alpha_     1
Module/802_16/MAC set loss_avg_alpha_       1
Module/802_16/MAC set throughput_avg_alpha_ 1
Module/802_16/MAC set throughput_delay_     0.02
Module/802_16/MAC set print_stats_          0

Module/WimaxCtrl set debug_ 0
Module/WimaxCtrl set adv_interval_ 1.0
Module/WimaxCtrl set default_association_level_ 0
Module/WimaxCtrl set synch_frame_delay_ 50

WimaxScheduler/BS set dlratio_ 0.3
