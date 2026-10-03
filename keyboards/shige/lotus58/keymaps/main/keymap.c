// Copyright 2023 TheShige (@TheShige)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include <stdlib.h>
#include "features/achordion.h"
#include "transactions.h"

#define ____ KC_TRNS

// OLED setup
#define MIN_WALK_SPEED 10
#define MIN_RUN_SPEED 50

#define ANIM_FRAME_DURATION 200 // how long each frame lasts in ms
#define ANIM_SIZE 96            // number of bytes in array. If you change sprites, minimize for adequate firmware size. max is 1024
//
enum my_keycodes { SWITCH_COLEMAK = SAFE_RANGE };

typedef struct _sync_luna_status_t {
    bool isJumping;
    bool showedJump;
    bool isSneaking;
} sync_luna_status_t;

bool isSynced = true; // are the two halves are synced

uint32_t anim_timer    = 0;
uint8_t current_frame = 0;

/* status variables */
int current_wpm = 0;
led_t led_usb_state;
sync_luna_status_t luna_status;

int lastLayer = 0; // last layer before "SWITCH_COLEMAK" is pressed
int currentLayer = 0; // current layer
bool is_oled_enabled = true;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [0] = LAYOUT(
    KC_ESC, 	    KC_1,	        KC_2,	        KC_3,	        KC_4,	        KC_5,	        KC_MPLY,	KC_NO,	KC_6,	        KC_7,	        KC_8,		    KC_9,		    KC_0,		        KC_MINS,
    KC_TAB, 	    KC_Q,	        KC_W,           KC_F,           KC_P,           KC_B,                               KC_J, 	        KC_L,           KC_U,   	    KC_Y,   	    KC_SCLN,   	        KC_EQL,
	KC_CAPS,        LGUI_T(KC_A), 	LALT_T(KC_R),   LCTL_T(KC_S),   LSFT_T(KC_T),   KC_G,                      	    KC_M, 	        RSFT_T(KC_N),   RCTL_T(KC_E),   LALT_T(KC_I),	RGUI_T(KC_O),       KC_QUOT,
	KC_LCTL, 	    KC_Z, 	        KC_X,           KC_C,           KC_D,           KC_V,           TG(1),		TG(6),  KC_K, 	        KC_H,           KC_COMMA,	    KC_DOT, 	    KC_SLSH, 	        KC_DEL,
						            LT(5, KC_LGUI), LT(4, KC_ESC),  LT(3,KC_SPC),   KC_TAB,    	                        KC_ENT, 	    LT(2,KC_BSPC),  KC_DEL,         KC_RALT
	),

    [1] = LAYOUT(
    KC_ESC, 	    KC_1,	        KC_2,	        KC_3,	        KC_4,	        KC_5,	        ____,	    ____,	KC_6,	        KC_7,	        KC_8,		    KC_9,		    KC_0,		        KC_MINS,
    KC_TAB, 	    KC_Q,	        KC_W,           KC_E,           KC_R,           KC_T,                               KC_Y, 	        KC_U,           KC_I,   	    KC_O,   	    KC_P,   	        KC_EQL,
    KC_CAPS,        LGUI_T(KC_A), 	LALT_T(KC_S),   LCTL_T(KC_D),   LSFT_T(KC_F),     KC_G,                      	    KC_H, 	        RSFT_T(KC_J),   RCTL_T(KC_K),   LALT_T(KC_L),	RGUI_T(KC_SCLN),    KC_QUOT,
	KC_LCTL, 	    KC_Z, 	        KC_X,           KC_C,           KC_V,           KC_B,           TG(1),		TG(6),  KC_N, 	        KC_M,           KC_COMMA,	    KC_DOT, 	    KC_SLSH, 	        KC_DEL,
						            LT(5, KC_LGUI), LT(4, KC_ESC),  LT(3,KC_SPC),   KC_TAB,    	                        KC_ENT, 	    LT(2,KC_BSPC),  KC_DEL,         KC_RALT
	),

    [2] = LAYOUT(
    ____,     KC_GRAVE,       LSFT(KC_9),     LSFT(KC_0),     KC_SCLN,        KC_COMMA,       ____,       ____,   ____,   ____,   ____,   ____,   ____,   ____,
	LSFT(KC_1),     LSFT(KC_LBRC),  KC_QUOT,        LSFT(KC_QUOT),  LSFT(KC_RBRC),  LSFT(KC_SLSH),                      ____,   ____,   ____,   ____,   ____,   ____,
	LSFT(KC_3),     LSFT(KC_6),     KC_EQL,         LSFT(KC_MINS),  LSFT(KC_4),     LSFT(KC_8),                         ____,   ____,   ____,   ____,   ____,   ____,
	LSFT(KC_GRV),   LSFT(KC_COMMA), LSFT(KC_BSLS),  KC_MINS,        LSFT(KC_DOT),   KC_SLSH,        ____,    ____,   ____,   ____,   ____,   ____,   ____,   ____,
	                                LSFT(KC_5),     KC_DOT,         LSFT(KC_SCLN),  KC_SCLN,                            ____,   ____,   ____,   ____
    ),

    [3] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, KC_HOME, KC_PGDN, KC_PGUP, KC_END, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [4] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    LSFT(KC_8), KC_7, KC_8, KC_9, LSFT(KC_SCLN), ____,
	____, ____, ____, ____, ____, ____,                    LSFT(KC_EQL), KC_4, KC_5, KC_6, KC_MINS, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, LSFT(KC_8), KC_1, KC_2, KC_3, KC_SLSH, ____,
					  ____, ____, ____, ____,      	 KC_DOT, KC_0, KC_COMMA, KC_EQL
    ),

    [5] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, RGB_MODE_FORWARD, RGB_MODE_REVERSE, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, RGB_SPI, RGB_VAI, RGB_VAD, RGB_SPD, RGB_TOG,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, RGB_HUI, RGB_SAI, RGB_SAD, RGB_HUD, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [6] = LAYOUT(
    KC_ESC, KC_1,    KC_2, KC_3, KC_4, KC_5,       ____,       ____,   KC_6, KC_7, KC_8, KC_9, KC_0, ____,
	KC_DEL, KC_TAB,  KC_Q, KC_W, KC_E, KC_R,                         KC_Y, KC_U, KC_I, KC_O, KC_P, ____,
	KC_T,   KC_LSFT, KC_A, KC_S, KC_D, KC_F,                          KC_H, KC_J, KC_K, KC_L, KC_SCLN, ____,
	KC_B,   KC_LCTL, KC_Z, KC_X, KC_C, KC_V,      KC_G,        TG(6), KC_N, KC_M, KC_COMMA, KC_DOT, KC_SLSH, ____,
				     KC_B, KC_LALT, KC_SPC, KC_LCTL,      	    KC_ENT, ____, ____, ____
    ),

    [7] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [8] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [9] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [10] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [11] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [12] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [13] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [14] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),

    [15] = LAYOUT(
    ____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____,                    ____, ____, ____, ____, ____, ____,
	____, ____, ____, ____, ____, ____, ____,        ____, ____, ____, ____, ____, ____, ____,
					  ____, ____, ____, ____,      	 ____, ____, ____, ____
    ),
};

