#pragma once
#include <windows.h>
#include <string>
#include <vector>
struct FileInfo {
    std::wstring name;
    std::wstring sizeStr;
    std::wstring typeStr;
    std::wstring dateStr;
    ULONGLONG size;
    FILETIME modTime;
    bool isDir;
};
extern HWND hListView;
extern std::wstring searchQuery;//
extern std::wstring currentPath;//хранение текущей рабочей директории
extern std::vector<FileInfo> files;
void initListView(HWND parent);
void fillCurrentDir(HWND mainWnd);
void refreshListView();
