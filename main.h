//*****************************************************************************
//  Copyright 2012 - 2015 Paul Chote
//  This file is part of vaisalasim, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef VAISALASIM_MAIN_H
#define VAISALASIM_MAIN_H

#include <stdbool.h>
#include <stdint.h>

// Configuration values
static float timescale[] = { 12, 120 };
static float wind_speed[] = { 0, 30 };
static float air_temperature[] = { 15, 25 };
static float relative_humidity[] = { 5, 95 };
static float air_pressure[] = { 700, 800 };
static float rain_accumulation = 0;
static float heater_temperature = 5;
static float heater_voltage = 18.5;

struct measurement
{
    // Current value
    float value;

    // Internal state
    float min_period;
    float max_period;
    float min_intensity;
    float max_intensity;
    float initial_intensity;

    float next_period;
    float accumulated_time;
    uint8_t start;

    // A circular buffer of the 4 control points for weighted-linear interpolation
    float points[4];
};

struct weather_data
{
    struct measurement Dm; // Wind direction (degrees)
    struct measurement Sm;  // Wind speed average (km / h)
    struct measurement Ta; // Air temperature: (deg C)
    struct measurement Ua; // Relative humidity (%)
    struct measurement Pa; // Air pressure (hPa)
    float Rc; // Rain accumulation (mm)
    float Th; // Heater temperature (deg C)
    float Vh; // Heater voltage (V)
};

#endif
