/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/i2c.h>
#include <sys/util.h>
#include <kernel.h>
#include <drivers/sensor.h>
#include <stdbool.h>

#include "bma421.h"

#include <logging/log.h>
LOG_MODULE_DECLARE(BMA421, CONFIG_SENSOR_LOG_LEVEL);

/*!
 *  @brief API sets the interrupt to either interrupt1 or
 *  interrupt2 pin in the sensor.
 */
static int bma4_map_intterupt(struct device *dev, uint8_t int_line, uint16_t int_map, bool enable)
{
	int retval = 0;
	uint8_t data[3] = { 0 };
	uint8_t index[2] = { BMA421_REG_INT_MAP_1, BMA421_REG_INT_MAP_2 };
	const struct bma421_config *cfg = dev->config;
	struct bma421_data *drv_data = dev->data;

	retval = i2c_burst_read(drv_data->i2c, cfg->i2c_addr, BMA421_REG_INT_MAP_1, data, 3);
	if (retval) {
		LOG_ERR("error when read %d", BMA421_REG_INT_MAP_1);
		return retval;
	}
	if (enable == true) {
		/* Feature interrupt mapping */
		data[int_line] = (uint8_t)(int_map & (0x00FF));

		/* Hardware interrupt mapping */
		data[2] = (uint8_t)((int_map & (0xFF00)) >> 8);
	} else {
		/* Feature interrupt un-mapping */
		data[int_line] &= (~(uint8_t)(int_map & (0x00FF)));

		/* Hardware interrupt un-mapping */
		data[2] &= (~(uint8_t)((int_map & (0xFF00)) >> 8));
	}

	retval = i2c_reg_write_byte(drv_data->i2c, cfg->i2c_addr, index[int_line], &data[int_line]);
	if (retval == 0)
	{
		retval = i2c_reg_write_byte(drv_data->i2c, cfg->i2c_addr, BMA421_REG_INT_MAP_DATA, &data[2]);
	}
	return retval;
}

int bma421_attr_set(struct device *dev,
			enum sensor_channel chan,
			enum sensor_attribute attr,
			const struct sensor_value *val)
{
	struct bma421_data *drv_data = dev->data;
	uint64_t slope_th;
uint8_t buf[BMA421_FEATURE_SIZE];
//default anymotion is selected
//todo set any parameter eg stepcounter, tap double tap, wrist tilt etc
// if (i2c_burst_read(drv_data->i2c, BMA421_I2C_ADDRESS, BMA421_REG_FEATURE, buf, BMA421_FEATURE_SIZE) < 0) {}

	if (chan != SENSOR_CHAN_ACCEL_XYZ) {
		return -ENOTSUP;
	}
//0x5E features register burst read
/*	if (attr == SENSOR_ATTR_SLOPE_TH) {
		}
	} else if (attr == SENSOR_ATTR_SLOPE_DUR) {
		}
	} else {
		return -ENOTSUP;
	}
*/
	return 0;
}

static void bma421_gpio_callback(const struct device *dev,
				 struct gpio_callback *cb, uint32_t pins)
{
	struct bma421_data *drv_data =
		CONTAINER_OF(cb, struct bma421_data, gpio_cb);

	ARG_UNUSED(pins);

//	gpio_pin_disable_callback(dev, CONFIG_BMA421_GPIO_PIN_NUM);

#if defined(CONFIG_BMA421_TRIGGER_OWN_THREAD)
	k_sem_give(&drv_data->gpio_sem);
#elif defined(CONFIG_BMA421_TRIGGER_GLOBAL_THREAD)
	k_work_submit(&drv_data->work);
#endif
}

static void bma421_thread_cb(void *arg)
{
	struct device *dev = arg;
	struct bma421_data *drv_data = dev->data;
	uint8_t status = 0U;
	int err = 0;

	/* check for data ready */
/*	err = i2c_reg_read_byte(drv_data->i2c, BMA421_I2C_ADDRESS,
				BMA421_REG_INT_STATUS_1, &status);
	if (status & BMA421_BIT_DATA_INT_STATUS &&
		drv_data->data_ready_handler != NULL &&
		err == 0) {
		drv_data->data_ready_handler(dev,
						 &drv_data->data_ready_trigger);
	}
*/
	/* check for any motion */
		/* clear latched interrupt -- according this is already done by reading the register*/
/*		err = i2c_reg_update_byte(drv_data->i2c, BMA421_I2C_ADDRESS,
					  BMA421_REG_INT_RST_LATCH,
					  BMA421_BIT_INT_LATCH_RESET,
					  BMA421_BIT_INT_LATCH_RESET);

		if (err < 0) {
			LOG_DBG("Could not update clear the interrupt");
			return;
		}*/


//	gpio_pin_enable_callback(drv_data->gpio, CONFIG_BMA421_GPIO_PIN_NUM);
}

#ifdef CONFIG_BMA421_TRIGGER_OWN_THREAD
static void bma421_thread(int dev_ptr, int unused)
{
	struct device *dev = INT_TO_POINTER(dev_ptr);
	struct bma421_data *drv_data = dev->data;

	ARG_UNUSED(unused);

	while (1) {
		k_sem_take(&drv_data->gpio_sem, K_FOREVER);
		bma421_thread_cb(dev);
	}
}
#endif

#ifdef CONFIG_BMA421_TRIGGER_GLOBAL_THREAD
static void bma421_work_cb(struct k_work *work)
{
	struct bma421_data *drv_data =
		CONTAINER_OF(work, struct bma421_data, work);

	bma421_thread_cb(drv_data->dev);
}
#endif

