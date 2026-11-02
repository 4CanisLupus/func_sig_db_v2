@echo off
rem Запуск: задайте пароль БД в PGPASSWORD (или раскомментируйте строку ниже)
rem set PGPASSWORD=ваш_пароль
set PATH=C:\msys64\mingw64\bin;%PATH%
cd /d "%~dp0build"
func_sig_db.exe
