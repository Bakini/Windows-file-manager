#include "sort_search.h"
#include "listview.h"
#include "dialogs.h"
#include <commctrl.h>
#include <algorithm>
#include <cwctype>
static int currentSortCol = -1;
static bool sortAsc = true;
static int compareFiles(const FileInfo& a, const FileInfo& b, int col)
{
    if (a.isDir != b.isDir) return a.isDir ? -1 : 1;
    switch (col) {
    case 0: return _wcsicmp(a.name.c_str(), b.name.c_str());
    case 1: return (a.size < b.size) ? -1 : (a.size > b.size) ? 1 : 0;
    case 2: return _wcsicmp(a.typeStr.c_str(), b.typeStr.c_str());
    case 3: return CompareFileTime(&a.modTime, &b.modTime);
    }
    return 0;
}
void sortByColumn(int col)
{
    if (currentSortCol == col) sortAsc = !sortAsc;
    else { currentSortCol = col; sortAsc = true; }
    bool asc = sortAsc;
    std::sort(files.begin(), files.end(),
        [col, asc](const FileInfo& a, const FileInfo& b) {
            int r = compareFiles(a, b, col);
            return asc ? r < 0 : r > 0;
        });
    refreshListView();
}
void findInList(HWND parent)
{
    std::wstring query = askName(parent, L"Поиск", L"Найти файл по имени (пусто — сброс):");
    searchQuery = query;          // запоминаем запрос (пустой = сброс)
    fillCurrentDir(parent);       // перечитываем папку с учётом поиска
}
