# SPDX-License-Identifier: MIT
# Compiled only when the USB CDC ACM stack is enabled (left half / central).
# The right half (no USB stack) builds nothing from this module.

if(CONFIG_ZMK_BOOTLOADER_TOUCH)
  zephyr_library()
  zephyr_library_sources(src/bootloader_touch.c)
endif()
