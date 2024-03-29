/*********************************************************************
 *
 *	sir_dev.c:	irda sir network device
 * 
 *	Copyright (c) 2002 Martin Diehl
 * 
 *	This program is free software; you can redistribute it and/or 
 *	modify it under the terms of the GNU General Public License as 
 *	published by the Free Software Foundation; either version 2 of 
 *	the License, or (at your option) any later version.
 *
 ********************************************************************/    
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/hardirq.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <net/irda/irda.h>
#include <net/irda/wrapper.h>
#include <net/irda/irda_device.h>

#include "sir-dev.h"




extern void msmsir_enable_rx(struct sir_dev *dev);
extern void msmsir_disable_rx(struct sir_dev *dev);
extern int msmsir_rx_receiving(struct sir_dev *dev);
extern int irda_discovery_state;




static int	localechodata;
static	int	rx_count_log;	
static unsigned char localechobuf[256];




static struct workqueue_struct *irda_sir_wq;

/* STATE MACHINE */

/* substate handler of the config-fsm to handle the cases where we want
 * to wait for transmit completion before changing the port configuration
 */

static int sirdev_tx_complete_fsm(struct sir_dev *dev)
{
	struct sir_fsm *fsm = &dev->fsm;
	unsigned next_state, delay;
	unsigned bytes_left;

	do {
		next_state = fsm->substate;	/* default: stay in current substate */
		delay = 0;

		switch(fsm->substate) {

		case SIRDEV_STATE_WAIT_XMIT:
			if (dev->drv->chars_in_buffer)
				bytes_left = dev->drv->chars_in_buffer(dev);
			else
				bytes_left = 0;
			if (!bytes_left) {
				next_state = SIRDEV_STATE_WAIT_UNTIL_SENT;
				break;
			}

			if (dev->speed > 115200)
				delay = (bytes_left*8*10000) / (dev->speed/100);
			else if (dev->speed > 0)
				delay = (bytes_left*10*10000) / (dev->speed/100);
			else
				delay = 0;
			/* expected delay (usec) until remaining bytes are sent */
			if (delay < 100) {
				udelay(delay);
				delay = 0;
				break;
			}
			/* sleep some longer delay (msec) */
			delay = (delay+999) / 1000;
			break;

		case SIRDEV_STATE_WAIT_UNTIL_SENT:
			/* block until underlaying hardware buffer are empty */
			if (dev->drv->wait_until_sent)
				dev->drv->wait_until_sent(dev);
			next_state = SIRDEV_STATE_TX_DONE;
			break;

		case SIRDEV_STATE_TX_DONE:
			return 0;

		default:
			IRDA_ERROR("%s - undefined state\n", __func__);
			return -EINVAL;
		}
		fsm->substate = next_state;
	} while (delay == 0);
	return delay;
}

/*
 * Function sirdev_config_fsm
 *
 * State machine to handle the configuration of the device (and attached dongle, if any).
 * This handler is scheduled for execution in kIrDAd context, so we can sleep.
 * however, kIrDAd is shared by all sir_dev devices so we better don't sleep there too
 * long. Instead, for longer delays we start a timer to reschedule us later.
 * On entry, fsm->sem is always locked and the netdev xmit queue stopped.
 * Both must be unlocked/restarted on completion - but only on final exit.
 */

