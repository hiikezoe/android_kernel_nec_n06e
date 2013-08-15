
/*
 * f_serial.c - generic USB serial function driver
 *
 * Copyright (C) 2003 Al Borchers (alborchers@steinerpoint.com)
 * Copyright (C) 2008 by David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/
















#include <linux/usb/oem_usb_custom.h>

#if defined(DVE021_USB_FUNCTION_DVE021_SERIAL)

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <mach/usb_gadget_xport.h>

#include "u_serial.h"
#include "gadget_chips.h"










#define DVE021_SERIAL_NO_PORTS 2


struct f_DVE022_serial {
	struct gserial			port;
	u8				data_id;
	u8				port_num;

	u8				online;
	enum transport_type		transport;
};

static unsigned int no_tty_ports_DVE022_serial;
static unsigned int nr_ports_DVE022_serial;

static struct DVE022_port_info {
	enum transport_type	transport;
	unsigned		port_num;
	unsigned		client_port_num;
} DVE022_serial_ports[DVE021_SERIAL_NO_PORTS];

static inline struct f_DVE022_serial *func_to_ncmcser(struct usb_function *f)
{
	return container_of(f, struct f_DVE022_serial, port.func);
}





static struct usb_interface_descriptor DVE022_serial_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	
};

static struct usb_endpoint_descriptor DVE022_serial_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor DVE022_serial_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *DVE022_serial_fs_function[] = {
	(struct usb_descriptor_header *) &DVE022_serial_interface_desc,
	(struct usb_descriptor_header *) &DVE022_serial_fs_in_desc,
	(struct usb_descriptor_header *) &DVE022_serial_fs_out_desc,
	NULL,
};

static struct usb_endpoint_descriptor DVE022_serial_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor DVE022_serial_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_descriptor_header *DVE022_serial_hs_function[] = {
	(struct usb_descriptor_header *) &DVE022_serial_interface_desc,
	(struct usb_descriptor_header *) &DVE022_serial_hs_in_desc,
	(struct usb_descriptor_header *) &DVE022_serial_hs_out_desc,
	NULL,
};

static struct usb_endpoint_descriptor DVE022_serial_ss_in_desc __initdata = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor DVE022_serial_ss_out_desc __initdata = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor DVE022_serial_ss_bulk_comp_desc __initdata = {
	.bLength =              sizeof DVE022_serial_ss_bulk_comp_desc,
	.bDescriptorType =      USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_descriptor_header *DVE022_serial_ss_function[] __initdata = {
	(struct usb_descriptor_header *) &DVE022_serial_interface_desc,
	(struct usb_descriptor_header *) &DVE022_serial_ss_in_desc,
	(struct usb_descriptor_header *) &DVE022_serial_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &DVE022_serial_ss_out_desc,
	(struct usb_descriptor_header *) &DVE022_serial_ss_bulk_comp_desc,
	NULL,
};



static struct usb_string DVE022_serial_string_defs[] = {
	[0].s = DVE021_USB_IF_DESC_NAME_DVE021_SERIAL,
	{  } 
};

static struct usb_gadget_strings DVE022_serial_string_table = {
	.language =		0x0409,	
	.strings =		DVE022_serial_string_defs,
};

static struct usb_gadget_strings *DVE022_serial_strings[] = {
	&DVE022_serial_string_table,
	NULL,
};





static void notify_DVE022_serial_status( enum usb_DVE022_serial_mode_status sta );

static void (*DVE022_serial_cb)( enum usb_DVE022_serial_mode_status status ) = NULL;
static bool DVE022_serial_usb_online = false;
static bool DVE022_serial_usb_notify = false;

static int DVE022_serial_gport_setup(struct usb_configuration *c)
{
	int ret = 0;

	pr_debug("%s: no_tty_ports_DVE022_serial: %u nr_ports_DVE022_serial: %u\n",
			__func__, no_tty_ports_DVE022_serial, nr_ports_DVE022_serial);

	if (no_tty_ports_DVE022_serial)
		ret = gserial_setup(c->cdev->gadget, no_tty_ports_DVE022_serial);
	return ret;
}

static int DVE022_serial_gport_connect(struct f_DVE022_serial *ncmcser)
{
	unsigned	port_num;

	pr_debug("%s: transport: %s f_DVE022_serial: %p gserial: %p port_num: %d\n",
			__func__, xport_to_str(ncmcser->transport),
			ncmcser, &ncmcser->port, ncmcser->port_num);

	port_num = DVE022_serial_ports[ncmcser->port_num].client_port_num;

	switch (ncmcser->transport) {
	case USB_GADGET_XPORT_TTY:
		gserial_connect(&ncmcser->port, port_num);
		break;
	default:
		pr_err("%s: Un-supported transport: %s\n", __func__,
				xport_to_str(ncmcser->transport));
		return -ENODEV;
	}

	return 0;
}

static int DVE022_serial_gport_disconnect(struct f_DVE022_serial *ncmcser)
{
	unsigned port_num;

	pr_debug("%s: transport: %s f_DVE022_serial: %p gserial: %p port_num: %d\n",
			__func__, xport_to_str(ncmcser->transport),
			ncmcser, &ncmcser->port, ncmcser->port_num);

	port_num = DVE022_serial_ports[ncmcser->port_num].client_port_num;

	switch (ncmcser->transport) {
	case USB_GADGET_XPORT_TTY:
		gserial_disconnect(&ncmcser->port);
		break;
	default:
		pr_err("%s: Un-supported transport:%s\n", __func__,
				xport_to_str(ncmcser->transport));
		return -ENODEV;
	}

	return 0;
}

static int DVE022_serial_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_DVE022_serial		*ncmcser = func_to_ncmcser(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	int rc = 0;

	

	if (ncmcser->port.in->driver_data) {
		DBG(cdev, "reset generic data ttyGS%d\n", ncmcser->port_num);
		DVE022_serial_gport_disconnect(ncmcser);
	}
	if (!ncmcser->port.in->desc || !ncmcser->port.out->desc) {
		DBG(cdev, "activate generic ttyGS%d\n", ncmcser->port_num);
		if (config_ep_by_speed(cdev->gadget, f, ncmcser->port.in) ||
			config_ep_by_speed(cdev->gadget, f, ncmcser->port.out)) {
			ncmcser->port.in->desc = NULL;
			ncmcser->port.out->desc = NULL;
			return -EINVAL;
		}
	}

	DVE022_serial_gport_connect(ncmcser);

	ncmcser->online = 1;
	
	DVE022_serial_usb_online = true;
	notify_DVE022_serial_status(USB_DVE021_SERIAL_MODE_ON);
	
	return rc;
}

static void DVE022_serial_disable(struct usb_function *f)
{
	struct f_DVE022_serial	*ncmcser = func_to_ncmcser(f);

	DVE022_serial_gport_disconnect(ncmcser);

	ncmcser->online = 0;
	DVE022_serial_usb_online = false;

	notify_DVE022_serial_status(USB_DVE021_SERIAL_MODE_OFF);
}





static int
DVE022_serial_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_DVE022_serial		*ncmcser = func_to_ncmcser(f);
	int			status;
	struct usb_ep		*ep;

	
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	ncmcser->data_id = status;
	DVE022_serial_interface_desc.bInterfaceNumber = status;

	status = -ENODEV;

	
	ep = usb_ep_autoconfig(cdev->gadget, &DVE022_serial_fs_in_desc);
	if (!ep)
		goto fail;
	ncmcser->port.in = ep;
	ep->driver_data = cdev;	

	ep = usb_ep_autoconfig(cdev->gadget, &DVE022_serial_fs_out_desc);
	if (!ep)
		goto fail;
	ncmcser->port.out = ep;
	ep->driver_data = cdev;	

	
	f->descriptors = usb_copy_descriptors(DVE022_serial_fs_function);

	if (!f->descriptors)
		goto fail;

	



	if (gadget_is_dualspeed(c->cdev->gadget)) {
		DVE022_serial_hs_in_desc.bEndpointAddress =
				DVE022_serial_fs_in_desc.bEndpointAddress;
		DVE022_serial_hs_out_desc.bEndpointAddress =
				DVE022_serial_fs_out_desc.bEndpointAddress;

		
		f->hs_descriptors = usb_copy_descriptors(DVE022_serial_hs_function);

		if (!f->hs_descriptors)
			goto fail;

	}
	if (gadget_is_superspeed(c->cdev->gadget)) {
		DVE022_serial_ss_in_desc.bEndpointAddress =
			DVE022_serial_fs_in_desc.bEndpointAddress;
		DVE022_serial_ss_out_desc.bEndpointAddress =
			DVE022_serial_fs_out_desc.bEndpointAddress;

		
		f->ss_descriptors = usb_copy_descriptors(DVE022_serial_ss_function);
		if (!f->ss_descriptors)
			goto fail;
	}

	DBG(cdev, "DVE022_serial ttyGS%d: %s speed IN/%s OUT/%s\n",
			ncmcser->port_num,
			gadget_is_superspeed(c->cdev->gadget) ? "super" :
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			ncmcser->port.in->name, ncmcser->port.out->name);
	return 0;

fail:
	if (f->descriptors)
		usb_free_descriptors(f->descriptors);

	
	if (ncmcser->port.out)
		ncmcser->port.out->driver_data = NULL;
	if (ncmcser->port.in)
		ncmcser->port.in->driver_data = NULL;

	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}

static void
DVE022_serial_unbind(struct usb_configuration *c, struct usb_function *f)
{
	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	if (gadget_is_superspeed(c->cdev->gadget))
		usb_free_descriptors(f->ss_descriptors);
	usb_free_descriptors(f->descriptors);
	kfree(func_to_ncmcser(f));
}













int DVE022_serial_bind_config(struct usb_configuration *c, u8 port_num)
{
	struct f_DVE022_serial	*ncmcser;
	int		status;

	



	
	if (DVE022_serial_string_defs[0].id == 0) {
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		DVE022_serial_string_defs[0].id = status;
		DVE022_serial_interface_desc.iInterface = status;
	}

	
	ncmcser = kzalloc(sizeof *ncmcser, GFP_KERNEL);
	if (!ncmcser)
		return -ENOMEM;

	ncmcser->port_num = port_num;

	ncmcser->port.func.name = "DVE022_serial";
	ncmcser->port.func.strings = DVE022_serial_strings;
	ncmcser->port.func.bind = DVE022_serial_bind;
	ncmcser->port.func.unbind = DVE022_serial_unbind;
	ncmcser->port.func.set_alt = DVE022_serial_set_alt;
	ncmcser->port.func.disable = DVE022_serial_disable;
	ncmcser->transport		= DVE022_serial_ports[port_num].transport;

	status = usb_add_function(c, &ncmcser->port.func);
	if (status)
		kfree(ncmcser);
	return status;
}




static int DVE022_serial_init_port(int port_num, const char *name)
{
	enum transport_type transport;

	if (port_num >= DVE021_SERIAL_NO_PORTS)
		return -ENODEV;

	transport = str_to_xport(name);
	pr_debug("%s, port:%d, transport:%s\n", __func__,
			port_num, xport_to_str(transport));

	DVE022_serial_ports[port_num].transport = transport;
	DVE022_serial_ports[port_num].port_num = port_num;

	switch (transport) {
	case USB_GADGET_XPORT_TTY:
		DVE022_serial_ports[port_num].client_port_num = no_tty_ports_DVE022_serial;
		no_tty_ports_DVE022_serial++;
		break;
	default:
		pr_err("%s: Un-supported transport transport: %u\n",
				__func__, DVE022_serial_ports[port_num].transport);
		return -ENODEV;
	}

	nr_ports_DVE022_serial++;

	return 0;
}














enum usb_register_result oem_usb_regsiter_DVE022_serial_notify( void* cb )
{
	
	
	if( cb == NULL )
	{
		printk(KERN_INFO "oem_usb_regsiter_DVE022_serial_notify REGISTER_RESULT_NG\n");
		return REGISTER_RESULT_NG;
	}
	
	
	if( DVE022_serial_cb != NULL )
	{
		printk(KERN_INFO "oem_usb_regsiter_DVE022_serial_notify REGISTER_RESULT_ALREADY\n");
		return REGISTER_RESULT_ALREADY;
	}
	
	
	DVE022_serial_cb = cb;
	
	
	if( DVE022_serial_usb_online )
	{
		notify_DVE022_serial_status( USB_DVE021_SERIAL_MODE_ON );
	}
	
	printk(KERN_INFO "oem_usb_regsiter_DVE022_serial_notify REGISTER_RESULT_OK\n");
	return REGISTER_RESULT_OK;

}
EXPORT_SYMBOL( oem_usb_regsiter_DVE022_serial_notify );








static void notify_DVE022_serial_status( enum usb_DVE022_serial_mode_status sta )
{
	if( DVE022_serial_cb == NULL )
	{
		
		return;
	}

	
	if( sta == USB_DVE021_SERIAL_MODE_ON )
	{
		
		if( !DVE022_serial_usb_notify )
		{
			printk(KERN_INFO "notify_DVE022_serial_status USB_DVE021_SERIAL_MODE_ON\n");
			(*DVE022_serial_cb)(sta);
			DVE022_serial_usb_notify = true;
		}
		else
		{
			
		}
	}
	
	else if( sta == USB_DVE021_SERIAL_MODE_OFF )
	{
		
		if( DVE022_serial_usb_notify )
		{
			printk(KERN_INFO "notify_DVE022_serial_status USB_DVE021_SERIAL_MODE_OFF\n");
			(*DVE022_serial_cb)(sta);
			DVE022_serial_usb_notify = false;
		}
		else
		{
			
		}
	}
}
#endif 

