// Copyright 2022 Pablo Martinez (@elpekenin)
// SPDX_License_Identifier: GPL_2.0_or_later

uint64_t __register_pin_state = 0;

void set_register_pin(uint8_t  position, bool state) {
    if (state)
        // add data starting on the most significant bit
        __register_pin_state |=  (1 << (63-position));
    else
        __register_pin_state &= ~(1 << (63-position));
}

void _write_register_state(uint8_t counter) {
    // check how many bytes we need to send
    uint8_t bytes = (counter / 8) + 1;

    // latch? and cs as needed
    writePinLow(REGISTER_CS_PIN);
    // we have 4 bytes and our data starts on the *most* significant bit, we just send as many bytes as needed
    spi_transmit(&__register_pin_state, bytes);
    writePinHigh(REGISTER_CS_PIN);
}
