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
 * $Id: initlib.cc 92 2008-03-25 19:35:54Z emil $
 */

#include <tclcl.h>
#include <clmessage.h>
#include <clsap.h>

ClMessage_t WIMAX_CLMSG_SET_PHYMODE = 0;
ClMessage_t WIMAX_CLMSG_SET_MODULATION = 0;
ClMessage_t WIMAX_CLMSG_SET_FREQ = 0;
ClMessage_t WIMAX_CLMSG_GET_SYMBOLTIME = 0;
ClMessage_t WIMAX_CLMSG_GET_PS = 0;
ClMessage_t WIMAX_CLMSG_GET_TRXTIME = 0;
ClMessage_t WIMAX_CLMSG_GET_TRXSYMBOLTIME = 0;
ClMessage_t WIMAX_CLMSG_SET_NODE_ON = 0;
ClMessage_t WIMAX_CLMSG_SET_NODE_OFF = 0;
ClMessage_t WIMAX_CLMSG_GET_SYMBOLPS = 0;
ClMessage_t WIMAX_CLMSG_GET_FREQ = 0;
ClMessage_t WIMAX_CLMSG_GET_MAX_PKT_SIZE = 0;

/** WIMAX - used by hdr_cmn class for tracing purpose */
packet_t 	PT_WIMAXBS;
packet_t 	PT_802_16 = 0;
packet_t 	PT_WIMAXOFDMPHY = 0;


extern EmbeddedTcl InitTclCode;

extern "C" int Wimax_Init()
{
	/*
	 * Put here all the commands which must be execute when the library
	 * is loaded (i.e. TCL script execution)
	 * Remember to return 0 if all is OK, otherwise return 1
	 */

	WIMAX_CLMSG_SET_PHYMODE  = ClMessage::addClMessage();
	WIMAX_CLMSG_SET_MODULATION = ClMessage::addClMessage();
	WIMAX_CLMSG_SET_FREQ = ClMessage::addClMessage();
	WIMAX_CLMSG_GET_SYMBOLTIME = ClMessage::addClMessage();
	WIMAX_CLMSG_GET_PS = ClMessage::addClMessage();
	WIMAX_CLMSG_GET_TRXTIME = ClMessage::addClMessage();
	WIMAX_CLMSG_GET_TRXSYMBOLTIME = ClMessage::addClMessage();
	WIMAX_CLMSG_SET_NODE_ON = ClMessage::addClMessage();
	WIMAX_CLMSG_SET_NODE_OFF = ClMessage::addClMessage();
	WIMAX_CLMSG_GET_SYMBOLPS = ClMessage::addClMessage();
	WIMAX_CLMSG_GET_FREQ = ClMessage::addClMessage();
	WIMAX_CLMSG_GET_MAX_PKT_SIZE = ClMessage::addClMessage();

	PT_WIMAXBS = p_info::addPacket("WimaxBS");
	PT_802_16 = p_info::addPacket("802_16");
	PT_WIMAXOFDMPHY = p_info::addPacket("OFDMPHY");
	
	InitTclCode.load();

	return 0;
}