const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [1] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [2] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [3] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [4] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [5] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [6] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [7] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [8] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [9] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [10] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [11] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [12] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [13] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [14] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [15] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS), ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
};

// RGB matrix
led_config_t g_led_config = {
    {
    // Key Matrix to LED Index
    // Left Half
    {  5,  4,  3,  2,  1,  0 },
    {  6,  7,  8,  9, 10, 11 },
    { 17, 16, 15, 14, 13, 12 },
    { 18, 19, 20, 21, 22, 23 },
    { NO_LED, 28, 27, 26, 25, 24 },
    // Right Half
    { 34, 33, 32, 31, 30, 29 },
    { 35, 36, 37, 38, 39, 40 },
    { 46, 45, 44, 43, 42, 41 },
    { 47, 48, 49, 50, 51, 52 },
    { NO_LED, 57, 56, 55, 54, 53 }
    },
    {
    // LED Index to Physical Position
    // Left Half
    { 80, 4  }, { 64, 2  }, { 48, 0  }, { 32, 2  }, { 16, 5  }, {  0, 7  },
    {  0, 23 }, { 16, 21 }, { 32, 18 }, { 48, 16 }, { 64, 18 }, { 80, 20 },
    { 80, 36 }, { 64, 34 }, { 48, 32 }, { 32, 34 }, { 16, 37 }, {  0, 38 },
    {  0, 55 }, { 16, 53 }, { 32, 50 }, { 48, 48 }, { 64, 50 }, { 80, 52 },
                                                                        { 100, 44 },
                                { 92, 61 },  { 72, 64 }, { 56, 62 }, { 40, 60 },

    // Right Half
            { 144, 4  }, { 160, 2  }, { 176, 0  }, { 192, 2  }, { 208, 5  }, { 224, 7  },
            { 224, 23 }, { 208, 21 }, { 192, 18 }, { 176, 16 }, { 160, 18 }, { 144, 20 },
            { 144, 36 }, { 160, 34 }, { 176, 32 }, { 192, 34 }, { 208, 37 }, { 224, 38 },
            { 224, 55 }, { 208, 53 }, { 192, 50 }, { 176, 48 }, { 160, 50 }, { 144, 52 },
    { 124, 44 },
        { 132, 61 }, { 152, 64 }, { 168, 62 }, { 184, 60 },

    },
    {
    // LED Index to Flag
    4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4,   4,
            4, 4, 4, 4,

        4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4,
    4,  4, 4, 4, 4, 4, 4,
       4, 4, 4, 4

    }
};

