/*-
 * Copyright (c) 2011, 2016 Chelsio Communications, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 *
 */

/* This file is automatically generated --- changes will be lost */

#ifndef _T4_TCB_DEFS_H
#define _T4_TCB_DEFS_H

/* 3:0 */
#define W_TCB_ULP_TYPE    0
#define S_TCB_ULP_TYPE    0
#define M_TCB_ULP_TYPE    0xfULL
#define V_TCB_ULP_TYPE(x) ((x) << S_TCB_ULP_TYPE)

/* 11:4 */
#define W_TCB_ULP_RAW    0
#define S_TCB_ULP_RAW    4
#define M_TCB_ULP_RAW    0xffULL
#define V_TCB_ULP_RAW(x) ((x) << S_TCB_ULP_RAW)

/* 23:12 */
#define W_TCB_L2T_IX    0
#define S_TCB_L2T_IX    12
#define M_TCB_L2T_IX    0xfffULL
#define V_TCB_L2T_IX(x) ((x) << S_TCB_L2T_IX)

/* 31:24 */
#define W_TCB_SMAC_SEL    0
#define S_TCB_SMAC_SEL    24
#define M_TCB_SMAC_SEL    0xffULL
#define V_TCB_SMAC_SEL(x) ((x) << S_TCB_SMAC_SEL)

/* 95:32 */
#define W_TCB_T_FLAGS    1
#define S_TCB_T_FLAGS    0
#define M_TCB_T_FLAGS    0xffffffffffffffffULL
#define V_TCB_T_FLAGS(x) ((__u64)(x) << S_TCB_T_FLAGS)

/* 105:96 */
#define W_TCB_RSS_INFO    3
#define S_TCB_RSS_INFO    0
#define M_TCB_RSS_INFO    0x3ffULL
#define V_TCB_RSS_INFO(x) ((x) << S_TCB_RSS_INFO)

/* 111:106 */
#define W_TCB_TOS    3
#define S_TCB_TOS    10
#define M_TCB_TOS    0x3fULL
#define V_TCB_TOS(x) ((x) << S_TCB_TOS)

/* 115:112 */
#define W_TCB_T_STATE    3
#define S_TCB_T_STATE    16
#define M_TCB_T_STATE    0xfULL
#define V_TCB_T_STATE(x) ((x) << S_TCB_T_STATE)

/* 119:116 */
#define W_TCB_MAX_RT    3
#define S_TCB_MAX_RT    20
#define M_TCB_MAX_RT    0xfULL
#define V_TCB_MAX_RT(x) ((x) << S_TCB_MAX_RT)

/* 123:120 */
#define W_TCB_T_MAXSEG    3
#define S_TCB_T_MAXSEG    24
#define M_TCB_T_MAXSEG    0xfULL
#define V_TCB_T_MAXSEG(x) ((x) << S_TCB_T_MAXSEG)

/* 127:124 */
#define W_TCB_SND_SCALE    3
#define S_TCB_SND_SCALE    28
#define M_TCB_SND_SCALE    0xfULL
#define V_TCB_SND_SCALE(x) ((x) << S_TCB_SND_SCALE)

/* 131:128 */
#define W_TCB_RCV_SCALE    4
#define S_TCB_RCV_SCALE    0
#define M_TCB_RCV_SCALE    0xfULL
#define V_TCB_RCV_SCALE(x) ((x) << S_TCB_RCV_SCALE)

/* 135:132 */
#define W_TCB_T_RXTSHIFT    4
#define S_TCB_T_RXTSHIFT    4
#define M_TCB_T_RXTSHIFT    0xfULL
#define V_TCB_T_RXTSHIFT(x) ((x) << S_TCB_T_RXTSHIFT)

/* 139:136 */
#define W_TCB_T_DUPACKS    4
#define S_TCB_T_DUPACKS    8
#define M_TCB_T_DUPACKS    0xfULL
#define V_TCB_T_DUPACKS(x) ((x) << S_TCB_T_DUPACKS)

/* 143:140 */
#define W_TCB_TIMESTAMP_OFFSET    4
#define S_TCB_TIMESTAMP_OFFSET    12
#define M_TCB_TIMESTAMP_OFFSET    0xfULL
#define V_TCB_TIMESTAMP_OFFSET(x) ((x) << S_TCB_TIMESTAMP_OFFSET)

/* 159:144 */
#define W_TCB_RCV_ADV    4
#define S_TCB_RCV_ADV    16
#define M_TCB_RCV_ADV    0xffffULL
#define V_TCB_RCV_ADV(x) ((x) << S_TCB_RCV_ADV)

/* 191:160 */
#define W_TCB_TIMESTAMP    5
#define S_TCB_TIMESTAMP    0
#define M_TCB_TIMESTAMP    0xffffffffULL
#define V_TCB_TIMESTAMP(x) ((x) << S_TCB_TIMESTAMP)

