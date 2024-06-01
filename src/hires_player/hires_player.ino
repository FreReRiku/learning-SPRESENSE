#include <SDHCI.h>
#include <Audio.h>

SDClass     theSD;
AudioClass  *theAudio;
File        myFile;

/* Global variable */
bool ErrEnd = false;

/*-------Internal error callback function-------*/
static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING) {
      ErrEnd = true;
  }
}

void setup()
{
  /* Initializing */
  theAudio = AudioClass::getInstance();
  theAudio->begin(audio_attention_cb);

  puts("Initializing...");

  /* Clock Mode */
  theAudio->setRenderingClockMode(AS_CLKMODE_HIRES);

  /* Output (SPeaker and HeadPhone) */
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  /* Player initial setting */
  err_t err = theAudio->initPlayer(AudioClass::Player0,
                                  AS_CODECTYPE_WAV,
                                  "/mnt/sd0/BIN",
                                  AS_SAMPLINGRATE_96000,
                                  AS_BITLENGTH_24,
                                  AS_CHANNEL_STEREO);

  /* Check initialization */
  if (err != AUDIOLIB_ECODE_OK) /* AUDIOLIB_ECODE_OK return 0 */
    {
      printf("Player0 initialize error\n");
      exit(1);
    }

  /* Initialize SD card */
  while (!theSD.begin())
    {
      /* Wait until mounted */
      Serial.println("Insert SD card.");
    }

  /* Open Sound source */
  myFile = theSD.open("HiResSound.wav");

  /* Check the source is opened */
  if (!myFile) // myFile == nullptr
    {
      printf("File open error\n");
      exit(1);
    }
  printf("Open! 0x%08lx\n", (uint32_t)myFile);

  /* Send the first frame to decode */
  err = theAudio->writeFrames(AudioClass::Player0, myFile);

  if (err != AUDIOLIB_ECODE_OK)
    {
      printf("File Read Error! =%d\n",err);
      myFile.close();
      exit(1);
    }

  puts("Play!");

  /* Playback volume（Initial value：-16.0 dB） */
  theAudio->setVolume(-160);
  theAudio->startPlayer(AudioClass::Player0);
}

/* Play sound source */
void loop()
{
  puts("loop!!");

  /* Decode new frames until end of sound file */
  int err = theAudio->writeFrames(AudioClass::Player0, myFile);

  /* Exit with error code */
  if (err)
    {
      /* Notify end of file */
      if (err == AUDIOLIB_ECODE_FILEEND)
        {
          printf("Main player File End!\n");
        }
      else
        {
          printf("Main player error code: %d\n", err);
        }
      goto stop_player;
    }

  if (ErrEnd)
    {
      printf("Error End\n");
      goto stop_player;
    }

  /* Pause to read file (Initial value: 1000) */
  /* If you do parallel processing, you may need to change this value. */
  usleep(1000);


  /* The process continues. */
  return;

stop_player:
  theAudio->stopPlayer(AudioClass::Player0);
  myFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  exit(1);
}
