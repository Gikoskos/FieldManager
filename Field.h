/*
    Field.h

    Copyright (C) 2018 George Koskeridis

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#pragma once

#include "Common.h"

typedef enum _CROP_TYPE {
    INV_CROP_TYPE = -1,
    //conventional
    TOBACCO = 0,
    COTTON,
    WHEAT,
    VETCH,
    BEET,
    RICE,
    LENTILS,
    //alternative
    BLUEBERRY,
    BASIL,
    HIPPOPHAES,
    ALOE,
    POMEGRANATE,
    CHOKEBERRIES,
    ROSEMARY,
    CORNUS,
    STEVIA,
    GOJIBERRY
} CROP_TYPE;

typedef struct _FIELD_DATA {
    CROP_TYPE crop;

    double acres;
    double fert_cost, seed_cost;

    struct {
        double rented_acres;
        double rented_cost;
    };

    double misc_cost;
    double tools_cost;

    double income, expenses, profit;
    int watering, processed;
} FIELD_DATA;

typedef struct _FARMER_DATA {
    FIELD_DATA fld;
    wchar_t *name;
} FARMER_DATA;


void ProcessFieldData(FIELD_DATA *fld);
