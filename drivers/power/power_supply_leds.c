/*
 *  LEDs triggers for power supply class
 *
 *  Copyright © 2007  Anton Vorontsov <cbou@mail.ru>
 *  Copyright © 2004  Szabolcs Gyurko
 *  Copyright © 2003  Ian Molton <spyro@f2s.com>
 *
 *  Modified: 2004, Oct     Szabolcs Gyurko
 *
 *  You may use this code as per GPL version 2
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/








#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>

#include "power_supply.h"

/* Battery specific LEDs triggers. */









































static int power_supply_create_bat_triggers(struct power_supply *psy)
{
	int rc = 0;

	psy->charging_full_trig_name = kasprintf(GFP_KERNEL,
					"%s-charging-or-full", psy->name);
	if (!psy->charging_full_trig_name)
		goto charging_full_failed;

	psy->charging_trig_name = kasprintf(GFP_KERNEL,
					"%s-charging", psy->name);
	if (!psy->charging_trig_name)
		goto charging_failed;

	psy->full_trig_name = kasprintf(GFP_KERNEL, "%s-full", psy->name);
	if (!psy->full_trig_name)
		goto full_failed;

	psy->charging_blink_full_solid_trig_name = kasprintf(GFP_KERNEL,
		"%s-charging-blink-full-solid", psy->name);
	if (!psy->charging_blink_full_solid_trig_name)
		goto charging_blink_full_solid_failed;

	led_trigger_register_simple(psy->charging_full_trig_name,
				    &psy->charging_full_trig);
	led_trigger_register_simple(psy->charging_trig_name,
				    &psy->charging_trig);
	led_trigger_register_simple(psy->full_trig_name,
				    &psy->full_trig);
	led_trigger_register_simple(psy->charging_blink_full_solid_trig_name,
				    &psy->charging_blink_full_solid_trig);

	goto success;

charging_blink_full_solid_failed:
	kfree(psy->full_trig_name);
full_failed:
	kfree(psy->charging_trig_name);
charging_failed:
	kfree(psy->charging_full_trig_name);
charging_full_failed:
	rc = -ENOMEM;
success:
	return rc;
}

static void power_supply_remove_bat_triggers(struct power_supply *psy)
{
	led_trigger_unregister_simple(psy->charging_full_trig);
	led_trigger_unregister_simple(psy->charging_trig);
	led_trigger_unregister_simple(psy->full_trig);
	led_trigger_unregister_simple(psy->charging_blink_full_solid_trig);
	kfree(psy->charging_blink_full_solid_trig_name);
	kfree(psy->full_trig_name);
	kfree(psy->charging_trig_name);
	kfree(psy->charging_full_trig_name);
}

/* Generated power specific LEDs triggers. */



















static int power_supply_create_gen_triggers(struct power_supply *psy)
{
	int rc = 0;

	psy->online_trig_name = kasprintf(GFP_KERNEL, "%s-online", psy->name);
	if (!psy->online_trig_name)
		goto online_failed;

	led_trigger_register_simple(psy->online_trig_name, &psy->online_trig);

	goto success;

online_failed:
	rc = -ENOMEM;
success:
	return rc;
}

static void power_supply_remove_gen_triggers(struct power_supply *psy)
{
	led_trigger_unregister_simple(psy->online_trig);
	kfree(psy->online_trig_name);
}

/* Choice what triggers to create&update. */

void power_supply_update_leds(struct power_supply *psy)
{


    if (psy->type == POWER_SUPPLY_TYPE_BATTERY)
        pr_debug("%s : Execution of power_supply_update_bat_leds() is not.\n", psy->name);
    else
        pr_debug("%s : Execution of power_supply_update_gen_leds() is not.\n", psy->name);








}

int power_supply_create_triggers(struct power_supply *psy)
{
	if (psy->type == POWER_SUPPLY_TYPE_BATTERY)
		return power_supply_create_bat_triggers(psy);
	return power_supply_create_gen_triggers(psy);
}

void power_supply_remove_triggers(struct power_supply *psy)
{
	if (psy->type == POWER_SUPPLY_TYPE_BATTERY)
		power_supply_remove_bat_triggers(psy);
	else
		power_supply_remove_gen_triggers(psy);
}
