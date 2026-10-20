#pragma once
#include <windows.h>
#include <string>
std::wstring askName(HWND parent, const wchar_t* title, const wchar_t* prompt, const wchar_t* initial = L"");
