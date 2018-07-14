/*
    Newsfeed.c

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

#include "Common.h"
#include <wininet.h>
#include <richedit.h>
#include "MsgBoxError.h"
#include "Newsfeed.h"
#include "Resources.h"
#include <mrss.h>
#include <ctype.h>
#include <pthread.h>

#define REFRESH_TIME_MS 1800000
#define APPENDTEXT(x) wide_str = ToWideStr(x); AppendNewsBoxW(wide_str); win_free(wide_str)
#define APPENDBCTEXT(x) wide_str = ToWideStr(x); AppendBoldClrNewsBoxW(wide_str); win_free(wide_str)
#define APPENDBTEXT(x) wide_str = ToWideStr(x); AppendBoldNewsBoxW(wide_str); win_free(wide_str)
#define SETTEXT(x) wide_str = ToWideStr(x); SetNewsBoxW(wide_str); win_free(wide_str)

void *newsfeed_lock;

static HWND hwnd;
static BOOL running = TRUE;
static pthread_t thrd_id;
static HANDLE wakeup;

static void AppendNewsBoxW(PCWSTR newText);
static void SetNewsBoxW(PCWSTR newText);
static BOOL RefreshRSSFeeds(char **urls, size_t len);

void AppendNewsBoxW(PCWSTR newText)
{
    if (newText) {
        GETTEXTLENGTHEX opt1 = {.flags = GTL_DEFAULT, .codepage = 1200};
        SETTEXTEX opt2 = {.flags = ST_SELECTION | ST_UNICODE, .codepage = 1200};

        int outLength = SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_GETTEXTLENGTHEX, (WPARAM)&opt1, (LPARAM)0);

        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETSEL, (WPARAM)outLength, (LPARAM)outLength);

        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETTEXTEX, (WPARAM)&opt2, (LPARAM)newText);
    }
}

void AppendBoldClrNewsBoxW(PCWSTR newText)
{
    if (newText) {

        CHARRANGE cr;
        CHARFORMAT2W cf = { 0 };
        GETTEXTLENGTHEX opt1 = {.flags = GTL_DEFAULT, .codepage = 1200};
        SETTEXTEX opt2 = {.flags = ST_SELECTION | ST_UNICODE, .codepage = 1200};

        cf.dwMask = CFM_BOLD | CFM_COLOR;
        cf.crTextColor = RGB(0, 100, 100);
        cf.dwEffects = CFE_BOLD;
        cf.cbSize = sizeof(CHARFORMAT2W);

        int outLength = SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_GETTEXTLENGTHEX, (WPARAM)&opt1, (LPARAM)0);

        cr.cpMin = outLength;
        cr.cpMax = outLength + lstrlenW(newText);

        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETTEXTEX, (WPARAM)&opt2, (LPARAM)newText);
        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_EXSETSEL , (WPARAM)0, (LPARAM)&cr);
        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
    }
}

void AppendBoldNewsBoxW(PCWSTR newText)
{
    if (newText) {
        CHARRANGE cr;
        CHARFORMAT2W cf = { 0 };
        GETTEXTLENGTHEX opt1 = {.flags = GTL_DEFAULT, .codepage = 1200};
        SETTEXTEX opt2 = {.flags = ST_SELECTION | ST_UNICODE, .codepage = 1200};

        cf.dwMask = CFM_BOLD;
        cf.dwEffects = CFE_BOLD;
        cf.cbSize = sizeof(CHARFORMAT2W);

        int outLength = SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_GETTEXTLENGTHEX, (WPARAM)&opt1, (LPARAM)0);

        cr.cpMin = outLength;
        cr.cpMax = outLength + lstrlenW(newText);

        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETTEXTEX, (WPARAM)&opt2, (LPARAM)newText);
        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_EXSETSEL , (WPARAM)0, (LPARAM)&cr);
        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
    }
}

void SetNewsBoxW(PCWSTR newText)
{
    if (newText) {
        GETTEXTLENGTHEX opt1 = {.flags = GTL_DEFAULT, .codepage = 1200};
        SETTEXTEX opt2 = {.flags = ST_SELECTION | ST_UNICODE, .codepage = 1200};

        int outLength = SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_GETTEXTLENGTHEX, (WPARAM)&opt1, (LPARAM)0);

        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETSEL, (WPARAM)0, (LPARAM)outLength);

        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETTEXTEX, (WPARAM)&opt2, (LPARAM)newText);
    }
}
/*
void AppendLinkNewsBoxW(PCWSTR link, PCWSTR name)
{
    CHARFORMAT2W cf = { 0 };
    GETTEXTLENGTHEX opt1 = {.flags = GTL_DEFAULT, .codepage = 1200};
    SETTEXTEX opt2 = {.flags = ST_SELECTION | ST_UNICODE, .codepage = 1200};

    cf.dwMask = CFM_LINK | CFM_LINKPROTECTED;
    //cf.crTextColor = RGB(0, 0, 255);
    cf.dwEffects = CFE_LINK | CFE_LINKPROTECTED;
    cf.cbSize = sizeof(CHARFORMAT2W);

    int outLength = SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_GETTEXTLENGTHEX, (WPARAM)&opt1, (LPARAM)0);

    SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETTEXTEX, (WPARAM)&opt2, (LPARAM)link);
    SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETSEL, (WPARAM)outLength, (LPARAM)(outLength + lstrlenW(link)));
    SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
}
*/
BOOL RefreshRSSFeeds(char **urls, size_t len)
{
    mrss_t *data;
    mrss_error_t ret;
    mrss_item_t *item;
    CURLcode code;
    wchar_t *wide_str;
    char *str;

    SetNewsBoxW(L"");

    for (size_t i = 0; i < len; i++) {
        ret = mrss_parse_url_with_options_and_error(urls[i], &data, NULL, &code);

        if (!running) {
            if (!ret)
                mrss_free(data);
            return TRUE;
        }

        if (ret || !data) {
            SetNewsBoxW(L"Σφάλμα: ");
            APPENDTEXT((MRSS_ERR_DOWNLOAD) ? mrss_curl_strerror(code) : mrss_strerror(ret));
            return FALSE;
        }

        APPENDBCTEXT(data->title);
        AppendNewsBoxW(L"\r\nLink: ");
        APPENDTEXT(data->link);

        item = data->item;

        while (item) {
            AppendNewsBoxW(L"\r\n\r\n");
            APPENDBTEXT(item->title);
            AppendNewsBoxW(L"\r\n");
            str = strchr(item->description, '>');

            if (str) {
                str = str + 1;
            } else {
                str = item->description;
            }

            APPENDTEXT(str);

            AppendNewsBoxW(L"\r\n");
            AppendBoldNewsBoxW(L"Link:");
            AppendNewsBoxW(L" ");
            APPENDTEXT(item->guid);

            item = item->next;
        }

        AppendNewsBoxW(L"\r\n\r\n______________________________\r\n\r\n");

        mrss_free(data);
    }

    return TRUE;
}

