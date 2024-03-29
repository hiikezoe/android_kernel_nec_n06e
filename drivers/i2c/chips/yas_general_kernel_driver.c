/*
 * Copyright (c) 2010-2011 Yamaha Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2013                      */
/***********************************************************************/

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/workqueue.h>



#include "../../../include/linux/i2c/yas.h"


#if SENSOR_TYPE == 1
#define SENSOR_NAME "accelerometer"
#elif SENSOR_TYPE == 2
#define SENSOR_NAME "geomagnetic"
#elif SENSOR_TYPE == 3
#define SENSOR_NAME "orientation"
#elif SENSOR_TYPE == 4
#define SENSOR_NAME "gyroscope"
#elif SENSOR_TYPE == 5
#define SENSOR_NAME "light"
#elif SENSOR_TYPE == 6
#define SENSOR_NAME "pressure"
#elif SENSOR_TYPE == 7
#define SENSOR_NAME "temperature"
#elif SENSOR_TYPE == 8
#define SENSOR_NAME "proximity"
#elif SENSOR_TYPE == 9
#define SENSOR_NAME "gravity"
#elif SENSOR_TYPE == 10
#define SENSOR_NAME "linear_acceleration"
#elif SENSOR_TYPE == 11
#define SENSOR_NAME "rotation_vector"
#endif

#define SENSOR_DEFAULT_DELAY		(200)   
#define SENSOR_MAX_DELAY		(2000)  
#define ABS_STATUS			(ABS_BRAKE)
#define ABS_WAKE			(ABS_MISC)
#define ABS_CONTROL_REPORT		(ABS_THROTTLE)

static int suspend(void);
static int resume(void);

struct sensor_data {
	struct mutex mutex;
	int enabled;
	int delay;
#if DEBUG
	int suspend;
#endif



};

static struct platform_device *sensor_pdev;
static struct input_dev *this_data;










































































































































































































static int
suspend(void)
{
	
	YLOGD(("%s: suspend\n", SENSOR_NAME));

	if (strcmp(SENSOR_NAME, "gyroscope") == 0)
		; 
	else if (strcmp(SENSOR_NAME, "light") == 0)
		; 
	else if (strcmp(SENSOR_NAME, "pressure") == 0)
		; 
	else if (strcmp(SENSOR_NAME, "temperature") == 0)
		; 
	else if (strcmp(SENSOR_NAME, "proximity") == 0)
		; 

	return 0;
}

static int
resume(void)
{
	
	YLOGD(("%s: resume\n", SENSOR_NAME));

	if (strcmp(SENSOR_NAME, "gyroscope") == 0)
		; 
	else if (strcmp(SENSOR_NAME, "light") == 0)
		; 
	else if (strcmp(SENSOR_NAME, "pressure") == 0)
		; 
	else if (strcmp(SENSOR_NAME, "temperature") == 0)
		; 
	else if (strcmp(SENSOR_NAME, "proximity") == 0)
		; 

#if DEBUG
	{
		struct sensor_data *data = input_get_drvdata(this_data);
		data->suspend = 0;
	}
#endif 

	return 0;
}



static ssize_t
sensor_delay_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct sensor_data *data = input_get_drvdata(input_data);
	int delay;

	mutex_lock(&data->mutex);

	delay = data->delay;

	mutex_unlock(&data->mutex);

	return sprintf(buf, "%d\n", delay);
}

static ssize_t
sensor_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct sensor_data *data = input_get_drvdata(input_data);
	long value;




	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;
	if (value < 0)
		return count;

	if (SENSOR_MAX_DELAY < value)
		value = SENSOR_MAX_DELAY;

	mutex_lock(&data->mutex);





	data->delay = value;

	input_report_abs(input_data, ABS_CONTROL_REPORT,
			(data->enabled<<16) | value);
	input_sync(input_data);


	mutex_unlock(&data->mutex);

	return count;
}

