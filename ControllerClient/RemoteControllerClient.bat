@echo off

cd /D "%~dp0"

start remote_client.exe -cconfig.txt -tpad_table.txt