/* 223:192 */
#define W_TCB_T_RTT_TS_RECENT_AGE    6
#define S_TCB_T_RTT_TS_RECENT_AGE    0
#define M_TCB_T_RTT_TS_RECENT_AGE    0xffffffffULL
#define V_TCB_T_RTT_TS_RECENT_AGE(x) ((x) << S_TCB_T_RTT_TS_RECENT_AGE)

/* 255:224 */
#define W_TCB_T_RTSEQ_RECENT    7
#define S_TCB_T_RTSEQ_RECENT    0
#define M_TCB_T_RTSEQ_RECENT    0xffffffffULL
#define V_TCB_T_RTSEQ_RECENT(x) ((x) << S_TCB_T_RTSEQ_RECENT)

/* 271:256 */
#define W_TCB_T_SRTT    8
#define S_TCB_T_SRTT    0
#define M_TCB_T_SRTT    0xffffULL
#define V_TCB_T_SRTT(x) ((x) << S_TCB_T_SRTT)

/* 287:272 */
#define W_TCB_T_RTTVAR    8
#define S_TCB_T_RTTVAR    16
#define M_TCB_T_RTTVAR    0xffffULL
#define V_TCB_T_RTTVAR(x) ((x) << S_TCB_T_RTTVAR)

/* 319:288 */
#define W_TCB_TX_MAX    9
#define S_TCB_TX_MAX    0
#define M_TCB_TX_MAX    0xffffffffULL
#define V_TCB_TX_MAX(x) ((x) << S_TCB_TX_MAX)

/* 347:320 */
#define W_TCB_SND_UNA_RAW    10
#define S_TCB_SND_UNA_RAW    0
#define M_TCB_SND_UNA_RAW    0xfffffffULL
#define V_TCB_SND_UNA_RAW(x) ((x) << S_TCB_SND_UNA_RAW)

/* 375:348 */
#define W_TCB_SND_NXT_RAW    10
#define S_TCB_SND_NXT_RAW    28
#define M_TCB_SND_NXT_RAW    0xfffffffULL
#define V_TCB_SND_NXT_RAW(x) ((__u64)(x) << S_TCB_SND_NXT_RAW)

/* 403:376 */
#define W_TCB_SND_MAX_RAW    11
#define S_TCB_SND_MAX_RAW    24
#define M_TCB_SND_MAX_RAW    0xfffffffULL
#define V_TCB_SND_MAX_RAW(x) ((__u64)(x) << S_TCB_SND_MAX_RAW)

/* 431:404 */
#define W_TCB_SND_REC_RAW    12
#define S_TCB_SND_REC_RAW    20
#define M_TCB_SND_REC_RAW    0xfffffffULL
#define V_TCB_SND_REC_RAW(x) ((__u64)(x) << S_TCB_SND_REC_RAW)

/* 459:432 */
#define W_TCB_SND_CWND    13
#define S_TCB_SND_CWND    16
#define M_TCB_SND_CWND    0xfffffffULL
#define V_TCB_SND_CWND(x) ((__u64)(x) << S_TCB_SND_CWND)

/* 487:460 */
#define W_TCB_SND_SSTHRESH    14
#define S_TCB_SND_SSTHRESH    12
#define M_TCB_SND_SSTHRESH    0xfffffffULL
#define V_TCB_SND_SSTHRESH(x) ((__u64)(x) << S_TCB_SND_SSTHRESH)

/* 504:488 */
#define W_TCB_TX_HDR_PTR_RAW    15
#define S_TCB_TX_HDR_PTR_RAW    8
#define M_TCB_TX_HDR_PTR_RAW    0x1ffffULL
#define V_TCB_TX_HDR_PTR_RAW(x) ((x) << S_TCB_TX_HDR_PTR_RAW)

/* 521:505 */
#define W_TCB_TX_LAST_PTR_RAW    15
#define S_TCB_TX_LAST_PTR_RAW    25
#define M_TCB_TX_LAST_PTR_RAW    0x1ffffULL
#define V_TCB_TX_LAST_PTR_RAW(x) ((__u64)(x) << S_TCB_TX_LAST_PTR_RAW)

/* 553:522 */
#define W_TCB_RCV_NXT    16
#define S_TCB_RCV_NXT    10
#define M_TCB_RCV_NXT    0xffffffffULL
#define V_TCB_RCV_NXT(x) ((__u64)(x) << S_TCB_RCV_NXT)

/* 581:554 */
#define W_TCB_RCV_WND    17
#define S_TCB_RCV_WND    10
#define M_TCB_RCV_WND    0xfffffffULL
#define V_TCB_RCV_WND(x) ((__u64)(x) << S_TCB_RCV_WND)

/* 609:582 */
#define W_TCB_RX_HDR_OFFSET    18
#define S_TCB_RX_HDR_OFFSET    6
#define M_TCB_RX_HDR_OFFSET    0xfffffffULL
#define V_TCB_RX_HDR_OFFSET(x) ((__u64)(x) << S_TCB_RX_HDR_OFFSET)

