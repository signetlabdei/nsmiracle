/*
 * Copyright (c) 2008, Karlstad University
 * Erik Andersson, Emil Ljungdahl, Lars-Olof Moilanen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This software is based on the NIST NS2 WiMAX module, which can be found at
 * http://www.antd.nist.gov/seamlessandsecure/doc.html
 *
 * $Id: mac802_16pkt.h 92 2008-03-25 19:35:54Z emil $
 */

#ifndef MAC802_16PKT_H
#define MAC802_16PKT_H

#include "packet.h"

//#define HDR_MAC802_16(p)	((hdr_mac802_16 *)hdr_mac::access(p))
#define HDR_MAC802_16(p)	(hdr_mac802_16::access(p))
#define HDR_MAC802_16_SIZE 6
#define HDR_MAC802_16_FRAGSUB_SIZE 2

/** Define fragmentation encoding */
enum fragment_status {
  FRAG_NOFRAG = 0,
  FRAG_LAST,
  FRAG_FIRST,
  FRAG_CONT
};
  
/** Define Mac management type */
enum mngmt_type {
  MAC_UCD = 0,
  MAC_DCD,
  MAC_DL_MAP,
  MAC_UL_MAP,
  MAC_RNG_REQ,
  MAC_RNG_RSP,
  MAC_REG_REQ,
  MAC_REG_RSP,
  // 8: reserved
  // 9-10: Privacy key, not used 
  MAC_DSA_REQ=11,
  MAC_DSA_RSP,
  MAC_DSA_ACK,
  
  MAC_MOB_NBR_ADV=53,
  MAC_MOB_SCN_REQ,
  MAC_MOB_SCN_RSP,
  MAC_MOB_BSHO_REQ,
  MAC_MOB_MSHO_REQ,
  MAC_MOB_BSHO_RSP,
  MAC_MOB_HO_IND,
  MAC_MOB_SCN_REP,
  MAC_MOB_ASC_REP=66
};

/** Enumeration of scanning types */
enum wimax_scanning_type {
  SCAN_WITHOUT_ASSOC,
  SCAN_ASSOC_LVL0,
  SCAN_ASSOC_LVL1,
  SCAN_ASSOC_LVL2
};

/** Define generic MAC header */
struct gen_mac_header_t {
  u_char ht : 1;
  u_char ec : 1;

  u_char type_mesh       : 1;  // mesh subheader: 1=present, 0=absent
  u_char type_arqfb      : 1;  // ARQ Feedback payload: 1=present, 0=absent
  u_char type_ext        : 1;  // Extended type
  u_char type_frag       : 1;  // Fragmentation subheader: 1=present, 0=absent
  u_char type_pck        : 1;  // Packing subheader: 1=present, 0=absent
  u_char type_fbgm       : 1;  // DL: FAST-FEEDBACK allocation subheader, UL: Grant Management subheader

  u_char rsv1: 1;
  u_char ci: 1;
  u_char eks: 2;
  u_char rsv2: 1;
  u_int16_t len: 11;
  u_int16_t cid;
  u_char hcs;
};

/** Define bandwidth request header */
struct bw_req_header_t {
  u_char ht : 1;
  u_char ec : 1;
  u_char type : 3;  
  u_int32_t br : 19;
  u_int16_t cid;
  u_char hcs;  
};

/** Define the structures for subheaders **/

/** Fragmentation subheader **/
struct frag_subheader_s {
  char fc : 2;  //00:no fragmentation, 01:last, 10: first, 11: middle
  char fsn : 3; //fragmentation number
  
  u_int16_t sn:11; //If this is an ARQ-enabled connection, this is the BSN, otherwise it is the FSN
                    //The size of FSN is 3 bits if the type extended is not set
  u_char rsv:3;
};

/** Grant Management subheader**/

//stucture for UGS service type
struct grant_map_ugs_s {
  u_char si:1; //Slipt indicator
  u_char pm:1; //Poll me indication
  u_int16_t rsv:14;
};

struct grant_subheader_s { //Could be cast to grant_map_ugs_s for UGS service type
  u_int16_t piggyback_req:16; 
};

