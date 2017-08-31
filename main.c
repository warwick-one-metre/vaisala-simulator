//*****************************************************************************
//  Copyright 2012 - 2015 Paul Chote
//  This file is part of vaisalasim, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "rng.h"
#include "measurement.h"

static uint8_t output_buffer[256];
static volatile uint8_t output_read = 0;
static volatile uint8_t output_write = 0;

static float wrap_degrees(float angle)
{
    while (angle > 360)
        angle -= 360;
    
    while (angle < 0)
        angle += 360;
    
    return angle;
}

// Add a byte to the send buffer.
// Will block if the buffer is full
void queue_byte(uint8_t b)
{
    // Don't overwrite data that hasn't been sent yet
    while (output_write == (uint8_t)(output_read - 1));

    output_buffer[output_write++] = b;

    // Enable transmit if necessary
    UCSR0B |= _BV(UDRIE0);
}

void queue_bytes(const char *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        queue_byte(data[i]);
}

ISR(USART_UDRE_vect)
{
    if (output_write != output_read)
        UDR0 = output_buffer[output_read++];

    // Ran out of data to send - disable the interrupt
    if (output_write == output_read)
        UCSR0B &= ~_BV(UDRIE0);
}

volatile bool send_message = true;
struct weather_data data;

void tick()
{    
    if (send_message)
    {
        uint8_t buf[16];
        uint8_t len;
        queue_bytes("0R0,", 4);
        len = snprintf(buf, 16, "Dm=%0.0fD,", wrap_degrees(data.Dm.value));
        queue_bytes(buf, len);
        len = snprintf(buf, 16, "Sm=%0.1fK,", data.Sm.value);
        queue_bytes(buf, len);
        len = snprintf(buf, 16, "Ta=%0.1fC,", data.Ta.value);
        queue_bytes(buf, len);
        len = snprintf(buf, 16, "Ua=%0.1fP,", data.Ua.value);
        queue_bytes(buf, len);
        len = snprintf(buf, 16, "Pa=%0.1fH,", data.Pa.value);
        queue_bytes(buf, len);
        len = snprintf(buf, 16, "Rc=%0.2fM,", data.Rc);
        queue_bytes(buf, len);
        len = snprintf(buf, 16, "Ri=%0.1fM,", data.Ri);
        queue_bytes(buf, len);
        len = snprintf(buf, 16, "Th=%0.1fC,", data.Th);
        queue_bytes(buf, len);
        len = snprintf(buf, 16, "Vh=%0.1fN", data.Vh);
        queue_bytes(buf, len);
        queue_bytes("\r\n", 2);

        send_message = false;
    }
}

int main()
{
    // Configure timer1 to interrupt every 4.00 seconds
    OCR1A = 62496;
    TCCR1B = _BV(CS12) | _BV(CS10) | _BV(WGM12);
    TIMSK1 |= _BV(OCIE1B);

#define BAUD 9600
#include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A = _BV(U2X0);
#endif

    // Enable USB transmit only
    UCSR0B = _BV(TXEN0) | _BV(RXCIE0);
    output_read = output_write = 0;
    rng_init();

    // Wait a bit for the RNG to populate
    _delay_ms(20);

    // Wind direction varies +/- 20 around a fixed value that isn't too near the rollover
    float wind_dir = wrap_degrees(rng_next());
    measurement_init(&data.Dm, (float[]){ 0, 360 }, timescale);
    measurement_init(&data.Sm, wind_speed, timescale);
    measurement_init(&data.Ta, air_temperature, timescale);
    measurement_init(&data.Ua, relative_humidity, timescale);
    measurement_init(&data.Pa, air_pressure, timescale);
    data.Rc = rain_accumulation;
    data.Ri = 0;
    data.Th = heater_temperature;
    data.Vh = heater_voltage;

    sei();
    for (;;)
        tick();
}

// Intensity update interrupt.
ISR(TIMER1_COMPB_vect)
{
    float timestep = 4;
    measurement_step(&data.Dm, timestep);
    measurement_step(&data.Sm, timestep);
    measurement_step(&data.Ta, timestep);
    measurement_step(&data.Ua, timestep);
    measurement_step(&data.Pa, timestep);

    send_message = true;
}