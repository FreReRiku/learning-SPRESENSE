# hires_player.ino

## About this skectch

このスケッチはハイレゾ音源を再生するためのSpresense用スケッチです。

Arduino IDEを使用している場合は書き込むだけでそのまま使用することができます。

## Technical Specification

### プレイヤー仕様

- クロックモード: Hi-res Audio
- 出力デバイス: スピーカー
- デコード設定: ステレオwavファイル
- サンプリングレート: 96000Hz
- 量子化ビット数: 24bit
- システムディレクトリの場所: /mnt/sd0/BIN
- 開くファイル名: "HiResSound.wav" (テスト用)
- マスターボリューム: -16.0dB (初期値)

### Detailed Specifications

- 使用するライブラリ
    - SDHCI.h
    - Audio.h

- インスタンスの宣言

    - SDClass
    - AudioClass
    - File

- bool型変数 ErrEnd を falseで初期化

- コールバック関数(audio_attention_cb)の作成


#### setup関数内

- オーディオシステムの開始

- オーディオシステムを初期化(引数: audio_attention_cb)

- クロックモードの設定

- 出力先設定 (I2Sに変更したい場合は、引数に"AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT"を指定する)

- プレイヤー初期化(以下引数)
  - 初期化するプレイヤー
  - 再生拡張子
  - システムディレクトリ
  - サンプリングレート
  - 量子化ビット数
  - 再生時のチャンネル数

- プレーヤーの初期化の確認

- SDカードの初期化

- ファイルが開いていることを確認する

- デコードする最初のフレームを送信

- ボリュームの設定

#### loop関数内

- ファイル終了までループで新しいフレームをデコードに送る

再生終了の処理

- プレーヤーからエラーコードを表示して停止する
  - ファイルの終了を知らせる -> stop_playerへジャンプ
  - エラー時の処理も加える  -> stop_playerへジャンプ

- オーディオファイル用スリープ時間の追加

- return処理

stop_player:
  - Player0をストップ
  - ファイルをクローズする
  - Ready状態に遷移させる
  - 終了させる
  - exit関数を用いてスケッチの終了
