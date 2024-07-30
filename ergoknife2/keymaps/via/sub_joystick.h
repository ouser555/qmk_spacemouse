/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
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

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"

#ifndef ANALOG_JOYSTICK_X_AXIS_PIN
#    error No pin specified for X Axis
#endif
#ifndef ANALOG_JOYSTICK_Y_AXIS_PIN
#    error No pin specified for Y Axis
#endif

#ifndef ANALOG_JOYSTICK_AXIS_MIN
#    define ANALOG_JOYSTICK_AXIS_MIN 0
#endif
#ifndef ANALOG_JOYSTICK_AXIS_MAX
#    define ANALOG_JOYSTICK_AXIS_MAX 1023
#endif
#ifndef ANALOG_JOYSTICK_SPEED_REGULATOR
#    define ANALOG_JOYSTICK_SPEED_REGULATOR 15
#endif
#ifndef ANALOG_JOYSTICK_READ_INTERVAL
#    define ANALOG_JOYSTICK_READ_INTERVAL 10
#endif
#ifndef ANALOG_JOYSTICK_SPEED_MAX
#    define ANALOG_JOYSTICK_SPEED_MAX 2
#endif

#if 0
typedef struct {
    int8_t x;
    int8_t y;
    bool   button;
} report_sub_joystick_t;
#else
typedef struct {
    int8_t x1;
    int8_t y1;
    int8_t x2;
    int8_t y2;
    bool   button;
} report_sub_joystick_t;
#endif

//report_sub_joystick_t sub_joystick_read(void);
//report_sub_joystick_t sub_joystick_trans(int8_t Xcoordinate,int8_t Ycoordinate);
report_sub_joystick_t sub_joystick_trans(int8_t Xcoordinate1,int8_t Ycoordinate1,int8_t Xcoordinate2,int8_t Ycoordinate2);
void                     sub_joystick_init(void);
int8_t subaxisCoordinate(pin_t pin, uint16_t origin);
