/*
 * Copyright 2022 Kevin Gee <info@controller.works>
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
#include QMK_KEYBOARD_H

//#include "pmw3610.h"
#include "analog.h"
#include "pointing_device.h"
//#include "./drivers/sensors/adns5050.h"
#include "./drivers/sensors/analog_joystick.h"
#include "sub_joystick.h"
//#include "transactions.h"
//#include "print.h"
#include "azoteq_iqs5xx.h"

enum layers {
    _BASE = 0,
    _LOWER,
    _RAISE,
    _ADJUST,
    _EXTRA01,
    _EXTRA02
};

enum custom_keycodes {
#ifdef VIA_ENABLE
  //QWERTY = USER00,
  BASE = QK_KB_0,
  //QWERTY = CS00, // qmk v1.1.1
#else
  QWERTTY = SAFE_RANGE,
#endif
  LOWER,
  RAISE,
  ADJUST,
  EXTRA01,
  EXTRA02,
  MJMODE,
  SJMODE,
  JOYRST
};

enum jmodes {
    _MOUSE = 0,
    _SCROLL,
    _WASD,
    _UWASD,
    _JOYSL,
    _JOYSR,
    _JOYSZ,
};

enum zmodes {
    _ZZOOM = 0,
    _ZVOLUM,
    _ZCUSTOM,
    _Z3DMOUS,
    _ZDISABLE,
};

typedef union {
  uint32_t raw;
  struct {
    uint8_t MjMode;
    uint8_t SjMode;
  };
} user_config_t;

user_config_t user_config;
uint8_t MjMode;
uint8_t SjMode;

typedef struct {
    int8_t jdx;
    int8_t jdy;
} report_jdelta_t;

report_jdelta_t master;
report_jdelta_t slave;


#if 0
void joysitck_mode_master_INC(void) {
  if(MjMode == _JOYSZ) {
    MjMode = _MOUSE;
  }else{
    MjMode++;
    if(MjMode == SjMode && MjMode > _UWASD){
      if(MjMode == _JOYSZ) {
        MjMode = _MOUSE;
      }else{
        MjMode++;
      }
    }
  }
}

void joysitck_mode_slave_INC(void) {
  if(SjMode == _JOYSZ) {
    SjMode = _MOUSE;
  }else{
    SjMode++;
    if(MjMode == SjMode && SjMode > _UWASD){
      if(SjMode == _JOYSZ) {
        SjMode = _MOUSE;
      }else{
        SjMode++;
      }
    }
  }
}
#else

void joysitck_mode_master_INC(void) {
  if(MjMode == _JOYSZ) {
    MjMode = _MOUSE;
  }else{
    MjMode++;
  }
}

void zoom_mode_slave_INC(void){
  if(SjMode == _ZDISABLE) {
    SjMode = _ZZOOM;
  }else{
    SjMode++;
  }
}
#endif

int16_t xOrigin, yOrigin;
#if 0
int16_t XUpThreshold = 0;
int16_t XDownThreshold = 0;
int16_t YUpThreshold = 0;
int16_t YDownThreshold = 0;
#endif

void JoyOriginReset(void){
  xOrigin = analogReadPin(JSH);
  yOrigin = analogReadPin(JSV);
  //XUpThreshold   = xOrigin - _DEAD;
  //XDownThreshold = xOrigin + _DEAD;
  //YUpThreshold   = yOrigin - _DEAD;
  //YDownThreshold = yOrigin + _DEAD;
}

#define TRACKPAD_THREE_FINGER_CLICK_TIME 300 //300ms

#define SCROLL_DIVISOR_H 8.0
#define SCROLL_DIVISOR_V 8.0

#define EISU LALT(KC_GRV)

#define CONSTRAIN_HID(amt) ((amt) < INT8_MIN ? INT8_MIN : ((amt) > INT8_MAX ? INT8_MAX : (amt)))
#define CONSTRAIN_HID_XY(amt) ((amt) < XY_REPORT_MIN ? XY_REPORT_MIN : ((amt) > XY_REPORT_MAX ? XY_REPORT_MAX : (amt)))

static i2c_status_t azoteq_iqs5xx_init_status = 1;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  /* Qwerty
   * ,----------------------------------------------------------------------------------------------------------------------.
   * | ESC  |   1  |   2  |   3  |   4  |   5  |   [  |                    |   ]  |   6  |   7  |   8  |   9  |   0  |Pscree|
   * |------+------+------+------+------+------+------+--------------------+------+------+------+------+------+------+------|
   * |  `   |   Q  |   W  |   E  |   R  |   T  |   -  |                    |   =  |   Y  |   U  |   I  |   O  |   P  |  \   |
   * |------+------+------+------+------+------+------+--------------------+------+------+------+------+------+------+------|
   * | Tab  |   A  |   S  |   D  |   F  |   G  |  Del |                    | Bksp |   H  |   J  |   K  |   L  |   ;  |  "   |
   * |------+------+------+------+------+------+---------------------------+------+------+------+------+------+------+------|
   * | Shift|   Z  |   X  |   C  |   V  |   B  | Space|                    | Enter|   N  |   M  |   ,  |   .  |   /  | Shift|
   * |-------------+------+------+------+------+------+------+------+------+------+------+------+------+------+-------------|
   * | Ctrl |  GUI |  ALt | EISU |||||||| Lower| Space|  Del |||||||| Bksp | Enter| Raise|||||||| Left | Down |  Up  | Right|
   * ,----------------------------------------------------------------------------------------------------------------------.
   */
  [_BASE] = LAYOUT_7x5x2(
    KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_LBRC,                        KC_RBRC, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_PSCR,
    KC_GRV,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_MINS,                        KC_EQL , KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLS,
    KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_DEL,                         KC_BSPC, KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_SPC,                         KC_ENT , KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
    KC_LCTL, KC_LGUI, KC_LALT, EISU,             LOWER,   KC_SPC , KC_DEL,        KC_BSPC,KC_ENT , RAISE,            KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT,
    _______, _______,    KC_X, _______, _______, _______,_______,                         _______, _______, _______, _______, _______, _______, _______
  ),

  /* Lower
   * ,----------------------------------------------------------------------------------------------------------------------.
   * |  F11 |  F1  |  F2  |  F3  |  F4  |  F5  |   {  |                    |   }  |  F6  |  F7  |  F8  |  F9  |  F10 |  F12 |
   * |------+------+------+------+------+------+------+--------------------+------+------+------+------+------+------+------|
   * |  ~   |   !  |   @  |   #  |   $  |   %  |   _  |                    |   +  |   ^  |   &  |   *  |   (  |   )  |  |   |
   * |------+------+------+------+------+------+------+--------------------+------+------+------+------+------+------+------|
   * | Tab  |   1  |   2  |   3  |   4  |   5  |  Del |                    | Bksp |   H  |   J  |   K  |   L  |   :  |  "   |
   * |------+------+------+------+------+------+---------------------------+------+------+------+------+------+------+------|
   * | Shift|   6  |   7  |   8  |   9  |   0  | Space|                    | Enter|   N  |   M  |   <  |   >  |   ?  | Shift|
   * |-------------+------+------+------+------+------+------+------+------+------+------+------+------+------+-------------|
   * | Ctrl |  GUI |  ALt | EISU |||||||| Lower| Space|  Del |||||||| Bksp | Enter| Raise|||||||| Home |PageDn|PageUp|  End |
   * ,----------------------------------------------------------------------------------------------------------------------.
   */
  [_LOWER] = LAYOUT_7x5x2(
    KC_F11,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_LCBR,                        KC_RCBR, KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F12,
    KC_TILD, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC, KC_UNDS,                        KC_PLUS, KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_PIPE,
    KC_TAB,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_DEL,                         KC_BSPC, KC_H,    KC_J,    KC_K,    KC_L,    KC_COLN, KC_DQT ,
    KC_LSFT, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_SPC,                         KC_ENT , KC_N,    KC_M,    KC_LT,   KC_GT,   KC_QUES, KC_RSFT,
    QK_BOOT, MJMODE,  SJMODE,  EISU,             LOWER,   KC_SPC ,KC_DEL,         KC_BSPC,KC_ENT , RAISE,            KC_HOME, KC_PGDN, KC_PGUP, KC_END,
    _______, _______, _______, _______, _______, _______,_______,                         _______, _______, _______, _______, _______, _______, _______
  ),

  /* Raise
   * ,----------------------------------------------------------------------------------------------------------------------.
   * |  F11 |  F1  |  F2  |  F3  |  F4  |  F5  |   {  |                    |   }  |  F6  |  F7  |  F8  |  F9  |  F10 |  F12 |
   * |------+------+------+------+------+------+------+--------------------+------+------+------+------+------+------+------|
   * |  ~   |   !  |   @  |   #  |   $  |   %  |   _  |                    |   +  |   ^  |   &  |   *  |   (  |   )  |  |   |
   * |------+------+------+------+------+------+------+--------------------+------+------+------+------+------+------+------|
   * | Tab  |   1  |   2  |   3  |   4  |   5  |  Del |                    | Bksp |   H  |   J  |   K  |   L  |   :  |  "   |
   * |------+------+------+------+------+------+---------------------------+------+------+------+------+------+------+------|
   * | Shift|   6  |   7  |   8  |   9  |   0  | Space|                    | Enter|   N  |   M  |   <  |   >  |   ?  | Shift|
   * |-------------+------+------+------+------+------+------+------+------+------+------+------+------+------+-------------|
   * | Ctrl |  GUI |  ALt | EISU |||||||| Lower| Space|  Del |||||||| Bksp | Enter| Raise|||||||| Home |PageDn|PageUp|  End |
   * ,----------------------------------------------------------------------------------------------------------------------.
   */
  [_RAISE] = LAYOUT_7x5x2(
    KC_F11,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_LCBR,                        KC_RCBR, KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F12,
    KC_TILD, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC, KC_UNDS,                        KC_PLUS, KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_PIPE,
    KC_TAB,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_DEL,                         KC_BSPC, KC_H,    KC_J,    KC_K,    KC_L,    KC_COLN, KC_DQT ,
    KC_LSFT, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_SPC,                         KC_ENT , KC_N,    KC_M,    KC_LT,   KC_GT,   KC_QUES, KC_RSFT,
    KC_LCTL, KC_LGUI, KC_LALT, EISU,             LOWER,   KC_SPC ,KC_DEL,         KC_BSPC,KC_ENT , RAISE,            KC_HOME, KC_PGDN, KC_PGUP, KC_END,
    _______, _______, _______, _______, _______, _______,_______,                         _______, _______, _______, _______, _______, _______, _______
  ),

  /* Adjust
   * ,----------------------------------------------------------------------------------------------------------------------.
   * |      |      |      |      |      |      |      |                    |      |      |      |      |      |      |      |
   * |------+------+------+------+------+------+---------------------------+------+------+------+------+------+------+------|
   * |      | Reset|RGB ON|  MODE|  HUE-|  HUE+|      |                    |      |  SAT-|  SAT+|  VAL-|  VAL+|      |      |
   * |------+------+------+------+------+------+---------------------------+------+------+------+------+------+------+------|
   * |      |      | BL ON|  BRTG|  INC|   DEC|      |                    |      |      |      |      |      |      |      |
   * |------+------+------+------+------+------+---------------------------+------+------+------+------+------+------+------|
   * |      |      |      |      |      |      |      |                    |      |      |      |      |      |      |      |
   * |-------------+------+------+------+------+------+------+------+------+------+------+------+------+------+-------------|
   * |      |      |      |      ||||||||      |      |      ||||||||      |      |      ||||||||      |      |      |      |
   * ,----------------------------------------------------------------------------------------------------------------------.
   */
  [_ADJUST] = LAYOUT_7x5x2(
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, QK_BOOT, RGB_TOG, RGB_MOD, RGB_HUD, RGB_HUI,_______,                       _______, RGB_SAD, RGB_SAI, RGB_VAD, RGB_VAI, _______, _______,
    _______, _______, BL_TOGG, BL_BRTG, BL_UP  , BL_DOWN,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______,          _______,_______,_______,       _______,_______, _______,          _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______
  ),

  [_EXTRA01] = LAYOUT_7x5x2(
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, QK_BOOT, RGB_TOG, RGB_MOD, RGB_HUD, RGB_HUI,_______,                       _______, RGB_SAD, RGB_SAI, RGB_VAD, RGB_VAI, _______, _______,
    _______, _______, BL_TOGG, BL_BRTG, BL_UP  , BL_DOWN,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______,          _______,_______,_______,       _______,_______, _______,          _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______
  ),
  [_EXTRA02] = LAYOUT_7x5x2(
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, QK_BOOT, RGB_TOG, RGB_MOD, RGB_HUD, RGB_HUI,_______,                       _______, RGB_SAD, RGB_SAI, RGB_VAD, RGB_VAI, _______, _______,
    _______, _______, BL_TOGG, BL_BRTG, BL_UP  , BL_DOWN,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______,          _______,_______,_______,       _______,_______, _______,          _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,_______,                       _______, _______, _______, _______, _______, _______, _______
  )
};

