# Python API

API 仕様はまだ未確定です。今後変更される可能性があります。


## API 初期化

Tools/py/RemoteConsole2API.py を improt してください。

api 呼び出しのために ConsoleAPI() オブジェクトを作成します。

```python
from RemoteConsole2API import ConsoleAPI,Controller,Event

api= ConsoleAPI( Options() )
api.connect()
api.send_console_command( 'stat fps' )
api.close()
```

with 構文を使用すると明示的に close() を呼び出す必要がなくなります。

```python
from RemoteConsole2API import ConsoleAPI,Controller,Event

with ConsoleAPI( Options() ) as api:
    api.connect()
    api.send_console_command( 'stat fps' )
```

接続先の IP アドレスやポート番号を指定する場合は Options() オブジェクトを使用します。


```python
options= Options( host='localhost', port=10101 )
with ConsoleAPI( options ) as api:
    api.connect()
    ～
```


host を指定することで他の PC 上で起動したゲームに接続することができます。
また port を変更することで、同一 PC 上の複数のゲームに接続することができます。

Options() に指定可能なパラメータは以下のとおりです。

| オプション   | デフォルト | 内容                                       |
|:-------------|:-----------|:-------------------------------------------|
| host         | localhost  | 接続するホスト名または IP アドレス         |
| port         | 10101      | 接続するポート番号                         |
| ipv          | 4          | IPV4 の場合は 4、IPv6 の場合は 6 を設定    |
| echo\_log    | True       | logger 起動時にログを表示するかどうか      |
| log\_limit   | 6000       | logger が保持するログの最大行数            |
| color        | True       | ログ表示とコマンドの色分け表示を行う       |
| net\_echo    | False      | デバッグ用、送受信したコマンドを表示する   |


ConsoleAPI() オブジェクトはゲームとの接続セッションを所有しています。
もし同時に複数のゲームと接続を行いたい場合は、複数の ConsoleAPI() を作成してください。

複数のゲームを起動して同時に接続を行う例

```python
api1= ConsoleAPI( Options( port=10101 ) )
api1.connect()

api2= ConsoleAPI( Options( port=10102 ) )
api2.connect()

api3= ConsoleAPI( Options( port=10103 ) )
api3.connect()

～
```


複数のゲームを起動して同時にコンソールコマンドを実行する例。

```python
api_list= []
# それぞれ接続待ちを行う
for port in [ 10101, 10102, 10103 ]:
    api= ConsoleAPI( Options( port=port ) )
    api.connect()
    api_list.append( api )

# 同時にコンソールコマンドを送信する
for api in api_list:
    api.send_console_command( 'stat fps' )

# 切断する
for api in api_list:
    api.close()
```



# API 詳細

## 接続

### ゲームとの接続を行います

```python
api.connect()
```

ゲーム側の準備ができていない場合は接続できるようになるまで待機します。
そのため先にスクリプトを起動しておき、あとからゲームを起動することができます。


### ゲームから切断します

```python
api.close()
```

with 構文を使用した場合は、スコープを抜けたタイミングで自動的に close() が呼ばれます。



## Logger の起動

ゲームと相互通信を行う場合は受信スレッドが必要です。
コマンドの送信のみ行う場合は不要なので、デフォルトでは無効化されています。
ログの受信やパラメータの受取など、相互通信を行う場合は connect() のあとに ```start_logger()``` を呼び出してください。


```python
with ConsoleAPI( options ) as api:
    api.connect()
    api.start_logger()
```


## Console Command の送信


### コンソールコマンドを実行します

```python
api.send_console_command( 'stat fps' )
```



## コントローラー入力

### コントローラーステートオブジェクトを作成します

```python
controller= Controller()
```

状態の送信には ```api.send_controller()``` を使用します。



### コントローラー状態の送信

```python
api.send_controller( controller )
```

ステートオブジェクトを使用して現在のコントローラーの状態を送信します。

状態変更 API はそれぞれ self を返すため、"." で接続して記述することができます。


```python
# 何も押していない状態を送信
api.send_controller( Controller().reset() )
```

```python
# A と X ボタンを同時に押した状態を送信
api.send_controller( Controller().on( 'A' ).on( 'X' ) )
```

```python
controller= Controller().reset()

# A ボタンを押した状態を送信
api.send_controller( controller.on( 'A' ) )

# A ボタンを押したまま左スティックを上に倒した状態を送信
api.send_controller( controller.ly( -1.0 ) )

# A ボタンを離して左スティックを上に倒したまま右スティックを下に倒した状態を送信
api.send_controller( controller.off( 'A' ).ry( 1.0 ) )
```


