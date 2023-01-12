VPATH += $(USER_PATH)/painter \
         $(USER_PATH)/painter/images \
         $(USER_PATH)/painter/fonts \
         $(USER_PATH)/registers \
         $(USER_PATH)/spi \
         $(USER_PATH)/touch


ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)  # check if QP is enabled
    mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))  # current path

    SRC += graphics.c  # include all fonts/images & defines `load_qp_resources` to read them
    SRC += $(shell find $(dir $(mkfile_path))/painter/images -name "*.c")  # add images
    SRC += $(shell find $(dir $(mkfile_path))/painter/fonts -name "*.c")  # add fonts

    ifeq ($(strip $(XAP_ENABLE)), yes)  # if XAP is also enabled
        QP_XAP ?= yes
        ifeq ($(strip $(QP_XAP)), yes)  # check if user wanted to disable QP over XAP
            OPT_DEFS += -DQP_XAP
            SRC += qp_over_xap.c
        endif
    endif
endif
QP_XAP ?= no

# ===========================
# ===== Custom features =====
# ===========================
ONE_HAND ?= no
ifeq ($(strip $(ONE_HAND)), yes)
    OPT_DEFS += -DONE_HAND_ENABLE
endif

REGISTER_PINS ?= no
ifeq ($(strip $(REGISTER_PINS)), yes)
    OPT_DEFS += -DSIPO_PINS
    SRC += sipo_pins.c

    # needs a second SPI driver to work, currently not supported on QMK
    SRC += custom_spi_master.c
endif

TOUCH_SCREEN ?= no
ifeq ($(strip $(TOUCH_SCREEN)), yes)
    QUANTUM_LIB_SRC += spi_master.c
    OPT_DEFS += -DTOUCH_SCREEN
    SRC += touch_driver.c
endif

$(info --- Custom features ---)
$(info ONE_HAND      = $(ONE_HAND))
$(info REGISTER_PINS = $(REGISTER_PINS))
$(info TOUCH_SCREEN  = $(TOUCH_SCREEN))
$(info QP_XAP        = $(QP_XAP))
$(info -----------------------)
$(info )