/** Packing subheader **/
struct pack_subheader_s {
  u_char fc:2;
  u_int16_t sn:11; //If this is an ARQ-enabled connection, this is the BSN, otherwise it is the FSN
                    //The size of FSN is 3 bits if the type extended is not set
  u_int16_t length:11;
};

/** Fast Feedback **/
struct ffb_subheader_s {
  u_char alloc_offset:6;
  u_char type:2;
};

/** ARQ Feedback **/
#define MAX_ARQ_FB_IE 10
//ARQ feedback IE struct
struct arq_fb_ie {
  u_int16_t   cid;                    //ID of the connection being referenced
  u_char      last:1;                   // LAST ARQ IE, false = 0, true = 1
  u_char      ack_type:2;     // Type of ACK - selective =0 , cumulative = 1, cumulative with selective ACK = 2, reserved = 3
  u_int32_t   bsn             :11;    // sequence number
  //Should we add ACK MAP?
};


/** Define structure for packet information */
struct hdr_mac802_16
{
  //generic mac header or bw request header
  gen_mac_header_t header;

  //Possible subheader (type in header indicates if subheader is present or not)
  struct frag_subheader_s frag_subheader;
  struct pack_subheader_s pack_subheader;
  struct grant_subheader_s grant_subheader; //present only on uplink
  struct ffb_subheader_s ffb_subheader; //present only on downlink
  struct arq_fb_ie arq_fb_ie[MAX_ARQ_FB_IE]; //if the type_arqfb is true, we have at least 1 element
                                                       //we use the last bit to check if it is the last element

  //for management frames, we put 
  //payload in the packet payload
  //because the size changes

  //Packet header access functions
  static int offset_;
  inline static int& offset() {return offset_;}
  inline static hdr_mac802_16* access(const Packet* p)
  {
    return (hdr_mac802_16*) p->access(offset_);
  }
};

/**** Defines some constant for the maximum size of messages ****/
/* When sending a packet, the message is copied for each 
 * destination. In that case, we need to store all the information
 * in the packet itself and not use pointer. We then use arrays.
 */
#define MAX_MAP_IE  10
#define MAX_PROFILE 10
#define MAX_NBR     10

/**** Defines IEs ****/
/** Defines DL_MAP IE (see p462.) */
struct mac802_16_dlmap_ie {
  u_int16_t cid;
  u_char diuc : 4;            //p463
  u_char preamble : 1;
  u_int16_t start_time : 11;
  //may contain extended info
};
#define DL_MAP_IE_SIZE 4

/** fast Ranging IE (802.16e) */
struct mac802_16_fast_ranging_ie {
  int mac_addr;  //48 bits
  u_char uiuc : 4; 
  u_int16_t duration : 12; //in OFDM symbols
};
#define FAST_RANGING_IE_SIZE 9

/** Defines UL_MAP IE (see p464.) */
struct mac802_16_ulmap_ie {
  u_int16_t cid;
  u_int16_t start_time : 11;
  u_char sub_channel_index : 5;
  u_char uiuc : 4;
  u_int16_t duration : 11;
  u_char midamble_rep : 2;
  //may contain additional info depending on uiuc
  u_char extended_uiuc : 4;
  u_char length : 4;
  mac802_16_fast_ranging_ie fast_ranging;
};
#define UL_MAP_IE_SIZE 6

/**** Defines burst profiles ****/

/* Burst profiles are TLV encoded...we just pick the info we 
 * are interested in.
 */
/** Defines DCD profile */
struct mac802_16_dcd_profile {
  u_char diuc : 4;
  u_int32_t frequency;
  u_char fec;
  //may have more info
};
#define DCD_PROFILE_SIZE 12

/** Defines UCD profile */
struct mac802_16_ucd_profile {
  u_char uiuc : 4;
  u_char fec;
  //may have more info
};
#define UCD_PROFILE_SIZE 6