/* 637:610 */
#define W_TCB_TS_LAST_ACK_SENT_RAW    19
#define S_TCB_TS_LAST_ACK_SENT_RAW    2
#define M_TCB_TS_LAST_ACK_SENT_RAW    0xfffffffULL
#define V_TCB_TS_LAST_ACK_SENT_RAW(x) ((x) << S_TCB_TS_LAST_ACK_SENT_RAW)

/* 665:638 */
#define W_TCB_RX_FRAG0_START_IDX_RAW    19
#define S_TCB_RX_FRAG0_START_IDX_RAW    30
#define M_TCB_RX_FRAG0_START_IDX_RAW    0xfffffffULL
#define V_TCB_RX_FRAG0_START_IDX_RAW(x) ((__u64)(x) << S_TCB_RX_FRAG0_START_IDX_RAW)

/* 693:666 */
#define W_TCB_RX_FRAG1_START_IDX_OFFSET    20
#define S_TCB_RX_FRAG1_START_IDX_OFFSET    26
#define M_TCB_RX_FRAG1_START_IDX_OFFSET    0xfffffffULL
#define V_TCB_RX_FRAG1_START_IDX_OFFSET(x) ((__u64)(x) << S_TCB_RX_FRAG1_START_IDX_OFFSET)

/* 721:694 */
#define W_TCB_RX_FRAG0_LEN    21
#define S_TCB_RX_FRAG0_LEN    22
#define M_TCB_RX_FRAG0_LEN    0xfffffffULL
#define V_TCB_RX_FRAG0_LEN(x) ((__u64)(x) << S_TCB_RX_FRAG0_LEN)

/* 749:722 */
#define W_TCB_RX_FRAG1_LEN    22
#define S_TCB_RX_FRAG1_LEN    18
#define M_TCB_RX_FRAG1_LEN    0xfffffffULL
#define V_TCB_RX_FRAG1_LEN(x) ((__u64)(x) << S_TCB_RX_FRAG1_LEN)

/* 765:750 */
#define W_TCB_PDU_LEN    23
#define S_TCB_PDU_LEN    14
#define M_TCB_PDU_LEN    0xffffULL
#define V_TCB_PDU_LEN(x) ((x) << S_TCB_PDU_LEN)

/* 782:766 */
#define W_TCB_RX_PTR_RAW    23
#define S_TCB_RX_PTR_RAW    30
#define M_TCB_RX_PTR_RAW    0x1ffffULL
#define V_TCB_RX_PTR_RAW(x) ((__u64)(x) << S_TCB_RX_PTR_RAW)

/* 799:783 */
#define W_TCB_RX_FRAG1_PTR_RAW    24
#define S_TCB_RX_FRAG1_PTR_RAW    15
#define M_TCB_RX_FRAG1_PTR_RAW    0x1ffffULL
#define V_TCB_RX_FRAG1_PTR_RAW(x) ((x) << S_TCB_RX_FRAG1_PTR_RAW)

/* 831:800 */
#define W_TCB_MAIN_SLUSH    25
#define S_TCB_MAIN_SLUSH    0
#define M_TCB_MAIN_SLUSH    0xffffffffULL
#define V_TCB_MAIN_SLUSH(x) ((x) << S_TCB_MAIN_SLUSH)

/* 846:832 */
#define W_TCB_AUX1_SLUSH0    26
#define S_TCB_AUX1_SLUSH0    0
#define M_TCB_AUX1_SLUSH0    0x7fffULL
#define V_TCB_AUX1_SLUSH0(x) ((x) << S_TCB_AUX1_SLUSH0)

/* 874:847 */
#define W_TCB_RX_FRAG2_START_IDX_OFFSET_RAW    26
#define S_TCB_RX_FRAG2_START_IDX_OFFSET_RAW    15
#define M_TCB_RX_FRAG2_START_IDX_OFFSET_RAW    0xfffffffULL
#define V_TCB_RX_FRAG2_START_IDX_OFFSET_RAW(x) ((__u64)(x) << S_TCB_RX_FRAG2_START_IDX_OFFSET_RAW)

/* 891:875 */
#define W_TCB_RX_FRAG2_PTR_RAW    27
#define S_TCB_RX_FRAG2_PTR_RAW    11
#define M_TCB_RX_FRAG2_PTR_RAW    0x1ffffULL
#define V_TCB_RX_FRAG2_PTR_RAW(x) ((x) << S_TCB_RX_FRAG2_PTR_RAW)

/* 919:892 */
#define W_TCB_RX_FRAG2_LEN_RAW    27
#define S_TCB_RX_FRAG2_LEN_RAW    28
#define M_TCB_RX_FRAG2_LEN_RAW    0xfffffffULL
#define V_TCB_RX_FRAG2_LEN_RAW(x) ((__u64)(x) << S_TCB_RX_FRAG2_LEN_RAW)

