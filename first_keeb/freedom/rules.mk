WS2812_DRIVER = vendor
EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = legacy

CONSOLE_ENABLE = yes
DEFAULT_FOLDER = magkey/freedom/3k

RGBLIGHT_DRIVER = WS2812
WS2812_DRIVER = pwm

SRC += matrix.c
SRC += analog.c