#if 1
layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}
#else
void persistent_default_layer_set(uint16_t default_layer) {
  eeconfig_update_default_layer(default_layer);
  default_layer_set(default_layer);
}
#endif

#if 1
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case BASE:
      if (record->event.pressed) {
         //print("mode just switched to qwerty and this is a huge string\n");
        set_single_persistent_default_layer(_BASE);
      }
      return false;
      //break;
    case LOWER:
      if (record->event.pressed) {
        layer_on(_LOWER);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      } else {
        layer_off(_LOWER);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      }
      return false;
      //break;
    case RAISE:
      if (record->event.pressed) {
        layer_on(_RAISE);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      } else {
        layer_off(_RAISE);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      }
      return false;
      //break;
    case ADJUST:
      if (record->event.pressed) {
        layer_on(_ADJUST);
      } else {
        layer_off(_ADJUST);
      }
      return false;
      //break;

    case MJMODE:
      if (record->event.pressed) {
        joysitck_mode_master_INC();
        user_config.MjMode = MjMode;
        eeconfig_update_user(user_config.raw); 
      }
      //dprintf("mjoymode \n");
      return false;

    case SJMODE:
      if (record->event.pressed) {
        //joysitck_mode_slave_INC();
        zoom_mode_slave_INC();
        user_config.SjMode = SjMode;
        eeconfig_update_user(user_config.raw); 
      }
      return false;

    case JOYRST:
      if (record->event.pressed) {
        JoyOriginReset();
      }
      return false;

  }
  return true;
}
#endif

