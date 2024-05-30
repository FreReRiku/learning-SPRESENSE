/*
* INTRO: About this sketch.
* このスケッチではマイクを使って録音をすることができる。
* このスケッチをSPRESENSEに書き込むと、書き込み完了直後、もしくはリセット直後から10秒間の録音が行われる。
* 録音データは「Record.mp3」という名前で保存される。
*/

// スケッチの宣言部
/* 
* Spresenseのオーディオ機能はAudio.hに定義されている。SDカード用のライブラリ(SDHCIライブラリ)はSDHCI.hというヘッダーファイルで定義されている。
* Audio機能はシングルトンとして実装されており、getInstance()関数でその実体を取得することができる。
* グローバル変数については2種類宣言しており、以下に説明する。
* 1. const int32_t recording_time_ms は定数であり、録音時間の10秒(10000ミリ秒)が定義されている。
* 2. int32_t start_time_ms は変数であり、録音開始時間を記憶するための変数である。
*/

// setup関数の処理
/*
* setup関数で行う処理
* ・コンソール出力用のSerialライブラリの初期化
* ・SDカードの初期化
* ・Audioライブラリの初期化
*------------------------------------------
* Serialはコンソールにメッセージを表示するためのライブラリで、このスケッチでは通信速度115200bpsで初期化しています。
* SDHCIライブラリは、SDカードが挿入されていない場合はbegin関数がfalseを返すので、SDカードが挿入されるまで待ちます。
*/


#include <Audio.h>
#include <SDHCI.h>

SDClass SD;
File myFile;

// AudioClass: Audioライブラリの中核となるクラス
// *theAudio: AudioClass型のポインタ変数
// AudioClass::getInstance(): AudioClassのインスタンスを生成し、そのポインタ変数に格納する役割を果たします。
AudioClass *theAudio = AudioClass::getInstance();

// 録音時間10秒
const int32_t recording_time_ms = 10000;
// 録音開始時間
int32_t start_time_ms; 

void setup()
{
  Serial.begin(115200);
  while(!SD.begin()){
    Serial.println("Insert SD card.");
  }
  theAudio->begin();

  // ハイレゾ用のクロックに変更
  theAudio->setRenderingClockMode(AS_CLKMODE_HIRES);
  // 入力をマイクに設定
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 3, (500 * 1024));
  // 録音設定：フォーマット, DSPファイルの場所, サンプリングレート, チャンネル数の指定
  theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/sd0/BIN", AS_SAMPLINGRATE_192000, AS_CHANNEL_MONO);

  // Record001.mp3 があったら削除
  if(SD.exists("Record001.wav")){
    SD.remove("Record001.wav");
  }

  // Record001.mp3 を書き込みモードでオープン
  myFile = SD.open("Record001.wav", FILE_WRITE);
  if(!myFile){
    Serial.println("File open error\n");
    while(1);
  }

  // 録音を開始
  theAudio->startRecorder();

  // 録音開始時間を記録
  start_time_ms = millis();
  Serial.println("Start Recording");
}


// loop関数の処理
/*
* loop関数は、Arduinoでは繰り返し呼ばれる関数です。このスケッチではひたすらreadFrame関数を呼び続けていますが、何をしている関数でしょうか。
* Spresenseはマイクからのアナログ信号をデジタル化し、FIFOに蓄積します。FIFOに蓄積したデータは、適宜読み出さないとオーバーフローを起こして録音動作が止まってしまいます。
* そのため、FIFO内のデータを連続で読み出し、ファイルに記録する必要があります。その処理をreadFrames関数が行っています。
*-----------------------------------------------------------------------------------------------------------------------------------------------
* duration_ms は録音の経過時間を記録しています。
* duration_msの時間がrecording_time_ms(10000ミリ秒)を経過したら、stopRecorderで録音を停止します。
* 録音データを格納したファイルはcloseOutputFileでクローズします。
* (※忘れずにこの関数を呼び出さないと、FIFOバッファーに残された一部データが正しく書き込まれない可能性があります。)
* その後、setReadyMode関数でFIFOのクローズとメモリーの解放を行い、end関数でAudioライブラリを終了します。
*/


void loop()
{
  // 録音時間計測
  uint32_t duration_ms = millis() - start_time_ms;

  // FIFOのデータを読み出し、MP3で記録
  err_t err = theAudio->readFrames(myFile);

  // if文の条件では、録音時間が設定値(この場合10秒)を超えたか確認
  if(duration_ms > recording_time_ms || err != AUDIOLIB_ECODE_OK){

    //録音時間を経過したのでレコーダーをストップ
    theAudio->stopRecorder();
    theAudio->closeOutputFile(myFile);
    theAudio->setReadyMode();
    theAudio->end();
    Serial.println("End Recording");
    while(1);
  }
}