// Key overrides setup
const key_override_t key1 = ko_make_with_layers(MOD_MASK_SHIFT, LSFT(KC_6), KC_LBRC, 4);
const key_override_t key2 = ko_make_with_layers(MOD_MASK_SHIFT, KC_EQL, LSFT(KC_7), 4);
const key_override_t key3 = ko_make_with_layers(MOD_MASK_SHIFT, LSFT(KC_MINS), LSFT(KC_EQL), 4);
const key_override_t key4 = ko_make_with_layers(MOD_MASK_SHIFT, LSFT(KC_4), KC_RBRC, 4);
const key_override_t key5 = ko_make_with_layers(MOD_MASK_SHIFT, LSFT(KC_8), KC_BSLS, 4);
const key_override_t key6 = ko_make_with_layers(MOD_MASK_SHIFT, LSFT(KC_3), LSFT(KC_2), 4);

const key_override_t **key_overrides = (const key_override_t *[]){
    &key1,
    &key2,
    &key3,
    &key4,
    &key5,
    &key6,
    NULL
};

// Achordion setup
uint16_t achordion_timeout(uint16_t tap_hold_keycode) {
    switch (tap_hold_keycode) {
        case LT(2, KC_BSPC):
        case LT(3, KC_SPC):
        case LT(4, KC_ESC):
        case LT(5, KC_LGUI):
            return 0;
    }

    return 800;
}

bool achordion_chord(uint16_t tap_hold_keycode,
                     keyrecord_t *tap_hold_record,
                     uint16_t other_keycode,
                     keyrecord_t *other_record) {
    switch (tap_hold_keycode) {
        case LALT_T(KC_R):
        case LALT_T(KC_S):
            if (other_keycode == KC_TAB) return true;
            break;
    }

    return achordion_opposite_hands(tap_hold_record, other_record);
}