static void sirdev_config_fsm(struct work_struct *work)
{
	struct sir_dev *dev = container_of(work, struct sir_dev, fsm.work.work);
	struct sir_fsm *fsm = &dev->fsm;
	int next_state;
	int ret = -1;
	unsigned delay;

	IRDA_DEBUG(2, "%s(), <%ld>\n", __func__, jiffies);

	do {
		IRDA_DEBUG(3, "%s - state=0x%04x / substate=0x%04x\n",
			__func__, fsm->state, fsm->substate);

		next_state = fsm->state;
		delay = 0;

		switch(fsm->state) {

		case SIRDEV_STATE_DONGLE_OPEN:
			if (dev->dongle_drv != NULL) {
				ret = sirdev_put_dongle(dev);
				if (ret) {
					fsm->result = -EINVAL;
					next_state = SIRDEV_STATE_ERROR;
					break;
				}
			}

			/* Initialize dongle */
			ret = sirdev_get_dongle(dev, fsm->param);
			if (ret) {
				fsm->result = ret;
				next_state = SIRDEV_STATE_ERROR;
				break;
			}

			/* Dongles are powered through the modem control lines which
			 * were just set during open. Before resetting, let's wait for
			 * the power to stabilize. This is what some dongle drivers did
			 * in open before, while others didn't - should be safe anyway.
			 */

			delay = 50;
			fsm->substate = SIRDEV_STATE_DONGLE_RESET;
			next_state = SIRDEV_STATE_DONGLE_RESET;

			fsm->param = 9600;

			break;

		case SIRDEV_STATE_DONGLE_CLOSE:
			/* shouldn't we just treat this as success=? */
			if (dev->dongle_drv == NULL) {
				fsm->result = -EINVAL;
				next_state = SIRDEV_STATE_ERROR;
				break;
			}

			ret = sirdev_put_dongle(dev);
			if (ret) {
				fsm->result = ret;
				next_state = SIRDEV_STATE_ERROR;
				break;
			}
			next_state = SIRDEV_STATE_DONE;
			break;

		case SIRDEV_STATE_SET_DTR_RTS:
			ret = sirdev_set_dtr_rts(dev,
				(fsm->param&0x02) ? TRUE : FALSE,
				(fsm->param&0x01) ? TRUE : FALSE);
			next_state = SIRDEV_STATE_DONE;
			break;

		case SIRDEV_STATE_SET_SPEED:
			fsm->substate = SIRDEV_STATE_WAIT_XMIT;
			next_state = SIRDEV_STATE_DONGLE_CHECK;
			break;

		case SIRDEV_STATE_DONGLE_CHECK:
			ret = sirdev_tx_complete_fsm(dev);
			if (ret < 0) {
				fsm->result = ret;
				next_state = SIRDEV_STATE_ERROR;
				break;
			}
			if ((delay=ret) != 0)
				break;

			if (dev->dongle_drv) {
				fsm->substate = SIRDEV_STATE_DONGLE_RESET;
				next_state = SIRDEV_STATE_DONGLE_RESET;
			}
			else {
				dev->speed = fsm->param;
				next_state = SIRDEV_STATE_PORT_SPEED;
			}
			break;

		case SIRDEV_STATE_DONGLE_RESET:
			if (dev->dongle_drv->reset) {
				ret = dev->dongle_drv->reset(dev);
				if (ret < 0) {
					fsm->result = ret;
					next_state = SIRDEV_STATE_ERROR;
					break;
				}
			}
			else
				ret = 0;
			if ((delay=ret) == 0) {
				/* set serial port according to dongle default speed */
				if (dev->drv->set_speed)
					dev->drv->set_speed(dev, dev->speed);
				fsm->substate = SIRDEV_STATE_DONGLE_SPEED;
				next_state = SIRDEV_STATE_DONGLE_SPEED;
			}
			break;

		case SIRDEV_STATE_DONGLE_SPEED:
			if (dev->dongle_drv->reset) {
				ret = dev->dongle_drv->set_speed(dev, fsm->param);
				if (ret < 0) {
					fsm->result = ret;
					next_state = SIRDEV_STATE_ERROR;
					break;
				}
			}
			else
				ret = 0;
			if ((delay=ret) == 0)
				next_state = SIRDEV_STATE_PORT_SPEED;
			break;

		case SIRDEV_STATE_PORT_SPEED:
			/* Finally we are ready to change the serial port speed */
			if (dev->drv->set_speed)
				dev->drv->set_speed(dev, dev->speed);
			dev->new_speed = 0;
			next_state = SIRDEV_STATE_DONE;
			break;

		case SIRDEV_STATE_DONE:
			/* Signal network layer so it can send more frames */
			netif_wake_queue(dev->netdev);
			next_state = SIRDEV_STATE_COMPLETE;
			break;

		default:
			IRDA_ERROR("%s - undefined state\n", __func__);
			fsm->result = -EINVAL;
			/* fall thru */

		case SIRDEV_STATE_ERROR:
			IRDA_ERROR("%s - error: %d\n", __func__, fsm->result);




			netif_wake_queue(dev->netdev);

			/* fall thru */

		case SIRDEV_STATE_COMPLETE:
			/* config change finished, so we are not busy any longer */
			sirdev_enable_rx(dev);
			up(&fsm->sem);
			return;
		}
		fsm->state = next_state;
	} while(!delay);

	queue_delayed_work(irda_sir_wq, &fsm->work, msecs_to_jiffies(delay));
}

/* schedule some device configuration task for execution by kIrDAd
 * on behalf of the above state machine.
 * can be called from process or interrupt/tasklet context.
 */

int sirdev_schedule_request(struct sir_dev *dev, int initial_state, unsigned param)
{
	struct sir_fsm *fsm = &dev->fsm;

	IRDA_DEBUG(2, "%s - state=0x%04x / param=%u\n", __func__,
			initial_state, param);

	if (down_trylock(&fsm->sem)) {
		if (in_interrupt()  ||  in_atomic()  ||  irqs_disabled()) {
			IRDA_DEBUG(1, "%s(), state machine busy!\n", __func__);
			return -EWOULDBLOCK;
		} else
			down(&fsm->sem);
	}

	if (fsm->state == SIRDEV_STATE_DEAD) {
		/* race with sirdev_close should never happen */
		IRDA_ERROR("%s(), instance staled!\n", __func__);
		up(&fsm->sem);
		return -ESTALE;		/* or better EPIPE? */
	}

	netif_stop_queue(dev->netdev);
	atomic_set(&dev->enable_rx, 0);

	fsm->state = initial_state;
	fsm->param = param;
	fsm->result = 0;

	INIT_DELAYED_WORK(&fsm->work, sirdev_config_fsm);
	queue_delayed_work(irda_sir_wq, &fsm->work, 0);
	return 0;
}


