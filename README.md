# UE5 RemoteConsole2 Plugin

UE5 にネットワーク経由でさまざまなコマンドの送信や操作ができます。
Windows PC 及び Android/Linux で動作確認しています。


## Install 手順

1. Python 3.x (3.6以上推奨) をインストールしてパスを通しておきます
2. RemoteConsole2 フォルダを UE5 Project/Plugins/ 以下にコピーします


## Console Command 送信の例

1. RemoteConsole2/Tool/SendCommand.bat を実行します。
2. host の欄にゲームを起動している Target (PC 等) の IP address を入れます。
3. Command を入力して Send ボタンを押します。


## コマンドライン版

直接 Tool/py/SendCommandLine.py を使用することができます。
Console Command だけでなく擬似的なキーボードやコントローラーの入力もできます。

これはログの取得を行うコマンドの例です。DefaultEngine.ini で bCaptureLog=true の設定が必要です。
```
python SendCommandLine.py --bg_logger --log_echo --sleep 300
```


## Configuration

DefaultEngine.ini でポート番号設定やログ取得の許可ができます。


```ini
[RemoteConsole2Plugin]
bEnabled=true
bCaptureLog=true
Port=10101
IPV=4
```

