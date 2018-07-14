/*
    Chart.c

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

#include "GlobalData.h"
#include "Chart.h"
#include "Field.h"
#include "MsgBoxError.h"
#include "Resources.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <commctrl.h>
#include <windowsx.h>
#include <mCtrl/chart.h>
#include <mCtrl/dialog.h>



static HWND hwndChart = NULL, hwndOK = NULL;
static BOOL window_is_open = FALSE;
static BOOL running = TRUE;
static pthread_t thrd_id;
static HANDLE job_is_available, job_is_finished;

static POINT prev_c;

static MC_CHDATASET dataSet;

static VOID CenterChild(const HWND hwnd);

static VOID SetupChart(VOID);
static INT_PTR OnClose(HWND hwnd);
static INT_PTR CALLBACK ChartDialogProcedure(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


VOID CenterChild(const HWND hwnd)
{
    HWND hwndOwner = GetParent(hwnd);
    RECT rc, rcDlg, rcOwner;
    INT final_x, final_y;

    if (!hwndOwner) {
        hwndOwner = GetAncestor(hwnd, GA_PARENT);
        if (!hwndOwner)
            hwndOwner = GetDesktopWindow();
    }

    GetWindowRect(hwndOwner, &rcOwner);
    GetWindowRect(hwnd, &rcDlg);

    prev_c = (POINT){.x = (rcDlg.right - rcDlg.left), .y = (rcDlg.bottom - rcDlg.top)};

    CopyRect(&rc, &rcOwner);
    OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
    OffsetRect(&rc, -rc.left, -rc.top);
    OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

    final_x = rcOwner.left + (rc.right / 2);
    final_y = rcOwner.top + (rc.bottom / 2);
    if (final_x < 0) final_x = 0;
    if (final_y < 0) final_y = 0;
    SetWindowPos(hwnd, HWND_TOP, final_x, final_y, 0, 0, SWP_NOSIZE);
}

VOID SetupChart(VOID)
{
    dataSet.dwCount = total_farmers;

    SetWindowTextW(hwndChart, L"Εσόδα, εξόδα και κέρδος ανά αγρότη");

    SendMessageW(hwndChart, MC_CHM_SETAXISLEGEND, 1, (LPARAM)L"Αγρότες");
    SendMessageW(hwndChart, MC_CHM_SETAXISLEGEND, 2, (LPARAM)L"Χρήματα [€]");

    if (total_farmers) {
        dataSet.piValues = win_malloc(sizeof(int) * total_farmers);
        if (!dataSet.piValues) {
            MSGBOX_LASTERR(NULL, L"win_malloc");
            exit(EXIT_FAILURE);
        }

        SendMessageW(hwndChart, MC_CHM_SETAXISOFFSET, (WPARAM)1, (LPARAM)1);

        for (ULONG i = 0; i < total_farmers; i++) {
            if (!farmers[i].fld.processed) {
                ProcessFieldData(&farmers[i].fld);
            }

            dataSet.piValues[i] = (int)farmers[i].fld.income;
        }

        SendMessageW(hwndChart, MC_CHM_INSERTDATASET, 0, (LPARAM)&dataSet);
        SendMessageW(hwndChart, MC_CHM_SETDATASETLEGEND, 0, (LPARAM)L"Έσοδα");

        for (ULONG i = 0; i < total_farmers; i++) {
            dataSet.piValues[i] = (int)farmers[i].fld.expenses;
        }

        SendMessageW(hwndChart, MC_CHM_INSERTDATASET, 1, (LPARAM)&dataSet);
        SendMessageW(hwndChart, MC_CHM_SETDATASETLEGEND, 1, (LPARAM)L"Έξοδα");

        for (ULONG i = 0; i < total_farmers; i++) {
            dataSet.piValues[i] = (int)farmers[i].fld.profit;
        }

        SendMessageW(hwndChart, MC_CHM_INSERTDATASET, 2, (LPARAM)&dataSet);
        SendMessageW(hwndChart, MC_CHM_SETDATASETLEGEND, 2, (LPARAM)L"Κέρδος");
    }
}

INT_PTR OnInitDialog(HWND hwnd,
                     HWND hkey,
                     LPARAM lParam)
{
    hwndChart = GetDlgItem(hwnd, IDC_CHART_COLUMN);
    hwndOK = GetDlgItem(hwnd, IDOK);
    CenterChild(hwnd);
    ReleaseSemaphore(job_is_available, 1, NULL);
    window_is_open = TRUE;
    return TRUE;
}

BOOL MoveOKButton(HWND hwnd,
                  int cx,
                  int cy,
                  int diff_x,
                  int diff_y,
                  POINT *pt)
{
    static RECT rcOK;
    LONG btn_mid, dlg_mid;

    if (!GetWindowRect(hwnd, &rcOK)) {
        MSGBOX_LASTERR(hwnd, L"GetWindowRect");
        return FALSE;
    }

    dlg_mid = rcOK.right - cx/2;

    if (!GetWindowRect(hwndOK, &rcOK)) {
        MSGBOX_LASTERR(hwnd, L"GetWindowRect");
        return FALSE;
    }

    btn_mid = rcOK.right - ((rcOK.right - rcOK.left)/2);

    *pt = (POINT){.x = rcOK.left, .y = rcOK.top};
    if (!ScreenToClient(hwnd, pt)) { //transform the upper left coordinates to be relative to the window
        MSGBOX_LASTERR(hwnd, L"ScreenToClient");
        return FALSE;
    }

    if (btn_mid != dlg_mid) {
        pt->x +=  (dlg_mid - btn_mid);
    }
    pt->y -= diff_y;
    
    return TRUE;
}

BOOL ResizeChart(HWND hwnd, 
                 int diff_x,
                 int diff_y,
                 POINT *pt)
{
    static RECT rcChart;

    if (!GetWindowRect(hwndChart, &rcChart)) {
        MSGBOX_LASTERR(hwnd, L"GetWindowRect");
        return FALSE;
    }

    pt->x = (rcChart.right - rcChart.left) - diff_x;
    pt->y = (rcChart.bottom - rcChart.top) - diff_y;

    return TRUE;
}

INT_PTR HandleResize(HWND hwnd,
                     int cx,
                     int cy)
{
    if (!window_is_open)
        return FALSE;

    POINT pt;
    int diff_x, diff_y;

    diff_x = prev_c.x - cx;
    diff_y = prev_c.y - cy;


    HDWP hdwp = BeginDeferWindowPos(2);
    if (!hdwp) {
        MSGBOX_LASTERR(hwnd, L"BeginDeferWindowPos");
        return OnClose(hwnd);
    }

    if (!MoveOKButton(hwnd, cx, cy, diff_x, diff_y, &pt)) {
        return OnClose(hwnd);
    }

    hdwp = DeferWindowPos(hdwp, hwndOK, NULL, pt.x, pt.y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);

    if (hdwp) {
        if (!ResizeChart(hwnd, diff_x, diff_y, &pt)) {
            return OnClose(hwnd);
        }

        hdwp = DeferWindowPos(hdwp, hwndChart, NULL, 0, 0, pt.x, pt.y, SWP_NOOWNERZORDER | SWP_NOMOVE);
    }


    if (!EndDeferWindowPos(hdwp)) {
        MSGBOX_LASTERR(hwnd, L"EndDeferWindowPos");
        return OnClose(hwnd);
    }

    prev_c = (POINT){.x = cx, .y = cy};

    return TRUE;
}

INT_PTR OnCommand(HWND hwnd,
                  int wParamLow,
                  HWND hctl,
                  UINT wParamHigh)
{
    switch(wParamLow) {
    case IDOK:
        return OnClose(hwnd);
    }

    return FALSE;
}

void OnWindowPosChanged(HWND hwnd,
                        const WINDOWPOS *pwp)
{
    if (!(pwp->flags & SWP_NOSIZE)) {
        (void)HandleResize(hwnd, pwp->cx, pwp->cy);
    }
}

INT_PTR OnMinMaxInfo(HWND hwnd,
                     PMINMAXINFO mmi)
{
    mmi->ptMinTrackSize.x = 550;
    mmi->ptMinTrackSize.y = 400;
    return TRUE;
}

INT_PTR OnClose(HWND hwnd)
{
    WaitForSingleObject(job_is_finished, INFINITE);
    EndDialog(hwnd, 0);
    win_free(dataSet.piValues);
    window_is_open = FALSE;
    return TRUE;
}


INT_PTR CALLBACK ChartDialogProcedure(HWND hwnd,
                                      UINT msg,
                                      WPARAM wParam,
                                      LPARAM lParam)
{
    switch(msg) {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_GETMINMAXINFO, OnMinMaxInfo);
        HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED, OnWindowPosChanged);
        HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
    }

    return FALSE;
}

BOOL InitChartDialog(HWND hwnd)
{
    mcDialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CHARTDLG), hwnd, ChartDialogProcedure, MC_DF_DEFAULTFONT);

    return TRUE;
}

void *DrawChartThread(void *arg)
{
    while (running) {
        WaitForSingleObject(job_is_available, INFINITE);

        if (!running)
            break;

        SetupChart();

        ReleaseSemaphore(job_is_finished, 1, NULL);
    }

    return NULL;
}

VOID StopChartThread(VOID)
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

BOOL RunChartThread(VOID)
{
    int err;

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
    err = pthread_create(&thrd_id, NULL, DrawChartThread, NULL);
    if (err) {
        running = 0;
        _set_errno(err);
        MSGBOX_ERRNO(NULL, L"pthread_create");
        return FALSE;
    }

    return TRUE;
}