/***************************************************************************/

void sirdev_enable_rx(struct sir_dev *dev)
{
	if (unlikely(atomic_read(&dev->enable_rx)))
		return;

	/* flush rx-buffer - should also help in case of problems with echo cancelation */
	dev->rx_buff.data = dev->rx_buff.head;
	dev->rx_buff.len = 0;
	dev->rx_buff.in_frame = FALSE;
	dev->rx_buff.state = OUTSIDE_FRAME;
	atomic_set(&dev->enable_rx, 1);




	msmsir_enable_rx(dev);




}

static int sirdev_is_receiving(struct sir_dev *dev)
{



	int	ret;
	int rx_count;

	if (!atomic_read(&dev->enable_rx)){
		ret = 0;
		IRDA_DEBUG(4, "%s : rx is disabled, ret = %d.\n", __func__, ret);
	}
	else {
		rx_count = msmsir_rx_receiving(dev);

		
		if ( (rx_count > 15) && (rx_count_log != rx_count)){

		
			ret =  TRUE;
			IRDA_DEBUG(4, "%s : data in buffer, ret = %d, rx_count = %d.\n", __func__, ret, rx_count);
		}
		else {
			ret = dev->rx_buff.state != OUTSIDE_FRAME;
			IRDA_DEBUG(4, "%s : no data in buffer, ret = %d.\n", __func__, ret);
		}
	}
	return ret;








}

int sirdev_set_dongle(struct sir_dev *dev, IRDA_DONGLE type)
{
	int err;

	IRDA_DEBUG(3, "%s : requesting dongle %d.\n", __func__, type);

	err = sirdev_schedule_dongle_open(dev, type);
	if (unlikely(err))
		return err;
	down(&dev->fsm.sem);		/* block until config change completed */
	err = dev->fsm.result;
	up(&dev->fsm.sem);
	return err;
}
EXPORT_SYMBOL(sirdev_set_dongle);

/* used by dongle drivers for dongle programming */

int sirdev_raw_write(struct sir_dev *dev, const char *buf, int len)
{
	unsigned long flags;
	int ret;

	if (unlikely(len > dev->tx_buff.truesize))
		return -ENOSPC;

	spin_lock_irqsave(&dev->tx_lock, flags);	/* serialize with other tx operations */
	while (dev->tx_buff.len > 0) {			/* wait until tx idle */
		spin_unlock_irqrestore(&dev->tx_lock, flags);
		msleep(10);
		spin_lock_irqsave(&dev->tx_lock, flags);
	}

	dev->tx_buff.data = dev->tx_buff.head;
	memcpy(dev->tx_buff.data, buf, len);	
	dev->tx_buff.len = len;

	ret = dev->drv->do_write(dev, dev->tx_buff.data, dev->tx_buff.len);
	if (ret > 0) {
		IRDA_DEBUG(3, "%s(), raw-tx started\n", __func__);

		dev->tx_buff.data += ret;
		dev->tx_buff.len -= ret;
		dev->raw_tx = 1;
		ret = len;		/* all data is going to be sent */
	}
	spin_unlock_irqrestore(&dev->tx_lock, flags);
	return ret;
}
EXPORT_SYMBOL(sirdev_raw_write);

/* seems some dongle drivers may need this */

int sirdev_raw_read(struct sir_dev *dev, char *buf, int len)
{
	int count;

	if (atomic_read(&dev->enable_rx))
		return -EIO;		/* fail if we expect irda-frames */

	count = (len < dev->rx_buff.len) ? len : dev->rx_buff.len;

	if (count > 0) {
		memcpy(buf, dev->rx_buff.data, count);
		dev->rx_buff.data += count;
		dev->rx_buff.len -= count;
	}

	/* remaining stuff gets flushed when re-enabling normal rx */

	return count;
}
EXPORT_SYMBOL(sirdev_raw_read);

int sirdev_set_dtr_rts(struct sir_dev *dev, int dtr, int rts)
{
	int ret = -ENXIO;
	if (dev->drv->set_dtr_rts)
		ret =  dev->drv->set_dtr_rts(dev, dtr, rts);
	return ret;
}
EXPORT_SYMBOL(sirdev_set_dtr_rts);

/**********************************************************************/

/* called from client driver - likely with bh-context - to indicate
 * it made some progress with transmission. Hence we send the next
 * chunk, if any, or complete the skb otherwise
 */

