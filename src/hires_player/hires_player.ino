#include <SDHCI.h>
#include <Audio.h>

SDClass theSD;
AudioClass *theAudio;

File myFile;

bool ErrEnd = false;

/** Audio attention callback **
 * オーディオ内部エラーが発生した場合、この関数が呼び出されます。
 */

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING) {
      ErrEnd = true;
  }
}

/** wavファイルを再生するためのプレイヤー側のセットアップ **
 * ・クロックモード：Hi-res Audio
 * ・出力デバイス：スピーカー
 * ・デコード設定：ステレオwavファイル
 * ・サンプリングレート：96000Hz
 * ・量子化ビット数：24bit
 * ・システムディレクトリの場所：/mnt/sd0/BIN
 * ・開くファイル名："HiResSound.wav"
 * ・マスターボリューム：-16.0 dB（初期値）
 */
void setup()
{
  /* オーディオシステムの開始 */
  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  puts("initialization Audio Library");

  /* クロックモードの設定 */
  theAudio->setRenderingClockMode(AS_CLKMODE_HIRES);

  /* 出力をスピーカーに設定 // SPeaker and HeadPhone.
   * （出力デバイスをI2Sに変更したい場合は、引数に"AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT"を指定する。）
   */
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  /*
   * SPRESENSEでステレオwavファイルをデコードするように設定する。
   * サンプリングレートは96000Hzに設定する。
   * "mnt/sd0/BIN"ディレクトリでwavデコーダーを検索する。
   */
  err_t err = theAudio->initPlayer(AudioClass::Player0,
                                  AS_CODECTYPE_WAV,
                                  "/mnt/sd0/BIN",
                                  AS_SAMPLINGRATE_96000,
                                  AS_BITLENGTH_24,
                                  AS_CHANNEL_STEREO);

  /* プレーヤーの初期化を確認する。 */
  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  /* SDカードの初期化 */
  while (!theSD.begin())
    {
      /* SDカードがマウントされるまで待つ。 */
      Serial.println("Insert SD card.");
    }

  /* SDカード上のファイルを開く。 */
  myFile = theSD.open("HiResSound.wav");

  /* ファイルが開いていることを確認する。 */
  if (!myFile) // myFile == nullptr
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! 0x%08lx\n", (uint32_t)myFile);

  /* デコードする最初のフレームを送信する。 */
  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
    }

  puts("Play!");

  /* ボリューム設定（初期値：-16.0 dB） */
  theAudio->setVolume(-160);
  theAudio->startPlayer(AudioClass::Player0);
}

/** ファイルの再生 **/
void loop()
{
  puts("loop!!");

  /* ファイル終了までループで新しいフレームをデコードに送る */
  int err = theAudio->writeFrames(AudioClass::Player0, myFile);

  /* プレーヤーからエラーコードを表示して停止する */
  if (err)
    {
      /* ファイルの終了を知らせる */
      if (err == AUDIOLIB_ECODE_FILEEND)
        {
          printf("Main player File End!\n");
        }
      else
        {
          printf("Main player error code: %d\n", err);
        }
      goto stop_player; // 下のstop_playerを貼り付けても良い
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      goto stop_player; // 下のstop_playerを貼り付けても良い
    }

  /* 
   * このスリープは、オーディオストリームファイルの読み込み時間によって調整されます。
   * アプリケーションで同時に処理されている処理内容に応じて調整してください。usleep()関数は、呼び出したスレッドの実行をusecマイクロ秒だけ一時停止します。
   * ただし、タイマーの分解能はOSのシステムティックタイムに依存し、デフォルトでは10ミリ秒（10,000マイクロ秒）です。
   * したがって、ここで要求された時間よりも長い時間スリープすることになる。
   */

  usleep(1000);


  /* これ以上進めずに再生を続ける */
  return;

stop_player:
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  exit(1);
}
