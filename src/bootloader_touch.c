/*
 * SPDX-License-Identifier: MIT
 *
 * "1200 baud touch": when the host sets the CDC ACM baud rate to 1200,
 * reboot into the UF2 bootloader, exactly like the &bootloader behavior
 * (ZMK behavior_reset.c: sys_reboot(RST_UF2)).
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/uart/cdc_acm.h>

/* Not on this library's include path (ZMK adds app/include to the app target only). */
#ifndef RST_UF2
#define RST_UF2 0x57 /* UF2 bootloader magic, as ZMK dt-bindings/zmk/reset.h */
#endif

/* Pick the CDC ACM device: ZMK Studio RPC UART if it is one, else the first okay instance. */
#if DT_HAS_CHOSEN(zmk_studio_rpc_uart)
#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zmk_studio_rpc_uart), zephyr_cdc_acm_uart)
#define BOOTLOADER_TOUCH_DEV DEVICE_DT_GET(DT_CHOSEN(zmk_studio_rpc_uart))
#endif
#endif

#ifndef BOOTLOADER_TOUCH_DEV
#if DT_HAS_COMPAT_STATUS_OKAY(zephyr_cdc_acm_uart)
#define BOOTLOADER_TOUCH_DEV DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart)
#endif
#endif

#ifdef BOOTLOADER_TOUCH_DEV

#define TOUCH_BAUD 1200
#define REBOOT_DELAY_MS 50

static void reboot_work_handler(struct k_work *work) { sys_reboot(RST_UF2); }

static K_WORK_DELAYABLE_DEFINE(reboot_work, reboot_work_handler);

static void dte_rate_cb(const struct device *dev, uint32_t rate) {
    if (rate == TOUCH_BAUD) {
        k_work_schedule(&reboot_work, K_MSEC(REBOOT_DELAY_MS));
    }
}

static int bootloader_touch_init(void) {
    const struct device *dev = BOOTLOADER_TOUCH_DEV;

    if (!device_is_ready(dev)) {
        return -ENODEV;
    }

    return cdc_acm_dte_rate_callback_set(dev, dte_rate_cb);
}

SYS_INIT(bootloader_touch_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif /* BOOTLOADER_TOUCH_DEV */
