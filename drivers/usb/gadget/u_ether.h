/*
 * u_ether.h -- interface to USB gadget "ethernet link" utilities
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#ifndef __U_ETHER_H
#define __U_ETHER_H

#include <linux/err.h>
#include <linux/if_ether.h>
#include <linux/usb/composite.h>
#include <linux/usb/cdc.h>

#include "gadget_chips.h"

#undef USB_ANDROID_NCM


#define USB_ANDROID_NCM

#define USE_ETHER_XMIT_FOR_NCM


/*
 * This represents the USB side of an "ethernet" link, managed by a USB
 * function which provides control and (maybe) framing.  Two functions
 * in different configurations could share the same ethernet link/netdev,
 * using different host interaction models.
 *
 * There is a current limitation that only one instance of this link may
 * be present in any given configuration.  When that's a problem, network
 * layer facilities can be used to package multiple logical links on this
 * single "physical" one.
 */
struct gether {
	struct usb_function		func;

	/* updated by gether_{connect,disconnect} */
	struct eth_dev			*ioport;

	/* endpoints handle full and/or high speeds */
	struct usb_ep			*in_ep;
	struct usb_ep			*out_ep;

	bool				is_zlp_ok;

	u16				cdc_filter;

	/* hooks for added framing, as needed for RNDIS and EEM. */
	u32				header_len;
	/* NCM requires fixed size bundles */
	bool				is_fixed;
	u32				fixed_out_len;
	u32				fixed_in_len;
/* Max number of SKB packets to be used to create Multi Packet RNDIS */
#define TX_SKB_HOLD_THRESHOLD		3
	bool				multi_pkt_xfer;
	struct sk_buff			*(*wrap)(struct gether *port,
						struct sk_buff *skb);
	int				(*unwrap)(struct gether *port,
						struct sk_buff *skb,
						struct sk_buff_head *list);

	/* called on network open/close */
	void				(*open)(struct gether *);
	void				(*close)(struct gether *);
#if defined(USB_ANDROID_NCM)

	struct net_device*		net;
	atomic_t			pending_writes;	

#endif 
};

#define	DEFAULT_FILTER	(USB_CDC_PACKET_TYPE_BROADCAST \
			|USB_CDC_PACKET_TYPE_ALL_MULTICAST \
			|USB_CDC_PACKET_TYPE_PROMISCUOUS \
			|USB_CDC_PACKET_TYPE_DIRECTED)


/* netdev setup/teardown as directed by the gadget driver */
int gether_setup(struct usb_gadget *g, u8 ethaddr[ETH_ALEN]);
void gether_cleanup(void);
#if !defined(USB_ANDROID_NCM)
/* variant of gether_setup that allows customizing network device name */
int gether_setup_name(struct usb_gadget *g, u8 ethaddr[ETH_ALEN],
		const char *netname);
#else 

int gether_setup_name_mtu(struct usb_gadget *g, u8 ethaddr[ETH_ALEN],
		const char *netname, unsigned int mtu);
#endif 
/* connect/disconnect is handled by individual functions */
struct net_device *gether_connect(struct gether *);
void gether_disconnect(struct gether *);

/* Some controllers can't support CDC Ethernet (ECM) ... */
static inline bool can_support_ecm(struct usb_gadget *gadget)
{
	if (!gadget_supports_altsettings(gadget))
		return false;

	/* Everything else is *presumably* fine ... but this is a bit
	 * chancy, so be **CERTAIN** there are no hardware issues with
	 * your controller.  Add it above if it can't handle CDC.
	 */
	return true;
}

/* each configuration may bind one instance of an ethernet link */
int geth_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN]);
int ecm_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN]);
int ncm_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN]);
int eem_bind_config(struct usb_configuration *c);

#ifdef USB_ETH_RNDIS

int rndis_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN]);
int rndis_bind_config_vendor(struct usb_configuration *c, u8 ethaddr[ETH_ALEN],
				u32 vendorID, const char *manufacturer);

#else

static inline int
rndis_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN])
{
	return 0;
}

static inline int
rndis_bind_config_vendor(struct usb_configuration *c, u8 ethaddr[ETH_ALEN],
				u32 vendorID, const char *manufacturer)
{
	return 0;
}

#endif

#if defined(USB_ANDROID_NCM)


#if !defined(USE_ETHER_XMIT_FOR_NCM)
#else
netdev_tx_t eth_start_xmit(struct sk_buff *skb,
					struct net_device *net);
#endif

struct eth_dev {
	


	spinlock_t		lock;
	struct gether		*port_usb;

	struct net_device	*net;
	struct usb_gadget	*gadget;

	spinlock_t		req_lock;	
	struct list_head	tx_reqs, rx_reqs;

	unsigned		tx_qlen;


#define TX_REQ_THRESHOLD	5
	int			no_tx_req_used;
	int			tx_skb_hold_count;
	u32			tx_req_bufsize;

	struct sk_buff_head	rx_frames;

	unsigned		header_len;
	struct sk_buff		*(*wrap)(struct gether *, struct sk_buff *skb);
	int			(*unwrap)(struct gether *,
						struct sk_buff *skb,
						struct sk_buff_head *list);

	struct work_struct	work;
	struct work_struct	rx_work;

	unsigned long		todo;
#define	WORK_RX_MEMORY		0

	bool			zlp;
	u8			host_mac[ETH_ALEN];
};

#endif 

#endif /* __U_ETHER_H */
