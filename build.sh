#!/bin/sh
x86_64-w64-mingw32-g++ -O2 -municode -mwindows -static \
main.cpp listview.cpp actions.cpp dialogs.cpp sort_search.cpp asm_bro.S \
-lcomctl32 -o BAKINI.exe