/**** Defines frames ****/
/** DCD frame */
struct mac802_16_dcd_frame {
  u_char type;
  u_char dcid;
  u_char config_change_count;
  //info for all channels: TLV encoded
  u_char frame_duration_code;
  u_int32_t frame_number : 24;
  u_char ttg;
  u_char rtg;
  u_int32_t frequency;
  //downlink burst profiles
  u_int32_t nb_prof;
  mac802_16_dcd_profile profiles[MAX_PROFILE];
};

//+3 for the end of map profile
#define GET_DCD_SIZE(X) 22+X*DCD_PROFILE_SIZE+3

/** DL_MAP frame */
struct mac802_16_dl_map_frame {
  u_char type;
  u_char dcd_count;
  int bsid; //normaly 48 bits
  //DL_MAP IEs
  u_int32_t nb_ies;
  mac802_16_dlmap_ie ies[MAX_MAP_IE]; 
};

//there is X IEs 
#define GET_DL_MAP_SIZE(X) 8+X*DL_MAP_IE_SIZE

/** UCD frame */
struct mac802_16_ucd_frame {
  u_char type;
  u_char config_change_count;
  u_char rng_backoff_start;
  u_char rng_backoff_end;
  u_char req_backoff_start;
  u_char req_backoff_end;
  //info for overall channel
  /*
  u_int16_t rsv_timeout;
  */
  u_int16_t bw_req_size;
  u_int16_t rng_req_size;
  //uplink burst profile
  u_int32_t nb_prof;
  mac802_16_ucd_profile profiles[MAX_PROFILE];
};

//+3 for the end of map profile
#define GET_UCD_SIZE(X) 14+X*UCD_PROFILE_SIZE+3

/** UL_MAP frame */
struct mac802_16_ul_map_frame {
  u_char type;
  u_char ucid;
  u_char ucd_count;
  u_int32_t allocation_start;
  //UL_MAP IEs
  u_int32_t nb_ies;
  mac802_16_ulmap_ie ies[MAX_MAP_IE];
};

#define GET_UL_MAP_SIZE(X) 7+X*UL_MAP_IE_SIZE

/**** Defines ranging messages ****/
/** Ranging request frame */
struct mac802_16_rng_req_frame {
  u_char type;
  u_char dc_id;
  /*TLV values*/
  /* Requested Downlink Burst Profile
   * bits 0-3: DIUC of the downlink burst profile
   * bits 4-7: LSB of Configuration Change Count value
   * of DCD defining the burst profile assocciated with DIUC
   */
  u_char req_dl_burst_profile;
  int ss_mac_address; //should be 6 bytes
  //u_char aas_bc_cap; //broadcast capability. optional
};
#define RNG_REQ_SIZE 13 //max value

/** Ranging status */
enum ranging_status {
  RNG_CONTINUE = 1,
  RNG_ABORT,
  RNG_SUCCESS,
  RNG_RERANGE
};

/** Ranging response frame */
struct mac802_16_rng_rsp_frame {
  u_char type;
  u_char uc_id;
  /*TLV values*/
  u_char pw_adjust;
  u_int32_t freq_adjust;
  u_char rng_status;
  /* byte 0: the least robust DIUC that may be used by BS for 
   * transmissions to the SS
   * byte 1: Configuration Change Count of DCD defining the burst profile
   * associated with DIUC 
   */
  u_int16_t dl_op_burst_profile;
  int ss_mac_address; //6 bytes
  u_int16_t basic_cid;
  u_int16_t primary_cid;
  u_char aas_bc_perm; 
};
#define RNG_RSP_SIZE 28

/**** Defines registration messages ****/
/** Registration request frame */
struct mac802_16_reg_req_frame {
  u_char type;
  /*TLV values*/
  u_char ss_mngmt_support;
  u_char ip_mngmt_support;
  u_int16_t uplink_cid_support;
  
};
#define REG_REQ_SIZE 12

/** Registration response frame */
struct mac802_16_reg_rsp_frame {
  u_char type;
  u_char response;
  /*TLV values*/
  u_char ss_mngmt_support;
  u_int16_t sec_mngmt_cid;
};
#define REG_RSP_SIZE 12

