/*
    MsgBoxError.c

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

#include "MsgBoxError.h"

char curl_errbuf[CURL_ERROR_SIZE];

PWSTR ToWideStr(PCSTR to_convert)
{
    wchar_t *wide_str;
    int len = lstrlenA(to_convert) + 1;

    wide_str = win_malloc(sizeof(wchar_t) * len);

    if (!wide_str) {
        MessageBoxExW(NULL, L"MsgBoxError.c:11", L"Η malloc απέτυχε!", MB_OK | MB_ICONERROR,
                      MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
        exit(EXIT_FAILURE);
    }

    MultiByteToWideChar(CP_UTF8, 0, to_convert, len, wide_str, len);

    return wide_str;
}

void MsgBoxDefaultFuncError(HWND hwnd,
                            PCWSTR szFailedFuncName,
                            PCWSTR szCallerFuncName,
                            const ULONG uLine,
                            const INT err,
                            const int flag)
{ 
    PVOID pDisplayBuf;

    if (flag == __GETLASTERR) {
        PVOID formatted_buff;

        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, err, MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE),
                       (PWSTR)&formatted_buff, 0, NULL);

        pDisplayBuf = win_calloc(lstrlenW(formatted_buff) + 
                                 100 +
                                 lstrlenW(szFailedFuncName) +
                                 lstrlenW(szCallerFuncName), sizeof(wchar_t));

        if (!pDisplayBuf) {
            MessageBoxExW(hwnd, L"MsgBoxError.c:39", L"Η malloc απέτυχε!", MB_OK | MB_ICONERROR,
                          MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
            exit(EXIT_FAILURE);
        }

        StringCchPrintfW((PWSTR)pDisplayBuf, LocalSize(pDisplayBuf) / sizeof(wchar_t),
                         L"%s():%s():%lu\r\nGetLastError %lu: %s",
                         szFailedFuncName, szCallerFuncName, uLine, err, formatted_buff);

        MessageBoxExW(hwnd, (PCWSTR)pDisplayBuf, L"Σφάλμα!", MB_OK | MB_ICONERROR,
                      MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));

        //fwprintf(stderr, L"== Error at %s():%s():%lu\r\n-> %s ==\n", szCallerFuncName, szFailedFuncName, uLine, formatted_buff);

        win_free((PWSTR)szCallerFuncName);
        win_free(pDisplayBuf);
        LocalFree(formatted_buff);
    } else if (flag == __GETERRNO) {
        char tmperrno_buff[255];
        wchar_t *errno_buff;

        strerror_s(tmperrno_buff, 255, err);

        errno_buff = ToWideStr(tmperrno_buff);

        pDisplayBuf = win_calloc(lstrlenW(errno_buff) +
                                 100 +
                                 lstrlenW(szFailedFuncName) +
                                 lstrlenW(szCallerFuncName), sizeof(wchar_t));

        if (!pDisplayBuf) {
            MessageBoxExW(hwnd, L"MsgBoxError.c:72", L"Η malloc απέτυχε!", MB_OK | MB_ICONERROR,
                          MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
            exit(EXIT_FAILURE);
        }

        StringCchPrintfW((PWSTR)pDisplayBuf, LocalSize(pDisplayBuf) / sizeof(wchar_t),
                         L"%s():%s():%lu\r\nerrno %lu: %s",
                         szFailedFuncName, szCallerFuncName, uLine, err, errno_buff);

        MessageBoxExW(hwnd, (PCWSTR)pDisplayBuf, L"Σφάλμα!", MB_OK | MB_ICONERROR,
                      MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));

        //fwprintf(stderr, L"== Error at %s():%s():%lu -> %s ==\n", caller_func, szFailedFuncName, uLine, tmperrno_buff);

        win_free((PWSTR)szCallerFuncName);
        win_free(errno_buff);
        win_free(pDisplayBuf);
    } else if (flag == __GETCURLERR && curl_errbuf[0]) {
        wchar_t *wide_err_buff;

        wide_err_buff = ToWideStr(curl_errbuf);

        pDisplayBuf = win_calloc(lstrlenW(wide_err_buff) +
                                 100 +
                                 lstrlenW(szFailedFuncName) +
                                 lstrlenW(szCallerFuncName), sizeof(wchar_t));

        if (!pDisplayBuf) {
            MessageBoxExW(hwnd, L"MsgBoxError.c:104", L"Η malloc απέτυχε!", MB_OK | MB_ICONERROR,
                          MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
            exit(EXIT_FAILURE);
        }

        StringCchPrintfW((PWSTR)pDisplayBuf, LocalSize(pDisplayBuf) / sizeof(wchar_t),
                         L"%s():%s():%lu\r\ncurl error %lu: %s",
                         szFailedFuncName, szCallerFuncName, uLine, err, wide_err_buff);

        MessageBoxExW(hwnd, (PCWSTR)pDisplayBuf, L"Σφάλμα!", MB_OK | MB_ICONERROR,
                      MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));

        //fwprintf(stderr, L"== Error at %s():%s():%lu -> %s ==\n", caller_func, szFailedFuncName, uLine, wide_err_buff);

        win_free((PWSTR)szCallerFuncName);
        win_free(wide_err_buff);
        win_free(pDisplayBuf);
        curl_errbuf[0] = 0;
    }
}
