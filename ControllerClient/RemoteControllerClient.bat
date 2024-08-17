@echo off

cd /D "%~dp0"

if exist win32\x64\Release\remote_client2022.exe (
copy win32\x64\Release\remote_client2022.exe remote_client.exe
)

start remote_client.exe -cconfig.txt -tpad_table.txt
