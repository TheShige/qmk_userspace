# Features
ENCODER_MAP_ENABLE = yes
OLED_ENABLE = yes
WPM_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes

# Debouncing
DEBOUNCE_TYPE = asym_eager_defer_pk

# RP2040 Community Edition converter for RP2040 Pro Micro clones
CONVERT_TO = rp2040_ce

# Achordion mod
SRC += features/achordion.c