/* 936:920 */
#define W_TCB_RX_FRAG3_PTR_RAW    28
#define S_TCB_RX_FRAG3_PTR_RAW    24
#define M_TCB_RX_FRAG3_PTR_RAW    0x1ffffULL
#define V_TCB_RX_FRAG3_PTR_RAW(x) ((__u64)(x) << S_TCB_RX_FRAG3_PTR_RAW)

/* 964:937 */
#define W_TCB_RX_FRAG3_LEN_RAW    29
#define S_TCB_RX_FRAG3_LEN_RAW    9
#define M_TCB_RX_FRAG3_LEN_RAW    0xfffffffULL
#define V_TCB_RX_FRAG3_LEN_RAW(x) ((__u64)(x) << S_TCB_RX_FRAG3_LEN_RAW)

/* 992:965 */
#define W_TCB_RX_FRAG3_START_IDX_OFFSET_RAW    30
#define S_TCB_RX_FRAG3_START_IDX_OFFSET_RAW    5
#define M_TCB_RX_FRAG3_START_IDX_OFFSET_RAW    0xfffffffULL
#define V_TCB_RX_FRAG3_START_IDX_OFFSET_RAW(x) ((__u64)(x) << S_TCB_RX_FRAG3_START_IDX_OFFSET_RAW)

/* 1000:993 */
#define W_TCB_PDU_HDR_LEN    31
#define S_TCB_PDU_HDR_LEN    1
#define M_TCB_PDU_HDR_LEN    0xffULL
#define V_TCB_PDU_HDR_LEN(x) ((x) << S_TCB_PDU_HDR_LEN)

/* 1019:1001 */
#define W_TCB_AUX1_SLUSH1    31
#define S_TCB_AUX1_SLUSH1    9
#define M_TCB_AUX1_SLUSH1    0x7ffffULL
#define V_TCB_AUX1_SLUSH1(x) ((x) << S_TCB_AUX1_SLUSH1)

/* 1023:1020 */
#define W_TCB_ULP_EXT    31
#define S_TCP_ULP_EXT    28
#define M_TCB_ULP_EXT    0xfULL
#define V_TCB_ULP_EXT(x) ((x) << S_TCP_ULP_EXT)


/* 840:832 */
#define W_TCB_IRS_ULP    26
#define S_TCB_IRS_ULP    0
#define M_TCB_IRS_ULP    0x1ffULL
#define V_TCB_IRS_ULP(x) ((x) << S_TCB_IRS_ULP)

/* 849:841 */
#define W_TCB_ISS_ULP    26
#define S_TCB_ISS_ULP    9
#define M_TCB_ISS_ULP    0x1ffULL
#define V_TCB_ISS_ULP(x) ((x) << S_TCB_ISS_ULP)

/* 863:850 */
#define W_TCB_TX_PDU_LEN    26
#define S_TCB_TX_PDU_LEN    18
#define M_TCB_TX_PDU_LEN    0x3fffULL
#define V_TCB_TX_PDU_LEN(x) ((x) << S_TCB_TX_PDU_LEN)

/* 879:864 */
#define W_TCB_CQ_IDX_SQ    27
#define S_TCB_CQ_IDX_SQ    0
#define M_TCB_CQ_IDX_SQ    0xffffULL
#define V_TCB_CQ_IDX_SQ(x) ((x) << S_TCB_CQ_IDX_SQ)

/* 895:880 */
#define W_TCB_CQ_IDX_RQ    27
#define S_TCB_CQ_IDX_RQ    16
#define M_TCB_CQ_IDX_RQ    0xffffULL
#define V_TCB_CQ_IDX_RQ(x) ((x) << S_TCB_CQ_IDX_RQ)

/* 911:896 */
#define W_TCB_QP_ID    28
#define S_TCB_QP_ID    0
#define M_TCB_QP_ID    0xffffULL
#define V_TCB_QP_ID(x) ((x) << S_TCB_QP_ID)

/* 927:912 */
#define W_TCB_PD_ID    28
#define S_TCB_PD_ID    16
#define M_TCB_PD_ID    0xffffULL
#define V_TCB_PD_ID(x) ((x) << S_TCB_PD_ID)

/* 959:928 */
#define W_TCB_STAG    29
#define S_TCB_STAG    0
#define M_TCB_STAG    0xffffffffULL
#define V_TCB_STAG(x) ((x) << S_TCB_STAG)

/* 985:960 */
#define W_TCB_RQ_START    30
#define S_TCB_RQ_START    0
#define M_TCB_RQ_START    0x3ffffffULL
#define V_TCB_RQ_START(x) ((x) << S_TCB_RQ_START)

/* 998:986 */
#define W_TCB_RQ_MSN    30
#define S_TCB_RQ_MSN    26
#define M_TCB_RQ_MSN    0x1fffULL
#define V_TCB_RQ_MSN(x) ((__u64)(x) << S_TCB_RQ_MSN)

