#include "listview.h"
#include <commctrl.h>
#include <cwctype>//for towlower

extern "C" unsigned __int64 addAsm(unsigned __int64 a, unsigned __int64 b);

std::wstring searchQuery;   // активный поисковый запрос (пустой = поиск выключен)
std::wstring currentPath;
std::vector<FileInfo> files;//what find

void initListView(HWND parent)
{
    hListView = CreateWindowExW(
        0, WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
        0, 0, 0, 0,
        parent, (HMENU)1001, nullptr, nullptr
    );

    ListView_SetExtendedListViewStyle(hListView,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);//str all

    LVCOLUMNW col = {};//what about every column
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    struct { const wchar_t* name; int width; } cols[] = {
        { L"Имя", 350 },
        { L"Размер", 120 },
        { L"Тип", 150 },
        { L"Изменён", 180 },
    };//struct used only here,mas made out of 4 struct

    for (int i = 0; i < 4; ++i) {
        col.pszText = (LPWSTR)cols[i].name;
        col.cx = cols[i].width;
        col.iSubItem = i;
        ListView_InsertColumn(hListView, i, &col);
    }
}

static std::wstring formatSize(ULONGLONG bytes, bool isDir)
{
    if (isDir) return L"";
    wchar_t buf[64];
    if (bytes < 1024)
        swprintf_s(buf, 64, L"%llu Б", bytes);
    else if (bytes < 1024ULL * 1024)
        swprintf_s(buf, 64, L"%llu КБ", bytes / 1024);
    else if (bytes < 1024ULL * 1024 * 1024)
        swprintf_s(buf, 64, L"%llu МБ", bytes / (1024 * 1024));
    else
        swprintf_s(buf, 64, L"%llu ГБ", bytes / (1024ULL * 1024 * 1024));
    return buf;
}//ULL for security,32;

static std::wstring formatTime(FILETIME ft)
{
    SYSTEMTIME st;//struct with comfy poliami
    FILETIME local;
    FileTimeToLocalFileTime(&ft, &local);//all things saved in UTC
    FileTimeToSystemTime(&local, &st);//FILETIME into SYSTEMTIME
    wchar_t buf[64];
    wsprintfW(buf, L"%02d.%02d.%d %02d:%02d",
        st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
    return buf;
}

static std::wstring getFileType(const std::wstring& name, bool isDir)
{
    if (isDir) return L"Папка";
    size_t dot = name.rfind(L'.');
    if (dot == std::wstring::npos) return L"Файл";
    return L"Файл " + name.substr(dot);
}//dot for ile

void fillCurrentDir(HWND mainWnd)
{
    files.clear();

    std::wstring searchPath = currentPath + L"\\*";

    //обход папки
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(searchPath.c_str(), &fd);//заполняет fd данными первого айла и возвращает декриптор h
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(fd.cFileName, L".") == 0) continue;
            if (wcscmp(fd.cFileName, L"..") == 0) continue;

            if (!searchQuery.empty()) {
                std::wstring lname = fd.cFileName;
                std::wstring lq = searchQuery;
                for (auto& c : lname) c = towlower(c);
                for (auto& c : lq) c = towlower(c);
                if (lname.find(lq) == std::wstring::npos) continue;  // не совпало — пропускаем
            }

            FileInfo fi;
            fi.name = fd.cFileName;
            fi.isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;//FILE_ATTRIBUTE_DIRECTORY its a mask by windows
            fi.size = ((ULONGLONG)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
            fi.modTime = fd.ftLastWriteTime;//time of last change

            //Форматирование для отображения
            fi.sizeStr = formatSize(fi.size, fi.isDir);
            fi.typeStr = getFileType(fi.name, fi.isDir);
            fi.dateStr = formatTime(fi.modTime);
            files.push_back(fi);//добавление структуры в вектор
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }

    refreshListView();

    // Подсчёт суммарного размера файлов через ассемблер
    unsigned __int64 total = 0;
    for (size_t i = 0; i < files.size(); ++i) {
        if (!files[i].isDir) {
            total = addAsm(total, files[i].size);
        }
    }

    wchar_t title[1024];
    swprintf_s(title, 1024, L"BAKINI — %s (всего: %llu Б)",
        currentPath.c_str(), total);
    SetWindowTextW(mainWnd, title);
}

void refreshListView()
{
    ListView_DeleteAllItems(hListView);
    for (size_t i = 0; i < files.size(); ++i) {//in vector file struct FileInfo
        LVITEMW item = {};//сброс полей
        item.mask = LVIF_TEXT;
        item.iItem = (int)i;
        item.pszText = (LPWSTR)files[i].name.c_str();

        //создание строки в таблице
        ListView_InsertItem(hListView, &item);
        ListView_SetItemText(hListView, (int)i, 1, (LPWSTR)files[i].sizeStr.c_str());//В таблице hListView, в строке i, выстави для колонки 1 (Размер) текст из sizeStr
        ListView_SetItemText(hListView, (int)i, 2, (LPWSTR)files[i].typeStr.c_str());
        ListView_SetItemText(hListView, (int)i, 3, (LPWSTR)files[i].dateStr.c_str());
    }
}
