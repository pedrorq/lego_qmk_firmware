# access

![access](imgur.com image replace me!)

*A short description of the keyboard/project*

* Keyboard Maintainer: [Pablo Martinez](https://github.com/elpekenin)
* Hardware Supported: The PCBs use RP2040s
* Hardware Availability: https://github.com/elpekenin/access_kb

Make example for this keyboard (after setting up your build environment):

    make elpekenin/access:default
    qmk compile -kb elpekenin/access -km default

Flashing:

    Put your board in bootloader mode and copy the `.uf2` file on the drive that appears on your file explorer

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 4 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (Escape) and plug in the keyboard
* **Physical reset button**: Hold the button on the back of the PCB and plug in the keyboard
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available

## Custom features
### 1. QP over XAP
`Automatically added if you have both features enabled`
This feature allows the control of your Quantum Painter display over XAP.
If you want to add this on your own branch/fork/keyboard, you'd need:
- Start from the XAP branch (or checkout the needed files)
- Add a `xap.hjson` file to your keyboard or keymap-level folder (easier if you copy mine), so the new routes are added
- Small patches at `qp.h`, `qp.c`, `qp_draw_image.c` and `qp_draw_image.c` so your displays, images and fonts are stored in an array and can later be indexed
- Add `qp_xap.c` according to your `.hjson` definitions, to create the functions that get called upon receiving the new routes' messages
- You can test it using the `qp_xap.py` script

#### :warning:  Notes :warning:
- I couldn't think of a decent way of init'ing displays, or loading fonts/images over XAP, so that has to be done on the firmware code
- If you use a different `hjson` you'd need to change the id's on the Python script, so they run the appropiate function
    - 1st byte being a 2 means you are using a keyboard-level file
    - 1st byte being a 3 means keymap-level file

### 1. One hand mode
:exclamation: :exclamation: This is more a proof of concept than a tested feature.

It uses a custom RGB_MATRIX animation, so that only a key is turned on. This marker moves when using a joystick so you can select a key, and then input it by pressing the joystick's button.