### ボタンの状態変更

```python
controller.on( BUTTON_NAME )
controller.off( BUTTON_NAME )
```

コントローラーのデジタルボタンを操作します。


| BUTTON\_NAME  | Xbox         | PS4/PS5         | Switch        |
|:--------------|:-------------|:----------------|:--------------|
| 'U'           | ↑           | ↑              | ↑            |
| 'D'           | ↓           | ↓              | ↓            |
| 'L'           | ←           | ←              | ←            |
| 'R'           | →           | →              | →            |
| 'A'           | A            | ×              | B             |
| 'B'           | B            | ◯              | A             |
| 'X'           | X            | □              | Y             |
| 'Y'           | Y            | △              | X             |
| 'L1'          | ショルダーL  | L1              | L             |
| 'R1'          | ショルダーR  | R1              | R             |
| 'L2'          | トリガーL    | L2              | ZL            |
| 'R2'          | トリガーR    | R2              | ZR            |
| 'L3'          | サムボタンL  | L3              | L StickButton |
| 'R3'          | サムボタンR  | R3              | R StickButton |
| 'M0'          | BACK/View    | SHARE/CREATE    | -             |
| 'M1'          | START/Menu   | OPTIONS         | +             |
| 'M2'          |              | PS              | HOME          |
| 'M3'          |              | TouchPad Click  | Capture       |


### スティックの状態変更


```python
controller.lx( STICKVALUE )
controller.ly( STICKVALUE )
controller.rx( STICKVALUE )
controller.ry( STICKVALUE )
```

STICKVALUE には -1.0～1.0 の値を指定します。
この範囲を超えた場合は -1.0～1.0 の範囲にクランプされます。



### トリガーの状態変更


```python
controller.tr( TRIGGERVALUE )
controller.tl( TRIGGERVALUE )
```

TRIGGERVALUE には 0.0～1.0 の値を指定します。
この範囲を超えた場合は 0.0～1.0 の範囲にクランプされます。



### コントローラー状態のリセット

```python
controller.reset()
```

ボタンやトリガーをすべて離した状態にし、スティックをニュートラルに戻します。
Controller() オブジェクト作成直後は reset() 状態と同じです。




### コントローラーのボタンを押してすぐ離す動作をシミュレートします

```python
api.click_controller( BUTTON_NAME, controller=None, duration=0.1 )
```

controller オブジェクトが None の場合は内部で新規作成します。
duration で押している時間(秒)を指定できます。



## キー入力

### キーボードイベントを送信します

```python
api.send_key( KEY_NAME, ACTION )
```

KEY\_NAME には 'A'～'Z', 'SPACE', 'RETURN', 'SHIFT' 等のキー名を指定します。
ACTION には Event.KEY\_UP, Event.KEY\_DOWN, Event.KEY\_CHAR を指定できます。

表にないキーを送信したい場合は追加するので連絡ください。


| KEY\_NAME        | 説明             |
|:-----------------|:-----------------|
| 'A'～'Z',その他記号等   | 文字キー  |
| 'BACK'           | BACKSPACE キー   |
| 'TAB'            | TAB キー         |
| 'RETURN'         | RETURN キー      |
| 'SHIFT'          | SHIFT キー       |
| 'CONTROL'        | CONTROL キー     |
| 'MENU'           | ALT キー         |
| 'ESCAPE'         | ESCAPE キー      |
| 'SPACE'          | SPACE キー       |
| 'PRIOR'          | PageUp キー      |
| 'NEXT'           | PageDown キー    |
| 'END'            | END キー         |
| 'HOME'           | HOME キー        |
| 'LEFT'           | ← キー          |
| 'UP'             | ↑ キー          |
| 'RIGHT'          | → キー          |
| 'DOWN'           | ↓ キー          |
| 'F1'             | F1 キー          |
| 'F2'             | F2 キー          |
| 'F3'             | F3 キー          |
| 'F4'             | F4 キー          |
| 'F5'             | F5 キー          |
| 'F6'             | F6 キー          |
| 'F7'             | F7 キー          |
| 'F8'             | F8 キー          |
| 'F9'             | F9 キー          |
| 'F10'            | F10 キー         |
| 'F11'            | F11 キー         |
| 'F12'            | F12 キー         |