#ifdef JOYSTICK_ENABLE

int16_t xPos = 0;
int16_t yPos = 0;

//bool wasdShiftMode = false;
//bool autorun = false;

bool LyDownHeld = false;
bool LyUpHeld = false;
bool LxLeftHeld = false;
bool LxRightHeld = false;
//bool LshiftHeld = false;

bool RyDownHeld = false;
bool RyUpHeld = false;
bool RxLeftHeld = false;
bool RxRightHeld = false;
//bool RshiftHeld = false;



void matrix_scan_user(void) {
  if(is_keyboard_master()){
#if 1
    // left
    if(MjMode == _SCROLL) {

        if(!LyUpHeld    && master.jdy <=  - _TRESHOLD){
          LyUpHeld = true;
          register_code(KC_WH_U);
        }else if(LyUpHeld && master.jdy > - _TRESHOLD){
          LyUpHeld = false;
          unregister_code(KC_WH_U);
        }else if(!LyDownHeld  && master.jdy >= _TRESHOLD){
          LyDownHeld = true;
          register_code(KC_WH_D);
        }else if(LyDownHeld && master.jdy < _TRESHOLD){
          LyDownHeld = false;
          unregister_code(KC_WH_D);
        }
        // x axis
        if(!LxLeftHeld  && master.jdx <=  - _TRESHOLD){
          LxLeftHeld = true;
          register_code(KC_WH_L);
        }else if(LxLeftHeld && master.jdx > - _TRESHOLD){
          LxLeftHeld = false;
          unregister_code(KC_WH_L);
        }else if(!LxRightHeld && master.jdx >= _TRESHOLD){
          LxRightHeld = true;
          register_code(KC_WH_R);
        }else if(LxRightHeld && master.jdx < _TRESHOLD){
          LxRightHeld = false;
          unregister_code(KC_WH_R);
        }

    }else if(MjMode == _WASD){

        if(!LyUpHeld    && master.jdy <=  - _TRESHOLD){
          LyUpHeld = true;
          register_code(KC_W);
        }else if(LyUpHeld && master.jdy > - _TRESHOLD){
          LyUpHeld = false;
          unregister_code(KC_W);
        }else if(!LyDownHeld  && master.jdy >= _TRESHOLD){
          LyDownHeld = true;
          register_code(KC_S);
        }else if(LyDownHeld && master.jdy < _TRESHOLD){
          LyDownHeld = false;
          unregister_code(KC_S);
        }
        // x axis
        if(!LxLeftHeld  && master.jdx <= - _TRESHOLD){
          LxLeftHeld = true;
          register_code(KC_A);
        }else if(LxLeftHeld && master.jdx > - _TRESHOLD){
          LxLeftHeld = false;
          unregister_code(KC_A);
        }else if(!LxRightHeld && master.jdx >= _TRESHOLD){
          LxRightHeld = true;
          register_code(KC_D);
        }else if(LxRightHeld && master.jdx < _TRESHOLD){
          LxRightHeld = false;
          unregister_code(KC_D);
        }

    }else if (MjMode == _UWASD){

        if(!LyUpHeld    && master.jdy <= - _TRESHOLD){
          LyUpHeld = true;
          register_code(dynamic_keymap_get_keycode(biton32(layer_state),5,0));
        }else if(LyUpHeld && master.jdy > - _TRESHOLD){
          LyUpHeld = false;
          unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),5,0));
        }else if(!LyDownHeld  && master.jdy >= _TRESHOLD){
          LyDownHeld = true;
          register_code(dynamic_keymap_get_keycode(biton32(layer_state),5,4));
        }else if(LyDownHeld && master.jdy < _TRESHOLD){
          LyDownHeld = false;
          unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),5,4));
        }
        // x axis
        if(!LxLeftHeld  && master.jdx <= - _TRESHOLD){
          LxLeftHeld = true;
          register_code(dynamic_keymap_get_keycode(biton32(layer_state),5,1));
        }else if(LxLeftHeld && master.jdx > - _TRESHOLD){
          LxLeftHeld = false;
          unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),5,1));
        }else if(!LxRightHeld && master.jdx >= _TRESHOLD){
          LxRightHeld = true;
          register_code(dynamic_keymap_get_keycode(biton32(layer_state),5,3));
        }else if(LxRightHeld && master.jdx < _TRESHOLD){
          LxRightHeld = false;
          unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),5,3));
        }

    }else if(MjMode == _JOYSL){
          joystick_set_axis( 0, master.jdx*2048/100);
          joystick_set_axis( 1, master.jdy*2048/100);
    }else if(MjMode == _JOYSR){
          joystick_set_axis( 3, master.jdx*2048/100);
          joystick_set_axis( 4, master.jdy*2048/100);
#if 1          
    }else if(MjMode == _JOYSZ){
          joystick_set_axis( 2, master.jdx*2048/100);
          joystick_set_axis( 5, master.jdy*2048/100);
#else
    }else if(MjMode == _JOYSZ){
          report_3dmouse1_t report3dm1;
          report_3dmouse2_t report3dm2;


#if 1
          report3dm1.x = master.jdx;
          report3dm1.y = master.jdy;
          report3dm1.z = 0;

          report3dm2.roll = 0;
          report3dm2.pitch = 0;
          report3dm2.yaw = 0;
#else
          report3dm1.x = 0;
          report3dm1.y = 0;
          report3dm1.z = 0;
          
          report3dm2.roll = 0;
          report3dm2.pitch = master.jdy;
          report3dm2.yaw = -master.jdx;

          //left_report.x = 0;
          //left_report.y = 0;
          //right_report.x = 0;
          //right_report.y = 0;
#endif
          void host_3Dmouse1_send(report_3dmouse1_t *report);
          void host_3Dmouse2_send(report_3dmouse2_t *report);
          host_3Dmouse1_send(&report3dm1);
          host_3Dmouse2_send(&report3dm2);
#endif
#if 0
    }else if(MjMode == 3DPAN){
          
          report_3dmouse1_t report3dm1;

          report3dm1.x = master.jdx*2048/100;
          report3dm1.y = master.jdy*2048/100;
          report3dm1.z = 0;

          //left_report.x = 0;
          //left_report.y = 0;
          //right_report.x = 0;
          //right_report.y = 0;

          void host_3Dmouse1_send(report_3dmouse1_t *report);
          host_3Dmouse1_send(&report3dm1);
          
#endif
    //----------------------mouse mode------------------------------
    }else if(MjMode == _MOUSE){

    }
