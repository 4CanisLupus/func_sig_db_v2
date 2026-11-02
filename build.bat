@echo off
rem Сборка под Windows (MinGW-w64 из MSYS2, libpq из mingw-w64-x86_64-postgresql)
set PATH=C:\msys64\mingw64\bin;%PATH%
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release || exit /b 1
cmake --build build || exit /b 1