void sirdev_write_complete(struct sir_dev *dev)
{
	unsigned long flags;
	struct sk_buff *skb;
	int actual = 0;
	int err;
	
	spin_lock_irqsave(&dev->tx_lock, flags);

	IRDA_DEBUG(3, "%s() - dev->tx_buff.len = %d\n",
		   __func__, dev->tx_buff.len);

	if (likely(dev->tx_buff.len > 0))  {
		/* Write data left in transmit buffer */
		actual = dev->drv->do_write(dev, dev->tx_buff.data, dev->tx_buff.len);

		if (likely(actual>0)) {
			dev->tx_buff.data += actual;
			dev->tx_buff.len  -= actual;
		}
		else if (unlikely(actual<0)) {
			/* could be dropped later when we have tx_timeout to recover */
			IRDA_ERROR("%s: drv->do_write failed (%d)\n",
				   __func__, actual);
			if ((skb=dev->tx_skb) != NULL) {
				dev->tx_skb = NULL;
				dev_kfree_skb_any(skb);
				dev->netdev->stats.tx_errors++;
				dev->netdev->stats.tx_dropped++;
			}
			dev->tx_buff.len = 0;
		}
		if (dev->tx_buff.len > 0)
			goto done;	/* more data to send later */
	}

	if (unlikely(dev->raw_tx != 0)) {
		/* in raw mode we are just done now after the buffer was sent
		 * completely. Since this was requested by some dongle driver
		 * running under the control of the irda-thread we must take
		 * care here not to re-enable the queue. The queue will be
		 * restarted when the irda-thread has completed the request.
		 */

		IRDA_DEBUG(3, "%s(), raw-tx done\n", __func__);
		dev->raw_tx = 0;
		goto done;	/* no post-frame handling in raw mode */
	}

	/* we have finished now sending this skb.
	 * update statistics and free the skb.
	 * finally we check and trigger a pending speed change, if any.
	 * if not we switch to rx mode and wake the queue for further
	 * packets.
	 * note the scheduled speed request blocks until the lower
	 * client driver and the corresponding hardware has really
	 * finished sending all data (xmit fifo drained f.e.)
	 * before the speed change gets finally done and the queue
	 * re-activated.
	 */

	IRDA_DEBUG(5, "%s(), finished with frame!\n", __func__);
		
	if ((skb=dev->tx_skb) != NULL) {
		dev->tx_skb = NULL;
		dev->netdev->stats.tx_packets++;
		dev->netdev->stats.tx_bytes += skb->len;
		dev_kfree_skb_any(skb);
	}

	if (unlikely(dev->new_speed > 0)) {
		IRDA_DEBUG(5, "%s(), Changing speed!\n", __func__);
		err = sirdev_schedule_speed(dev, dev->new_speed);
		if (unlikely(err)) {
			/* should never happen
			 * forget the speed change and hope the stack recovers
			 */
			IRDA_ERROR("%s - schedule speed change failed: %d\n",
				   __func__, err);
			netif_wake_queue(dev->netdev);
		}
		/* else: success
		 *	speed change in progress now
		 *	on completion dev->new_speed gets cleared,
		 *	rx-reenabled and the queue restarted
		 */
	}
	else {
		sirdev_enable_rx(dev);
		netif_wake_queue(dev->netdev);
	}

done:
	spin_unlock_irqrestore(&dev->tx_lock, flags);
}
EXPORT_SYMBOL(sirdev_write_complete);

/* called from client driver - likely with bh-context - to give us
 * some more received bytes. We put them into the rx-buffer,
 * normally unwrapping and building LAP-skb's (unless rx disabled)
 */

