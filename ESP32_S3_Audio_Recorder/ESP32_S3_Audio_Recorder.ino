// #includes
  #include "esp_sleep.h"         // Sleep mode.
  #include "ESP32Time.h"         // RTC.
  #include "WiFi.h"              // RTC update.
  #include "SD_MMC.h"            // SD MMC mode.
  #include "driver/i2s_std.h"    // I2S STD driver.
  #include "driver/gpio.h"       // I2S STD driver?
//

// SD MMC #defines.
  #define MMC_D2 4  
  #define MMC_D3 5
  #define MMC_CMD 6
  #define MMC_CLK 7
  #define MMC_D0 15
  #define MMC_D1 16
  #define BLOCK_SD 1024*3        // 3KB is the fastest size to save data in chunks to the SD in MMC mode.
  #define SDMMC_SPEED SDMMC_FREQ_HIGHSPEED
  /* Possible MMC clock speed:
  SDMMC_FREQ_PROBING      400         SD/MMC probing speed
  SDMMC_FREQ_DEFAULT      20000       SD/MMC Default speed (limited by clock divider)
  SDMMC_FREQ_26M          26000       MMC 26MHz speed
  SDMMC_FREQ_HIGHSPEED    40000       SD High speed (limited by clock divider)
  SDMMC_FREQ_52M          52000       MMC 52MHz speed
  */
//

// I2S #defines.
  #define I2S_CHANNEL I2S_NUM_0                                 // I2S peripheral number (0 and 1 available).
  #define SAMPLERATE 44100                                      // PMOD I2S2 sample rate.
  #define PM_MCK 14                                             // PMOD Master ClocK.
  #define PM_WS 13                                              // PMOD Word Select
  #define PM_BCK 12                                             // PMOD Bit ClocK
  #define PM_SDO 11                                             // PMOD Serial Data OUT
  #define PM_SDIN 10                                            // PMOD Serial Data IN
  #define MAX_CICLE_COUNT 1000                                  // PSRAM (MAX 8MB / I2S_BUFFERSIZE)
  #define BUF_COUNT 16                                          // I2S DMA Buffer count.
  #define BUF_LEN 512                                           // I2S DMA Buffer length.
  #define I2S_BUFFERSIZE ((BUF_COUNT - 1) * BUF_LEN)            // Min. 50 ms 16bits@44.1khz required by SD_MMC.begin()
  #define PSRAM_BUFFER_SIZE (MAX_CICLE_COUNT * I2S_BUFFERSIZE)  // 7.5 MB max for headroom.
  //  PSRAM_BUFFER_SIZE is MAX_CICLE_COUNT * (BUF_COUNT-1) * BUF_LEN = ~7.32 MB.
  #define SD_WRITE_TIMER 75                                     // Max sd write time (limited by I2S_BUFFERSIZE).
//

// Global variables.
  i2s_chan_handle_t tx_handle, rx_handle;           // I2S TX/RX handlers. Required by I2S STD driver.
  unsigned long sdWritePos = 0, I2SWritePos = 0;    // SD index - I2S buffer index.
  uint16_t rxbuf[I2S_BUFFERSIZE];                   // I2S Rx buffer.
  uint16_t *psramBuffer;                            // PSRAM buffer pointer.
  String audioFileName = "/testfile.bin";           // Default audio file name.
  unsigned long cycleCount = 0;                     // AUX variable. Controls SD recording.
  enum state { sdBegin,                             // AUX variable. Controls SD recording.
                sdRecording };
  state actualState = sdBegin;                      // AUX variable init value.
  unsigned long timeRecording = 0;                  // DEBUG aux variable / Serial print.
  unsigned long cycleTime = 0;                      // DEBUG aux variable / Serial print.
  TaskHandle_t taskSD;                              // Init SD task.

  File audioFile;
  ESP32Time rtc;

//

void setup()
{
  initPinOut();     // LED and SD MMC pin config.
  initSerial();     // Init Serial port. Debugging purposes.

  GIAS();
}

void loop()
{
}