#endif
#if 0
    // right
    if(SjMode == _SCROLL) {

        if(!RyUpHeld    && slave.jdy <=  - _TRESHOLD){
          RyUpHeld = true;
          register_code(KC_WH_U);
        }else if(RyUpHeld && slave.jdy > - _TRESHOLD){
          RyUpHeld = false;
          unregister_code(KC_WH_U);
        }else if(!RyDownHeld  && slave.jdy >= _TRESHOLD){
          RyDownHeld = true;
          register_code(KC_WH_D);
        }else if(RyDownHeld && slave.jdy < _TRESHOLD){
          RyDownHeld = false;
          unregister_code(KC_WH_D);
        }
        // x axis
        if(!RxLeftHeld  && slave.jdx <=  - _TRESHOLD){
          RxLeftHeld = true;
          register_code(KC_WH_L);
        }else if(RxLeftHeld && slave.jdx > - _TRESHOLD){
          RxLeftHeld = false;
          unregister_code(KC_WH_L);
        }else if(!RxRightHeld && slave.jdx >= _TRESHOLD){
          RxRightHeld = true;
          register_code(KC_WH_R);
        }else if(RxRightHeld && slave.jdx < _TRESHOLD){
          RxRightHeld = false;
          unregister_code(KC_WH_R);
        }

    }else if(SjMode == _WASD){

        if(!RyUpHeld    && slave.jdy <=  - _TRESHOLD){
          RyUpHeld = true;
          register_code(KC_W);
        }else if(RyUpHeld && slave.jdy > - _TRESHOLD){
          RyUpHeld = false;
          unregister_code(KC_W);
        }else if(!RyDownHeld  && slave.jdy >= _TRESHOLD){
          RyDownHeld = true;
          register_code(KC_S);
        }else if(RyDownHeld && slave.jdy < _TRESHOLD){
          RyDownHeld = false;
          unregister_code(KC_S);
        }
        // x axis
        if(!RxLeftHeld  && slave.jdx <= - _TRESHOLD){
          RxLeftHeld = true;
          register_code(KC_A);
        }else if(RxLeftHeld && slave.jdx > - _TRESHOLD){
          RxLeftHeld = false;
          unregister_code(KC_A);
        }else if(!RxRightHeld && slave.jdx >= _TRESHOLD){
          RxRightHeld = true;
          register_code(KC_D);
        }else if(RxRightHeld && slave.jdx < _TRESHOLD){
          RxRightHeld = false;
          unregister_code(KC_D);
        }

    }else if (SjMode == _UWASD){

        if(!RyUpHeld    && slave.jdy <= - _TRESHOLD){
          RyUpHeld = true;
          register_code(dynamic_keymap_get_keycode(biton32(layer_state),11,0));
        }else if(RyUpHeld && slave.jdy > - _TRESHOLD){
          RyUpHeld = false;
          unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),11,0));
        }else if(!RyDownHeld  && slave.jdy >= _TRESHOLD){
          RyDownHeld = true;
          register_code(dynamic_keymap_get_keycode(biton32(layer_state),11,4));
        }else if(RyDownHeld && slave.jdy < _TRESHOLD){
          RyDownHeld = false;
          unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),11,4));
        }
        // x axis
        if(!RxLeftHeld  && slave.jdx <= - _TRESHOLD){
          RxLeftHeld = true;
          register_code(dynamic_keymap_get_keycode(biton32(layer_state),11,3));
        }else if(RxLeftHeld && slave.jdx > - _TRESHOLD){
          RxLeftHeld = false;
          unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),11,3));
        }else if(!RxRightHeld && slave.jdx >= _TRESHOLD){
          RxRightHeld = true;
          register_code(dynamic_keymap_get_keycode(biton32(layer_state),11,1));
        }else if(RxRightHeld && slave.jdx < _TRESHOLD){
          RxRightHeld = false;
          unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),11,1));
        }

    }else if(SjMode == _JOYSL){
          joystick_set_axis( 0, slave.jdx*2048/100);
          joystick_set_axis( 1, slave.jdy*2048/100);
    }else if(SjMode == _JOYSR){
          joystick_set_axis( 3, slave.jdx*2048/100);
          joystick_set_axis( 4, slave.jdy*2048/100);
    }else if(SjMode == _JOYSZ){
          joystick_set_axis( 2, slave.jdx*2048/100);
          joystick_set_axis( 5, slave.jdy*2048/100);
    //----------------------mouse mode------------------------------
    }else if(SjMode == _MOUSE){

    }