/**** Defines Dynamic Service Addition messages ****/
/** DSA request frame */
struct mac802_16_dsa_req_frame {
  u_char type; //11
  u_int16_t transaction_id;
  /*TLV values*/
  bool uplink; //direction of the flow, normaly coded in TLV with
               //type 145 or 146 (see p647).
  u_int16_t cid;
};
//parameter X indicates if cid is present (i.e request from BS)
#define GET_DSA_REQ_SIZE(X) 6+4*X

/** DSA response frame */
struct mac802_16_dsa_rsp_frame {
  u_char type; //12
  u_int16_t transaction_id;
  u_char confirmation_code;
  /*TLV values*/
  bool uplink; //direction of the flow, normaly coded in TLV with
               //type 145 or 146 (see p647).
  u_int16_t cid;
};
//parameter X indicates if cid is present (i.e response from BS)
#define GET_DSA_RSP_SIZE(X) 6+4*X

/** DSA Acknowledgement frame */
struct mac802_16_dsa_ack_frame {
  u_char type; //13
  u_int16_t transaction_id;
  u_char confirmation_code;
  /*TLV values*/
  bool uplink; //direction of the flow, normaly coded in TLV with
               //type 145 or 146 (see p647).
};
#define DSA_ACK_SIZE 6


/**** Defines Mobility messages (802.16e) ****/
/** Structure of physical profile ID */
struct mac802_16_phy_profile_id {
  u_char colocatedFA: 1;
  u_char FAconfig: 1;
  u_char timefreq_synch: 2;
  u_char bs_eirp: 1;
  u_char dcducd_ref: 1;
  u_char FAindex: 1;
  u_char trigger_ref: 1;
};

/** Structure of physical mode ID */
struct mac802_16_phy_mode_id {
  u_char bandwidth: 7;
  u_char fttsize: 3;
  u_char cp: 2;
  u_char duration_code: 4;
};

/** Information about a neighbor BS */
struct mac802_16_nbr_adv_info {
  u_char length;
  mac802_16_phy_profile_id phy_profile_id;
  u_char fa_index; //if FA index indicator=1 in phy_profile_id
  u_char bs_eirp;
  int nbr_bsid;    
  u_char preamble_index; //in OFDM, 5lsb=
  u_char ho_process_opt;
  u_char sched_srv_supported;
  u_char dcd_ccc: 4;
  u_char ucd_ccc: 4;
  /* other TLV information */
  bool dcd_included; //tell if the dcd is included
  mac802_16_dcd_frame dcd_settings;
  bool ucd_included; //tell if the ucd is included
  mac802_16_ucd_frame ucd_settings;
  bool phy_included; //tell if the phy mode ID is included
  mac802_16_phy_mode_id phy_mode_id;
};

/** Neighbor advertisment frame */
struct mac802_16_mob_nbr_adv_frame {
  u_char type; //53
  u_char skip_opt_field;
  u_int32_t operatorID: 24;
  u_char ccc;
  u_char frag_index: 4;
  u_char total_frag: 4;
  u_char n_neighbors; //number of neighbors
  mac802_16_nbr_adv_info nbr_info[MAX_NBR];
};

/** Code BS using index in scan request */
struct mac802_16_mob_scn_req_bs_index {
  u_char neighbor_bs_index;
  u_char scanning_type: 3; //0: scanning without association
                           //1: scanning+assoc level 0
                           //2: scanning+assoc level 1
                           //3: scanning+assoc level 2
                           //4-7: reserved
};
 
/** Code BS using full address in scan request */
struct mac802_16_mob_scn_req_bs_full {
  int recommended_bs_id;   /* 6 bytes */
  u_char scanning_type: 3; //0: scanning without association
                           //1: scanning+assoc level 0
                           //2: scanning+assoc level 1
                           //3: scanning+assoc level 2
                           //4-7: reserved
};

