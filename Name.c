/*
    Name.c

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

#include "Name.h"
#include "MsgBoxError.h"
#include "GlobalData.h"
#include "Resources.h"
#include <stdlib.h>
#include <windowsx.h>
#include <wchar.h>

#define NAME_FLD_LEN 256

static wchar_t tmp_buff[NAME_FLD_LEN];


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

static INT_PTR OnInitDialog(HWND hwnd,
                            HWND hkey,
                            LPARAM lParam)
{
    UNREFERENCED_PARAMETER(hkey);
    UNREFERENCED_PARAMETER(lParam);

    SendDlgItemMessage(hwnd, IDT_NAME, EM_SETLIMITTEXT, (WPARAM)NAME_FLD_LEN, 0);
    CenterChild(hwnd);
    return TRUE;
}

static INT_PTR OnCommand(HWND hwnd,
                         int wParamLow,
                         HWND hctl,
                         UINT wParamHigh)
{
    switch (wParamLow) {
    case IDOK:
        {
            int name_len;

            name_len = SendDlgItemMessage(hwnd, IDT_NAME, WM_GETTEXTLENGTH, 0, 0);
            if (name_len) {
                curr_farmer = (int)total_farmers;
                total_farmers++;

                if (total_farmers > farmer_array_len) {
                    farmer_array_len += DEFAULT_ALLOC_SIZE;
                    farmers = win_realloc(farmers, sizeof(FARMER_DATA) * farmer_array_len);
                    if (!farmers) {
                        MSGBOX_LASTERR(hwnd, L"win_realloc");
                        EndDialog(hwnd, FALSE);
                    }
                }

                SecureZeroMemory(&farmers[curr_farmer], sizeof(FARMER_DATA));

                name_len += 36;

                farmers[curr_farmer].name = win_malloc(sizeof(wchar_t) * name_len);
                if (!farmers[curr_farmer].name) {
                    MSGBOX_LASTERR(NULL, L"win_malloc");
                    EndDialog(hwnd, FALSE);
                    return TRUE;
                }

                if (!GetDlgItemTextW(hwnd, IDT_NAME, tmp_buff, 256)) {
                    MSGBOX_LASTERR(NULL, L"GetDlgItemTextW");
                    EndDialog(hwnd, FALSE);
                    return TRUE;
                }

                if (FAILED(StringCchPrintfW(farmers[curr_farmer].name, name_len, L"#%lu %s", total_farmers, tmp_buff))) {
                    MessageBoxExW(hwnd, L"Η StringCchPrintfW απέτυχε. Name.c:54",
                                  L"Κάτι συνέβη!", MB_OK | MB_ICONERROR,
                                  MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
                }

                EndDialog(hwnd, TRUE);
            } else {
                MessageBoxExW(hwnd, L"Το πεδίο ονόματος είναι κενό.",
                              L"Σφάλμα!", MB_OK | MB_ICONERROR,
                              MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
            }
        }
        return TRUE;
    case IDCANCEL:
        EndDialog(hwnd, FALSE);
        return TRUE;
    }

    return FALSE;
}

INT_PTR OnClose(HWND hwnd)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}

INT_PTR CALLBACK NameDialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
    }

    return FALSE;
}

BOOL InitNameDialog(HWND hwnd)
{
    return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_NAMEDLG), hwnd, NameDialogProcedure);
}