#endif
  }
}

  //joystick config
  joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL
    //JOYSTICK_AXIS_IN(JSH, _MAX, _CENTER, _MIN),
    //JOYSTICK_AXIS_IN(JSV, _MIN, _CENTER, _MAX)  
  };
#endif // joystick

void pointing_device_driver_init(void) {

  if (is_keyboard_master()){
    analog_joystick_init();
    //sub_joystick_init();
    //xOrigin = analogReadPin(JSH);
    //yOrigin = analogReadPin(JSV);
    JoyOriginReset();
    //pmw3610_sync();
    //pmw3610_init();
    //adns5050_sync();
    //adns5050_init();
  }else{
    i2c_init();
    azoteq_iqs5xx_wake();
    azoteq_iqs5xx_reset_suspend(true, false, true);
    wait_ms(100);
    azoteq_iqs5xx_wake();
    if (azoteq_iqs5xx_get_product() != AZOTEQ_IQS5XX_UNKNOWN) {
        azoteq_iqs5xx_setup_resolution();
        azoteq_iqs5xx_init_status = azoteq_iqs5xx_set_report_rate(AZOTEQ_IQS5XX_REPORT_RATE, AZOTEQ_IQS5XX_ACTIVE, false);
        azoteq_iqs5xx_init_status |= azoteq_iqs5xx_set_event_mode(false, false);
        azoteq_iqs5xx_init_status |= azoteq_iqs5xx_set_reati(true, false);
#if 0
        azoteq_iqs5xx_init_status |= azoteq_iqs5xx_setCh();
#endif
#if 1
        azoteq_iqs5xx_init_status |= ChannelSetupPXM0002();
#endif

#    if defined(AZOTEQ_IQS5XX_ROTATION_90)
        azoteq_iqs5xx_init_status |= azoteq_iqs5xx_set_xy_config(false, true, true, true, false);
#    elif defined(AZOTEQ_IQS5XX_ROTATION_180)
        azoteq_iqs5xx_init_status |= azoteq_iqs5xx_set_xy_config(true, true, false, true, false);
#    elif defined(AZOTEQ_IQS5XX_ROTATION_270)
        azoteq_iqs5xx_init_status |= azoteq_iqs5xx_set_xy_config(true, false, true, true, false);
#    else
        azoteq_iqs5xx_init_status |= azoteq_iqs5xx_set_xy_config(false, false, false, true, false);
#    endif
        azoteq_iqs5xx_init_status |= azoteq_iqs5xx_set_gesture_config(true);
        wait_ms(AZOTEQ_IQS5XX_REPORT_RATE + 1);
    }
  }
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report){

  if (is_keyboard_master()){

     mouse_report.x = subaxisCoordinate(JSH, xOrigin);
     mouse_report.y = subaxisCoordinate(JSV, yOrigin);
     return mouse_report;

  }else{

    report_mouse_t temp_report           = {0};    
    static uint8_t previous_button_state = 0;
    static uint8_t read_error_count      = 0;


    static uint32_t threeFingerPressTime = 0;
    static bool  threeFingersPressed = false;


    if (azoteq_iqs5xx_init_status == I2C_STATUS_SUCCESS) {
        azoteq_iqs5xx_base_data_t base_data = {0};
#    if !defined(POINTING_DEVICE_MOTION_PIN)
        azoteq_iqs5xx_wake();
#    endif
        i2c_status_t status          = azoteq_iqs5xx_get_base_data(&base_data);
        bool         ignore_movement = false;

        if (status == I2C_STATUS_SUCCESS) {
            // pd_dprintf("IQS5XX - previous cycle time: %d \n", base_data.previous_cycle_time);
            read_error_count = 0;
            if (base_data.gesture_events_0.single_tap || base_data.gesture_events_0.press_and_hold) {
                //pd_dprintf("IQS5XX - Single tap/hold.\n");
                temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON1);
            } else if (base_data.gesture_events_1.two_finger_tap) {
#if 1
                //pd_dprintf("IQS5XX - Two finger tap.\n");
                temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON2);
#else              
                if(base_data.number_of_fingers == 0){
                  //pd_dprintf("IQS5XX - Two finger tap.\n");
                  temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON2);
                //}else if(base_data.number_of_fingers == 1){
                }else{
                  //pd_dprintf("IQS5XX - Three finger tap.\n");
                  temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON3);
                }
#endif
            } else if (base_data.gesture_events_0.swipe_x_neg) {
                //pd_dprintf("IQS5XX - X-.\n");
                temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON4);
                ignore_movement     = true;
            } else if (base_data.gesture_events_0.swipe_x_pos) {
                //pd_dprintf("IQS5XX - X+.\n");
                temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON5);
                ignore_movement     = true;
            } else if (base_data.gesture_events_0.swipe_y_neg) {
                //pd_dprintf("IQS5XX - Y-.\n");
                temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON6);
                ignore_movement     = true;
            } else if (base_data.gesture_events_0.swipe_y_pos) {
                //pd_dprintf("IQS5XX - Y+.\n");
                temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON7);
                ignore_movement     = true;
            } else if (base_data.gesture_events_1.zoom) {
#if 1
                temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON8); //zoom flag
                //ignore_movement     = true;
                temp_report.x = CONSTRAIN_HID_XY(AZOTEQ_IQS5XX_COMBINE_H_L_BYTES(base_data.x.h, base_data.x.l));
                temp_report.y = CONSTRAIN_HID_XY(AZOTEQ_IQS5XX_COMBINE_H_L_BYTES(base_data.y.h, base_data.y.l));
