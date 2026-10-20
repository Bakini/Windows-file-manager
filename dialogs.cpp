#include "dialogs.h"
static std::wstring dlgResult;
static HWND dlgEdit;
static const wchar_t dlgClassName[] = L"FileManagerDialog";
static bool dlgClassRegistered = false;
static std::wstring dlgInitial;
static LRESULT CALLBACK dlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE: {
        CREATESTRUCTW* cs = (CREATESTRUCTW*)lParam;
        const wchar_t* prompt = (const wchar_t*)cs->lpCreateParams;
        CreateWindowExW(0, L"STATIC", prompt, WS_CHILD | WS_VISIBLE,
            10, 10, 320, 20, hwnd, nullptr, nullptr, nullptr);
        dlgEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", dlgInitial.c_str(),
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            10, 35, 320, 25, hwnd, nullptr, nullptr, nullptr);
        CreateWindowExW(0, L"BUTTON", L"OK",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
            170, 75, 75, 25, hwnd, (HMENU)IDOK, nullptr, nullptr);
        CreateWindowExW(0, L"BUTTON", L"Отмена",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            255, 75, 75, 25, hwnd, (HMENU)IDCANCEL, nullptr, nullptr);
        SetFocus(dlgEdit);
        SendMessageW(dlgEdit, EM_SETSEL, 0, -1);   // выделить весь текст
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == IDOK) {
            wchar_t buf[256];
            GetWindowTextW(dlgEdit, buf, 256);
            dlgResult = buf;
            DestroyWindow(hwnd);
        }
        else if (id == IDCANCEL) {
            dlgResult = L"";
            DestroyWindow(hwnd);
        }
        return 0;
    }
    case WM_CLOSE:
        dlgResult = L"";
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
std::wstring askName(HWND parent, const wchar_t* title, const wchar_t* prompt, const wchar_t* initial)
{
    dlgResult = L"";
    dlgInitial = initial;
    if (!dlgClassRegistered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = dlgProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = dlgClassName;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClassW(&wc);
        dlgClassRegistered = true;
    }
    RECT pr;
    GetWindowRect(parent, &pr);
    int dlgW = 360, dlgH = 150;
    int dlgX = pr.left + (pr.right - pr.left - dlgW) / 2;
    int dlgY = pr.top + (pr.bottom - pr.top - dlgH) / 2;
    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, dlgClassName, title,
        WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        dlgX, dlgY, dlgW, dlgH,
        parent, nullptr, GetModuleHandleW(nullptr), (LPVOID)prompt);
    EnableWindow(parent, FALSE);
    MSG msg;
    while (IsWindow(dlg) && GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(dlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    EnableWindow(parent, TRUE);
    SetForegroundWindow(parent);
    return dlgResult;
}
