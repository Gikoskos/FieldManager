/*
    Newsfeed.h

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


extern void *newsfeed_lock;

void StopNewsfeedThread(void);
int RunNewsfeedThread(HWND hDlg);
