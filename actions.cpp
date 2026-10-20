#include "actions.h"
#include "listview.h"
#include "dialogs.h"
#include <commctrl.h>
#include <shellapi.h>//ShellExecuteW
#include <vector>

static std::vector<std::wstring> pathHistory;
static std::wstring clipboardPath;   // путь вырезанного файла

void createNewFile(HWND parent)
{
    int n = 1;
    std::wstring name;
    while (true) {
        wchar_t buf[64];
        wsprintfW(buf, L"new_file_%d.txt", n);
        name = buf;
        std::wstring full = currentPath + L"\\" + name;//полный путь к файлу
        if (GetFileAttributesW(full.c_str()) == INVALID_FILE_ATTRIBUTES) break;//апрашивает атрибуты файла. .c_str() преобразует std::wstring в указатель на массив символов const wchar_t*
        n++;
    }

    std::wstring full = currentPath + L"\\" + name;
    HANDLE h = CreateFileW(full.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);//GENERIC_WRITE — открываем с правами на запись,также выдаст ошибку если файл уже есть(якобы это флаг)
    if (h == INVALID_HANDLE_VALUE) {
        MessageBoxW(parent, L"Не удалось создать файл", L"Ошибка", MB_ICONERROR);
        return;
    }
    CloseHandle(h);
    fillCurrentDir(parent);
}

void createNewFolder(HWND parent)
{
    std::wstring name = askName(parent, L"Создать папку", L"Введите имя папки:");
    if (name.empty()) return;

    std::wstring full = currentPath + L"\\" + name;
    if (!CreateDirectoryW(full.c_str(), nullptr)) {
        MessageBoxW(parent, L"Не удалось создать папку", L"Ошибка", MB_ICONERROR);
        return;
    }
    fillCurrentDir(parent);
}

void deleteSelected(HWND parent)
{
    // собираем индексы всех выделенных элементов
    std::vector<int> sel;
    int idx = -1;
    while ((idx = ListView_GetNextItem(hListView, idx, LVNI_SELECTED)) != -1) {
        sel.push_back(idx);
    }

    if (sel.empty()) {
        MessageBoxW(parent, L"Ничего не выделено", L"Удаление", MB_ICONINFORMATION);
        return;
    }

    // подтверждение(одно на всё)
    std::wstring msg;
    if (sel.size() == 1)
        msg = L"Удалить «" + files[sel[0]].name + L"»?";
    else
        msg = L"Удалить выбранные элементы (" + std::to_wstring(sel.size()) + L")?";

    if (MessageBoxW(parent, msg.c_str(), L"Подтверждение",
        MB_YESNO | MB_ICONQUESTION) != IDYES) return;

    // удаляем каждый
    int errors = 0;
    for (size_t i = 0; i < sel.size(); ++i) {
        int k = sel[i];
        std::wstring fullPath = currentPath + L"\\" + files[k].name;
        BOOL ok;
        if (files[k].isDir) ok = RemoveDirectoryW(fullPath.c_str());
        else ok = DeleteFileW(fullPath.c_str());
        if (!ok) errors++;
    }

    if (errors > 0) {
        wchar_t buf[128];
        wsprintfW(buf, L"Не удалось удалить элементов: %d", errors);
        MessageBoxW(parent, buf, L"Ошибка", MB_ICONERROR);
    }

    fillCurrentDir(parent);
}

void navigateInto(HWND parent)
{
    int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (idx < 0) return;

    if (files[idx].isDir) {
        pathHistory.push_back(currentPath);
        currentPath = currentPath + L"\\" + files[idx].name;
        fillCurrentDir(parent);
    }
    else {
        std::wstring fullPath = currentPath + L"\\" + files[idx].name;
        ShellExecuteW(parent, L"open", fullPath.c_str(), nullptr, nullptr, SW_SHOW);
    }
}

void navigateBack(HWND parent)
{
    if (pathHistory.empty()) {
        MessageBoxW(parent, L"Это уже корневая папка", L"Назад", MB_ICONINFORMATION);
        return;
    }
    currentPath = pathHistory.back();
    pathHistory.pop_back();
    fillCurrentDir(parent);
}


