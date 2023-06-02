CUSTOM_MATRIX = lite
SRC += matrix.c graphics.c lightbulb_off_outline.qgf.c
WS2812_DRIVER = vendor        #
QUANTUM_LIB_SRC += spi_master.c
QUANTUM_PAINTER_DRIVERS += ssd1680_spi