void *RefreshFeedThread(void *arg)
{
    static char *URLS[] = {
        "http://www.agronews.gr/rss/?catid=9&la=1",
        "http://www.agronews.gr/rss/?catid=37&la=1"
    };

    do {
        WaitForSingleObject((HANDLE)newsfeed_lock, INFINITE);

        if (!InternetGetConnectedState(NULL, 0)) {
            SetNewsBoxW(L"Δεν υπάρχει πρόσβαση στο διαδίκτυο.");
        } else {
            SetNewsBoxW(L"Φορτώνει...");
            RefreshRSSFeeds(URLS, ARRAYSIZE(URLS));
        }

        ReleaseMutex(newsfeed_lock);
        
    } while (WaitForSingleObject(wakeup, REFRESH_TIME_MS) != WAIT_OBJECT_0);

    return NULL;
}

VOID StopNewsfeedThread(VOID)
{
    if (running) {
        running = 0;
        ReleaseSemaphore(wakeup, 1, NULL);
        pthread_join(thrd_id, NULL);
    }

    if (wakeup)
        CloseHandle(wakeup);

    if (newsfeed_lock)
        CloseHandle((HANDLE)newsfeed_lock);
}

int RunNewsfeedThread(HWND hDlg)
{
    int err;

    hwnd = hDlg;

    wakeup = CreateSemaphore(NULL, 0, 1, NULL);
    if (!wakeup) {
        MSGBOX_LASTERR(NULL, L"CreateSemaphore");
        return FALSE;
    }

    newsfeed_lock = (void *)CreateMutex(NULL, FALSE, NULL);
    if (!newsfeed_lock) {
        MSGBOX_LASTERR(NULL, L"CreateMutex");
        return FALSE;
    }

    SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_AUTOURLDETECT, (WPARAM)1, (LPARAM)0);

    running = 1;
    err = pthread_create(&thrd_id, NULL, RefreshFeedThread, NULL);
    if (err) {
        running = 0;
        _set_errno(err);
        MSGBOX_ERRNO(NULL, L"pthread_create");
        return FALSE;
    }

    return TRUE;
}
