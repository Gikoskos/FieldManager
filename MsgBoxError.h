/*
    MsgBoxError.h

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

#ifndef __MSGBOXERR_HDR_H__
#define __MSGBOXERR_HDR_H__


#include "Common.h"

#ifdef _MSC_VER
# define FUNCSTR __FUNCTIONW__
#else
# define FUNCSTR (PCWSTR)ToWideStr(__func__)
#endif


//Error handling

//flags for the __MsgBoxDefaultFuncError function
#define __GETLASTERR 0x00
#define __GETERRNO 0x01
#define __GETCURLERR 0x02

#define MSGBOX_ERR(x, y, z) \
    do { \
        MsgBoxDefaultFuncError(x, y, FUNCSTR, __LINE__ - 1, z, __GETLASTERR); \
    } while (0) \

#define MSGBOX_LASTERR(x, y) \
    do { \
        int ___err = GetLastError(); \
        MsgBoxDefaultFuncError(x, y, FUNCSTR, __LINE__ - 1, ___err, __GETLASTERR); \
    } while (0) \

#define MSGBOX_CURL(x, y) \
    do { \
        int ___err; \
        _get_errno(&___err); \
        MsgBoxDefaultFuncError(x, y, FUNCSTR, __LINE__ - 1, ___err, __GETCURLERR); \
    } while (0) \

#define MSGBOX_ERRNO(x, y) \
    do { \
        int ___err; \
        _get_errno(&___err); \
        MsgBoxDefaultFuncError(x, y, FUNCSTR, __LINE__ - 1, ___err, __GETERRNO); \
    } while (0) \

#define MSGBOX_WSAERR(x, y) \
    do { \
        int ___err = WSAGetLastError(); \
        MsgBoxDefaultFuncError(x, y, FUNCSTR, __LINE__ - 1, ___err, __GETLASTERR); \
    } while (0) \

//MsgBoxError.c

extern char curl_errbuf[CURL_ERROR_SIZE];

PWSTR ToWideStr(PCSTR to_convert);
void MsgBoxDefaultFuncError(HWND hwnd, PCWSTR failed_func, PCWSTR caller_func, const ULONG line, const INT err,  const int flag);


#endif //__MSGBOXERR_HDR_H__