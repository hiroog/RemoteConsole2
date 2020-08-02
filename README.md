# UE4 Remote Console Command Plugin

Keyboard が使えない状態で UE4 の Console Command を実行します。
同一 LAN に接続した PC 上で command 入力できます。

## install

1. Python 3.x (3.6以上推奨) を install して path を通しておく
1. RemoteConsole2 フォルダを UE4 Project/Plugins/ 以下に copy


## send Console Command

1. RemoteConsole2/Tool/SendCommand.bat を実行します。
1. host の欄にゲームを起動している Target (PC他) の IP address を入れます。
1. Command を入力して Send ボタンを押します。

UE4 側の Server が ipv6 非対応なので ipv6 の checkbox は off のまま使ってください。

## configuration

Server の Port 番号は 10101 です。
RemoteConsole2/Source/RemoteConsole2/RemoteConsoleServer2.h の Port で変更できます。