int sirdev_receive(struct sir_dev *dev, const unsigned char *cp, size_t count) 
{


	int i;
	unsigned char c[32];

	unsigned char *eofp, *ebufp;
	int	framelen;




	if (!dev || !dev->netdev) {
		IRDA_WARNING("%s(), not ready yet!\n", __func__);
		return -1;
	}

	if (!dev->irlap) {
		IRDA_WARNING("%s - too early: %p / %zd!\n",
			     __func__, cp, count);
		return -1;
	}

	if (cp==NULL) {
		/* error already at lower level receive
		 * just update stats and set media busy
		 */
		irda_device_set_media_busy(dev->netdev, TRUE);
		dev->netdev->stats.rx_dropped++;
		IRDA_DEBUG(0, "%s; rx-drop: %zd\n", __func__, count);
		return 0;
	}



	IRDA_DEBUG(3, "%s; get rx: %zd\n", __func__, count);



	/* Read the characters into the buffer */
	if (likely(atomic_read(&dev->enable_rx))) {

		
		rx_count_log = count;
		
		
		if (count >0){
			for(i= 0; i<32; i ++){
				if (i<count){
					c[i] = cp[i];
				}
				else {
					c[i] = 0x00;
				}
			}
			IRDA_DEBUG(3, "RcvData= %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", 
					 c[0], c[1], c[2], c[3], c[4], c[5],c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],c[14],c[15],
					 c[16], c[17], c[18], c[19], c[20], c[21],c[22], c[23], c[24], c[25], c[26], c[27], c[28], c[29],c[30],c[31]
					 );
		}
		if (count >= 32){
			for(i= 0; i<32; i ++){
				if ( i < count-32){
					c[i] = cp[i+32];
				}
				else {
					c[i] = 0x00;
				}
			}
			IRDA_DEBUG(3, "RcvData= %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", 
					 c[0], c[1], c[2], c[3], c[4], c[5],c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],c[14],c[15],
					 c[16], c[17], c[18], c[19], c[20], c[21],c[22], c[23], c[24], c[25], c[26], c[27], c[28], c[29],c[30],c[31]
					 );
		}
		if (count >= 64){
			for(i= 0; i< 32; i ++){
				if ( i < count-64){
					c[i] = cp[i+64];
				}
				else {
					c[i] = 0x00;
				}
			}
			IRDA_DEBUG(3, "RcvData= %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", 
					 c[0], c[1], c[2], c[3], c[4], c[5],c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],c[14],c[15],
					 c[16], c[17], c[18], c[19], c[20], c[21],c[22], c[23], c[24], c[25], c[26], c[27], c[28], c[29],c[30],c[31]
					 );
		}
		if (count >=96){
			for(i= 0; i<32; i ++){
				if ( i < count-96){
					c[i] = cp[i+96];
				}
				else {
					c[i] = 0x00;
				}
			}
			IRDA_DEBUG(3, "RcvData= %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", 
					 c[0], c[1], c[2], c[3], c[4], c[5],c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],c[14],c[15],
					 c[16], c[17], c[18], c[19], c[20], c[21],c[22], c[23], c[24], c[25], c[26], c[27], c[28], c[29],c[30],c[31]
					 );
		}
		



		ebufp = localechobuf;

		if ( ( count < 20) && localechodata > 0 && dev->rx_buff.state == OUTSIDE_FRAME){
			if ( count > localechodata){
				if (!memcmp(localechobuf, cp + count - localechodata, localechodata)){
					IRDA_DEBUG(4, "%s; data mutch trush %d byte!\n", __func__, count);
					localechodata = 0;
					cp += count;
					count = 0;
				}
			}
			else {
				if (!memcmp(localechobuf + localechodata - count, cp , count)){
					IRDA_DEBUG(4, "%s; data mutch trush %d byte!\n", __func__, count);
					localechodata = 0;
					cp += count;
					count = 0;
				}
			}
		}
		while (count--){
			
			if (localechodata > 0 && dev->rx_buff.state == OUTSIDE_FRAME){
				switch(*cp) {
				case BOF:

					if ( count > 1 && *(cp + 1) == BOF){
						ebufp ++;
						localechodata--;
						cp++;
						continue;
						break;
					}
					else {
						eofp = (unsigned char *)memchr( (void *)cp, (int)EOF, count);
						if ( eofp  == NULL){
							IRDA_DEBUG(4, "%s; not find eof, get start of next frame.\n", __func__);
							localechodata = 0;
							break;
						}
						else {
							framelen = eofp - cp +1 ;
							if ( framelen <= localechodata &&
								   !memcmp((void *)cp, ebufp + localechodata - framelen, framelen)){
								IRDA_DEBUG(4, "%s; find one frame, skip it\n", __func__);
								cp += framelen;
								count -= framelen;
								localechodata = 0;
								break;
							}
							else {
								IRDA_DEBUG(4, "%s; frame line =%d, localechodata = %d.\n", __func__, framelen, localechodata);
								IRDA_DEBUG(4, "%s; find start of next frame.\n", __func__);
								localechodata = 0;
								break;
							}
						}
					}





				case EOF:
					IRDA_DEBUG(4, "%s; find end of trush data!\n", __func__);
					ebufp ++;		
					localechodata = 0;
					cp++;
					continue;
					break;
				default:
					ebufp ++;		
					localechodata--;
					cp++;
					continue;
					break;
				}
			}
			async_unwrap_char(dev->netdev, &dev->netdev->stats,
					  &dev->rx_buff, *cp++);
		}







	} else {
		while (count--) {
			/* rx not enabled: save the raw bytes and never
			 * trigger any netif_rx. The received bytes are flushed
			 * later when we re-enable rx but might be read meanwhile
			 * by the dongle driver.
			 */
			dev->rx_buff.data[dev->rx_buff.len++] = *cp++;

			/* What should we do when the buffer is full? */
			if (unlikely(dev->rx_buff.len == dev->rx_buff.truesize))
				dev->rx_buff.len = 0;
		}
	}

	return 0;
}
EXPORT_SYMBOL(sirdev_receive);

/**********************************************************************/

/* callbacks from network layer */

static netdev_tx_t sirdev_hard_xmit(struct sk_buff *skb,
					  struct net_device *ndev)
{
	struct sir_dev *dev = netdev_priv(ndev);
	unsigned long flags;
	int actual = 0;
	int err;
	s32 speed;

	IRDA_ASSERT(dev != NULL, return NETDEV_TX_OK;);

	netif_stop_queue(ndev);


    if (irda_discovery_state == 1) {
		IRDA_ERROR("%s(), STATE ERROR irda_discovery_state => 1\n", __func__);
		return NETDEV_TX_OK;
	}