#else              
                if (AZOTEQ_IQS5XX_COMBINE_H_L_BYTES(base_data.x.h, base_data.x.l) < 0) {
                    //pd_dprintf("IQS5XX - Zoom out.\n");
                    //temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON7);
                    temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON8);
                    //tap_code(KC_PGUP);
                    //tap_code(KC_VOLU);
                } else if (AZOTEQ_IQS5XX_COMBINE_H_L_BYTES(base_data.x.h, base_data.x.l) > 0) {
                    //pd_dprintf("IQS5XX - Zoom in.\n");
                    //temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON8);
                    temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON8);
                    //tap_code(KC_PGDN);
                    //tap_code(KC_VOLD);
                }
#endif
            } else if (base_data.gesture_events_1.scroll) {
                //pd_dprintf("IQS5XX - Scroll.\n");
                temp_report.h = CONSTRAIN_HID(AZOTEQ_IQS5XX_COMBINE_H_L_BYTES(base_data.x.h, base_data.x.l));
                temp_report.v = CONSTRAIN_HID(AZOTEQ_IQS5XX_COMBINE_H_L_BYTES(base_data.y.h, base_data.y.l));
            }
            if (base_data.number_of_fingers == 1 && !ignore_movement) {
                temp_report.x = CONSTRAIN_HID_XY(AZOTEQ_IQS5XX_COMBINE_H_L_BYTES(base_data.x.h, base_data.x.l));
                temp_report.y = CONSTRAIN_HID_XY(AZOTEQ_IQS5XX_COMBINE_H_L_BYTES(base_data.y.h, base_data.y.l));
#if 0
            }
#else
            }else if(base_data.number_of_fingers == 3 && !threeFingersPressed){
                threeFingerPressTime = timer_read32();
                threeFingersPressed = true;
            }

            if(base_data.number_of_fingers == 0){
                if(threeFingersPressed && timer_elapsed32(threeFingerPressTime) > TRACKPAD_THREE_FINGER_CLICK_TIME) {
                    //pd_dprintf("IQS5XX - Three finger tap.\n");
                    temp_report.buttons = pointing_device_handle_buttons(temp_report.buttons, true, POINTING_DEVICE_BUTTON3);
                }
                threeFingersPressed = false;
                //threeFingerPressTime = 0;
            }
#endif

            previous_button_state = temp_report.buttons;

        } else {
            if (read_error_count > 10) {
                read_error_count      = 0;
                previous_button_state = 0;
            } else {
                read_error_count++;
            }
            temp_report.buttons = previous_button_state;
            //pd_dprintf("IQS5XX - get report failed: %d \n", status);
        }
    } else {
        //pd_dprintf("IQS5XX - Init failed: %d \n", azoteq_iqs5xx_init_status);
    }

    //return temp_report;
    mouse_report = temp_report;
    return mouse_report;
  }
}


#if 1
report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {

    void host_3Dmouse1_send(report_3dmouse1_t *report);
    void host_3Dmouse2_send(report_3dmouse2_t *report);
    report_sub_joystick_t data;
    report_3dmouse1_t report3dm1;
    report_3dmouse2_t report3dm2;
    static float scroll_accumulated_h = 0;
    static float scroll_accumulated_v = 0;
    uint8_t sbutton;

    master.jdx = left_report.x;
    master.jdy = -left_report.y;

    left_report.x = 0;
    left_report.y = 0;
    //right_report.x = 0;
    //right_report.y = 0;

    if(SjMode == _Z3DMOUS){
      report3dm2.roll = -right_report.y * 20;
      report3dm2.pitch = 0;
      report3dm2.yaw = -right_report.x * 20;

      report3dm1.x = right_report.h * 10;
      report3dm1.y = -right_report.v * 10;
      report3dm1.z = 0;
      
      if(right_report.buttons & MOUSE_BTN8){
        report3dm1.z = right_report.x * 256 * 2;
      }

      right_report.h = 0;
      right_report.v = 0;
      right_report.x = 0;
      right_report.y = 0;
      
      host_3Dmouse1_send(&report3dm1);
      host_3Dmouse2_send(&report3dm2);

    }else{
      slave.jdx = right_report.x;
      slave.jdy = -right_report.y;

      scroll_accumulated_h += (float)right_report.h / SCROLL_DIVISOR_H;
      scroll_accumulated_v += (float)right_report.v / SCROLL_DIVISOR_V;

      right_report.h = (int8_t)scroll_accumulated_h;
      right_report.v = (int8_t)scroll_accumulated_v;

      scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
      scroll_accumulated_v -= (int8_t)scroll_accumulated_v;
    }    

    data = sub_joystick_trans( master.jdx, master.jdy, slave.jdx, slave.jdy) ;

    if(MjMode == _MOUSE){
      left_report.x = data.x1;
      left_report.y = data.y1;
    }
#if 0
    if(SjMode == _Z3DMOUS){

      report3dm1.x = right_report.h * 10;
      report3dm1.y = -right_report.v * 10;
      report3dm1.z = 0;

      right_report.h = 0;
      right_report.v = 0;

    }else{

      scroll_accumulated_h += (float)right_report.h / SCROLL_DIVISOR_H;
      scroll_accumulated_v += (float)right_report.v / SCROLL_DIVISOR_V;

      right_report.h = (int8_t)scroll_accumulated_h;
      right_report.v = (int8_t)scroll_accumulated_v;

      scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
      scroll_accumulated_v -= (int8_t)scroll_accumulated_v;
    }
#endif
    

    //void host_3Dmouse1_send(report_3dmouse1_t *report);
    //void host_3Dmouse2_send(report_3dmouse2_t *report);
    //host_3Dmouse1_send(&report3dm1);
    //host_3Dmouse2_send(&report3dm2);



#if 0
    if(right_report.h !=0 || right_report.v != 0){
      right_report.x = right_report.h;
      right_report.y = right_report.v;
      register_code(KC_LSFT);
      wait_us(800);
      //key_timer = timer_read32();
      //timer_elapsed32(key_timer);
      register_code(KC_BTN3);
      //register_code16(S(KC_BTN3));
    }else{
      unregister_code(KC_LSFT);
      unregister_code(KC_BTN3);
      //unregister_code16(S(KC_BTN3));
    }

#endif



    //sbutton = right_report.buttons &(MOUSE_BTN4|MOUSE_BTN5|MOUSE_BTN6|MOUSE_BTN7|MOUSE_BTN8);
    sbutton = right_report.buttons &(MOUSE_BTN3|MOUSE_BTN4|MOUSE_BTN5|MOUSE_BTN6|MOUSE_BTN7|MOUSE_BTN8);


    if(sbutton & MOUSE_BTN4){
      tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,3));
