// SPDX-License-Identifier: GPL-2.0
/*
 * Atlas LED demo driver fragment for course experiment.
 *
 * This file demonstrates the kind of code tracked by Git in an LED
 * experiment: probe matching, GPIO-backed brightness control, and
 * registration into the Linux LED subsystem.
 */

#include <linux/gpio/consumer.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/platform_device.h>

struct atlas_led {
	struct led_classdev cdev;
	struct gpio_desc *gpiod;
};

static void atlas_led_brightness_set(struct led_classdev *cdev,
				     enum led_brightness brightness)
{
	struct atlas_led *led = container_of(cdev, struct atlas_led, cdev);

	gpiod_set_value_cansleep(led->gpiod, brightness ? 1 : 0);
}

static int atlas_led_probe(struct platform_device *pdev)
{
	struct atlas_led *led;

	led = devm_kzalloc(&pdev->dev, sizeof(*led), GFP_KERNEL);
	if (!led)
		return -ENOMEM;

	led->gpiod = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
	if (IS_ERR(led->gpiod))
		return dev_err_probe(&pdev->dev, PTR_ERR(led->gpiod),
				     "failed to request LED gpio\n");

	led->cdev.name = "led0";
	led->cdev.max_brightness = 1;
	led->cdev.brightness_set_blocking = atlas_led_brightness_set;

	dev_info(&pdev->dev, "atlas led probe completed\n");
	return devm_led_classdev_register(&pdev->dev, &led->cdev);
}

static const struct of_device_id atlas_led_of_match[] = {
	{ .compatible = "atlas,led-demo" },
	{ }
};
MODULE_DEVICE_TABLE(of, atlas_led_of_match);

static struct platform_driver atlas_led_driver = {
	.probe = atlas_led_probe,
	.driver = {
		.name = "atlas-led-demo",
		.of_match_table = atlas_led_of_match,
	},
};
module_platform_driver(atlas_led_driver);

MODULE_AUTHOR("54078");
MODULE_DESCRIPTION("Atlas LED demo driver for Git course report");
MODULE_LICENSE("GPL");