| ACTION           | 説明   |
|:-----------------|:-------|
| Event.KEY\_DOWN  | キーを押します        |
| Event.KEY\_UP    | キーを離します        |
| Event.KEY\_CHAR  | キーコードではなく文字入力として送信します        |


### キーを押してすぐに離す動作をシミュレートします

```python
api.click_key( KEY_NAME, duration=0.1 )
```

duration は押している時間(秒)を指定します。



## マウス入力

### マウスのボタンイベントを送信します

```python
api.send_mouse_button( BUTTON_NAME, ACTION )
```


| BUTTON\_NAME     | 説明            |
|:-----------------|:----------------|
| 'L'              | 左ボタン        |
| 'M'              | 中ボタン        |
| 'R'              | 右ボタン        |
| 'T1'             | サイドボタン1   |
| 'T2'             | サイドボタン2   |



| ACTION               | 説明                  |
|:---------------------|:----------------------|
| Event.MOUSE\_DOWN    | ボタンを押します      |
| Event.MOUSE\_UP      | ボタンを離します      |
| Event.MOUSE\_DCLICK  | ダブルクリックします  |



### マウスのカーソル移動イベントを発行します

```python
api.send_mouse_move( cursor_x, cursor_y )
```

相対移動します。
一般的なマウスの MOVE イベントをシミュレートします。



### マウスのカーソルの位置を変更します

```python
api.send_mouse_setpos( cursor_x, cursor_y )
```

絶対座標で直接カーソル位置を指定します。



## UMG 操作

### UMG のボタンに関するイベントを送信します

```python
api.send_ui_button( BUTTON_NAME, ACTION )
```

BUTTON\_NAME は UMG Widget の内部名称です。
この名前は ```api.send_ui_dump( Event.DUMP_BUTTON )``` コマンドを使用して調べることができます。



| ACTION                       | 説明                                      |
|:-----------------------------|:------------------------------------------|
| Event.UI\_BUTTON\_CLICKED    | UMG のボタンを押して離します              |
| Event.UI\_BUTTON\_PRESSED    | UMG のボタンを離します                    |
| Event.UI\_BUTTON\_RELEASED   | UMG のボタンを離します                    |
| Event.UI\_BUTTON\_HOVERED    | UMG の上にカーソルを乗せた状態にします    |
| Event.UI\_BUTTON\_UNHOVERED  | UMG の上からカーソルが離れた状態にします  |



### UMG の Widget の状態を調べます

```python
api.send_ui_dump( EVENT )
```

| EVENT                   | 説明                                        |
|:------------------------|:--------------------------------------------|
| Event.UI\_DUMP\_ALL     | UMG の Widget 一覧を UE のログに表示します  |
| Event.UI\_DUMP\_BUTTON  | UMG の Button 一覧を UE のログに表示します  |



### フォーカスを制御します

```python
api.set_focus( WIDGET_NAME, force=False )
```

* WIDGET\_NAME で指定した Widget をフォーカス状態にします。
* WIDGET\_NAME が None の場合は、フォーカスを UI ではなく Game 入力に切り替えます。
* force が True の場合は、同時に Windows 上でゲームウィンドウを強制的に最前面にします。



## Console 変数の読み取り

### コンソール変数の値を読み取ります

```python
result= api.get_console_var( VARIABLE_NAME )
```

このコマンドは logger を使用します。起動時に ```api.start_logger()``` を呼び出しておく必要があります。

受け取った値は Feature() オブジェクトに格納されるため、値を読み出す場合は get() を使用してください。

例

```python
resolution= api.get_console_var( 'r.SetRes' ).get()
```

get() の呼び出しを必要なタイミングまで遅らせることで同期処理の並列化ができます。


```python
resolution= api.get_console_var( 'r.SetRes' )

～ # 同期待ちせずに、この間に別の処理が可能

print( resolution.get() )
```

変数の値はすべて文字列で受け取ります。

コンソール変数の書き込みには ```api.send_console_command()``` を使用してください。

```python
if int(api.get_console_var( 'r.HDR.EnableHDROutput' ).get()) == 0:
    api.send_console_command( 'r.HDR.EnableHDROutput 1' )
```



## ログの監視

ログの情報アクセスするには logger の起動が必要です。最初の接続時に ```api.start_logger()``` を実行しておいてください。
ゲーム中に出力されたログのモニタリングを行います。