	IRDA_DEBUG(3, "%s(), skb->len = %d\n", __func__, skb->len);

	speed = irda_get_next_speed(skb);
	if ((speed != dev->speed) && (speed != -1)) {
		if (!skb->len) {
			err = sirdev_schedule_speed(dev, speed);
			if (unlikely(err == -EWOULDBLOCK)) {
				/* Failed to initiate the speed change, likely the fsm
				 * is still busy (pretty unlikely, but...)
				 * We refuse to accept the skb and return with the queue
				 * stopped so the network layer will retry after the
				 * fsm completes and wakes the queue.
				 */
				 return NETDEV_TX_BUSY;
			}
			else if (unlikely(err)) {
				/* other fatal error - forget the speed change and
				 * hope the stack will recover somehow
				 */
				 netif_start_queue(ndev);
			}
			/* else: success
			 *	speed change in progress now
			 *	on completion the queue gets restarted
			 */

			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		} else
			dev->new_speed = speed;
	}

	/* Init tx buffer*/
	dev->tx_buff.data = dev->tx_buff.head;

	/* Check problems */
	if(spin_is_locked(&dev->tx_lock)) {
		IRDA_DEBUG(3, "%s(), write not completed\n", __func__);
	}

	/* serialize with write completion */
	spin_lock_irqsave(&dev->tx_lock, flags);

        /* Copy skb to tx_buff while wrapping, stuffing and making CRC */
	dev->tx_buff.len = async_wrap_skb(skb, dev->tx_buff.data, dev->tx_buff.truesize); 

	/* transmission will start now - disable receive.
	 * if we are just in the middle of an incoming frame,
	 * treat it as collision. probably it's a good idea to
	 * reset the rx_buf OUTSIDE_FRAME in this case too?
	 */
	atomic_set(&dev->enable_rx, 0);

		
		rx_count_log = 0;
		

	if (unlikely(sirdev_is_receiving(dev)))
		dev->netdev->stats.collisions++;



	
	{
		int i;
		unsigned char c[32];
		
		if (dev->tx_buff.len >0){
			for(i= 0; i<32; i ++){
				if (i<dev->tx_buff.len){
					c[i] = dev->tx_buff.data[i];
				}
				else {
					c[i] = 0x00;
				}
			}
			IRDA_DEBUG(3, "SendData= %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", 
					 c[0], c[1], c[2], c[3], c[4], c[5],c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],c[14],c[15],
					 c[16], c[17], c[18], c[19], c[20], c[21],c[22], c[23], c[24], c[25], c[26], c[27], c[28], c[29],c[30],c[31]
					 );
		}
		if (dev->tx_buff.len >=32){
			for(i= 0; i<32; i ++){
				if ( i < dev->tx_buff.len -32){
					c[i] = dev->tx_buff.data[i+32];
				}
				else {
					c[i] = 0x00;
				}
			}
			IRDA_DEBUG(3, "SendData= %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", 
					 c[0], c[1], c[2], c[3], c[4], c[5],c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],c[14],c[15],
					 c[16], c[17], c[18], c[19], c[20], c[21],c[22], c[23], c[24], c[25], c[26], c[27], c[28], c[29],c[30],c[31]
					 );
		}
		if (dev->tx_buff.len >=64){
			for(i= 0; i<32; i ++){
				if ( i < dev->tx_buff.len -64){
					c[i] = dev->tx_buff.data[i+64];
				}
				else {
					c[i] = 0x00;
				}
			}
			IRDA_DEBUG(3, "SendData= %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", 
					 c[0], c[1], c[2], c[3], c[4], c[5],c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],c[14],c[15],
					 c[16], c[17], c[18], c[19], c[20], c[21],c[22], c[23], c[24], c[25], c[26], c[27], c[28], c[29],c[30],c[31]
					 );
		}
		if (dev->tx_buff.len >=96){
			for(i= 0; i<32; i ++){
				if ( i < dev->tx_buff.len -96){
					c[i] = dev->tx_buff.data[i+96];
				}
				else {
					c[i] = 0x00;
				}
			}
			IRDA_DEBUG(3, "SendData= %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", 
					 c[0], c[1], c[2], c[3], c[4], c[5],c[6], c[7], c[8], c[9], c[10], c[11], c[12], c[13],c[14],c[15],
					 c[16], c[17], c[18], c[19], c[20], c[21],c[22], c[23], c[24], c[25], c[26], c[27], c[28], c[29],c[30],c[31]
					 );
		}
	}
	


	actual = dev->drv->do_write(dev, dev->tx_buff.data, dev->tx_buff.len);

	if (likely(actual > 0)) {



		if ( actual <= 20)
			localechodata = actual;
		else
			localechodata = 20;
		memcpy(localechobuf, dev->tx_buff.data + actual - localechodata, localechodata);



		dev->tx_skb = skb;
		dev->tx_buff.data += actual;
		dev->tx_buff.len -= actual;
	}
	else if (unlikely(actual < 0)) {
		/* could be dropped later when we have tx_timeout to recover */
		IRDA_ERROR("%s: drv->do_write failed (%d)\n",
			   __func__, actual);
		dev_kfree_skb_any(skb);
		dev->netdev->stats.tx_errors++;
		dev->netdev->stats.tx_dropped++;
		netif_wake_queue(ndev);
	}
	spin_unlock_irqrestore(&dev->tx_lock, flags);

	return NETDEV_TX_OK;
}

