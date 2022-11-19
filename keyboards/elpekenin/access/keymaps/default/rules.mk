# POINTING_DEVICE_DRIVER = analog_joystick
# POINTING_DEVICE_ENABLE = yes

XAP_ENABLE = yes

# Custom features
ONE_HAND_MODE = no
TOUCH_SCREEN = yes
QP_XAP = yes

# ------------------ Extra logic ------------------
# Doesn't seem to work if added at keyboard level
# New code
VPATH += keyboards/elpekenin/access/code \
         keyboards/elpekenin/access/code/generated

# QP resources
ifeq ($(strip $(XAP_ENABLE)), yes)
    SRC += graphics.c \
           fira_code.qff.c \
           rp2040.qgf.c
endif

# Custom features
ifeq ($(strip $(POINTING_DEVICE_ENABLE)), yes)
    ifeq ($(strip $(ONE_HAND_MODE)), yes)
        OPT_DEFS += -DONE_HAND_MODE
    endif
endif

ifeq ($(strip $(TOUCH_SCREEN)), yes)
    OPT_DEFS += -DTOUCH_SCREEN
    SRC += touch_driver.c
endif

ifeq ($(strip $(QP_XAP)), yes)
    OPT_DEFS += -DQP_XAP
    SRC += qp_xap.c
endif