/** Scan request frame */
struct mac802_16_mob_scn_req_frame {
  u_char type; //54
  u_char scan_duration; //units of frames
  u_char interleaving_interval; //units of frames
  u_char scan_iteration; //in frame
  u_char n_recommended_bs_index; //number of BS recommended
  u_char ccc; //present if n_recommended_bs_index!= 0
  mac802_16_mob_scn_req_bs_index rec_bs_index[MAX_NBR];
  u_char n_recommended_bs_full;
  mac802_16_mob_scn_req_bs_full rec_bs_full[MAX_NBR];
  /* TLV info*/
};

/** Code BS using index in scan response */
struct mac802_16_mob_scn_rsp_bs_index {
  u_char neighbor_bs_index;
  u_char scanning_type: 3; //0: scanning without association
                           //1: scanning+assoc level 0
                           //2: scanning+assoc level 1
                           //3: scanning+assoc level 2
                           //4-7: reserved
  /* next present if scanning is 2 or 3*/
  u_char rdv_time;
  u_char cdma_code;
  u_char transmission_opp_offset;
};

/** Code BS using full address in scan response */
struct mac802_16_mob_scn_rsp_bs_full {
  int recommended_bs_id;   /* 6 bytes */
  u_char scanning_type: 3; //0: scanning without association
                           //1: scanning+assoc level 0
                           //2: scanning+assoc level 1
                           //3: scanning+assoc level 2
                           //4-7: reserved
  /* next present if scanning is 2 or 3*/
  u_char rdv_time;
  u_char cdma_code;
  u_char transmission_opp_offset;
};

/** Scan response frame */
struct mac802_16_mob_scn_rsp_frame {
  u_char type; //55
  u_char scan_duration; //units of frames
  u_char report_mode: 2;
  u_char report_period;
  u_char report_metric;
  /*next information present only if scan duration !=0*/
  u_char start_frame: 4;
  u_char interleaving_interval;
  u_char scan_iteration;
  u_char n_recommended_bs_index;
  //next if n_recommended_bs_index !=0
  u_char ccc_mob_nbr_adv;
  mac802_16_mob_scn_rsp_bs_index rec_bs_index[MAX_NBR];
  u_char n_recommended_bs_full;
  mac802_16_mob_scn_rsp_bs_full rec_bs_full[MAX_NBR];
  /* end if scan duration !=0 */
  /* TLV information */
};

/** Measurements about current BS */
struct mac802_16_mob_scn_rep_current_bs {
  u_char temp_bsid: 4;
  u_char bs_cinr_mean; //if report_metric[0]==1
  u_char bs_rssi_mean; //if report_metric[1]==1
  u_char relative_delay; //if report_metric[2]==1
  u_char bs_rtd; //if report_metric[3]==1
};

/** Measurements about neighbor BS using index */
struct mac802_16_mob_scn_rep_bs_index {
  u_char neighbor_bs_index;
  u_char bs_cinr_mean; //if report_metric[0]==1
  u_char bs_rssi_mean; //if report_metric[1]==1
  u_char relative_delay; //if report_metric[2]==1
};

/** Measurements about neighbor BS using full address */
struct mac802_16_mob_scn_rep_bs_full {
  int neighbor_bs_id; /* 6 bytes */
  u_char bs_cinr_mean; //if report_metric[0]==1
  u_char bs_rssi_mean; //if report_metric[1]==1
  u_char relative_delay; //if report_metric[2]==1
};

/** Scan report frame */
struct mac802_16_mob_scn_rep_frame {
  u_char type; //60
  u_char report_mode: 1;
  u_char comp_nbr_bsid_ind: 1;
  u_char n_current_bs: 3;
  u_char report_metric: 8;
  mac802_16_mob_scn_rep_current_bs current_bs[MAX_NBR];
  u_char n_neighbor_bs_index;
  //next if n_recommended_bs_index !=0
  u_char ccc_mob_nbr_adv;
  mac802_16_mob_scn_rep_bs_index nbr_bs_index[MAX_NBR];
  u_char n_recommended_bs_full;
  mac802_16_mob_scn_rep_bs_full nbr_bs_full[MAX_NBR];
  /* other TLV information */
};

/** Code BS using index in association result report */
struct mac802_16_mob_asc_rep_bs_index {
  u_char neighbor_bs_index;
  uint32_t timing_adjust;
  u_char power_level_adjust;
  uint32_t offset_freq_adjust;
  u_char rng_status;
  u_char service_level_prediction;
};

