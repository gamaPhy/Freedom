# firmware

## TODO

- [ ] switch inputs to analog
- [ ] calibrate keys
- [x] create settings menu
  - [x] save/load settings
  - [x] actuation settings
  - [x] rgb settings
- [ ] usb wakeup (any other usb callbacks like mount, suspend, resume, etc?)
- [x] tactile buttons
  - [x] activate internal pull ups on GPIO pins
- [ ] add "hold" feature in software for buttons while in menu
- [x] refactor buttons into arrays/enum/structs so we can loop through them
- [x] look into weird bug where the first boot after a new .uf2 update starts in the menu for some reason and sometimes freezes while leaving the menu

## stretch

- [ ] add oled effects while using the keypad
- [ ] cute minigame(s)
- [ ] web serial FW flash
