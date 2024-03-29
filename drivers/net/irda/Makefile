#
# Makefile for the Linux IrDA infrared port device drivers.
#
# 9 Aug 2000, Christoph Hellwig <hch@infradead.org>
# Rewritten to use lists instead of if-statements.
#
#/***********************************************************************/
#/* Modified by                                                         */
#/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
#/***********************************************************************/

# FIR drivers
obj-$(CONFIG_USB_IRDA)		+= irda-usb.o
obj-$(CONFIG_SIGMATEL_FIR)	+= stir4200.o
obj-$(CONFIG_NSC_FIR)		+= nsc-ircc.o
obj-$(CONFIG_WINBOND_FIR)	+= w83977af_ir.o
obj-$(CONFIG_SA1100_FIR)	+= sa1100_ir.o
obj-$(CONFIG_TOSHIBA_FIR)	+= donauboe.o
obj-$(CONFIG_SMC_IRCC_FIR)	+= smsc-ircc2.o
obj-$(CONFIG_ALI_FIR)		+= ali-ircc.o
obj-$(CONFIG_VLSI_FIR)		+= vlsi_ir.o
obj-$(CONFIG_VIA_FIR)		+= via-ircc.o
obj-$(CONFIG_PXA_FICP)	        += pxaficp_ir.o
obj-$(CONFIG_MCS_FIR)	        += mcs7780.o
obj-$(CONFIG_AU1000_FIR)	+= au1k_ir.o
obj-$(CONFIG_SH_IRDA)		+= sh_irda.o
# SIR drivers
obj-$(CONFIG_IRTTY_SIR)		+= irtty-sir.o	sir-dev.o
obj-$(CONFIG_BFIN_SIR)		+= bfin_sir.o
obj-$(CONFIG_SH_SIR)		+= sh_sir.o
# dongle drivers for SIR drivers
obj-$(CONFIG_ESI_DONGLE)	+= esi-sir.o
obj-$(CONFIG_TEKRAM_DONGLE)	+= tekram-sir.o
obj-$(CONFIG_ACTISYS_DONGLE)	+= actisys-sir.o
obj-$(CONFIG_LITELINK_DONGLE)	+= litelink-sir.o
obj-$(CONFIG_GIRBIL_DONGLE)	+= girbil-sir.o
obj-$(CONFIG_OLD_BELKIN_DONGLE)	+= old_belkin-sir.o
obj-$(CONFIG_MCP2120_DONGLE)	+= mcp2120-sir.o
obj-$(CONFIG_ACT200L_DONGLE)	+= act200l-sir.o
obj-$(CONFIG_MA600_DONGLE)	+= ma600-sir.o
obj-$(CONFIG_TOIM3232_DONGLE)	+= toim3232-sir.o
obj-$(CONFIG_EP7211_DONGLE)	+= ep7211-sir.o
obj-$(CONFIG_KINGSUN_DONGLE)	+= kingsun-sir.o
obj-$(CONFIG_KSDAZZLE_DONGLE)	+= ksdazzle-sir.o
obj-$(CONFIG_KS959_DONGLE)    	+= ks959-sir.o
obj-$(CONFIG_MSM_SIR)           +=msm-sir.o

# The SIR helper module
sir-dev-objs := sir_dev.o sir_dongle.o
