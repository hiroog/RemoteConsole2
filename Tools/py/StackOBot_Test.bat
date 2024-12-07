python SendCommandLine.py --cmd open MainMenu --sleep 4
python SendCommandLine.py --ui_clicked UMG_Button_Start,ButtonWidget --sleep 4
python SendCommandLine.py --ly 1 --sleep 3
python SendCommandLine.py --on A --sleep 1 --pad_reset --sleep 3
python SendCommandLine.py --ly -1 --sleep 3
python SendCommandLine.py --on A --sleep 1 --pad_reset --sleep 3
python SendCommandLine.py --key_down ESCAPE --sleep 1 --key_up ESCAPE --sleep 4
python SendCommandLine.py --ui_clicked UMG_Button_Reset,ButtonWidget --sleep 4
python SendCommandLine.py --key_down ESCAPE --sleep 1 --key_up ESCAPE --sleep 4
python SendCommandLine.py --ui_clicked UMG_Button_Quit,ButtonWidget

pause