/* 1002:999 */
#define W_TCB_RQ_MAX_OFFSET    31
#define S_TCB_RQ_MAX_OFFSET    7
#define M_TCB_RQ_MAX_OFFSET    0xfULL
#define V_TCB_RQ_MAX_OFFSET(x) ((x) << S_TCB_RQ_MAX_OFFSET)

/* 1015:1003 */
#define W_TCB_RQ_WRITE_PTR    31
#define S_TCB_RQ_WRITE_PTR    11
#define M_TCB_RQ_WRITE_PTR    0x1fffULL
#define V_TCB_RQ_WRITE_PTR(x) ((x) << S_TCB_RQ_WRITE_PTR)

/* 1019:1016 */
#define W_TCB_RDMAP_OPCODE    31
#define S_TCB_RDMAP_OPCODE    24
#define M_TCB_RDMAP_OPCODE    0xfULL
#define V_TCB_RDMAP_OPCODE(x) ((x) << S_TCB_RDMAP_OPCODE)

/* 1020:1020 */
#define W_TCB_ORD_L_BIT_VLD    31
#define S_TCB_ORD_L_BIT_VLD    28
#define M_TCB_ORD_L_BIT_VLD    0x1ULL
#define V_TCB_ORD_L_BIT_VLD(x) ((x) << S_TCB_ORD_L_BIT_VLD)

/* 1021:1021 */
#define W_TCB_TX_FLUSH    31
#define S_TCB_TX_FLUSH    29
#define M_TCB_TX_FLUSH    0x1ULL
#define V_TCB_TX_FLUSH(x) ((x) << S_TCB_TX_FLUSH)

/* 1022:1022 */
#define W_TCB_TX_OOS_RXMT    31
#define S_TCB_TX_OOS_RXMT    30
#define M_TCB_TX_OOS_RXMT    0x1ULL
#define V_TCB_TX_OOS_RXMT(x) ((x) << S_TCB_TX_OOS_RXMT)

/* 1023:1023 */
#define W_TCB_TX_OOS_TXMT    31
#define S_TCB_TX_OOS_TXMT    31
#define M_TCB_TX_OOS_TXMT    0x1ULL
#define V_TCB_TX_OOS_TXMT(x) ((x) << S_TCB_TX_OOS_TXMT)

/* 855:832 */
#define W_TCB_RX_DDP_BUF0_OFFSET    26
#define S_TCB_RX_DDP_BUF0_OFFSET    0
#define M_TCB_RX_DDP_BUF0_OFFSET    0xffffffULL
#define V_TCB_RX_DDP_BUF0_OFFSET(x) ((x) << S_TCB_RX_DDP_BUF0_OFFSET)

/* 879:856 */
#define W_TCB_RX_DDP_BUF0_LEN    26
#define S_TCB_RX_DDP_BUF0_LEN    24
#define M_TCB_RX_DDP_BUF0_LEN    0xffffffULL
#define V_TCB_RX_DDP_BUF0_LEN(x) ((__u64)(x) << S_TCB_RX_DDP_BUF0_LEN)

/* 903:880 */
#define W_TCB_RX_DDP_FLAGS    27
#define S_TCB_RX_DDP_FLAGS    16
#define M_TCB_RX_DDP_FLAGS    0xffffffULL
#define V_TCB_RX_DDP_FLAGS(x) ((__u64)(x) << S_TCB_RX_DDP_FLAGS)

/* 927:904 */
#define W_TCB_RX_DDP_BUF1_OFFSET    28
#define S_TCB_RX_DDP_BUF1_OFFSET    8
#define M_TCB_RX_DDP_BUF1_OFFSET    0xffffffULL
#define V_TCB_RX_DDP_BUF1_OFFSET(x) ((x) << S_TCB_RX_DDP_BUF1_OFFSET)

/* 951:928 */
#define W_TCB_RX_DDP_BUF1_LEN    29
#define S_TCB_RX_DDP_BUF1_LEN    0
#define M_TCB_RX_DDP_BUF1_LEN    0xffffffULL
#define V_TCB_RX_DDP_BUF1_LEN(x) ((x) << S_TCB_RX_DDP_BUF1_LEN)

/* 959:952 */
#define W_TCB_AUX3_SLUSH    29
#define S_TCB_AUX3_SLUSH    24
#define M_TCB_AUX3_SLUSH    0xffULL
#define V_TCB_AUX3_SLUSH(x) ((x) << S_TCB_AUX3_SLUSH)

/* 991:960 */
#define W_TCB_RX_DDP_BUF0_TAG    30
#define S_TCB_RX_DDP_BUF0_TAG    0
#define M_TCB_RX_DDP_BUF0_TAG    0xffffffffULL
#define V_TCB_RX_DDP_BUF0_TAG(x) ((x) << S_TCB_RX_DDP_BUF0_TAG)