#if 1      
    }else if(sbutton & MOUSE_BTN3){ 
      tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,2));
#endif
    }else if(sbutton & MOUSE_BTN5){ 
      tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,1));
    }else if(sbutton & MOUSE_BTN6){ 
      tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,4));
    }else if(sbutton & MOUSE_BTN7){ 
      tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,0));
    }else if(sbutton & MOUSE_BTN8){
#if 1
      if(right_report.x < 0){

        switch(SjMode){
          case _ZZOOM:
            tap_code16(LCTL(KC_PMNS));
            //report3dm1.z = right_report.x;
            break;
          case _ZVOLUM:
            tap_code(KC_VOLD);
            break;
          case _ZCUSTOM:
            tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,6));
            //tap_code16(dynamic_keymap_get_keycode(biton32(layer_state),11,6));
            break;
          case _Z3DMOUS:
            //tap_code(KC_WH_U);
            //report3dm1.z = right_report.x * 256;
            break;
          case _ZDISABLE:
          default:
            break;
        }
        //tap_code(KC_VOLD);
        //tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,6));
        //tap_code16(dynamic_keymap_get_keycode(biton32(layer_state),11,6));
        //tap_code16(LCTL(KC_PMNS));
      } else if(right_report.x > 0){

        switch(SjMode){
          case _ZZOOM:
            tap_code16(LCTL(KC_PPLS));
            //report3dm1.z = right_report.x;
            break;
          case _ZVOLUM:
            tap_code(KC_VOLU);
            break;
          case _ZCUSTOM:
            tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,5));
            //tap_code16(dynamic_keymap_get_keycode(biton32(layer_state),11,6));
            break;
          case _Z3DMOUS:
            //tap_code(KC_WH_D);
            //report3dm1.z = right_report.x * 256;
            break;
          case _ZDISABLE:
          default:
            break;
        }
        //tap_code(KC_VOLU);
        //tap_code(dynamic_keymap_get_keycode(biton32(layer_state),11,5));
        //tap_code16(dynamic_keymap_get_keycode(biton32(layer_state),11,5));
        //tap_code16(LCTL(KC_PPLS));

      }
#else
        report3dm1.z = right_report.x * 256;

#endif

      //right_report.x = 0;
      //right_report.y = 0;
    }

#if 0
    if(SjMode == _Z3DMOUS){
      right_report.x = 0;
      right_report.y = 0;
      void host_3Dmouse1_send(report_3dmouse1_t *report);
      void host_3Dmouse2_send(report_3dmouse2_t *report);
      host_3Dmouse1_send(&report3dm1);
      host_3Dmouse2_send(&report3dm2);
    }
#endif

    //sbutton = (MOUSE_BTN4|MOUSE_BTN5|MOUSE_BTN6|MOUSE_BTN7|MOUSE_BTN8);
    
    if(SjMode == _Z3DMOUS){
      right_report.buttons = 0b00000000; //disable all
    }else{
      sbutton = (MOUSE_BTN3|MOUSE_BTN4|MOUSE_BTN5|MOUSE_BTN6|MOUSE_BTN7|MOUSE_BTN8);
      right_report.buttons &= ~sbutton;
    }

    //if(SjMode == _MOUSE){
      //right_report.x = data.x2;
      //right_report.y = data.y2;
      right_report.x =  right_report.x;
      right_report.y = -right_report.y;

#if 1
      //right_report.h = right_report.h/2;
      //right_report.v = right_report.v/2;
#else      
      if(abs(right_report.h) < 4 && abs(right_report.h) > 1){
        if(right_report.h > 0){
          right_report.h =  1;
        }else{
          right_report.h = -1;
        }        
      }else{
        right_report.h = right_report.h/4;
      }
      
      if(abs(right_report.v) < 4 && abs(right_report.v) > 1){
        if(right_report.v > 0){
          right_report.v =  1;
        }else{
          right_report.v = -1;
        }  
      }else{
        right_report.v = right_report.v/4;
      }
#endif

    //}

