# Set of custom features

## One Hand Mode
**WIP** The goal for this feature is to add a new *RGB Matrix* animation which only lights a single LED, used as a "marker" so that you can then virtually press the selected key. This will allow for accessibility, because the direction in which the LED moves and the trigger of the press is completely customizable (code does just the bare minimum), thus you can change which events trigger moving around and pressing, eg using different pointing devices, or a set of keys(arrows).

Config:
 - Add `ONE_HAND = yes` to your **rules.mk**

## Quantum Painter over XAP
XAP bindings that expose Quantum Painter's drawing and `get_geometry` functions.

Config:
 - Will automatically get added if you have both XAP and Quantum Painter enabled
 - You can disable it when both features are enabled by adding `QP_XAP = no`

I have a collection of converted Material Design Icons in [this repo](https://github.com/elpekenin/mdi-icons-qgf)

## Touch screen driver
Custom code to interact with my XPT2046-based touchscreen modules, the code is designed such that other SPI sensors should be somewhat easy to integrate. In a similar philosophy to the one-hand mode, this code does the bare minimum: **read the sensor**, what to do based on it... up to your imagination.

Config:
 - Add `TOUCH_SCREEN = yes` to your **rules.mk**

## Shift register "pins"
**WIP** A set of macros and functions that allow to easily use a SerialIn-ParallelOut(SIPO) shift register to control several signals (in my use case, several screens' CS and RST pins) using a single GPIO on the MCU - namely the CS to control the register.
The status is stored on `uint8_t[]`, so chaining an arbitrary amount registers may also work with current code but hasn't been tested.

How to use:
 - Add `REGISTER_PINS = yes` to your **rules.mk**
 - Configure the register's CS pin by adding `#define REGISTER_CS_PIN <Pin>`, eg: `GP10` on a RP2040
 - Configure the amount of pins you'll use `#define REGISTER_PINS <N_Pins>`
 - Create your pin(s) name(s) using: `CONFIGURE_REGISTER_PINS(NAME1, NAME2, ...)`
 - Change a pin's state by doing:
   - Manually set state: `set_register_pin(<pin_name>, true)` or `set_register_pin(<pin_name>, false)`
   - Using helper macros: `register_pin_high(<pin_name>)` or `register_pin_low(<pin_name>)`
 - Send the status to the registers: `write_register_state()`