/* 1023:992 */
#define W_TCB_RX_DDP_BUF1_TAG    31
#define S_TCB_RX_DDP_BUF1_TAG    0
#define M_TCB_RX_DDP_BUF1_TAG    0xffffffffULL
#define V_TCB_RX_DDP_BUF1_TAG(x) ((x) << S_TCB_RX_DDP_BUF1_TAG)

#define S_TF_MIGRATING    0
#define V_TF_MIGRATING(x) ((x) << S_TF_MIGRATING)

#define S_TF_NON_OFFLOAD    1
#define V_TF_NON_OFFLOAD(x) ((x) << S_TF_NON_OFFLOAD)

#define S_TF_LOCK_TID    2
#define V_TF_LOCK_TID(x) ((x) << S_TF_LOCK_TID)

#define S_TF_KEEPALIVE    3
#define V_TF_KEEPALIVE(x) ((x) << S_TF_KEEPALIVE)

#define S_TF_DACK    4
#define V_TF_DACK(x) ((x) << S_TF_DACK)

#define S_TF_DACK_MSS    5
#define V_TF_DACK_MSS(x) ((x) << S_TF_DACK_MSS)

#define S_TF_DACK_NOT_ACKED    6
#define V_TF_DACK_NOT_ACKED(x) ((x) << S_TF_DACK_NOT_ACKED)

#define S_TF_NAGLE    7
#define V_TF_NAGLE(x) ((x) << S_TF_NAGLE)

#define S_TF_SSWS_DISABLED    8
#define V_TF_SSWS_DISABLED(x) ((x) << S_TF_SSWS_DISABLED)

#define S_TF_RX_FLOW_CONTROL_DDP    9
#define V_TF_RX_FLOW_CONTROL_DDP(x) ((x) << S_TF_RX_FLOW_CONTROL_DDP)

#define S_TF_RX_FLOW_CONTROL_DISABLE    10
#define V_TF_RX_FLOW_CONTROL_DISABLE(x) ((x) << S_TF_RX_FLOW_CONTROL_DISABLE)

#define S_TF_RX_CHANNEL    11
#define V_TF_RX_CHANNEL(x) ((x) << S_TF_RX_CHANNEL)

#define S_TF_TX_CHANNEL0    12
#define V_TF_TX_CHANNEL0(x) ((x) << S_TF_TX_CHANNEL0)

#define S_TF_TX_CHANNEL1    13
#define V_TF_TX_CHANNEL1(x) ((x) << S_TF_TX_CHANNEL1)

#define S_TF_TX_QUIESCE    14
#define V_TF_TX_QUIESCE(x) ((x) << S_TF_TX_QUIESCE)

#define S_TF_RX_QUIESCE    15
#define V_TF_RX_QUIESCE(x) ((x) << S_TF_RX_QUIESCE)

#define S_TF_TX_PACE_AUTO    16
#define V_TF_TX_PACE_AUTO(x) ((x) << S_TF_TX_PACE_AUTO)

#define S_TF_MASK_HASH    16
#define V_TF_MASK_HASH(x) ((x) << S_TF_MASK_HASH)

#define S_TF_TX_PACE_FIXED    17
#define V_TF_TX_PACE_FIXED(x) ((x) << S_TF_TX_PACE_FIXED)

#define S_TF_DIRECT_STEER_HASH    17
#define V_TF_DIRECT_STEER_HASH(x) ((x) << S_TF_DIRECT_STEER_HASH)

#define S_TF_TX_QUEUE    18
#define M_TF_TX_QUEUE    0x7ULL
#define V_TF_TX_QUEUE(x) ((x) << S_TF_TX_QUEUE)

#define S_TF_TURBO    21
#define V_TF_TURBO(x) ((x) << S_TF_TURBO)

#define S_TF_REPORT_TID    21
#define V_TF_REPORT_TID(x) ((x) << S_TF_REPORT_TID)

#define S_TF_CCTRL_SEL0    22
#define V_TF_CCTRL_SEL0(x) ((x) << S_TF_CCTRL_SEL0)

#define S_TF_DROP    22
#define V_TF_DROP(x) ((x) << S_TF_DROP)

#define S_TF_CCTRL_SEL1    23
#define V_TF_CCTRL_SEL1(x) ((x) << S_TF_CCTRL_SEL1)

#define S_TF_DIRECT_STEER    23
#define V_TF_DIRECT_STEER(x) ((x) << S_TF_DIRECT_STEER)

#define S_TF_CORE_FIN    24
#define V_TF_CORE_FIN(x) ((x) << S_TF_CORE_FIN)

#define S_TF_CORE_URG    25
#define V_TF_CORE_URG(x) ((x) << S_TF_CORE_URG)

#define S_TF_CORE_MORE    26
#define V_TF_CORE_MORE(x) ((x) << S_TF_CORE_MORE)

