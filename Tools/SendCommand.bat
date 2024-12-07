@echo off
if exist %~dp0\Bin\python.exe (
    %~dp0\Bin\python.exe %~dp0\py\SendCommandGUI.py -c %~dp0\py
) else (
    python %~dp0\py\SendCommandGUI.py -c %~dp0\py
)