/* called from network layer with rtnl hold */

static int sirdev_ioctl(struct net_device *ndev, struct ifreq *rq, int cmd)
{
	struct if_irda_req *irq = (struct if_irda_req *) rq;
	struct sir_dev *dev = netdev_priv(ndev);
	int ret = 0;

	IRDA_ASSERT(dev != NULL, return -1;);

	IRDA_DEBUG(3, "%s(), %s, (cmd=0x%X)\n", __func__, ndev->name, cmd);
	
	switch (cmd) {
	case SIOCSBANDWIDTH: /* Set bandwidth */
		if (!capable(CAP_NET_ADMIN))
			ret = -EPERM;
		else
			ret = sirdev_schedule_speed(dev, irq->ifr_baudrate);
		/* cannot sleep here for completion
		 * we are called from network layer with rtnl hold
		 */
		break;

	case SIOCSDONGLE: /* Set dongle */
		if (!capable(CAP_NET_ADMIN))
			ret = -EPERM;
		else
			ret = sirdev_schedule_dongle_open(dev, irq->ifr_dongle);
		/* cannot sleep here for completion
		 * we are called from network layer with rtnl hold
		 */
		break;

	case SIOCSMEDIABUSY: /* Set media busy */
		if (!capable(CAP_NET_ADMIN))
			ret = -EPERM;
		else
			irda_device_set_media_busy(dev->netdev, TRUE);
		break;

	case SIOCGRECEIVING: /* Check if we are receiving right now */
		irq->ifr_receiving = sirdev_is_receiving(dev);
		break;

	case SIOCSDTRRTS:
		if (!capable(CAP_NET_ADMIN))
			ret = -EPERM;
		else
			ret = sirdev_schedule_dtr_rts(dev, irq->ifr_dtr, irq->ifr_rts);
		/* cannot sleep here for completion
		 * we are called from network layer with rtnl hold
		 */
		break;

	case SIOCSMODE:










	default:
		ret = -EOPNOTSUPP;
	}
	
	return ret;
}

/* ----------------------------------------------------------------------------- */

#define SIRBUF_ALLOCSIZE 4269	/* worst case size of a wrapped IrLAP frame */

static int sirdev_alloc_buffers(struct sir_dev *dev)
{
	dev->tx_buff.truesize = SIRBUF_ALLOCSIZE;
	dev->rx_buff.truesize = IRDA_SKB_MAX_MTU; 

	/* Bootstrap ZeroCopy Rx */
	dev->rx_buff.skb = __netdev_alloc_skb(dev->netdev, dev->rx_buff.truesize,
					      GFP_KERNEL);
	if (dev->rx_buff.skb == NULL)
		return -ENOMEM;
	skb_reserve(dev->rx_buff.skb, 1);
	dev->rx_buff.head = dev->rx_buff.skb->data;

	dev->tx_buff.head = kmalloc(dev->tx_buff.truesize, GFP_KERNEL);
	if (dev->tx_buff.head == NULL) {
		kfree_skb(dev->rx_buff.skb);
		dev->rx_buff.skb = NULL;
		dev->rx_buff.head = NULL;
		return -ENOMEM;
	}

	dev->tx_buff.data = dev->tx_buff.head;
	dev->rx_buff.data = dev->rx_buff.head;
	dev->tx_buff.len = 0;
	dev->rx_buff.len = 0;

	dev->rx_buff.in_frame = FALSE;
	dev->rx_buff.state = OUTSIDE_FRAME;
	return 0;
};

static void sirdev_free_buffers(struct sir_dev *dev)
{
	kfree_skb(dev->rx_buff.skb);
	kfree(dev->tx_buff.head);
	dev->rx_buff.head = dev->tx_buff.head = NULL;
	dev->rx_buff.skb = NULL;
}

static int sirdev_open(struct net_device *ndev)
{
	struct sir_dev *dev = netdev_priv(ndev);
	const struct sir_driver *drv = dev->drv;

	if (!drv)
		return -ENODEV;

	/* increase the reference count of the driver module before doing serious stuff */
	if (!try_module_get(drv->owner))
		return -ESTALE;

	IRDA_DEBUG(2, "%s()\n", __func__);

	if (sirdev_alloc_buffers(dev))
		goto errout_dec;

	if (!dev->drv->start_dev  ||  dev->drv->start_dev(dev))
		goto errout_free;

	sirdev_enable_rx(dev);
	dev->raw_tx = 0;

	netif_start_queue(ndev);
	dev->irlap = irlap_open(ndev, &dev->qos, dev->hwname);
	if (!dev->irlap)
		goto errout_stop;

	netif_wake_queue(ndev);

	IRDA_DEBUG(2, "%s - done, speed = %d\n", __func__, dev->speed);

	return 0;

errout_stop:
	atomic_set(&dev->enable_rx, 0);
	if (dev->drv->stop_dev)
		dev->drv->stop_dev(dev);
errout_free:
	sirdev_free_buffers(dev);
errout_dec:
	module_put(drv->owner);
	return -EAGAIN;
}

