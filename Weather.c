/*
    Weather.c

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
#include "MsgBoxError.h"
#include "Resources.h"
#include "json.h"
#include <ctype.h>
#include <pthread.h>

#define MAX_JSON_LEN 4096

static BOOL running = TRUE;
static pthread_t thrd_id;
static HANDLE job_is_available, job_is_finished;

static HWND hwnd, printbox, inputbox;
static HBITMAP weather_bmp;


static char weather_buff[MAX_JSON_LEN];
static size_t buff_len = 0;

static BOOL download_err = FALSE;

struct named_id_s {
    char name[16];
    UINT id;
};

struct named_id_s weather_img_ids[18] = {
    {"01d", IDB_01D},
    {"01n", IDB_01N},
    {"02d", IDB_02D},
    {"02n", IDB_02N},
    {"03d", IDB_03D},
    {"03n", IDB_03N},
    {"04d", IDB_04D},
    {"04n", IDB_04N},
    {"09d", IDB_09D},
    {"09n", IDB_09N},
    {"10d", IDB_10D},
    {"10n", IDB_10N},
    {"11d", IDB_11D},
    {"11n", IDB_11N},
    {"13d", IDB_13D},
    {"13n", IDB_13N},
    {"50d", IDB_50D},
    {"50n", IDB_50N}
};

static char name_buff[256];
static char URL_buff[512];

static void AppendWeatherBoxW(PCWSTR newText);
static void SetWeatherBoxA(PCSTR newText);
static void SetWeatherBoxW(PCWSTR newText);
static BOOL DownloadXML(const char *url_tofetch);
static void PrintWeather(json_value *value);
static void SetWeatherImage(const char *name);
static BOOL IsValidWeatherReport(json_value *value);


static size_t WriteXMLCb(void *contents, size_t size, size_t nmemb, void *userp)
{
    (void)userp;

    size_t realsize = size * nmemb;

    if (realsize >= (MAX_JSON_LEN - 1)) {
        download_err = TRUE;
        return 0;
    }

    memcpy(&(weather_buff[buff_len]), contents, realsize);
    buff_len += realsize;
    weather_buff[buff_len] = 0;

    return realsize;
}

BOOL DownloadXML(const char *url_tofetch)
{
    BOOL retvalue = FALSE;
    CURL *curl = curl_easy_init();

    download_err = FALSE;

    if (curl) {
        CURLcode res;

        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
        curl_errbuf[0] = '\0';

        curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
        curl_easy_setopt(curl, CURLOPT_URL, url_tofetch);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteXMLCb);

        if ((res = curl_easy_perform(curl)) == CURLE_OK && !download_err) {
            retvalue = TRUE;
        }

        curl_easy_cleanup(curl);
    }

    return retvalue;
}

void AppendWeatherBoxW(PCWSTR newText)
{
    if (newText) {
        int outLength = GetWindowTextLengthW(printbox);
        SendMessageW(printbox, EM_SETSEL, outLength, outLength);

        SendMessageW(printbox, EM_REPLACESEL, TRUE, (LPARAM)newText);
    }
}

void SetWeatherBoxA(PCSTR newText)
{
    if (newText)
        SendMessageA(printbox, WM_SETTEXT, 0, (LPARAM)newText);
}

void SetWeatherBoxW(PCWSTR newText)
{
    if (newText)
        SendMessageW(printbox, WM_SETTEXT, 0, (LPARAM)newText);
}

void SetValueWeatherBox(json_value* value)
{
    static wchar_t wchar_arr[_CVTBUFSIZE];
    wchar_t *wchar_ptr;

    switch (value->type) {
    case json_integer:
        {
            json_int_t temp = value->u.integer;
            StringCchPrintfW(wchar_arr, ARRAYSIZE(wchar_arr), L"%ld\r\n", temp);
            AppendWeatherBoxW(wchar_arr);
            break;
        }
    case json_double:
        {
            double temp = value->u.dbl;
            StringCchPrintfW(wchar_arr, ARRAYSIZE(wchar_arr), L"%.2f\r\n", temp);
            AppendWeatherBoxW(wchar_arr);
            break;
        }
    case json_string:
        {
            wchar_ptr = ToWideStr(value->u.string.ptr);
            AppendWeatherBoxW(wchar_ptr);
            win_free(wchar_ptr);
            AppendWeatherBoxW(L"\r\n");
            break;
        }
    default:
        break;
    }
}

void SetWeatherImage(const char *name)
{
    for (size_t i = 0; i < ARRAYSIZE(weather_img_ids); i++) {
        if (!strcmp(name, weather_img_ids[i].name)) {
            weather_bmp = (HBITMAP)LoadImage(GetModuleHandle(NULL),
                                             MAKEINTRESOURCE(weather_img_ids[i].id),
                                             IMAGE_BITMAP, 0, 0,
                                             LR_SHARED | LR_LOADTRANSPARENT | LR_DEFAULTSIZE);
            if (!weather_bmp) {
                MSGBOX_LASTERR(NULL, L"LoadImage");
                return;
            }

            SendDlgItemMessageA(hwnd, IDP_WEATHER, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)weather_bmp);
            break;
        }
    }
}

BOOL IsValidWeatherReport(json_value *value)
{
    unsigned int i;

    if (value && value->type == json_object) {
        for (i = 0; i < value->u.object.length; i++) {
            if (!strcmp(value->u.object.values[i].name, "cod")) {
                if (value->u.object.values[i].value->type == json_string) {
                    if (strcmp(value->u.object.values[i].value->u.string.ptr, "200")) {
                        return FALSE;
                    }
                }
                break;
            }
        }

        return TRUE;
    }

    return FALSE;
}

void PrintWeather(json_value *value)
{
    if (value) {

        SetWeatherBoxW(L"");

        for (unsigned int i = value->u.object.length - 1;; i--) {
            json_value *obj_val = value->u.object.values[i].value;
            char *obj_name = value->u.object.values[i].name;

            if (!strcmp(obj_name, "name")) {
                AppendWeatherBoxW(L"Όνομα περιοχής: ");
                SetValueWeatherBox(obj_val);
            } else if (!strcmp(obj_name, "id")) {
                AppendWeatherBoxW(L"ID περιοχής: ");
                SetValueWeatherBox(obj_val);
            } else if (!strcmp(obj_name, "wind")) {
                for (unsigned int j = 0; j < obj_val->u.object.length; j++) {
                    if (!strcmp(obj_val->u.object.values[j].name, "speed")) {
                        AppendWeatherBoxW(L"Ταχύτητα ανέμου (m/sec): ");
                        SetValueWeatherBox(obj_val->u.object.values[j].value);
                    } else if (!strcmp(obj_val->u.object.values[j].name, "deg")) {
                        AppendWeatherBoxW(L"Κατεύθυνση ανέμου σε μοίρες: ");
                        SetValueWeatherBox(obj_val->u.object.values[j].value);
                    }
                }
            } else if (!strcmp(obj_name, "sys")) {
                for (unsigned int j = 0; j < obj_val->u.object.length; j++) {
                    if (!strcmp(obj_val->u.object.values[j].name, "country")) {
                        AppendWeatherBoxW(L"Χώρα: ");
                        SetValueWeatherBox(obj_val->u.object.values[j].value);
                        break;
                    }
                }
            } else if (!strcmp(obj_name, "clouds")) {
                if (!strcmp(obj_val->u.object.values[0].name, "all")) {
                    AppendWeatherBoxW(L"Συννεφιά: ");
                    SetValueWeatherBox(obj_val->u.object.values[0].value);
                }
            } else if (!strcmp(obj_name, "rain")) {
                if (!strcmp(obj_val->u.object.values[0].name, "3h")) {
                    AppendWeatherBoxW(L"Όγκος βροχής τις τελευταίες 3 ώρες: ");
                    SetValueWeatherBox(obj_val->u.object.values[0].value);
                }
            } else if (!strcmp(obj_name, "snow")) {
                if (!strcmp(obj_val->u.object.values[0].name, "3h")) {
                    AppendWeatherBoxW(L"Όγκος χιονιού τις τελευταίες 3 ώρες: ");
                    SetValueWeatherBox(obj_val->u.object.values[0].value);
                }
            } else if (!strcmp(obj_name, "main")) {
                for (unsigned int j = 0; j < obj_val->u.object.length; j++) {
                    if (!strcmp(obj_val->u.object.values[j].name, "temp")) {
                        AppendWeatherBoxW(L"Θερμοκρασία (Celsius): ");
                        SetValueWeatherBox(obj_val->u.object.values[j].value);
                    } else if (!strcmp(obj_val->u.object.values[j].name, "pressure")) {
                        AppendWeatherBoxW(L"Ατμοσφαιρική πίεση (hPa): ");
                        SetValueWeatherBox(obj_val->u.object.values[j].value);
                    } else if (!strcmp(obj_val->u.object.values[j].name, "humidity")) {
                        AppendWeatherBoxW(L"Υγρασία: ");
                        SetValueWeatherBox(obj_val->u.object.values[j].value);
                    } else if (!strcmp(obj_val->u.object.values[j].name, "temp_min")) {
                        AppendWeatherBoxW(L"Ελάχιστη θερμοκρασία (Celsius): ");
                        SetValueWeatherBox(obj_val->u.object.values[j].value);
                    } else if (!strcmp(obj_val->u.object.values[j].name, "temp_max")) {
                        AppendWeatherBoxW(L"Μέγιστη θερμοκρασία (Celsius): ");
                        SetValueWeatherBox(obj_val->u.object.values[j].value);
                    }
                }
            } else if (!strcmp(obj_name, "weather")) {
                if (obj_val->type == json_array) {
                    for (unsigned int j = 0; j < obj_val->u.array.values[0]->u.object.length; j++) {
                        if (!strcmp(obj_val->u.array.values[0]->u.object.values[j].name, "description")) {
                            AppendWeatherBoxW(L"Κατάσταση καιρού: ");
                            SetValueWeatherBox(obj_val->u.array.values[0]->u.object.values[j].value);
                        } else if (!strcmp(obj_val->u.array.values[0]->u.object.values[j].name, "icon")) {
                            SetWeatherImage(obj_val->u.array.values[0]->u.object.values[j].value->u.string.ptr);
                        }
                    }
                }
            } else if (!strcmp(obj_name, "coord")) {
                AppendWeatherBoxW(L"Γεωγραφικό μήκος: ");
                SetValueWeatherBox(obj_val->u.object.values[0].value);

                AppendWeatherBoxW(L"Γεωγραφικό πλάτος: ");
                SetValueWeatherBox(obj_val->u.object.values[1].value);
            }

            if (!i)
                break;
        }

        SendMessageW(printbox, EM_SCROLL, (WPARAM)SB_PAGEUP, 0);
        SendMessageW(printbox, EM_SCROLL, (WPARAM)SB_PAGEUP, 0);
    }
}

BOOL GetWeather(void)
{
    if (!InternetGetConnectedState(NULL, 0)) {
        SendMessageW(printbox, WM_SETTEXT, 0, (LPARAM)L"Δεν υπάρχει πρόσβαση στο διαδίκτυο.");
        return FALSE;
    }

    name_buff[0] = 0;

    SendMessageA(inputbox, WM_GETTEXT, (WPARAM)ARRAYSIZE(name_buff), (LPARAM)name_buff);

    if (name_buff[0]) {
        SendMessageW(printbox, WM_SETTEXT, 0, (LPARAM)L"Φορτώνει...");

        ReleaseSemaphore(job_is_available, 1, NULL);
    }

    return TRUE;
}

void *GetWeatherThread(void *arg)
{
    BOOL is_id;
    json_settings settings = { 0 };
    json_value *value;

    while (running) {
        WaitForSingleObject(job_is_available, INFINITE);

        if (!running)
            break;

        is_id = TRUE;
        for (size_t i = 0; name_buff[i] && i < ARRAYSIZE(name_buff); i++) {
            if (!isdigit(name_buff[i])) {
                is_id = FALSE;
                break;
            }
        }

        if (!is_id) {
            StringCchPrintfA(URL_buff, sizeof URL_buff,
                             "http://api.openweathermap.org/data/2.5/weather?q=%s"
                             ",gr&lang=el&units=metric&APPID=4a81e292f93d91dd9b605858e554dc1d", name_buff);
        } else {
            StringCchPrintfA(URL_buff, sizeof URL_buff,
                             "http://api.openweathermap.org/data/2.5/weather?id=%s"
                             "&lang=el&units=metric&APPID=4a81e292f93d91dd9b605858e554dc1d", name_buff);
        }

        if (!DownloadXML(URL_buff)) {
            SetWeatherBoxW(L"Σφάλμα: ");
            if (download_err) {
                SetWeatherBoxW(L"Το μέγεθος του αρχείου υπερβαίνει το όριο");
            } else {
                SetWeatherBoxA(curl_errbuf);
            }
        } else {
            value = json_parse_ex(&settings, (json_char*)weather_buff, buff_len, name_buff);

            if (value) {
                if (!IsValidWeatherReport(value)) {
                    json_value_free(value);
                    if (is_id) {
                        SetWeatherBoxW(L"Δεν βρέθηκε πόλη με αυτό το ID");
                    } else {
                        StringCchPrintfA(URL_buff, sizeof URL_buff,
                                         "http://api.openweathermap.org/data/2.5/weather?q=%s"
                                         "&lang=el&units=metric&APPID=4a81e292f93d91dd9b605858e554dc1d", name_buff);

                        buff_len = 0;

                        if (!DownloadXML(URL_buff)) {
                            MSGBOX_CURL(hwnd, L"curl_easy_setopt");
                            exit(EXIT_FAILURE);
                        }

                        value = json_parse_ex(&settings, (json_char*)weather_buff, buff_len, name_buff);
                        if (value) {
                            if (!IsValidWeatherReport(value)) {
                                SetWeatherBoxW(L"Δεν βρέθηκε πόλη με αυτό το όνομα");
                            } else {
                                SendMessageA(inputbox, WM_SETTEXT, 0, (LPARAM)"");
                                PrintWeather(value);
                            }
                            json_value_free(value);
                        } else {
                            SetWeatherBoxA(name_buff);
                            /*AppendWeatherBoxW(L"\r\nΑρχείο:\r\n");
                            AppendWeatherBoxA(weather_buff);*/
                        }
                    }
                } else {
                    SendMessageA(inputbox, WM_SETTEXT, 0, (LPARAM)"");
                    PrintWeather(value);
                    json_value_free(value);
                }
            } else {
                SetWeatherBoxA(name_buff);
                /*AppendWeatherBoxW(L"\r\nΑρχείο:\r\n");
                AppendWeatherBoxA(weather_buff);*/
            }
        }

        buff_len = 0;

        ReleaseSemaphore(job_is_finished, 1, NULL);
    }

    return NULL;
}

