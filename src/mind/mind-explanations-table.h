﻿#pragma once

#include "system/angband.h"

#define MAX_MIND_POWERS 21 /*!< 超能力の数 / Mindcraft */

typedef struct mind_type {
    PLAYER_LEVEL min_lev;
    MANA_POINT mana_cost;
    PERCENTAGE fail;
    concptr name;
} mind_type;

typedef struct mind_power {
    mind_type info[MAX_MIND_POWERS];
} mind_power;

extern mind_power const mind_powers[5];
extern concptr const mind_tips[5][MAX_MIND_POWERS];
