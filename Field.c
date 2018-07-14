/*
    Field.c

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

#include "Field.h"

#define TAX_TOTAL 0.24

double crop_kg_per_acre[] = {
    300,
    220,
    350,
    750,
    2500,
    800,
    130,

    1000,
    400,
    900,
    5250,
    2500,
    800,
    300,
    1000,
    450,
    750
};

double crop_price_per_kg[] = {
    1.62,
    4.00,
    0.22,
    0.2,
    0.6,
    0.25,
    1.43,

    5,
    2,
    3,
    35,
    0.5,
    1,
    0.6,
    0.9,
    1,
    6
};


void ProcessFieldData(FIELD_DATA *fld)
{
    if (fld->crop != INV_CROP_TYPE) {
        fld->expenses = fld->misc_cost +
                        fld->tools_cost +
                        (fld->fert_cost * fld->acres) +
                        (fld->seed_cost * fld->acres) +
                        (fld->rented_cost * fld->rented_acres);

        if (fld->watering) {
            fld->income = (
                           (fld->acres + fld->rented_acres) *
                           (crop_kg_per_acre[fld->crop] + (crop_kg_per_acre[fld->crop] * 0.2))
                          ) * crop_price_per_kg[fld->crop];
        } else {
            fld->income = ((fld->acres + fld->rented_acres) * crop_kg_per_acre[fld->crop]) *
                          crop_price_per_kg[fld->crop];
        }

        fld->profit = fld->income - fld->expenses;

        fld->expenses = fld->expenses + fld->expenses * TAX_TOTAL;

        fld->processed = 1;
    }
}
