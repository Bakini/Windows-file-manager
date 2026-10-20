#pragma once
#include <windows.h>
void createNewFile(HWND parent);
void createNewFolder(HWND parent);
void deleteSelected(HWND parent);
void navigateInto(HWND parent);
void navigateBack(HWND parent);
void cutSelected(HWND parent);
void pasteFile(HWND parent);
bool hasClipboard();
void renameSelected(HWND parent);
void createFileNamed(HWND parent);
void createTextFileNamed(HWND parent);//