保持可能なログの最大行数は Options() の log_limit で変更できます。
デフォルトは 6000 行です。



### ログの出力待ち


```python
api.wait_log( PATTERN )
```

受け取ったログが引数で与えた PATTERN にマッチするまで待機します。
PATTERN には Python の正規表現を指定します。

この命令を使うことで対話的なスクリプトの実行を行うことができます。

wait_log() はマッチしたログの該当行を返します。
そのためパターンマッチを利用したログの解析やデータの読み取りを行うこともできます。

ログのモニタリングはバックグラウンドで常時行われているため、```api.wait_log()``` の呼び出しよりも前に受け取ったログにもマッチさせることが可能です。
そのため、スクリプトの命令実行タイミングのわずかなずれによって動作が不安定になるのを防いでいます。

例えば以下 wait_log() は、直前の send_print_string() の表示を見て次に進むことができます。

```python
api.send_print_string( 'LogString' )

api.wait_log( 'LogString' ) # 停止しないで直前の print_string() の結果にマッチして次に進むことができる
```

もし wait_log() 実行後のログだけを監視するようにしてしまうと、通信速度や実行タイミングによってマッチしたりマッチできなくなったりします。

また以下の例のように、同じ文字列に対する表示待ちを連続して行ってもきちんと動作します。

```python
api.send_print_string( 'LogString' ) # (A)

api.wait_log( 'LogString' ) # (A) によって次に進む

api.send_print_string( 'LogString' ) # (B)

api.wait_log( 'LogString' ) # (A) ではなく (B) によって次に進む
```

wait_log() のログのマッチングは次のような仕組みで動いています。

* 受け取ったログにはすべて行単位で異なる ID 番号が割り振られる
* wait_log() は前回マッチした ID + 1 の行から検索を行う


なお意図的に検索を開始する ID を書き換えることが可能です。
これによりログの待機開始位置を自由に変更することができます。


#### 現在の検索開始場所を取得

```python
index= api.logger.get_index()
```

#### 検索開始場所を設定

```python
api.logger.set_index( index )
```

例えば wait_log() の前に set_index( 0 ) を行うと、現在バッファリングしているすべてのログをマッチング対象とみなすことができます。

また wait_log() の代わりに find_log() を使って任意のログの文字列を参照することも可能です。


#### 同期待ちをしないで解析のみ行う場合

```python
result,index= api.logger.find_log( PATTERN )
```

PATTERN は Python の正規表現です。
result には PATTERN の結果が入ります。
マッチしなかった場合は None です。
index はマッチした行の ID 番号です。

この命令も wait_log() 同様 set_index() の影響を受けます。

バッファリングしているログ全体から特定の文字列を探して値を読み取りたい場合の例

```python
save_index= api.logger.get_index() # 現在の Index 値を保持
api.logger.set_index( 0 ) # 検索範囲をログ全体にする
result,index= api.logger.find_log( r'^LogDummy:Data=(\d+)' )

# 検索がヒットした場合のみ
if result:
    # "LogDummy:Data=数字" の数字だけを値として読み取る
    data= int( result.group( 1 ) )

api.logger.set_index( save_index ) # Index 値を戻す
```


## リプレイ機能

RemoteControllerClient を使用して、コントローラーやキー操作の送信データを記録することができます。
以下の API を使用して記録したデータを再生できます。


### リプレイ

```python
api.replay( FILE_NAME )
```

記録したデータのファイル名を与えてください。
すぐに再生が始まります。

3D シーンの移動情報はフレームレートの影響を強く受けます。
そのためアナログ操作は厳密に同じ挙動とならないので注意してください。
特にカメラの回転は、ごく僅かなずれでも移動先が大きく変化します。



## その他の API

### 時間待ちを行います

```python
api.sleep( DURATION )
```

指定時間スクリプトの実行を停止します。
秒単位で指定します。

### 現在のレベル名を取得します

```python
api.get_level_name()
```

この命令は get_console_var() 同様 Feature() を返すので、値の読み出しには get() が必要です。
logger が必要です。


## マルチセッション

RemoteConsole2 Plugin はマルチセッションに対応しており複数の Client が同時に接続してコマンドの送信を行うことが可能です。ただし logger を起動できるのはそのうちの 1つだけに制限されます。


例えば 1つのセッションでリプレイを再生しながら、他のセッションでログ待ちなどの対話的なスクリプトを同時に走らせるような使い方もできます。


## 更新

2024/12/21 小笠原博之


