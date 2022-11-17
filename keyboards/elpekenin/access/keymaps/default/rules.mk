VPATH += keyboards/elpekenin/access/code \
         keyboards/elpekenin/access/code/generated

XAP_ENABLE = yes

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
SRC += fira_code.qff.c \
       graphics.c \
       rp2040.qgf.c

ifeq ($(strip $(XAP_ENABLE)), yes)
SRC += qp_xap.c
endif

endif