#define S_TF_CORE_PUSH    27
#define V_TF_CORE_PUSH(x) ((x) << S_TF_CORE_PUSH)

#define S_TF_CORE_FLUSH    28
#define V_TF_CORE_FLUSH(x) ((x) << S_TF_CORE_FLUSH)

#define S_TF_RCV_COALESCE_ENABLE    29
#define V_TF_RCV_COALESCE_ENABLE(x) ((x) << S_TF_RCV_COALESCE_ENABLE)

#define S_TF_RCV_COALESCE_PUSH    30
#define V_TF_RCV_COALESCE_PUSH(x) ((x) << S_TF_RCV_COALESCE_PUSH)

#define S_TF_RCV_COALESCE_LAST_PSH    31
#define V_TF_RCV_COALESCE_LAST_PSH(x) ((x) << S_TF_RCV_COALESCE_LAST_PSH)

#define S_TF_RCV_COALESCE_HEARTBEAT    32
#define V_TF_RCV_COALESCE_HEARTBEAT(x) ((__u64)(x) << S_TF_RCV_COALESCE_HEARTBEAT)

#define S_TF_INIT    33
#define V_TF_INIT(x) ((__u64)(x) << S_TF_INIT)

#define S_TF_ACTIVE_OPEN    34
#define V_TF_ACTIVE_OPEN(x) ((__u64)(x) << S_TF_ACTIVE_OPEN)

#define S_TF_ASK_MODE    35
#define V_TF_ASK_MODE(x) ((__u64)(x) << S_TF_ASK_MODE)

#define S_TF_MOD_SCHD_REASON0    36
#define V_TF_MOD_SCHD_REASON0(x) ((__u64)(x) << S_TF_MOD_SCHD_REASON0)

#define S_TF_MOD_SCHD_REASON1    37
#define V_TF_MOD_SCHD_REASON1(x) ((__u64)(x) << S_TF_MOD_SCHD_REASON1)

#define S_TF_MOD_SCHD_REASON2    38
#define V_TF_MOD_SCHD_REASON2(x) ((__u64)(x) << S_TF_MOD_SCHD_REASON2)

#define S_TF_MOD_SCHD_TX    39
#define V_TF_MOD_SCHD_TX(x) ((__u64)(x) << S_TF_MOD_SCHD_TX)

#define S_TF_MOD_SCHD_RX    40
#define V_TF_MOD_SCHD_RX(x) ((__u64)(x) << S_TF_MOD_SCHD_RX)

#define S_TF_TIMER    41
#define V_TF_TIMER(x) ((__u64)(x) << S_TF_TIMER)

#define S_TF_DACK_TIMER    42
#define V_TF_DACK_TIMER(x) ((__u64)(x) << S_TF_DACK_TIMER)

#define S_TF_PEER_FIN    43
#define V_TF_PEER_FIN(x) ((__u64)(x) << S_TF_PEER_FIN)

#define S_TF_TX_COMPACT    44
#define V_TF_TX_COMPACT(x) ((__u64)(x) << S_TF_TX_COMPACT)

#define S_TF_RX_COMPACT    45
#define V_TF_RX_COMPACT(x) ((__u64)(x) << S_TF_RX_COMPACT)

#define S_TF_RDMA_ERROR    46
#define V_TF_RDMA_ERROR(x) ((__u64)(x) << S_TF_RDMA_ERROR)

#define S_TF_RDMA_FLM_ERROR    47
#define V_TF_RDMA_FLM_ERROR(x) ((__u64)(x) << S_TF_RDMA_FLM_ERROR)

#define S_TF_TX_PDU_OUT    48
#define V_TF_TX_PDU_OUT(x) ((__u64)(x) << S_TF_TX_PDU_OUT)

#define S_TF_RX_PDU_OUT    49
#define V_TF_RX_PDU_OUT(x) ((__u64)(x) << S_TF_RX_PDU_OUT)

#define S_TF_DUPACK_COUNT_ODD    50
#define V_TF_DUPACK_COUNT_ODD(x) ((__u64)(x) << S_TF_DUPACK_COUNT_ODD)

#define S_TF_FAST_RECOVERY    51
#define V_TF_FAST_RECOVERY(x) ((__u64)(x) << S_TF_FAST_RECOVERY)

#define S_TF_RECV_SCALE    52
#define V_TF_RECV_SCALE(x) ((__u64)(x) << S_TF_RECV_SCALE)

#define S_TF_RECV_TSTMP    53
#define V_TF_RECV_TSTMP(x) ((__u64)(x) << S_TF_RECV_TSTMP)

#define S_TF_RECV_SACK    54
#define V_TF_RECV_SACK(x) ((__u64)(x) << S_TF_RECV_SACK)

#define S_TF_PEND_CTL0    55
#define V_TF_PEND_CTL0(x) ((__u64)(x) << S_TF_PEND_CTL0)

#define S_TF_PEND_CTL1    56
#define V_TF_PEND_CTL1(x) ((__u64)(x) << S_TF_PEND_CTL1)

