//*****************************************************************************
//  Copyright 2012 - 2015 Paul Chote
//  This file is part of vaisalasim, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef VAISALASIM_MEASUREMENT_H
#define VAISALASIM_MEASUREMENT_H

#include "main.h"

void measurement_init(struct measurement *trace, float range[2], float timescale[2]);
void measurement_step(struct measurement *trace, float dt);

#endif