static ssize_t
sensor_enable_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct sensor_data *data = input_get_drvdata(input_data);
	int enabled;

	mutex_lock(&data->mutex);

	enabled = data->enabled;

	mutex_unlock(&data->mutex);

	return sprintf(buf, "%d\n", enabled);
}

static ssize_t
sensor_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	struct sensor_data *data = input_get_drvdata(input_data);
	long value;




	if (strict_strtol(buf, 10, &value) < 0)
		return -EINVAL;
	value = !!value;

	mutex_lock(&data->mutex);





	input_report_abs(input_data, ABS_CONTROL_REPORT,
			(value<<16) | data->delay);
	input_sync(input_data);


	if (data->enabled && !value)
		suspend();
	if (!data->enabled && value)
		resume();
	data->enabled = value;

	mutex_unlock(&data->mutex);

	return count;
}

static ssize_t
sensor_wake_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t count)
{
	struct input_dev *input_data = to_input_dev(dev);
	static int cnt = 1;









	input_report_abs(input_data, ABS_WAKE, cnt++);
	input_sync(input_data);


	return count;
}

#if DEBUG

static int sensor_suspend(struct platform_device *pdev, pm_message_t state);
static int sensor_resume(struct platform_device *pdev);

static ssize_t
sensor_debug_suspend_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct sensor_data *data = input_get_drvdata(input);

	return sprintf(buf, "%d\n", data->suspend);
}

static ssize_t
sensor_debug_suspend_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	long suspend;

	if (strict_strtol(buf, 10, &suspend) < 0)
		return -EINVAL;
	if (suspend) {
		pm_message_t msg;
		memset(&msg, 0, sizeof(msg));
		sensor_suspend(sensor_pdev, msg);
	} else {
		sensor_resume(sensor_pdev);
	}

	return count;
}

#endif 

static ssize_t
sensor_data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
#if SENSOR_TYPE <= 4 || 9 <= SENSOR_TYPE
	int x, y, z;
#else
	int x;
#endif

	x = input_abs_get_val(input_data, ABS_X);
#if SENSOR_TYPE <= 4 || 9 <= SENSOR_TYPE
	y = input_abs_get_val(input_data, ABS_Y);
	z = input_abs_get_val(input_data, ABS_Z);
#endif


#if SENSOR_TYPE <= 4 || 9 <= SENSOR_TYPE
	return sprintf(buf, "%d %d %d\n", x, y, z);
#else
	return sprintf(buf, "%d\n", x);
#endif
}

static ssize_t
sensor_status_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct input_dev *input_data = to_input_dev(dev);
	int status;
	status = input_abs_get_val(input_data, ABS_STATUS);

	return sprintf(buf, "%d\n", status);
}

static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		sensor_delay_show, sensor_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		sensor_enable_show, sensor_enable_store);
static DEVICE_ATTR(wake, S_IWUSR|S_IWGRP,
		NULL, sensor_wake_store);
static DEVICE_ATTR(data, S_IRUGO, sensor_data_show, NULL);
static DEVICE_ATTR(status, S_IRUGO, sensor_status_show, NULL);

#if DEBUG
static DEVICE_ATTR(debug_suspend, S_IRUGO|S_IWUSR,
		sensor_debug_suspend_show, sensor_debug_suspend_store);
#endif 

static struct attribute *sensor_attributes[] = {
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_wake.attr,
	&dev_attr_data.attr,
	&dev_attr_status.attr,
#if DEBUG
	&dev_attr_debug_suspend.attr,
#endif 
	NULL
};

static struct attribute_group sensor_attribute_group = {
	.attrs = sensor_attributes
};

static int
sensor_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct sensor_data *data = input_get_drvdata(this_data);
	int rt = 0;

	mutex_lock(&data->mutex);

	if (data->enabled) {






		input_report_abs(this_data, ABS_CONTROL_REPORT,
				(0<<16) | data->delay);
		input_sync(this_data);

		rt = suspend();
	}

	mutex_unlock(&data->mutex);

	return rt;
}