#define S_TF_PEND_CTL2    57
#define V_TF_PEND_CTL2(x) ((__u64)(x) << S_TF_PEND_CTL2)

#define S_TF_IP_VERSION    58
#define V_TF_IP_VERSION(x) ((__u64)(x) << S_TF_IP_VERSION)

#define S_TF_CCTRL_ECN    59
#define V_TF_CCTRL_ECN(x) ((__u64)(x) << S_TF_CCTRL_ECN)

#define S_TF_LPBK    59
#define V_TF_LPBK(x) ((__u64)(x) << S_TF_LPBK)

#define S_TF_CCTRL_ECE    60
#define V_TF_CCTRL_ECE(x) ((__u64)(x) << S_TF_CCTRL_ECE)

#define S_TF_REWRITE_DMAC    60
#define V_TF_REWRITE_DMAC(x) ((__u64)(x) << S_TF_REWRITE_DMAC)

#define S_TF_CCTRL_CWR    61
#define V_TF_CCTRL_CWR(x) ((__u64)(x) << S_TF_CCTRL_CWR)

#define S_TF_REWRITE_SMAC    61
#define V_TF_REWRITE_SMAC(x) ((__u64)(x) << S_TF_REWRITE_SMAC)

#define S_TF_CCTRL_RFR    62
#define V_TF_CCTRL_RFR(x) ((__u64)(x) << S_TF_CCTRL_RFR)

#define S_TF_DDP_INDICATE_OUT    16
#define V_TF_DDP_INDICATE_OUT(x) ((x) << S_TF_DDP_INDICATE_OUT)

#define S_TF_DDP_ACTIVE_BUF    17
#define V_TF_DDP_ACTIVE_BUF(x) ((x) << S_TF_DDP_ACTIVE_BUF)

#define S_TF_DDP_OFF    18
#define V_TF_DDP_OFF(x) ((x) << S_TF_DDP_OFF)

#define S_TF_DDP_WAIT_FRAG    19
#define V_TF_DDP_WAIT_FRAG(x) ((x) << S_TF_DDP_WAIT_FRAG)

#define S_TF_DDP_BUF_INF    20
#define V_TF_DDP_BUF_INF(x) ((x) << S_TF_DDP_BUF_INF)

#define S_TF_DDP_RX2TX    21
#define V_TF_DDP_RX2TX(x) ((x) << S_TF_DDP_RX2TX)

#define S_TF_DDP_BUF0_VALID    24
#define V_TF_DDP_BUF0_VALID(x) ((x) << S_TF_DDP_BUF0_VALID)

#define S_TF_DDP_BUF0_INDICATE    25
#define V_TF_DDP_BUF0_INDICATE(x) ((x) << S_TF_DDP_BUF0_INDICATE)

#define S_TF_DDP_BUF0_FLUSH    26
#define V_TF_DDP_BUF0_FLUSH(x) ((x) << S_TF_DDP_BUF0_FLUSH)

#define S_TF_DDP_PSHF_ENABLE_0    27
#define V_TF_DDP_PSHF_ENABLE_0(x) ((x) << S_TF_DDP_PSHF_ENABLE_0)

#define S_TF_DDP_PUSH_DISABLE_0    28
#define V_TF_DDP_PUSH_DISABLE_0(x) ((x) << S_TF_DDP_PUSH_DISABLE_0)

#define S_TF_DDP_PSH_NO_INVALIDATE0    29
#define V_TF_DDP_PSH_NO_INVALIDATE0(x) ((x) << S_TF_DDP_PSH_NO_INVALIDATE0)

#define S_TF_DDP_BUF1_VALID    32
#define V_TF_DDP_BUF1_VALID(x) ((__u64)(x) << S_TF_DDP_BUF1_VALID)

#define S_TF_DDP_BUF1_INDICATE    33
#define V_TF_DDP_BUF1_INDICATE(x) ((__u64)(x) << S_TF_DDP_BUF1_INDICATE)

#define S_TF_DDP_BUF1_FLUSH    34
#define V_TF_DDP_BUF1_FLUSH(x) ((__u64)(x) << S_TF_DDP_BUF1_FLUSH)

#define S_TF_DDP_PSHF_ENABLE_1    35
#define V_TF_DDP_PSHF_ENABLE_1(x) ((__u64)(x) << S_TF_DDP_PSHF_ENABLE_1)

#define S_TF_DDP_PUSH_DISABLE_1    36
#define V_TF_DDP_PUSH_DISABLE_1(x) ((__u64)(x) << S_TF_DDP_PUSH_DISABLE_1)

#define S_TF_DDP_PSH_NO_INVALIDATE1    37
#define V_TF_DDP_PSH_NO_INVALIDATE1(x) ((__u64)(x) << S_TF_DDP_PSH_NO_INVALIDATE1)

#endif /* _T4_TCB_DEFS_H */
