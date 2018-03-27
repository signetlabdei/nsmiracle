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
 * $Id: mac802_16pkt.cc 92 2008-03-25 19:35:54Z emil $
 */

#include "mac802_16pkt.h"

/**
 * Return the size of the MOB_NBR-ADV frame
 * @param frame The frame 
 */
int Mac802_16pkt::getMOB_NBR_ADV_size(mac802_16_mob_nbr_adv_frame *frame)
{
  int size = 4; //min size
  if ((frame->skip_opt_field & 0x1) == 0)
    size +=3; //add operator id
  
  for (int i = 0 ; i < frame->n_neighbors ; i++) {
    size += 4; //min size for neighbor info
    if (frame->nbr_info[i].phy_profile_id.FAindex & 0x1)
      size++;
    if (frame->nbr_info[i].phy_profile_id.bs_eirp & 0x1)
      size++;
    if ((frame->skip_opt_field & 0x2) == 0)
      size +=3; //contain neighbor bs id
    if ((frame->skip_opt_field & 0x4) == 0)
      size ++; //contain HO process optimization
    if ((frame->skip_opt_field & 0x8) == 0)
      size ++; //contain neighbor bs id
    if (frame->nbr_info[i].dcd_included)
      size += 2+GET_DCD_SIZE (frame->nbr_info[i].dcd_settings.nb_prof); 
    if (frame->nbr_info[i].ucd_included)
      size += 2+GET_UCD_SIZE (frame->nbr_info[i].ucd_settings.nb_prof); 
    if (frame->nbr_info[i].phy_included)
      size += 2;
  }

  return size;
}

/**
 * Return the size of the MOB_SCN-REQ 
 * @param frame The frame 
 */
int Mac802_16pkt::getMOB_SCN_REQ_size(mac802_16_mob_scn_req_frame *frame)
{
  int size=6;

  if (frame->n_recommended_bs_index != 0)
    size ++;
  int tmp = 11*(frame->n_recommended_bs_index+frame->n_recommended_bs_full);
  size += tmp/8;
  if ((tmp%8)!=0)
    size ++;

  return size;
}

/**
 * Return the size of the MOB_SCN-RSP 
 * @param frame The frame 
 */
int Mac802_16pkt::getMOB_SCN_RSP_size(mac802_16_mob_scn_rsp_frame *frame)
{
  int size=6;

  if (frame->scan_duration!=0) {
    size += 3;
    if (frame->n_recommended_bs_index!=0)
      size ++;
    int tmp = 0;
    for (int i = 0 ; i < frame->n_recommended_bs_index ; i++) {
      tmp+=11;
      if (frame->rec_bs_index[i].scanning_type==2 ||
	  frame->rec_bs_index[i].scanning_type==3)
	tmp+=24;
    }
    for (int i = 0 ; i < frame->n_recommended_bs_index ; i++) {
      tmp+=51;
      if (frame->rec_bs_index[i].scanning_type==2 ||
	  frame->rec_bs_index[i].scanning_type==3)
	tmp+=24;
    }
    size += tmp/8;
    if ((tmp%8)!=0)
      size ++;  
  }

  return size;
}

/**
 * Return the size of the MOB_MSHO-REQ 
 * @param frame The frame 
 */
int Mac802_16pkt::getMOB_MSHO_REQ_size(mac802_16_mob_msho_req_frame *frame)
{
  int size=4;
  int tmp, tmpB;

  tmp = 0;
  tmpB = 0;

  if (frame->n_new_bs_index !=0) 
    size++;

  if (frame->report_metric & 0x1) tmp++;
  if (frame->report_metric & 0x2) tmp++;
  if (frame->report_metric & 0x4) tmp++;

  for (int i = 0 ; i < frame->n_new_bs_index ; i++) {
    tmpB += 20 + 8*tmp;    
    if (frame->bs_index[i].arrival_time_diff_ind & 0x1)
      tmpB += 4;
  }
  //n_new_bs_full
  for (int i = 0 ; i < frame->n_new_bs_full ; i++) {
    tmpB += 20 + 8*tmp;
    if (frame->bs_full[i].arrival_time_diff_ind & 0x1)
      tmpB += 4;
  }
  tmpB += 4;
  //N_current
  if (frame->report_metric & 0x8) tmp++;
  for (int i = 0 ; i < frame->n_current_bs ; i++) {
    tmpB += 4 + 8*tmp;
  }
  //increase size
  size += tmp/8;
  if ((tmp%8)!=0)
    size ++;  //includes padding
  
  return size;
}

/**
 * Return the size of the MOB_MSHO-REQ 
 * @param frame The frame 
 */
int Mac802_16pkt::getMOB_BSHO_RSP_size(mac802_16_mob_bsho_rsp_frame *frame)
{
  int size=4;

  return size;
}

/**
 * Return the size of the MOB_HO-IND 
 * @param frame The frame 
 */
int Mac802_16pkt::getMOB_HO_IND_size(mac802_16_mob_ho_ind_frame *frame)
{
  int size=4;

  return size;
}
