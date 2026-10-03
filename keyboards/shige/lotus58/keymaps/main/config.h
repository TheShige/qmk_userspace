// Copyright 2023 TheShige (@TheShige)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// Disabling Lock Key support
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE

// Setting up split keyboard
#define OLED_TIMEOUT 0
#define OLED_BRIGHTNESS 150
#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_A

// Layer count
#define LAYER_STATE_16BIT
#define DYNAMIC_KEYMAP_LAYER_COUNT 16

// RGB Matrix effects
#define RGB_MATRIX_KEYPRESSES

#define ENABLE_RGB_MATRIX_CYCLE_ALL
#define ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