static int sirdev_close(struct net_device *ndev)
{
	struct sir_dev *dev = netdev_priv(ndev);
	const struct sir_driver *drv;

//	IRDA_DEBUG(0, "%s\n", __func__);

	netif_stop_queue(ndev);

	down(&dev->fsm.sem);		/* block on pending config completion */

	atomic_set(&dev->enable_rx, 0);

	if (unlikely(!dev->irlap))
		goto out;
	irlap_close(dev->irlap);
	dev->irlap = NULL;

	drv = dev->drv;
	if (unlikely(!drv  ||  !dev->priv))
		goto out;

	if (drv->stop_dev)
		drv->stop_dev(dev);

	sirdev_free_buffers(dev);
	module_put(drv->owner);

out:
	dev->speed = 0;
	up(&dev->fsm.sem);
	return 0;
}

static const struct net_device_ops sirdev_ops = {
	.ndo_start_xmit	= sirdev_hard_xmit,
	.ndo_open	= sirdev_open,
	.ndo_stop	= sirdev_close,
	.ndo_do_ioctl	= sirdev_ioctl,
};
/* ----------------------------------------------------------------------------- */

struct sir_dev * sirdev_get_instance(const struct sir_driver *drv, const char *name)
{
	struct net_device *ndev;
	struct sir_dev *dev;

	IRDA_DEBUG(0, "%s - %s\n", __func__, name);

	/* instead of adding tests to protect against drv->do_write==NULL
	 * at several places we refuse to create a sir_dev instance for
	 * drivers which don't implement do_write.
	 */
	if (!drv ||  !drv->do_write)
		return NULL;

	/*
	 *  Allocate new instance of the device
	 */
	ndev = alloc_irdadev(sizeof(*dev));
	if (ndev == NULL) {
		IRDA_ERROR("%s - Can't allocate memory for IrDA control block!\n", __func__);
		goto out;
	}
	dev = netdev_priv(ndev);

	irda_init_max_qos_capabilies(&dev->qos);
	dev->qos.baud_rate.bits = IR_9600|IR_19200|IR_38400|IR_57600|IR_115200;
	dev->qos.min_turn_time.bits = drv->qos_mtt_bits;
	irda_qos_bits_to_value(&dev->qos);

	strncpy(dev->hwname, name, sizeof(dev->hwname)-1);

	atomic_set(&dev->enable_rx, 0);
	dev->tx_skb = NULL;

	spin_lock_init(&dev->tx_lock);
	sema_init(&dev->fsm.sem, 1);

	dev->drv = drv;
	dev->netdev = ndev;

	/* Override the network functions we need to use */
	ndev->netdev_ops = &sirdev_ops;

	if (register_netdev(ndev)) {
		IRDA_ERROR("%s(), register_netdev() failed!\n", __func__);
		goto out_freenetdev;
	}

	return dev;

out_freenetdev:
	free_netdev(ndev);
out:
	return NULL;
}
EXPORT_SYMBOL(sirdev_get_instance);

int sirdev_put_instance(struct sir_dev *dev)
{
	int err = 0;

	IRDA_DEBUG(0, "%s\n", __func__);

	atomic_set(&dev->enable_rx, 0);

	netif_carrier_off(dev->netdev);
	netif_device_detach(dev->netdev);

	if (dev->dongle_drv)
		err = sirdev_schedule_dongle_close(dev);
	if (err)
		IRDA_ERROR("%s - error %d\n", __func__, err);

	sirdev_close(dev->netdev);

	down(&dev->fsm.sem);
	dev->fsm.state = SIRDEV_STATE_DEAD;	/* mark staled */
	dev->dongle_drv = NULL;
	dev->priv = NULL;
	up(&dev->fsm.sem);

	/* Remove netdevice */
	unregister_netdev(dev->netdev);

	free_netdev(dev->netdev);

	return 0;
}
EXPORT_SYMBOL(sirdev_put_instance);

static int __init sir_wq_init(void)
{
	irda_sir_wq = create_singlethread_workqueue("irda_sir_wq");
	if (!irda_sir_wq)
		return -ENOMEM;
	return 0;
}

static void __exit sir_wq_exit(void)
{
	destroy_workqueue(irda_sir_wq);
}

module_init(sir_wq_init);
module_exit(sir_wq_exit);

MODULE_AUTHOR("Martin Diehl <info@mdiehl.de>");
MODULE_DESCRIPTION("IrDA SIR core");
MODULE_LICENSE("GPL");
