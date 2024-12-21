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

with 構文を使用することができます。
スクリプト終了時に自動的に切断できるためこちらの方法をお勧めします。

```python
from RemoteConsole2API import ConsoleAPI,Controller,Event

with ConsoleAPI( Options() ) as api:
    api.connect()
    api.send_console_command( 'stat fps' )
```

接続先の IP アドレスやポート番号を指定する場合は Options() オブジェクトを使用します。


```python
options= Options()
options.host= 'localhost'
options.port= 10101

with ConsoleAPI( options ) as api:
    api.connect()
    ～
```


ConsoleAPI() オブジェクトはゲームとの接続セッションを所有しています。
もし同時に複数のゲームと接続を行いたい場合は、複数の ConsoleAPI() を作成してください。




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
api.send_controller( Controller().reset().on( 'A' ).on( 'X' ) )
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


| BUTTON\_NAME     | 説明   |
|:-----------------|:-------|
| 'U'              | ↑ ボタン       |
| 'D'              | ↓ ボタン       |
| 'L'              | ← ボタン       |
| 'R'              | → ボタン       |
| 'A'              | A ボタン/×     |
| 'B'              | B ボタン/◯     |
| 'X'              | X ボタン/□     |
| 'Y'              | Y ボタン/△     |
| 'L1'             | L1 ボタン/ショルダーL      |
| 'R1'             | R1 ボタン/ショルダーR      |
| 'L2'             | L2 ボタン/トリガーL        |
| 'R2'             | R2 ボタン/トリガーR        |
| 'L3'             | L3 ボタン/サムボタンL      |
| 'R3'             | R3 ボタン/サムボタンR      |
| 'M0'             | SELECT/BACK/SHARE      |
| 'M1'             | START/OPTIONS          |
| 'M2'             | PS/X/HOME              |
| 'M3'             | TouchPad Click         |


### スティックの状態変更


```python
controller.lx( STICKVALUE )
controller.ly( STICKVALUE )
controller.rx( STICKVALUE )
controller.ry( STICKVALUE )
```

STICKVALUE には -1.0～1.0 の値を指定します。



### トリガーの状態変更


```python
controller.tr( TRIGGERVALUE )
controller.tl( TRIGGERVALUE )
```

TRIGGERVALUE には 0.0～1.0 の値を指定します。



### コントローラー状態のリセット

```python
controller.reset()
```

ボタンやトリガーをすべて離した状態にし、スティックをニュートラルに戻します。




### コントローラーのボタンを押してすぐ離す動作をシミュレートします

```python
api.click_controller( BUTTON_NAME, controller=None, duration=0.1 )
```









## キー入力

### キーボードイベントを送信します

```python
api.send_key( KEY_NAME, ACTION )
```

KEY\_NAME には 'A'～'Z', 'SPACE', 'RETURN', 'SHIFT' 等のキー名を指定します。
ACTION には Event.KEY\_UP, Event.KEY\_DOWN, Event.KEY\_CHAR を指定できます。


| KEY\_NAME        | 説明   |
|:-----------------|:-------|
| 'A'～'Z',その他記号等   | 文字キー    |
| 'BACK'           | BACKSPACE キー     |
| 'TAB'            | TAB キー           |
| 'SHIFT'          | SHIFT キー         |
| 'CONTROL'        | CONTROL キー       |
| 'RETURN'         | RETURN キー        |
| 'ESCAPE'         | ESCAPE キー        |
| 'SPACE'          | SPACE キー         |
| 'LEFT'           | ← キー            |
| 'UP'             | ↑ キー            |
| 'RIGHT'          | → キー            |
| 'DOWN'           | ↓ キー            |



| ACTION           | 説明   |
|:-----------------|:-------|
| Event.KEY\_DOWN  | キーを押します        |
| Event.KEY\_UP    | キーを離します        |
| Event.KEY\_CHAR  | キーコードではなく文字入力として送信します        |


### キーを押してすぐに離す動作をシミュレートします

```python
api.click_key( KEY_NAME, duration=0.1 )
```






## マウス入力

### マウスのボタンイベントを送信します

```python
api.send_mouse_button( BUTTON_NAME, ACTION )
```


| BUTTON\_NAME     | 説明   |
|:-----------------|:-------|
| 'L'              | 左ボタン     |
| 'M'              | 中ボタン     |
| 'R'              | 右ボタン     |
| 'T1'             | サイドボタン1    |
| 'T2'             | サイドボタン2    |