void StopWeatherThread(void)
{
    if (running) {
        running = 0;
        ReleaseSemaphore(job_is_available, 1, NULL);
        pthread_join(thrd_id, NULL);
    }

    if (job_is_available)
        CloseHandle(job_is_available);

    if (job_is_finished)
        CloseHandle(job_is_finished);
}

int RunWeatherThread(HWND hDlg)
{
    int err;

    hwnd = hDlg;

    printbox = GetDlgItem(hwnd, IDT_WEATHER);
    if (!printbox) {
        MSGBOX_LASTERR(NULL, L"GetDlgItem");
        return FALSE;
    }

    inputbox = GetDlgItem(hwnd, IDT_WEATHERLOCATION);
    if (!inputbox) {
        MSGBOX_LASTERR(NULL, L"GetDlgItem");
        return FALSE;
    }

    job_is_available = CreateSemaphore(NULL, 0, 1, NULL);
    if (!job_is_available) {
        MSGBOX_LASTERR(NULL, L"CreateSemaphore");
        return FALSE;
    }

    job_is_finished = CreateSemaphore(NULL, 0, 1, NULL);
    if (!job_is_finished) {
        MSGBOX_LASTERR(NULL, L"CreateSemaphore");
        return FALSE;
    }

    running = 1;
    err = pthread_create(&thrd_id, NULL, GetWeatherThread, NULL);
    if (err) {
        running = 0;
        _set_errno(err);
        MSGBOX_ERRNO(NULL, L"pthread_create");
        return FALSE;
    }

    return TRUE;
}
