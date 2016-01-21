//*****************************************************************************
//  Copyright 2012 - 2015 Paul Chote
//  This file is part of vaisalasim, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include "measurement.h"
#include "rng.h"

static float lerp(float a, float b, float numerator, float denominator)
{
    return a + numerator * (b - a) / denominator;
}

static int8_t wrap(int8_t i)
{
    while (i < 0)
        i += 4;

    while (i >= 4)
        i -= 4;

    return i;
}

// Initialize first four parameters
void measurement_init(struct measurement *trace, float range[2], float timescale[2])
{
    trace->min_period = timescale[0];
    trace->max_period = timescale[1];
    trace->min_intensity = range[0];
    trace->max_intensity = range[1];
    trace->value = lerp(range[0], range[1], rng_next(), UINT16_MAX);

    for (uint8_t i = 0; i < 4; i++)
        trace->points[i] = trace->value;
}

void measurement_step(struct measurement *trace, float dt)
{
    trace->accumulated_time += dt;

    if (trace->accumulated_time > trace->next_period)
    {
        // Generate new control point value
        float next = lerp(trace->min_intensity, trace->max_intensity, rng_next(), UINT16_MAX);

        // Weight heavily towards the previous point
        trace->points[trace->start] = (2 * trace->points[wrap(trace->start + 3)] + next) / 3;

        // Increment control point
        trace->start = wrap(trace->start + 1);

        trace->accumulated_time -= trace->next_period;
        trace->next_period = lerp(trace->min_period, trace->max_period, rng_next(), UINT16_MAX);
    }

    // Evaluate the catmull-rom spline
    float t = trace->accumulated_time / trace->next_period;
    float p0 = trace->points[wrap(trace->start + 0)];
    float p1 = trace->points[wrap(trace->start + 1)];
    float p2 = trace->points[wrap(trace->start + 2)];
    float p3 = trace->points[wrap(trace->start + 3)];

    trace->value = p1 + 0.5f*t*(p2 - p0 + t*((2*p0 - 5*p1 + 4*p2 - p3) + t*(3*p1 - 3*p2 + p3 - p0)));
}