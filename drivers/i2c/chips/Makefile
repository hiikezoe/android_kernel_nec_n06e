#
# Makefile for miscellaneous I2C chip drivers.
#
# Do not add new drivers to this directory! It is DEPRECATED.
#
# Device drivers are better grouped according to the functionality they
# implement rather than to the bus they are connected to. In particular:
# * Hardware monitoring chip drivers go to drivers/hwmon
# * RTC chip drivers go to drivers/rtc
# * I/O expander drivers go to drivers/gpio
#
#/***********************************************************************/
#/* Modified by                                                         */
#/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
#/***********************************************************************/

ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif



obj-$(CONFIG_SENSORS_APDS990X)  += apds9900.o
obj-y += sensors_common.o

obj-$(CONFIG_SENSORS_AKM8977)   += akm8977.o
obj-$(CONFIG_SENSORS_AKM8977)   += akm8977_table.o
obj-$(CONFIG_SENSORS_AKM8977)   += akm8977_queue.o




EXTRA_CFLAGS += -mfloat-abi=softfp



obj-$(CONFIG_BD91401GW)     += bd91401gw.o


obj-$(CONFIG_BD91401GW)     += usb_switch_if_knl.o


ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif
