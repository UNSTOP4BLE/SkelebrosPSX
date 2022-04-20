/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_MOVIE_H
#define PSXF_GUARD_MOVIE_H

#include "stage.h"

//Movie interface
void Movie_Play(const char *path, u32 length);

typedef struct
{
  u8 id;
  u8 diff;
  boolean story;
  u8 select;
  boolean startmovie;
} Movie;

extern Movie movie;

void Movie_Tick();

#endif