//static - спецификатор видимости


void renameSelected(HWND parent)
{
    int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (idx < 0) {
        MessageBoxW(parent, L"Ничего не выделено", L"Переименование", MB_ICONINFORMATION);
        return;
    }

    std::wstring oldName = files[idx].name;
    std::wstring newName = askName(parent, L"Переименование", L"Новое имя:");
    if (newName.empty()) return;

    // для файлов сохраняем старое расширение, если пользователь его не указал
    if (!files[idx].isDir) {
        size_t oldDot = oldName.rfind(L'.');
        if (oldDot != std::wstring::npos && newName.rfind(L'.') == std::wstring::npos) {
            newName += oldName.substr(oldDot);   // приклеиваем ".txt", ".png" и т.п.
        }
    }

    std::wstring oldPath = currentPath + L"\\" + oldName;
    std::wstring newPath = currentPath + L"\\" + newName;
    if (!MoveFileW(oldPath.c_str(), newPath.c_str())) {
        MessageBoxW(parent, L"Не удалось переименовать", L"Ошибка", MB_ICONERROR);
        return;
    }
    fillCurrentDir(parent);
}

void createFileNamed(HWND parent)
{
    std::wstring name = askName(parent, L"Создать файл", L"Имя файла с расширением:");
    if (name.empty()) return;

    std::wstring full = currentPath + L"\\" + name;
    HANDLE h = CreateFileW(full.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        MessageBoxW(parent, L"Не удалось создать файл (возможно, уже существует)",
            L"Ошибка", MB_ICONERROR);
        return;
    }
    CloseHandle(h);
    fillCurrentDir(parent);
}

void cutSelected(HWND parent)
{
    int idx = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    if (idx < 0) {
        MessageBoxW(parent, L"Ничего не выделено", L"Вырезать", MB_ICONINFORMATION);
        return;
    }
    clipboardPath = currentPath + L"\\" + files[idx].name;// запоминаем путь
}

void pasteFile(HWND parent)
{
    if (clipboardPath.empty()) return;

    // выделяем имя файла из запомненного пути
    size_t slash = clipboardPath.rfind(L'\\');
    std::wstring fname = (slash != std::wstring::npos)
        ? clipboardPath.substr(slash + 1) : clipboardPath;

    std::wstring dst = currentPath + L"\\" + fname;
    if (clipboardPath == dst) return;   // уже в этой папке

    if (!MoveFileW(clipboardPath.c_str(), dst.c_str())) {
        MessageBoxW(parent, L"Не удалось вставить файл", L"Ошибка", MB_ICONERROR);
        return;
    }
    clipboardPath.clear();// буфер очищается — файл перемещён
    fillCurrentDir(parent);
}

bool hasClipboard()
{
    return !clipboardPath.empty();
}

void createTextFileNamed(HWND parent)
{
    // подбираем свободное автоимя new_file_N.txt для подсказки
    int n = 1;
    std::wstring suggested;
    while (true) {
        wchar_t buf[64];
        wsprintfW(buf, L"new_file_%d.txt", n);
        suggested = buf;
        std::wstring full = currentPath + L"\\" + suggested;
        if (GetFileAttributesW(full.c_str()) == INVALID_FILE_ATTRIBUTES) break;
        n++;
    }

    // спрашиваем имя, подставляя автоимя по умолчанию
    std::wstring name = askName(parent, L"Текстовый документ", L"Имя файла:", suggested.c_str());
    if (name.empty()) return;

    // текстовый документ всегда создаётся с расширением .txt
    size_t dot = name.rfind(L'.');
    if (dot != std::wstring::npos) name.erase(dot);// отбрасываем введённое расширение, если есть
    name += L".txt";

    std::wstring full = currentPath + L"\\" + name;
    HANDLE h = CreateFileW(full.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        MessageBoxW(parent, L"Не удалось создать файл (возможно, уже существует)",
            L"Ошибка", MB_ICONERROR);
        return;
    }
    CloseHandle(h);
    fillCurrentDir(parent);
}
