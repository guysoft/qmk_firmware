/* Copyright 2025 Epomaker
 * Copyright 2025 Epomaker <https://github.com/Epomaker>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../../../lib/rdr_lib/rdr_common.h"

void matrix_io_delay(void) {
}

void matrix_output_select_delay(void) {
}

void matrix_output_unselect_delay(uint8_t line, bool key_pressed) {
}

led_config_t g_led_config = { {
    { 0        , 1        , 2        , 3        , 4        , 5        , 6        , 7        , 8        , 9        , 10       , 11       , 12       , 13       , 81       , NO_LED   },
	{ 14       , 15       , 16       , 17       , 18       , 19       , 20       , 21       , 22       , 23       , 24       , 25       , 26       , 27       , 28       , NO_LED   },
	{ 29       , 30       , 31       , 32       , 33       , 34       , 35       , 36       , 37       , 38       , 39       , 40       , 41       , 42       , 43       , NO_LED   },
	{ 44       , 45       , 46       , 47       , 48       , 49       , 50       , 51       , 52       , 53       , 54       , 55       , NO_LED   , 56       , 57       , 71       },
	{ 58       , NO_LED   , 59       , 60       , 61       , 62       , 63       , 64       , 65       , 66       , 67       , 68       , NO_LED   , 69       , 70       , NO_LED   },
	{ 72       , 73       , 74       , NO_LED   , NO_LED   , 75       , NO_LED   , NO_LED   , NO_LED   , 76       , 77       , NO_LED   , NO_LED   , 78       , 79       , 80       }
},{
    { 0,  10},  { 17, 10}, { 32, 10}, { 47, 10}, { 62, 10}, { 79, 10}, { 94, 10}, {109, 10}, {124, 10}, { 141, 10}, { 156, 10}, { 171, 10}, { 186, 10}, { 206, 10},
    { 0,  20},  { 15, 20}, { 30, 20}, { 45, 20}, { 60, 20}, { 75, 20}, { 90, 20}, {105, 20}, {120, 20}, { 135, 20}, { 150, 20}, { 165, 20}, { 180, 20}, { 200, 20},             { 224, 20},            
    { 4,  30},  { 20, 30}, { 35, 30}, { 50, 30}, { 65, 30}, { 80, 30}, { 95, 30}, {110, 30}, {125, 30}, { 140, 30}, { 155, 30}, { 170, 30}, { 185, 30},             { 204, 30}, { 224, 30}, 
    { 6,  40},  { 24, 40}, { 39, 40}, { 54, 40}, { 69, 40}, { 84, 40}, { 99, 40}, {114, 40}, {129, 40}, { 144, 40}, { 159, 40}, { 174, 40},             { 199, 40},             { 224, 40},
    { 8,  50},             { 28, 50}, { 43, 50}, { 58, 50}, { 73, 50}, { 88, 50}, {103, 50}, {118, 50}, { 133, 50}, { 148, 50}, { 163, 50},             { 183, 50}, { 203, 50}, { 224, 50},
    { 0,  60},  { 20, 60}, { 40, 60},                       { 90, 60},                                  { 145, 60}, { 165, 60},                         { 188, 60}, { 203, 60}, { 224, 60}, { 224, 10}
}, {
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,      1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,      1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,      1,      1,
    1,      1,  1,  1,  1,  1,  1,  1,  1,  1,  1,      1,  1,  1,
    1,  1,  1,          1,              1,  1,          1,  1,  1,  1
} };

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    User_Led_Show();
    return false;
}

void notify_usb_device_state_change_user(enum usb_device_state usb_device_state)  {
    if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        if(usb_device_state == USB_DEVICE_STATE_CONFIGURED) {
            Usb_If_Ok = true;
            Usb_If_Ok_Led = true;
            Usb_If_Ok_Delay = 0;
        } else {
            Usb_If_Ok = false;
		    Usb_If_Ok_Led = false;
        }
    } else {
        Usb_If_Ok = false;
	    Usb_If_Ok_Led = false;
    }
}

void housekeeping_task_user(void) {
    es_chibios_user_idle_loop_hook();
    User_Keyboard_Reset();
}

void board_init(void) {
    User_Keyboard_Init();
}

void keyboard_post_init_user(void) {
    User_Keyboard_Post_Init();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    Usb_Change_Mode_Delay = 0;
    Usb_Change_Mode_Wakeup = false;

    Uart_Key_Mode_Key_Scan(keycode, record->event.pressed);

    if (Test_Led) {
        if ((keycode != KC_SPC) && (keycode != MO(2)) && (keycode != MO(3)) && (keycode != KC_LCTL)) {
            Test_Led = false;
        }
    }

    return User_process_record_user(keycode, record);
}