/** Code BS using address in association result report */
struct mac802_16_mob_asc_rep_bs_full {
  int neighbor_bs_id; /* 6 bytes */
  uint32_t timing_adjust;
  u_char power_level_adjust;
  uint32_t offset_freq_adjust;
  u_char rng_status;
  u_char service_level_prediction;
};

/** Association result report frame */
struct mac802_16_mob_asc_rep_frame {
  u_char type; //66
  u_char n_recommended_bs_index;
  //next if n_recommended_bs_index !=0
  u_char ccc_mob_nbr_adv;
  mac802_16_mob_asc_rep_bs_index rec_bs_index[MAX_NBR];
  u_char n_recommended_bs_full;
  mac802_16_mob_asc_rep_bs_full rec_bs_full[MAX_NBR];
}; 

/** Code request in mode 000 (HO request) */
struct mac802_16_mob_bsho_req_mode_000 {
  int neighbor_bsid; /* 6 bytes */
  u_char service_level_prediction;
  u_char preamble_index;
  u_char ho_process_optimization;
  u_char net_assisted_ho_supported: 1;
  u_char ho_id_included_indicator: 1;
  u_char ho_autho_policy_indicator: 1;
  //if ho_id_included_indicator==1
  u_char ho_id;
  //if ho_autho_policy_indicator==1
  u_char ho_autho_policy_support;
};

/** Structure for BSHO request */
struct mac802_16_mob_bsho_req_mode_new_bs {
  int neighbor_bsid; /* 6 bytes */
  u_char temp_bsid: 3;
 
};

/** Structure for BSHO request */
struct mac802_16_mob_bsho_req_mode_new_bs2 {
  int neighbor_bsid; /* 6 bytes */
  u_char temp_bsid: 3;
  uint16_t new_cid[MAX_NBR];
  uint16_t new_said[MAX_NBR];
 
};

/** Structure for BSHO request */
struct mac802_16_mob_bsho_req_mode_new_bs3 {
  int neighbor_bsid; /* 6 bytes */
  u_char temp_bsid: 3;
  uint16_t new_cid[MAX_NBR];
  uint16_t new_said[MAX_NBR];
  int cqich_id; //variable 
  u_char feedback_channel_off: 6;
  u_char period: 2;
  u_char frame_offset: 3;
  u_char duration: 3;
  u_char mimo_permutation_feedback: 2; 
};

/** Structure for BSHO request */
struct mac802_16_mob_bsho_req_mode_current_bs {
  u_char temp_bsid: 3;
};

/** BSHO request frame */
struct mac802_16_mob_bsho_req_frame {
  u_char type; //56
  u_char net_assisted_ho_supported: 1;
  u_char mode: 3;
  //if mode == 0b000
  u_char ho_op_mode: 1;
  u_char n_recommended;
  u_char resource_retain_flag: 1;
  mac802_16_mob_bsho_req_mode_000 n_rec[];
  //if mode == 0b001
  u_char tmp_bsid: 3;
  u_char ak_change_indicator: 1;
  u_char n_cids;
  uint16_t cids[MAX_NBR];
  u_char n_saids;
  uint16_t saids[MAX_NBR];
  
  //if mode == 0b010: attribute defined in mode 0b001
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;
  
  //if mode == 0b011
  u_char n_new_bs: 3;
  mac802_16_mob_bsho_req_mode_new_bs new_bs[MAX_NBR];
  u_char n_current_bs: 3;
  mac802_16_mob_bsho_req_mode_current_bs current_bs[MAX_NBR];
  //also include the following elements defined in mode 0b001
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;
  //u_char n_cids;
  //uint16_t cids[MAX_NBR];
  //u_char n_saids;
  //uint16_t saids[MAX_NBR];
  
  //if mode == 0b100, include the following attributes (already defined)
  //u_char n_new_bs: 3;
  //mac802_16_mob_bsho_req_mode_new_bs new_bs[MAX_NBR];
  //u_char n_current_bs: 3;
  //mac802_16_mob_bsho_req_mode_current_bs current_bs[MAX_NBR];
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;
  
