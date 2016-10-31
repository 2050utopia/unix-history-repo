/*-
 * Copyright (c) 2016 Microsoft Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _IF_HNVAR_H_
#define _IF_HNVAR_H_

#define HN_USE_TXDESC_BUFRING

#define HN_CHIM_SIZE			(15 * 1024 * 1024)

#define HN_RXBUF_SIZE			(16 * 1024 * 1024)
#define HN_RXBUF_SIZE_COMPAT		(15 * 1024 * 1024)

/* Claimed to be 12232B */
#define HN_MTU_MAX			(9 * 1024)

#define HN_TXBR_SIZE			(128 * PAGE_SIZE)
#define HN_RXBR_SIZE			(128 * PAGE_SIZE)

#define HN_XACT_REQ_PGCNT		2
#define HN_XACT_RESP_PGCNT		2
#define HN_XACT_REQ_SIZE		(HN_XACT_REQ_PGCNT * PAGE_SIZE)
#define HN_XACT_RESP_SIZE		(HN_XACT_RESP_PGCNT * PAGE_SIZE)

#define HN_GPACNT_MAX			32

struct hn_txdesc;
#ifndef HN_USE_TXDESC_BUFRING
SLIST_HEAD(hn_txdesc_list, hn_txdesc);
#else
struct buf_ring;
#endif
struct hn_tx_ring;

struct hn_rx_ring {
	struct ifnet	*hn_ifp;
	struct hn_tx_ring *hn_txr;
	void		*hn_pktbuf;
	int		hn_pktbuf_len;
	uint8_t		*hn_rxbuf;	/* shadow sc->hn_rxbuf */
	int		hn_rx_idx;

	/* Trust csum verification on host side */
	int		hn_trust_hcsum;	/* HN_TRUST_HCSUM_ */
	struct lro_ctrl	hn_lro;

	u_long		hn_csum_ip;
	u_long		hn_csum_tcp;
	u_long		hn_csum_udp;
	u_long		hn_csum_trusted;
	u_long		hn_lro_tried;
	u_long		hn_small_pkts;
	u_long		hn_pkts;
	u_long		hn_rss_pkts;

	/* Rarely used stuffs */
	struct sysctl_oid *hn_rx_sysctl_tree;
	int		hn_rx_flags;

	void		*hn_br;		/* TX/RX bufring */
	struct hyperv_dma hn_br_dma;
} __aligned(CACHE_LINE_SIZE);

#define HN_TRUST_HCSUM_IP	0x0001
#define HN_TRUST_HCSUM_TCP	0x0002
#define HN_TRUST_HCSUM_UDP	0x0004

#define HN_RX_FLAG_ATTACHED	0x1

struct hn_tx_ring {
#ifndef HN_USE_TXDESC_BUFRING
	struct mtx	hn_txlist_spin;
	struct hn_txdesc_list hn_txlist;
#else
	struct buf_ring	*hn_txdesc_br;
#endif
	int		hn_txdesc_cnt;
	int		hn_txdesc_avail;
	u_short		hn_has_txeof;
	u_short		hn_txdone_cnt;

	int		hn_sched_tx;
	void		(*hn_txeof)(struct hn_tx_ring *);
	struct taskqueue *hn_tx_taskq;
	struct task	hn_tx_task;
	struct task	hn_txeof_task;

	struct buf_ring	*hn_mbuf_br;
	int		hn_oactive;
	int		hn_tx_idx;
	int		hn_tx_flags;

	struct mtx	hn_tx_lock;
	struct hn_softc	*hn_sc;
	struct vmbus_channel *hn_chan;

	int		hn_direct_tx_size;
	int		hn_chim_size;
	bus_dma_tag_t	hn_tx_data_dtag;
	uint64_t	hn_csum_assist;

	int		(*hn_sendpkt)(struct hn_tx_ring *, struct hn_txdesc *);
	int		hn_suspended;
	int		hn_gpa_cnt;
	struct vmbus_gpa hn_gpa[HN_GPACNT_MAX];

