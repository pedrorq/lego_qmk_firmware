# POINTING_DEVICE_DRIVER = analog_joystick
# POINTING_DEVICE_ENABLE = yes

XAP_ENABLE = yes

# Custom features
ONE_HAND_MODE = no
TOUCH_SCREEN = yes
QP_XAP = no

# ------------------ Extra logic ------------------
# Doesn't seem to work if added at keyboard level
# New code
VPATH += keyboards/elpekenin/access/code \
         keyboards/elpekenin/access/code/generated

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    # QP resources
    SRC += graphics.c \
           fira_code.qff.c \
           rp2040.qgf.c

    # QP over XAP
    ifeq ($(strip $(XAP_ENABLE)), yes)
        ifeq ($(strip $(QP_XAP)), yes)
            OPT_DEFS += -DQP_XAP
            SRC += qp_xap.c
        endif
    endif
endif

# Custom features
ifeq ($(strip $(POINTING_DEVICE_ENABLE)), yes)
    ifeq ($(strip $(ONE_HAND_MODE)), yes)
        OPT_DEFS += -DONE_HAND_MODE
    endif
endif

ifeq ($(strip $(TOUCH_SCREEN)), yes)
    QUANTUM_LIB_SRC += spi_master.c
    OPT_DEFS += -DTOUCH_SCREEN
    SRC += touch_driver.c
endif