#if 0
      report_3dmouse1_t report3dm1;

      report3dm1.x = left_report.x;
      report3dm1.y = left_report.y;
      report3dm1.z = 0;

      left_report.x = 0;
      left_report.y = 0;
      //right_report.x = 0;
      //right_report.y = 0;

      void host_3Dmouse1_send(report_3dmouse1_t *report);
      host_3Dmouse1_send(&report3dm1);
      

#endif

    return pointing_device_combine_reports(left_report, right_report);
}
#endif

#ifdef ENCODER_ENABLE

#if 0
//#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise)
{
    if(clockwise){
      tap_code(KC_AUDIO_VOL_UP);
    }else{
      tap_code(KC_AUDIO_VOL_DOWN);
    }
    return true;
}
#else
bool encoder_update_user(uint8_t index, bool clockwise) {
	if(index == 0){
		if(clockwise){
			tap_code(dynamic_keymap_get_keycode(biton32(layer_state),0,3));
		}else{
			tap_code(dynamic_keymap_get_keycode(biton32(layer_state),1,3));
		}
    }
    return true;
}
#endif

#endif

#ifdef OLED_ENABLE

#if 0
static void render_logo(void) {
    static const char PROGMEM raw_logo[] = {
        // '115', 128x32px
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 
0xe0, 0xe0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x80, 
0xe0, 0xe0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 
0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x87, 0x07, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
0x03, 0x03, 0x03, 0x03, 0x03, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0f, 0x0f, 0x0f, 0x0f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x03, 
0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x07, 0x07, 
0x07, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}
#endif

#if 1
static void render_status(void) {
    // Host Keyboard Layer Status    
    oled_write_P(PSTR("-----LAYER"), false);
    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR(" BAS "), false);
            break;
        case _LOWER:
            oled_write_P(PSTR(" LWR "), false);
            break;
        case _RAISE:
            oled_write_P(PSTR(" RIS "), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR(" ADJ "), false);
            break;
        default:
            oled_write_P(PSTR(" UDF "), false);
    }
    oled_write_P(PSTR("-----STATS-----"), false);

    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.num_lock ? PSTR("NUM:@") : PSTR("NUM:_"), false);
    oled_write_P(led_state.caps_lock ? PSTR("CAP:@") : PSTR("CAP:_"), false);
    oled_write_P(led_state.scroll_lock ? PSTR("SCR:@") : PSTR("SCR:_"), false);
    //oled_write_P(PSTR("-----JMODLEFT "), false);
    oled_write_P(PSTR("-----MJMOD"), false);
    //oled_write_P(PSTR("LEFT "), false);
    switch(MjMode){
      case _MOUSE:
        oled_write_P(PSTR("MOUSE"), false);
        break;
      case _SCROLL:
        oled_write_P(PSTR("SCROL"), false);
        break;
      case _JOYSL:
        oled_write_P(PSTR("JOYSL"), false);
        break;
      case _JOYSR:
        oled_write_P(PSTR("JOYSR"), false);
        break;
      case _JOYSZ:
        oled_write_P(PSTR("JOYSZ"), false);
        break;
      case _WASD:
        oled_write_P(PSTR(" WASD"), false);
        break;
      case _UWASD:
        oled_write_P(PSTR("UWASD"), false);
        break;
      default:
        oled_write_P(PSTR(" XXX "), false);
        break;
    }

    oled_write_P(PSTR("-----SJMOD"), false);
    switch(SjMode){
#if 0
      case _MOUSE:
        oled_write_P(PSTR("MOUSE"), false);
        break;
      case _SCROLL:
        oled_write_P(PSTR("SCROL"), false);
        break;
      case _JOYSL:
        oled_write_P(PSTR("JOYSL"), false);
        break;
      case _JOYSR:
        oled_write_P(PSTR("JOYSR"), false);
        break;
      case _JOYSZ:
        oled_write_P(PSTR("JOYSZ"), false);
        break;
      case _WASD:
        oled_write_P(PSTR(" WASD"), false);
        break;
      case _UWASD:
        oled_write_P(PSTR("UWASD"), false);
        break;
      default:
        oled_write_P(PSTR(" XXX "), false);
        break;
#else
      case _ZZOOM:
        oled_write_P(PSTR(" ZOOM"), false);
        break;
      case _ZVOLUM:
        oled_write_P(PSTR("VOLUM"), false);
        break;
      case _ZCUSTOM:
        oled_write_P(PSTR("CUSTM"), false);
        break;
      case _Z3DMOUS:
        oled_write_P(PSTR("3DMUS"), false);
        break;
      case _ZDISABLE:
        oled_write_P(PSTR("DISAB"), false);
        break;
      default:
        oled_write_P(PSTR(" XXX "), false);
        break;
#endif
    }
    oled_write_P(PSTR("-----"), false);
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;  // flips the display 270 degrees if offhand
    }else{ // slaver
      //oled_scroll_set_speed(0);
    }
    return rotation;
}


bool oled_task_kb(void) {
  if (!oled_task_user()) { return false; }
    if (is_keyboard_left()) {
        render_status(); // Renders the current keyboard state (layer, lock, caps, scroll, etc)
    } else {
        //render_logo();
        //oled_scroll_right();
    }
    return true;
}
#endif
#endif



#if 1
void eeconfig_init_user(void) {  // EEPROM is getting reset!

  //bool dpi_change_f = false;
  user_config.raw = 0;

  user_config.raw = eeconfig_read_user();
  if(user_config.MjMode > _UWASD || user_config.MjMode < 0 ){
    user_config.MjMode = _MOUSE;
    //dpi_change_f = true;
  }
  if(user_config.SjMode > _UWASD || user_config.SjMode < 0 ){
    user_config.SjMode = _MOUSE;
    //dpi_change_f = true;
  }
  //jMode = user_config.jMode;

}
#endif

#if 1
void matrix_init_user(void){
# ifdef CONSOLE_ENABLE
  dprintf("init MS XY transform value \n");
# endif
  eeconfig_init_user();
}

#endif

void keyboard_post_init_user(void) {

    user_config.raw = eeconfig_read_user();
    MjMode = user_config.MjMode;
    SjMode = user_config.SjMode;

}