  //if mode == 0b101
  //u_char n_new_bs: 3;
  //u_char n_cids;
  //u_char n_saids;
  mac802_16_mob_bsho_req_mode_new_bs2 new_bs2[MAX_NBR];
  //u_char n_current_bs: 3;
  //mac802_16_mob_bsho_req_mode_current_bs current_bs[];
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;
  
  //if mode == 0b110
  //u_char n_new_bs: 3;
  //u_char n_cids;
  //u_char n_saids;
  mac802_16_mob_bsho_req_mode_new_bs3 new_bs3[MAX_NBR];
  //u_char n_current_bs: 3;
  //mac802_16_mob_bsho_req_mode_current_bs current_bs[];
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;

  u_char action_time: 7;
  /* TLV information */
};

/** Structure for MSHO request */
struct mac802_16_mob_msho_req_bs_index {
  u_char neighbor_bs_index;
  u_char preamble_index;
  u_char bs_cinr_mean; //if report_metric[0]==1
  u_char bs_rssi_mean; //if report_metric[1]==1
  u_char relative_delay; //if report_metric[2]==1
  u_char service_level_prediction: 3;
  u_char arrival_time_diff_ind: 1;
  //next if arrival_time_diff_ind==1
  u_char arrival_time_diff: 4;
};

/** Structure for MSHO request */
struct mac802_16_mob_msho_req_current_bs {
  u_char temp_bsid: 4;
  u_char bs_cinr_mean; //if report_metric[0]==1
  u_char bs_rssi_mean; //if report_metric[1]==1
  u_char relative_delay; //if report_metric[2]==1
  u_char bs_rtd; //if report_metric[3]==1
};

/** MSHO request frame */
struct mac802_16_mob_msho_req_frame {
  u_char type; //57
  u_char report_metric;
  u_char n_new_bs_index;
  //next if n_recommended_bs_index !=0
  u_char ccc_mob_nbr_adv;
  mac802_16_mob_msho_req_bs_index bs_index[MAX_NBR];
  //end 
  u_char n_new_bs_full;
  mac802_16_mob_msho_req_bs_index bs_full[MAX_NBR];
  u_char n_current_bs;
  mac802_16_mob_msho_req_current_bs bs_current[MAX_NBR];
  /* other TLV information */
};

/** Structure for BSHO response */
struct mac802_16_mob_bsho_rsp_rec {
  int neighbor_bsid; /* 6 bytes */
  u_char preamble_index;
  u_char service_level_prediction;
  u_char ho_process_optimization;
  u_char net_assisted_ho_supported: 1;
  u_char ho_id_included_indicator: 1;
  //if ho_id_included_indicator==1
  u_char ho_id;
  //end if
  u_char ho_autho_policy_indicator: 1;
  //if ho_autho_policy_indicator==1
  u_char ho_autho_policy_support;
  //end if
  
};

/** Structure for BSHO response */
struct mac802_16_mob_bsho_rsp_mode_new_bs2 {
  int neighbor_bsid; /* 6 bytes */
  u_char temp_bsid: 3;
  uint16_t new_cid[];
};

/** BSHO response frame */
struct mac802_16_mob_bsho_rsp_frame {
  u_char type; //58
  u_char mode: 3;
  
  //if mode == 0b000
  u_char ho_operation_mode: 1;
  u_char n_recommended;
  u_char resource_retain_flag: 1;
  mac802_16_mob_bsho_rsp_rec n_rec[MAX_NBR];

  //if mode == 0b001
  u_char tmp_bsid: 3;
  u_char ak_change_indicator: 1;
  u_char n_cids;
  uint16_t cids[MAX_NBR];
  u_char n_saids;
  uint16_t saids[MAX_NBR];

  //if mode == 0b010
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;

