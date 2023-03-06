# analog osu keypad

(image)

(short decription)

* Keyboard Maintainer: [Thailer](https://github.com/ThailerL) & [Theodore](https://github.com/tlietz) Lietz
* Hardware Supported: analog osu keypad (link to hardware repo) (RP2040)
* Hardware Availability: (link to store)

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 2 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (usually the top left key or Escape) and plug in the keyboard
* **Physical reset button**: Hold down the button on the front of the PCB while plugging in the keyboard. Some may have pads you must short instead.
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