| ACTION               | 説明   |
|:---------------------|:-------|
| Event.MOUSE\_DOWN    | ボタンを押します        |
| Event.MOUSE\_UP      | ボタンを離します        |
| Event.MOUSE\_DCLICK  | ダブルクリックします    |



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



| ACTION               | 説明   |
|:---------------------|:-------|
| Event.UI\_BUTTON\_CLICKED    | UMG のボタンを押して離します  |
| Event.UI\_BUTTON\_PRESSED    | UMG のボタンを離します        |
| Event.UI\_BUTTON\_RELEASED   | UMG のボタンを離します        |
| Event.UI\_BUTTON\_HOVERED    | UMG の上にカーソルを乗せた状態にします     |
| Event.UI\_BUTTON\_UNHOVERED  | UMG の上からカーソルが離れた状態にします   |



### UMG の Widget の状態を調べます

```python
api.send_ui_dump( EVENT )
```

| EVENT               | 説明   |
|:---------------------|:-------|
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

このコマンドを利用する場合は事前に ```api.start_logger()``` の呼び出しが必要です。

受け取った値は Feature() オブジェクトに格納されるため、値を読み出す場合は get() を使用してください。

例

```python
resolution= api.get_console_var( 'r.SetRes' ).get()
```

get() の呼び出しを必要なタイミングまで遅らせることで同期処理の並列化ができます。


```python
resolution= api.get_console_var( 'r.SetRes' )

～

print( resolution.get() )
```

変数の値はすべて文字列で受け取ります。

コンソール変数の書き込みには ```api.send_console_command()``` を使用してください。




## ログの受取

```api.start_logger()``` を実行すると、ゲーム中に出力されたログのモニタリングを行うようになります。


これを利用してログ出力の解析や表示待ちなどの同期を行うことが可能です。

ログの保持可能な最大行数は Options().log_limit で指定します。
デフォルトは 6000 行です。



### ログの出力待ち


```python
api.wait_log( PATTERN )
```

受け取ったログが引数で与えた PATTERN にマッチするまで待機します。
PATTERN には Python の正規表現を渡します。

wait_log() はマッチしたログの該当行を返します。
そのためパターンマッチを利用したログの解析やデータの読み取りを行うこともできます。


ログのモニタリングは ```api.start_logger()``` 実行後バックグラウンドで常時行われているため、```api.wait_log()``` の呼び出しよりも前に受け取ったログにもマッチさせることが可能です。

この仕様によって、スクリプトの命令実行タイミングのわずかなずれによって、ゲームとの同期が安定しなくなるのを防いでいます。

以下の例では wait_log() は、直前の send_print_string() の表示を必ず受け取ることができます。


```python
api.send_print_string( 'LogString' )

api.wait_log( 'LogString' ) # 停止しないで直前の print_string() の結果を見て次に進む
```

もし wait_log() 実行後のログだけを監視するようにしてしまうと、通信速度や実行タイミングの影響を強く受けます。
例えば print_log() の実行が遅れた場合は wait_log() が文字列を受け取り次に進み、print_log() の表示の方が早かった場合は wait_log() でスクリプトの実行が停止してしまいます。
遡ってログを監視することでこのような不確実性を避けることができます。


また以下の例のように、同じ文字列に対する表示待ちを連続して行っても意図したとおりに動作します。


```python
api.send_print_string( 'LogString' ) # (A)

api.wait_log( 'LogString' ) # (A) によって次に進む

api.send_print_string( 'LogString' ) # (B)

api.wait_log( 'LogString' ) # (A) ではなく (B) によって次に進む
```

wait_log() のログ解析には次のような仕組みが用いられています。

* 受け取ったログにはすべて行単位で異なる ID 番号が割り振られる
* wait_log() は前回マッチした ID + 1 の行から検索を行う

次の手順で wait_log() が検索を開始する ID を意図的に書き換えることが可能です。
これによりログの待機開始位置を任意に変更することができます。


#### 現在の検索開始場所を取得

```python
api.logger.get_index()
```

#### 検索開始場所を設定

```python
api.logger.set_index( index )
```

例えば wait_log() の前に set_index( 0 ) を行うと、現在バッファリングしているすべてのログを解析対象とみなすことができます。


#### 同期待ちをしないで解析のみ行う場合

```python
result,index= api.logger.find_log( PATTERN )
```

result には PATTERN の検索結果が入ります。
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



## 更新

2024/12/21 小笠原博之



