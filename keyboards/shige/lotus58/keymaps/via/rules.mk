# Enabling via
VIA_ENABLE = yes

# Features
ENCODER_MAP_ENABLE = yes
OLED_ENABLE = yes
WPM_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
KEY_INTERRUPT_ENABLE = yes

# Debouncing
DEBOUNCE_TYPE = asym_eager_defer_pk

# Achordion mod
SRC += features/achordion.c
