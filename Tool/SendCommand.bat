@echo off
if exist %~dp0\Python\python.exe (
    %~dp0\Python\python.exe %~dp0\py\SendCommandGUI.py -c %~dp0\py
) else (
    python %~dp0\py\SendCommandGUI.py -c %~dp0\py
)

