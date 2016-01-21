//*****************************************************************************
//  Copyright 2012 - 2015 Paul Chote
//  This file is part of vaisalasim, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

static uint16_t rand_value;

ISR(WDT_vect)
{
    uint8_t rnd = TCNT2;

    // Rotate the existing value a random number of places
    uint8_t shift = rnd & 0x0F;
    rand_value = (rand_value << shift) | (rand_value >> 16 - shift);

    // Mix new entropy with the previous value
    rand_value ^= rnd;
}

void rng_init()
{
    // Use the watchdog timer to trigger an interrupt every 16ms.
    // This interval is measured using a separate oscillator to the main clock
    // and so we can use the relative clock skew (via timer2) to generate a
    // random bit.
    MCUSR = 0;
    WDTCSR |= _BV(WDCE) | _BV(WDE);
    WDTCSR = _BV(WDIE);
    TCCR2B = _BV(CS20);

    // Initialize with the current value of TCNT2
    rand_value = TCNT2;
}

uint16_t rng_next()
{
    // Incrememnt value using a simple LCG to ensure repeated calls
    // return different values
    rand_value *= 48271;
    return rand_value;
}