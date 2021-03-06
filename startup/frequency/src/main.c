/*
 * Copyright (c) 2021 Phytec Messtechnik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <sys/printk.h>
#include <sys/__assert.h>
#include <string.h>

/* size of stack area used by each thread */
#define HEARTBEAT_STACKSIZE 2048
#define FREQUENCY_STACKSIZE 2048

/* scheduling priority used by each thread */
#define HEARTBEAT_PRIORITY 5
#define FREQUENCY_PRIORITY 5

#define LED_HEARTBEAT		DT_PATH(leds, heartbeat)
#define LED_HEARTBEAT_CTRL	DT_PROP(DT_PHANDLE_BY_IDX(LED_HEARTBEAT, gpios, 0), label)
#define LED_HEARTBEAT_PIN	DT_PHA_BY_IDX(LED_HEARTBEAT, gpios, 0, pin)
#define LED_HEARTBEAT_FLAGS	DT_PHA_BY_IDX(LED_HEARTBEAT, gpios, 0, flags)

void frequency(void)
{
	volatile unsigned int *device = (unsigned int *)0xF0060004;
	unsigned int count;
	unsigned long tmp;
	unsigned long value;

	while (1) {
		count = *device;
		tmp = (1 * 1000 * 1000) / ((count * 10 * 1000) / (256 * 4));
		value = tmp * 1000000;
		printk("%ld Hz (%d counts)\n", value, count);
		k_sleep(K_SECONDS(1));
	}
}
void heartbeat(void)
{
	const struct device *led_dev;

	led_dev = device_get_binding(LED_HEARTBEAT_CTRL);
	__ASSERT_NO_MSG(led_dev != NULL);

	gpio_pin_configure(led_dev, LED_HEARTBEAT_PIN, LED_HEARTBEAT_FLAGS);

	while (1) {
		gpio_pin_set(led_dev, LED_HEARTBEAT_PIN, 1);
		k_sleep(K_MSEC(150));
		gpio_pin_set(led_dev, LED_HEARTBEAT_PIN, 0);
		k_sleep(K_MSEC(50));
		gpio_pin_set(led_dev, LED_HEARTBEAT_PIN, 1);
		k_sleep(K_MSEC(150));
		gpio_pin_set(led_dev, LED_HEARTBEAT_PIN, 0);
		k_sleep(K_SECONDS(1));
	}
}

K_THREAD_DEFINE(heartbeat_tid, HEARTBEAT_STACKSIZE, heartbeat, NULL, NULL, NULL,
 		HEARTBEAT_PRIORITY, 0, 0);

K_THREAD_DEFINE(frequency_tid, FREQUENCY_STACKSIZE, frequency, NULL, NULL, NULL,
 		FREQUENCY_PRIORITY, 0, 0);