static int
sensor_resume(struct platform_device *pdev)
{
	struct sensor_data *data = input_get_drvdata(this_data);
	int rt = 0;

	mutex_lock(&data->mutex);

	if (data->enabled) {






		rt = resume();

		input_report_abs(this_data, ABS_CONTROL_REPORT,
				(1<<16) | data->delay);
		input_sync(this_data);

	}

	mutex_unlock(&data->mutex);

	return rt;
}

static int
sensor_probe(struct platform_device *pdev)
{
	struct sensor_data *data = NULL;
	struct input_dev *input_data = NULL;
	int input_registered = 0, sysfs_created = 0;
	int rt;

	data = kzalloc(sizeof(struct sensor_data), GFP_KERNEL);
	if (!data) {
		rt = -ENOMEM;
		goto err;
	}
	data->enabled = 0;
	data->delay = SENSOR_DEFAULT_DELAY;

	input_data = input_allocate_device();
	if (!input_data) {
		rt = -ENOMEM;
		YLOGE(("sensor_probe: Failed to allocate input_data device\n"));
		goto err;
	}

	set_bit(EV_ABS, input_data->evbit);
	input_set_abs_params(input_data, ABS_X, INT_MIN, INT_MAX, 0, 0);
#if SENSOR_TYPE <= 4 || 9 <= SENSOR_TYPE
	input_set_abs_params(input_data, ABS_Y, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_Z, INT_MIN, INT_MAX, 0, 0);
#endif
	input_set_abs_params(input_data, ABS_RUDDER, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_STATUS, 0, 3, 0, 0);
	input_set_abs_params(input_data, ABS_WAKE, INT_MIN, INT_MAX, 0, 0);
	input_set_abs_params(input_data, ABS_CONTROL_REPORT, INT_MIN, INT_MAX,
			0, 0);
	input_data->name = SENSOR_NAME;

	rt = input_register_device(input_data);
	if (rt) {
		YLOGE(("sensor_probe: Unable to register input_data device: "
					"%s\n", input_data->name));
		goto err;
	}
	input_set_drvdata(input_data, data);
	input_registered = 1;

	rt = sysfs_create_group(&input_data->dev.kobj,
			&sensor_attribute_group);
	if (rt) {
		YLOGE(("sensor_probe: sysfs_create_group failed[%s]\n",
					input_data->name));
		goto err;
	}
	sysfs_created = 1;
	mutex_init(&data->mutex);
	this_data = input_data;







	return 0;

err:
	if (data != NULL) {
		if (input_data != NULL) {
			if (sysfs_created) {
				sysfs_remove_group(&input_data->dev.kobj,
						&sensor_attribute_group);
			}
			if (input_registered)
				input_unregister_device(input_data);
			else
				input_free_device(input_data);
			input_data = NULL;
		}
		kfree(data);
	}

	return rt;
}

static int
sensor_remove(struct platform_device *pdev)
{
	struct sensor_data *data;




	if (this_data != NULL) {
		data = input_get_drvdata(this_data);
		sysfs_remove_group(&this_data->dev.kobj,
				&sensor_attribute_group);
		input_unregister_device(this_data);
		if (data != NULL)
			kfree(data);
	}

	return 0;
}




static struct platform_driver sensor_driver = {
	.probe		= sensor_probe,
	.remove		= sensor_remove,
	.suspend	= sensor_suspend,
	.resume		= sensor_resume,
	.driver = {
		.name	= SENSOR_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init sensor_init(void)
{
	sensor_pdev = platform_device_register_simple(SENSOR_NAME, 0, NULL, 0);
	if (IS_ERR(sensor_pdev))
		return -1;
	return platform_driver_register(&sensor_driver);
}
module_init(sensor_init);

static void __exit sensor_exit(void)
{
	platform_driver_unregister(&sensor_driver);
	platform_device_unregister(sensor_pdev);
}
module_exit(sensor_exit);

MODULE_AUTHOR("Yamaha Corporation");
MODULE_LICENSE("GPL");
MODULE_VERSION("4.3.701a");