	u_long		hn_no_txdescs;
	u_long		hn_send_failed;
	u_long		hn_txdma_failed;
	u_long		hn_tx_collapsed;
	u_long		hn_tx_chimney_tried;
	u_long		hn_tx_chimney;
	u_long		hn_pkts;

	/* Rarely used stuffs */
	struct hn_txdesc *hn_txdesc;
	bus_dma_tag_t	hn_tx_rndis_dtag;
	struct sysctl_oid *hn_tx_sysctl_tree;
} __aligned(CACHE_LINE_SIZE);

#define HN_TX_FLAG_ATTACHED	0x1
#define HN_TX_FLAG_HASHVAL	0x2	/* support HASHVAL pktinfo */

/*
 * Device-specific softc structure
 */
struct hn_softc {
	struct ifnet    *hn_ifp;
	struct ifmedia	hn_media;
	device_t        hn_dev;
	int             hn_if_flags;
	struct sx	hn_lock;
	struct vmbus_channel *hn_prichan;

	int		hn_rx_ring_cnt;
	int		hn_rx_ring_inuse;
	struct hn_rx_ring *hn_rx_ring;

	int		hn_tx_ring_cnt;
	int		hn_tx_ring_inuse;
	struct hn_tx_ring *hn_tx_ring;

	uint8_t		*hn_chim;
	u_long		*hn_chim_bmap;
	int		hn_chim_bmap_cnt;
	int		hn_chim_cnt;
	int		hn_chim_szmax;

	int		hn_cpu;
	struct taskqueue *hn_tx_taskq;
	struct sysctl_oid *hn_tx_sysctl_tree;
	struct sysctl_oid *hn_rx_sysctl_tree;
	struct vmbus_xact_ctx *hn_xact;
	uint32_t	hn_nvs_ver;
	uint32_t	hn_rx_filter;

	struct taskqueue	*hn_mgmt_taskq;
	struct taskqueue	*hn_mgmt_taskq0;
	struct task		hn_link_task;
	struct task		hn_netchg_init;
	struct timeout_task	hn_netchg_status;
	uint32_t		hn_link_flags;	/* HN_LINK_FLAG_ */

	uint32_t		hn_caps;	/* HN_CAP_ */
	uint32_t		hn_flags;	/* HN_FLAG_ */
	void			*hn_rxbuf;
	uint32_t		hn_rxbuf_gpadl;
	struct hyperv_dma	hn_rxbuf_dma;

	uint32_t		hn_chim_gpadl;
	struct hyperv_dma	hn_chim_dma;

	uint32_t		hn_rndis_rid;
	uint32_t		hn_ndis_ver;
	int			hn_ndis_tso_szmax;
	int			hn_ndis_tso_sgmin;

	int			hn_rss_ind_size;
	uint32_t		hn_rss_hash;	/* NDIS_HASH_ */
	struct ndis_rssprm_toeplitz hn_rss;
};

#define HN_FLAG_RXBUF_CONNECTED		0x0001
#define HN_FLAG_CHIM_CONNECTED		0x0002
#define HN_FLAG_HAS_RSSKEY		0x0004
#define HN_FLAG_HAS_RSSIND		0x0008
#define HN_FLAG_SYNTH_ATTACHED		0x0010

#define HN_CAP_VLAN			0x0001
#define HN_CAP_MTU			0x0002
#define HN_CAP_IPCS			0x0004
#define HN_CAP_TCP4CS			0x0008
#define HN_CAP_TCP6CS			0x0010
#define HN_CAP_UDP4CS			0x0020
#define HN_CAP_UDP6CS			0x0040
#define HN_CAP_TSO4			0x0080
#define HN_CAP_TSO6			0x0100
#define HN_CAP_HASHVAL			0x0200

/* Capability description for use with printf(9) %b identifier. */
#define HN_CAP_BITS				\
	"\020\1VLAN\2MTU\3IPCS\4TCP4CS\5TCP6CS"	\
	"\6UDP4CS\7UDP6CS\10TSO4\11TSO6\12HASHVAL"

#define HN_LINK_FLAG_LINKUP		0x0001
#define HN_LINK_FLAG_NETCHG		0x0002

#endif	/* !_IF_HNVAR_H_ */