int bma421_trigger_set(struct device *dev,
			   const struct sensor_trigger *trig,
			   sensor_trigger_handler_t handler)
{
	struct bma421_config *cfg = dev->config;
	struct bma421_data *drv_data = dev->data;

	if (trig->type == SENSOR_TRIG_DATA_READY) {
		/* disable data ready interrupt while changing trigger params */
/*		if (i2c_reg_update_byte(drv_data->i2c, cfg->i2c_addr,
					BMA421_REG_INT_CONFIG0,
					BMA421_BIT_DATA_EN, 0) < 0) {
			LOG_DBG("Could not disable data ready interrupt");
			return -EIO;
		}
*/
		drv_data->data_ready_handler = handler;
		if (handler == NULL) {
			return 0;
		}
		drv_data->data_ready_trigger = *trig;

		/* enable data ready interrupt */
#if 0
		if (i2c_reg_update_byte(drv_data->i2c, cfg->i2c_addr,
					BMA421_REG_INT_CONFIG0,
					BMA421_BIT_DATA_EN,
					BMA421_BIT_DATA_EN) < 0) {
			LOG_DBG("Could not enable data ready interrupt");
			return -EIO;
		}
#endif
	} else if (trig->type == SENSOR_TRIG_DELTA) {
		/* disable any-motion interrupt while changing trigger params */
		/*if (i2c_reg_update_byte(drv_data->i2c, cfg->i2c_addr,
					BMA421_REG_INT1_MAP,
					BMA421_INT_MAP_MOTION, 0) < 0) {
			LOG_ERR("Could not disable data ready interrupt");
			return -EIO;
		}*/

		drv_data->any_motion_handler = handler;
		if (handler == NULL) {
			return 0;
		}
		drv_data->any_motion_trigger = *trig;

		/* enable any-motion interrupt */
		/*if (i2c_reg_update_byte(drv_data->i2c, cfg->i2c_addr,
					BMA421_REG_INT1_MAP,
					BMA421_INT_MAP_MOTION,
					BMA421_INT_MAP_MOTION) < 0) {
			LOG_ERR("Could not enable data ready interrupt");
			return -EIO;
		}*/
	} else {
		return -ENOTSUP;
	}

	return 0;
}

int bma421_init_interrupt(struct device *dev)
{
	const struct bma421_config *cfg = dev->config;
	struct bma421_data *drv_data = dev->data;

	/* set latched interrupts */


	/* setup data ready gpio interrupt */
	drv_data->gpio = device_get_binding(cfg->drdy_controller);
	if (drv_data->gpio == NULL) {
		LOG_ERR("Cannot get pointer to %s device",
			cfg->drdy_controller);
		return -EINVAL;
	}

	gpio_pin_configure(drv_data->gpio, cfg->drdy_pin,
		GPIO_INPUT | GPIO_PULL_UP | GPIO_INT_EDGE_FALLING | cfg->drdy_flags);

	gpio_init_callback(&drv_data->gpio_cb,
			bma421_gpio_callback,
			BIT(cfg->drdy_pin));

	if (gpio_add_callback(drv_data->gpio, &drv_data->gpio_cb) < 0) {
		LOG_ERR("Could not set gpio callback");
		return -EIO;
	}
#if 0
	/* map data ready interrupt to INT1 */
	if (i2c_reg_update_byte(drv_data->i2c, cfg->i2c_addr,
				BMA421_REG_INT_MAP_1,
				BMA421_INT_MAP_1_BIT_DATA,
				BMA421_INT_MAP_1_BIT_DATA) < 0) {
		LOG_DBG("Could not map data ready interrupt pin");
		return -EIO;
	}

	/* map any-motion interrupt to INT1 */
	if (i2c_reg_update_byte(drv_data->i2c, cfg->i2c_addr,
				BMA421_REG_INT1_MAP,
				BMA421_INT_MAP_MOTION,
				BMA421_INT_MAP_MOTION) < 0) {
		LOG_ERR("Could not map any-motion interrupt pin");
		return -EIO;
	}
#endif
/*	if (i2c_reg_update_byte(drv_data->i2c, BMA421_I2C_ADDRESS,
				BMA421_REG_INT1_MAP,
				BMA421_BIT_DATA_EN, 0) < 0) {
		LOG_DBG("Could not disable data ready interrupt");
		return -EIO;
	}
*/
	/* disable any-motion interrupt */
/*	if (i2c_reg_update_byte(drv_data->i2c, BMA421_I2C_ADDRESS,
				BMA421_REG_INT_EN_0,
				BMA421_SLOPE_EN_XYZ, 0) < 0) {
		LOG_DBG("Could not disable data ready interrupt");
		return -EIO;
	}
*/
#if defined(CONFIG_BMA421_TRIGGER_OWN_THREAD)
	k_sem_init(&drv_data->gpio_sem, 0, UINT_MAX);

	k_thread_create(&drv_data->thread, drv_data->thread_stack,
			CONFIG_BMA421_THREAD_STACK_SIZE,
			(k_thread_entry_t)bma421_thread, dev,
			0, NULL, K_PRIO_COOP(CONFIG_BMA421_THREAD_PRIORITY),
			0, K_NO_WAIT);
#elif defined(CONFIG_BMA421_TRIGGER_GLOBAL_THREAD)
	drv_data->work.handler = bma421_work_cb;
	drv_data->dev = dev;
#endif

	return 0;
}
