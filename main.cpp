#include <windows.h>
#include <commctrl.h>
#include <string>
#include "listview.h"
#include "actions.h"
#include "sort_search.h"
#include <shellapi.h>   // DragAcceptFiles, DragQueryFileW, DragFinish
#pragma comment(lib, "comctl32.lib")

const wchar_t window_class_name[] = L"FileManagerMainWindow";
const wchar_t window_title[] = L"BAKINI";

const int window_width = 1000;
const int window_height = 700;

const int idListView = 1001;//id listview по которому ловятся события

const int idBtnBack = 2001;
const int idEditSearch = 2006;
const int idBtnClearSearch = 2007;

// контекстное меню файла
const int idMenuOpen = 3001;
const int idMenuDelete = 3002;
const int idMenuRename = 3003;
const int idMenuCut = 3004;
const int idMenuPaste = 3013;

// меню «Создать» на пустом месте
const int idMenuCreateFolder = 3010;
const int idMenuCreateTxt = 3011;
const int idMenuCreateFile = 3012;

//глобальные переменные
HWND hListView = nullptr;
HWND hBtnBack = nullptr;
HWND hEditSearch = nullptr;
HWND hBtnClearSearch = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMSg, WPARAM wParam, LPARAM lParam);

void initButtons(HWND parent, HINSTANCE hInst)
{
    hBtnBack = CreateWindowExW(0, L"BUTTON", L"Назад",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0, parent, (HMENU)(UINT_PTR)idBtnBack, hInst, nullptr);

    hEditSearch = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0, 0, 0, 0, parent, (HMENU)(UINT_PTR)idEditSearch, hInst, nullptr);
    SendMessageW(hEditSearch, EM_SETCUEBANNER, TRUE, (LPARAM)L"Поиск");

    hBtnClearSearch = CreateWindowExW(0, L"BUTTON", L"\u2715",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 0, 0, parent, (HMENU)(UINT_PTR)idBtnClearSearch, hInst, nullptr);
}

void layoutControls(HWND hwnd, int w, int h)
{
    int top = 5;
    int btnH = 28;
    int x = 5;

    MoveWindow(hBtnBack, x, top, 80, btnH, TRUE);

    int clearW = 28;
    int searchW = 200;
    MoveWindow(hBtnClearSearch, w - clearW - 10, top, clearW, btnH, TRUE);
    MoveWindow(hEditSearch, w - clearW - searchW - 12, top + 2, searchW, btnH - 4, TRUE);

    if (hListView) {
        MoveWindow(hListView, 0, 40, w, h - 40, TRUE);
    }
}

void showContextMenu(HWND hwnd)
{
    int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (idx < 0) return;

    POINT pt;
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, idMenuOpen, files[idx].isDir ? L"Войти" : L"Открыть");
    AppendMenuW(menu, MF_STRING, idMenuCut, L"Вырезать");
    AppendMenuW(menu, MF_STRING, idMenuRename, L"Переименовать");
    AppendMenuW(menu, MF_STRING, idMenuDelete, L"Удалить");

    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}

void showCreateMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU sub = CreatePopupMenu();
    AppendMenuW(sub, MF_STRING, idMenuCreateFolder, L"Папку");
    AppendMenuW(sub, MF_STRING, idMenuCreateTxt, L"Текстовый документ");
    AppendMenuW(sub, MF_STRING, idMenuCreateFile, L"Файл\u2026");

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)sub, L"Создать");

    if (hasClipboard()) {
        AppendMenuW(menu, MF_STRING, idMenuPaste, L"Вставить");
    }

    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int show)
{
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    //registr
    WNDCLASSW myclass = {};
    myclass.lpfnWndProc = WndProc;//when event come
    myclass.hInstance = hInstance;
    myclass.lpszClassName = window_class_name;
    myclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    myclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassW(&myclass)) {
        MessageBoxW(nullptr, L"Не удалось зарегистрировать класс окна", L"Ошибка", MB_ICONERROR);//Стандарнтая иконка ошибки, nullptr - handle родительского окна
        return 1;
    }

    HWND hwnd = CreateWindowExW(
        0,  // расширенные стили WS_EX_TOPMOST, WS_EX_TRANSPARENT, WS_EX_TOOLWINDOW, WS_EX_ACCEPTFILES, WS_EX_CLIENTEDGE |
        window_class_name,//создается окно по описанию
        window_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,   // позиция
        window_width, window_height,
        nullptr,    // родитель, окно
        nullptr,    // меню
        hInstance,
        nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"Не удалось создать окно", L"Ошибка", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);//wmkeydown
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE: {
        initListView(hwnd);
        initButtons(hwnd, ((LPCREATESTRUCT)lParam)->hInstance);

        // стартовая папка - папка где лежит exe
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);//полный путь к исполняемому файлу, куда записать
        wchar_t* lastSlash = wcsrchr(exePath, L'\\');
        if (lastSlash) *lastSlash = 0;
        currentPath = exePath;//сохранение пути

        fillCurrentDir(hwnd);//чтение папки
        DragAcceptFiles(hwnd, TRUE);   // разрешаем перетаскивать файлы в окно
        return 0;
    }

    case WM_SIZE:
        layoutControls(hwnd, LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);
        switch (id) {
        case idBtnBack:          navigateBack(hwnd); break;
        case idBtnClearSearch:   SetWindowTextW(hEditSearch, L""); break;
        case idMenuOpen:         navigateInto(hwnd); break;
        case idMenuDelete:       deleteSelected(hwnd); break;
        case idMenuRename:       renameSelected(hwnd); break;
        case idMenuCreateFolder: createNewFolder(hwnd); break;
        case idMenuCreateTxt:    createTextFileNamed(hwnd); break;
        case idMenuCreateFile:   createFileNamed(hwnd); break;
        case idMenuCut:          cutSelected(hwnd); break;
        case idMenuPaste:        pasteFile(hwnd); break;
        case idEditSearch:
            if (code == EN_CHANGE) {
                wchar_t buf[256];
                GetWindowTextW(hEditSearch, buf, 256);
                searchQuery = buf;
                fillCurrentDir(hwnd);
            }
            break;
        }
        return 0;
    }

    case WM_NOTIFY: {
        LPNMHDR hdr = (LPNMHDR)lParam;
        if (hdr->idFrom == idListView) {
            if (hdr->code == NM_DBLCLK) {
                navigateInto(hwnd);
            }
            else if (hdr->code == LVN_COLUMNCLICK) {
                LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
                sortByColumn(lv->iSubItem);
            }
            else if (hdr->code == NM_RCLICK) {
                LPNMITEMACTIVATE ia = (LPNMITEMACTIVATE)lParam;
                if (ia->iItem >= 0) {
                    ListView_SetItemState(hListView, ia->iItem,
                        LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                    showContextMenu(hwnd);     // клик на файле/папке
                }
                else {
                    showCreateMenu(hwnd);      // клик на пустом месте
                }
            }
            else if (hdr->code == LVN_KEYDOWN) {
                LPNMLVKEYDOWN kd = (LPNMLVKEYDOWN)lParam;
                if (kd->wVKey == VK_DELETE) {
                    deleteSelected(hwnd);      // удаление по клавише Delete
                }
            }
        }
        return 0;
    }

    case WM_ACTIVATEAPP:
        // приложение снова стало активным (вернулись из другой программы) —
        // перечитываем папку, чтобы обновить размеры и даты
        if (wParam == TRUE && !currentPath.empty()) {
            fillCurrentDir(hwnd);
        }
        return 0;

    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;

        // сколько файлов перетащили (0xFFFFFFFF = запрос количества)
        UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

        int errors = 0;
        for (UINT i = 0; i < count; ++i) {
            wchar_t srcPath[MAX_PATH];
            DragQueryFileW(hDrop, i, srcPath, MAX_PATH);   // полный путь i-го файла

            // выделяем имя файла из полного пути
            std::wstring src = srcPath;
            size_t slash = src.rfind(L'\\');
            std::wstring fname = (slash != std::wstring::npos) ? src.substr(slash + 1) : src;

            std::wstring dst = currentPath + L"\\" + fname;

            if (src == dst) continue;   // уже в этой папке — пропускаем

            if (!MoveFileW(src.c_str(), dst.c_str())) {
                errors++;
            }
        }

        DragFinish(hDrop);   // освобождаем данные перетаскивания

        if (errors > 0) {
            wchar_t buf[128];
            wsprintfW(buf, L"Не удалось переместить элементов: %d", errors);
            MessageBoxW(hwnd, buf, L"Ошибка", MB_ICONERROR);
        }

        fillCurrentDir(hwnd);   // обновляем список
        return 0;
    }


    case WM_DESTROY:// сами отправляем,кладет в очередь VM_QUIT,wdproc при выходе
        PostQuitMessage(0);//WM_QUIT
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
