# ergoknife2

![ergoknife2](https://i.imgur.com/JwhiE9ll.png)
![ergoknife2PCB](https://i.imgur.com/6NjZ8Cql.jpg)

*A pre-built, split mechanical keyboard with 70 keys based on the RP2040 processor*

* Keyboard Maintainer: [ouer555](https://github.com/ouser555)
* Hardware Supported: *ergoknife split ergonomic keyboard*

Make example for this keyboard (after setting up your build environment):

```sh
    make ergoknife2:via
```

Flashing example for this keyboard:

```sh
    make ergoknife2:via:flash
```
See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Features
- Highly integrated design with microcontroller, USB and TRRS connectors, and OLED all integrated on the main board with no secondary modules
- RP2040 processor by Raspberry Pi Foundation running at 130 MHz with 16MB flash memory
- 128x32 OLED on each keyboard half
- 70 per-key RGB LEDs are individually addressable
- USB C host connection
- Hot swap connectors for MX switches

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the upper left key on the left hand keyboard half or the upper right key on the right hand keyboard half while plugging in USB
* **Physical reset button**: Press the RST button twice, rapidly
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available