static void render_luna(int LUNA_X, int LUNA_Y) {
    /* Sit */
    static const char PROGMEM sit[2][ANIM_SIZE] = {/* 'sit1', 32x22px */
                                                   {
                                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1c, 0x02, 0x05, 0x02, 0x24, 0x04, 0x04, 0x02, 0xa9, 0x1e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x08, 0x68, 0x10, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x82, 0x7c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0c, 0x10, 0x10, 0x20, 0x20, 0x20, 0x28, 0x3e, 0x1c, 0x20, 0x20, 0x3e, 0x0f, 0x11, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   },

                                                   /* 'sit2', 32x22px */
                                                   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1c, 0x02, 0x05, 0x02, 0x24, 0x04, 0x04, 0x02, 0xa9, 0x1e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x90, 0x08, 0x18, 0x60, 0x10, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0x82, 0x7c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0c, 0x10, 0x10, 0x20, 0x20, 0x20, 0x28, 0x3e, 0x1c, 0x20, 0x20, 0x3e, 0x0f, 0x11, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* Walk */
    static const char PROGMEM walk[2][ANIM_SIZE] = {/* 'walk1', 32x22px */
                                                    {
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x10, 0x90, 0x90, 0x90, 0xa0, 0xc0, 0x80, 0x80, 0x80, 0x70, 0x08, 0x14, 0x08, 0x90, 0x10, 0x10, 0x08, 0xa4, 0x78, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x18, 0xea, 0x10, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1c, 0x20, 0x20, 0x3c, 0x0f, 0x11, 0x1f, 0x03, 0x06, 0x18, 0x20, 0x20, 0x3c, 0x0c, 0x12, 0x1e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    },

                                                    /* 'walk2', 32x22px */
                                                    {
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x20, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x28, 0x10, 0x20, 0x20, 0x20, 0x10, 0x48, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x20, 0xf8, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x30, 0xd5, 0x20, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x20, 0x30, 0x0c, 0x02, 0x05, 0x09, 0x12, 0x1e, 0x02, 0x1c, 0x14, 0x08, 0x10, 0x20, 0x2c, 0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    }};

    /* Run */
    static const char PROGMEM run[2][ANIM_SIZE] = {/* 'run1', 32x22px */
                                                   {
                                                       0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x08, 0x08, 0xc8, 0xb0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x3c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xc4, 0xa4, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc8, 0x58, 0x28, 0x2a, 0x10, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x09, 0x04, 0x04, 0x04, 0x04, 0x02, 0x03, 0x02, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   },

                                                   /* 'run2', 32x22px */
                                                   {
                                                       0x00, 0x00, 0x00, 0xe0, 0x10, 0x10, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x78, 0x28, 0x08, 0x10, 0x20, 0x30, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0xb0, 0x50, 0x55, 0x20, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x1e, 0x20, 0x20, 0x18, 0x0c, 0x14, 0x1e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   }};

    /* Bark */
    static const char PROGMEM bark[2][ANIM_SIZE] = {/* 'bark1', 32x22px */
                                                    {
                                                        0x00, 0xc0, 0x20, 0x10, 0xd0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x3c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc8, 0x48, 0x28, 0x2a, 0x10, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    },

                                                    /* 'bark2', 32x22px */
                                                    {
                                                        0x00, 0xe0, 0x10, 0x10, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x40, 0x2c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x48, 0x28, 0x2a, 0x10, 0x0f, 0x20, 0x4a, 0x09, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    }};

    /* Sneak */
    static const char PROGMEM sneak[2][ANIM_SIZE] = {/* 'sneak1', 32x22px */
                                                     {
                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x40, 0x40, 0x80, 0x00, 0x80, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x21, 0xf0, 0x04, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x04, 0x04, 0x04, 0x03, 0x01, 0x00, 0x00, 0x09, 0x01, 0x80, 0x80, 0xab, 0x04, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1c, 0x20, 0x20, 0x3c, 0x0f, 0x11, 0x1f, 0x02, 0x06, 0x18, 0x20, 0x20, 0x38, 0x08, 0x10, 0x18, 0x04, 0x04, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,
                                                     },

                                                     /* 'sneak2', 32x22px */
                                                     {
                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xa0, 0x20, 0x40, 0x80, 0xc0, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x41, 0xf0, 0x04, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x04, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40, 0x40, 0x55, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x20, 0x30, 0x0c, 0x02, 0x05, 0x09, 0x12, 0x1e, 0x04, 0x18, 0x10, 0x08, 0x10, 0x20, 0x28, 0x34, 0x06, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                     }};

    /* animation */
    void animate_luna(void) {
        /* jump */
        if (luna_status.isJumping || !luna_status.showedJump) {
            /* clear */
            oled_set_cursor(LUNA_X, LUNA_Y + 2);
            oled_write("     ", false);

            oled_set_cursor(LUNA_X, LUNA_Y - 1);

            luna_status.showedJump = true;
        } else {
            /* clear */
            oled_set_cursor(LUNA_X, LUNA_Y - 1);
            oled_write("     ", false);

            oled_set_cursor(LUNA_X, LUNA_Y);
        }

        /* switch frame */
        current_frame = (current_frame + 1) % 2;

        /* current status */
        if (led_usb_state.caps_lock) {
            oled_write_raw_P(bark[current_frame], ANIM_SIZE);
        } else if (luna_status.isSneaking) {
            oled_write_raw_P(sneak[current_frame], ANIM_SIZE);
        } else if (current_wpm <= MIN_WALK_SPEED) {
            oled_write_raw_P(sit[current_frame], ANIM_SIZE);
        } else if (current_wpm <= MIN_RUN_SPEED) {
            oled_write_raw_P(walk[current_frame], ANIM_SIZE);
        } else {
            oled_write_raw_P(run[current_frame], ANIM_SIZE);
        }
    }

    /* animation timer */
    if (timer_elapsed32(anim_timer) > ANIM_FRAME_DURATION) {
        anim_timer = timer_read32();
        animate_luna();
    }
}

static void print_status_narrow(void) {
    // Create OLED content
    oled_advance_page(true);
    oled_write_P(PSTR("Shige"), false);
    oled_advance_page(true);
    oled_advance_page(true);

    // Print current layer
    oled_write_P(PSTR("Layer"), false);
    switch (currentLayer) {
        case 0:
            oled_write_P(PSTR("-Clmk\n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 1:
            oled_write_P(PSTR("-Qwer\n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 2:
            oled_write_P(PSTR("-Symb\n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 3:
            oled_write_P(PSTR("-Nav \n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 4:
            oled_write_P(PSTR("-Num \n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 5:
            oled_write_P(PSTR("-RGB \n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 6:
            oled_write_P(PSTR("-Game\n"), false);
            oled_write_P(PSTR("*****\n"), false);
            break;
        case 7:
            oled_write_P(PSTR("-Game\n"), false);
            oled_write_P(PSTR("*Base\n"), false);
            break;
        case 8:
            oled_write_P(PSTR("-Game\n"), false);
            oled_write_P(PSTR("*VALO\n"), false);
            break;
        case 9:
            oled_write_P(PSTR("-Game\n"), false);
            oled_write_P(PSTR("*EFT \n"), false);
            break;
        case 10:
            oled_write_P(PSTR("-Game\n"), false);
            oled_write_P(PSTR("*Cust \n"), false);
            break;
        case 11:
            oled_write_P(PSTR("-11   \n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 12:
            oled_write_P(PSTR("-12   \n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 13:
            oled_write_P(PSTR("-13  \n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 14:
            oled_write_P(PSTR("-14  \n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        case 15:
            oled_write_P(PSTR("-15  \n"), false);
            oled_write_P(PSTR("     \n"), false);
            break;
        default:
            oled_write_P(PSTR("Undef"), false);
            oled_write_P(PSTR("     \n"), false);
    }

    /* wpm counter */
    uint8_t n = current_wpm;
    char    wpm_str[4];
    oled_set_cursor(0, 10);

    oled_write(" ", false);

    wpm_str[3] = '\0';
    wpm_str[2] = '0' + n % 10;
    wpm_str[1] = '0' + (n /= 10) % 10;
    wpm_str[0] = '0' + n / 10;

    oled_write(wpm_str, false);

    oled_set_cursor(0, 11);
    oled_write(" wpm ", false);

    render_luna(0, 13);
}

oled_rotation_t oled_init_kb(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

bool oled_task_user(void) {
    current_wpm = get_current_wpm();
    led_usb_state = host_keyboard_led_state();

    if (!is_keyboard_master()) {
        if (!is_oled_enabled) {
            oled_off();
            return false;
        } else {
            oled_on();
        }

        print_status_narrow();
    }

    return false;
}

void user_sync_a_slave_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    const sync_luna_status_t *m2s = (const sync_luna_status_t *)in_data;

    luna_status.isJumping = m2s->isJumping;
    luna_status.showedJump = m2s->showedJump;
    luna_status.isSneaking = m2s->isSneaking;
}

void keyboard_post_init_user(void) {
    luna_status.isJumping = false;
    luna_status.showedJump = true;
    luna_status.isSneaking = false;

    transaction_register_rpc(USER_SYNC_A, user_sync_a_slave_handler);
}

void housekeeping_task_user(void) {
    currentLayer = get_highest_layer(layer_state);
    if (is_keyboard_master()) {
        if (!isSynced) {
            if (transaction_rpc_send(USER_SYNC_A, sizeof(luna_status), &luna_status)) {
                isSynced = true;

                luna_status.isJumping = false;
                luna_status.showedJump = true;
                luna_status.isSneaking = false;
            }
        }
    } else {
        is_oled_enabled = (bool)(last_input_activity_elapsed() < 60000);
    }
}

// Input / matrix processing
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_LCTL:
        case KC_RCTL:
            if (record->event.pressed) {
                luna_status.isSneaking = true;
                isSynced = false;
            } else {
                luna_status.isSneaking = false;
                isSynced = false;
            }
            break;
        case LT(3, KC_SPC):
        case KC_SPC:
            if (record->event.pressed) {
                luna_status.isJumping  = true;
                luna_status.showedJump = false;
                isSynced = false;
            } else {
                luna_status.isJumping = false;
                isSynced = false;
            }
            break;
        case SWITCH_COLEMAK:
            if (record->event.pressed) {
                if (currentLayer != 0) {
                    lastLayer = currentLayer;
                    layer_clear();
                    layer_on(0);
                } else if (lastLayer != 0) {
                    layer_on(lastLayer);
                }
            }
            break;
    }

    if (!process_achordion(keycode, record)) {
        return false;
    }

    return true;
}

void matrix_scan_user(void) {
    achordion_task();
}