  //if mode == 0b011
  u_char n_new_bs: 3;
  mac802_16_mob_bsho_req_mode_new_bs new_bs[MAX_NBR];
  u_char n_current_bs: 3;
  mac802_16_mob_bsho_req_mode_current_bs current_bs[MAX_NBR];
  //also include the following elements defined in mode 0b001
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;
  //u_char n_cids;
  //uint16_t cids[MAX_NBR];
  //u_char n_saids;
  //uint16_t saids[MAX_NBR];

  //if mode == 0b100
  //u_char n_new_bs: 3;
  //mac802_16_mob_bsho_req_mode_new_bs new_bs[MAX_NBR]; //same struct as req
  //u_char n_current_bs: 3;
  //mac802_16_mob_bsho_req_mode_current_bs current_bs[MAX_NBR]; //same as req
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;

  //if mode == 0b101
  //u_char n_new_bs: 3;
  //u_char n_cids;
  mac802_16_mob_bsho_rsp_mode_new_bs2 new_bs2[MAX_NBR];
  //u_char n_current_bs: 3;
  //mac802_16_mob_bsho_req_mode_current_bs current_bs[MAX_NBR];
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;
  
  //if mode == 0b110
  //u_char n_new_bs: 3;
  //u_char n_cids;
  //u_char n_saids;
  mac802_16_mob_bsho_req_mode_new_bs3 new_bs3[MAX_NBR]; //same as request
  //u_char n_current_bs: 3;
  //mac802_16_mob_bsho_req_mode_current_bs current_bs[MAX_NBR];
  //u_char tmp_bsid: 3;
  //u_char ak_change_indicator: 1;

  u_char action_time: 7;
  /* TLV information */

};

/** Structure for Handover indication */
struct mac802_16_mob_ho_ind_bs {
  u_char temp_bsid: 3;
};

/** HO indication frame */
struct mac802_16_mob_ho_ind_frame {
  u_char type; //59
  u_char mode: 2;
  //if mode==0b00
  u_char ho_ind_type: 2;
  u_char rng_param_valid_ind: 2;
  //next valid if ho_ind_type==0b00
  int target_bsid; /* 6 bytes */
  
  //if mode==0b01
  u_char mdhofbss_ind_type: 2;
  //next valid if mdhofbss_ind_type==0b00
  u_char bsid: 3;
  u_char action_time;
  //end valid if mdhofbss_ind_type==0b00

  //if mode==0b10
  //u_char mdhofbss_ind_type: 2;
  //next valid if mdhofbss_ind_type==0b00
  u_char diversity_set_included;
  //next valid if diversity_set_included==1
  u_char anchor_bsid: 3;
  u_char n_bs;
  mac802_16_mob_ho_ind_bs bs[MAX_NBR];
  //end valid if diversity_set_included==1
  //u_char action_time;
  //end valid if mdhofbss_ind_type==0b00
  u_char preamble_index;
  
  /* TLV information */
};

/** This class contains helpers for manipulating 802.16 messages
 *  and getting the packet size 
 */
class Mac802_16pkt {
 public:
  /**
   * Return the size of the MOB_NBR-ADV frame
   * @param frame The frame 
   */
  static int getMOB_NBR_ADV_size(mac802_16_mob_nbr_adv_frame *frame);

  /**
   * Return the size of the MOB_SCN-REQ 
   * @param frame The frame 
   */
  static int getMOB_SCN_REQ_size(mac802_16_mob_scn_req_frame *frame);

  /**
   * Return the size of the MOB_SCN-RSP 
   * @param frame The frame 
   */
  static int getMOB_SCN_RSP_size(mac802_16_mob_scn_rsp_frame *frame);

  /**
   * Return the size of the MOB_MSHO-REQ 
   * @param frame The frame 
   */
  static int getMOB_MSHO_REQ_size(mac802_16_mob_msho_req_frame *frame);

  /**
   * Return the size of the MOB_BSHO-RSP 
   * @param frame The frame 
   */
  static int getMOB_BSHO_RSP_size(mac802_16_mob_bsho_rsp_frame *frame);

  /**
   * Return the size of the MOB_HO-IND 
   * @param frame The frame 
   */
  static int getMOB_HO_IND_size(mac802_16_mob_ho_ind_frame *frame);
};


#endif
