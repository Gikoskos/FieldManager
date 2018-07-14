/*
    Common.h

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

#ifndef COMMON_HDR_H__
#define COMMON_HDR_H__

#ifndef UNICODE
# define UNICODE 1
#endif

#ifndef _UNICODE
# define _UNICODE 1
#endif

#include <winsock2.h>
#include <curl/curl.h>

#include <windows.h>
#include <winnt.h> //language macros
#include <winerror.h> //error messages
#include <commctrl.h> //for the progress bar

#include <errno.h>
#include <tchar.h>

#ifndef _MSC_VER
# undef __CRT__NO_INLINE
#endif
#include <strsafe.h> //win32 native string handling

#ifndef _WINAPI_EASY_HEAP_MACROS
# define win_free(x) HeapFree(GetProcessHeap(), 0, x)
# define win_malloc(x) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, x)
# define win_realloc(x, y) HeapReAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, x, y)
# define win_calloc(x, y) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, (x) * y)
# define _WINAPI_EASY_HEAP_MACROS 1
#endif


#endif //COMMON_HDR_H__
