# 🚀 Audio Recorder with ESP32 S3 N16R8 and SD Card in 4-bit SDIO mode.

# *📜 Description*
This project allows recording audio at **16 bits @ 44.1 kHz**, storing the data as raw values in a `.bin` file (not as a standard audio file).
It uses an 4-bit SD card in SDIO mode, ESP32 S3 B16R8, and an audio codec (PMOD I2S2).

---

# *🛠️ Hardware Requirements*

## *ESP32 S3 N16R8*
- Microcontroller responsible for recording audio data and interfacing with the audio codec and SD card.  
- Equipped with sufficient serial peripherals to manage I2S in full duplex mode, Master Clock, and the SDIO.
- 8 MB of RAM allows power efficiency improvements (not implemented yet).

<div style="text-align: center;">
  <img src="images/ESP32_S3.jpg" alt="ESP32 S3" width="400"/>
  <p><em>ESP32 S3</em></p>
</div>

---

## *[PMOD I2S2 Audio Codec](https://digilent.com/reference/pmod/pmodi2s2/reference-manual)* 
- Converts audio signals to I2S format.  
- Requires Master Clock.  
- This module includes both DAC and ADC functionalities, simplifying audio testing.


<div style="text-align: center;">
  <img src="images/PMOD_I2S2.JPG" alt="PMOD I2S2" width="400"/>
  <p><em>PMOD I2S2</em></p>
</div>

<div style="text-align: center;">
  <img src="images/PMOD_PINOUT.jpg" alt="PMOD I2S2 PINOUT" width="400"/>
  <p><em>PMOD I2S2 Pinout</em></p>
</div>


---

## *SD card (SDIO compatible)*
- SD cards are able to work in SDIO 4-bit mode, increasing data transfer compared to SPI mode.
- SD cards can be connected directly to 3V3. SD card adapter can be used in order to utilize micro SD cards in this mode.

<div style="text-align: center;">
  <img src="images/SD_Module.jpg" alt="SD Module" width="400"/>
  <p><em>SD Card pinout</em></p>
</div>

- SD Card speeds are often indicated by logos printed on them.
- Audio file size can be calculated with [Audio File Size Calculator](https://www.omnicalculator.com/other/audio-file-size).

<div style="text-align: center;">
  <a href="https://www.sdcard.org/consumers/about-sd-memory-card-choices/speed-class-standards-for-video-recording/">
    <img src="images/SD_Speed_Class.jpg" alt="SD Speed Class" width="400"/>
  </a>
  <p><em>SD Speed Class</em></p>
</div>

# *Sketch*
- The sketch is developed using Arduino IDE, allowing comparison of different MCUs without the need to learn new IDEs for each platform. Currently, only ESP MCUs have been tested because I2S with MCLK is not widely supported on other MCU platforms.
- The I2S driver provided by Espressif Systems is unique and not compatible with other platforms.

# *Arduino IDE setup.*
-  PSRAM enabled: OPI PSRAM.
-  Avoid using GPIO 35, 36 and 37 if OPI PSRAM is enabled.
- CPU frequency can be as low as 80 MHz. 
- At 40 MHz wifi won't connect. I2S will halt the system.
#

# *Future releases:*
  - External RTC must be implemented. Time shift while in sleep mode could be as high as 4 mins per day.
  - Check SD capacity left before start recording.
  - Re-define macros for debugging functions such as digitalWrite and and Serial print, also add a datalog.txt.
  - Battery level checker (requires hardware modifications).

---

# *SD Card SD/MMC 4 BIT BUS:*
```c
  #define BLOCK_SD 1024*3
```
  3KB buffer is the fastest to save data to SD card in MMC mode. Tested.
  GPIO 34 is not broken-out on ESP32-S3-DevKitC-1 v1.1 Avoid GPIO 35, 36 and 37 if OPI PSRAM is enabled.
```c
  #define MMC_D2 4  
  #define MMC_D3 5
  #define MMC_CMD 6
  #define MMC_CLK 7
  #define MMC_D0 15
  #define MMC_D1 16
```
## Pin Configuration for SD/MMC 4 BIT BUS

### Pin name and SD pin number mapping:

| Pin Name         | D1 | D0 | GND | CLK | 3V3 | GND | CMD | D3 | D2 |
|------------------|----|----|-----|-----|-----|-----|-----|----|----|
| SD Pin Number    |  8 |  7 |  6  |  5  |  4  |  3  |  2  |  1 |  9 |
| Custom ESP32 S3  | 16 | 15 | GND |  7  | 3V3 | GND |  6  |  5 |  4 |

### Pin Assignment Warning
 
> **WARNING**: All data pins (D0, D1, D2, D3, CMA andn CLK) must be pulled up to 3.3V with an external 10k Ohm resistor! [SD Pull-up Requirement](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/sd_pullup_requirements.html)

  -   SD MMC library: https://github.com/espressif/arduino-esp32/tree/master/libraries/SD_MMC
#

# PMOD I2S2 connection

| Pin   | D/A       | GPIO | Pin   | A/D       | GPIO |
|-------|-----------|------|-------|-----------|------|
| 01    | D/A MCLK  | GPIO 14 | 07    | A/D MCLK  | GPIO 14 |
| 02    | D/A WS    | GPIO 13 | 08    | A/D WS    | GPIO 13 |
| 03    | D/A SCLK  | GPIO 12 | 09    | A/D SCLK  | GPIO 12 |
| 04    | D/A SDOUT | GPIO 11 | 10    | A/D SDIN  | GPIO 10 |
| 05    | D/A GND   | GND    | 11    | A/D GND   | GND |
| 06    | D/A VCC   | 3V3    | 12    | A/D VCC   | 3V3 |

# File system.
 SD Card should be formatted in Fat32. guiformat.exe provided by Ridgecrop Consultants Ltd can format perform this task 
 on SD Cards with 32GB and 64BG. 128 GB not tested.

 http://ridgecrop.co.uk/index.htm?guiformat.htm
 
 Old SD Cards might not work with SDIO MMC mode.
#

# I2S, DMA and PSRAM Buffer size explanation.
```c
  #define MAX_CICLE_COUNT 1000                                  // PSRAM (MAX 8MB / I2S_BUFFERSIZE)  
  #define BUF_COUNT 16                                          // I2S DMA Buffer count.  
  #define BUF_LEN 512                                           // I2S DMA Buffer length.  
  #define I2S_BUFFERSIZE ((BUF_COUNT - 1) * BUF_LEN)            // min. 50 ms 16bits@44.1khz required by SD_MMC.begin()  
  #define PSRAM_BUFFER_SIZE (MAX_CICLE_COUNT * I2S_BUFFERSIZE)  // 7.5 MB max for headroom.  
```
  PSRAM is 8 MB. 8*1024*1024 8-bit bytes.
  Working at 16 bits, it means:
  8*1024*1024 / 44100 (sample rate) / 2 (int in bytes) = ~95 secs of audio at 44.1KHz@16 bits. (1 channel).
  Since C++/ESP IDF/FreeRTOS uses some bytes in the PSRAM, ~7.5 MB  (~87 secs) is the PSRAM Buffer size used.
  
  The DMA peripheral uses BUF_COUNT buffers of BUF_LEN bytes to store I2S data without loading the CPU.
  i2s_channel_read reads all those buffers and saves the amount of bytes passed as a parameter:
```c
  i2s_channel_read(rx_handle, rxbuf, sizeof(rxbuf), &readsize, 1000);
```
  If sizeof(rxbuf) is larger than BUF_COUNT * BUF_LEN, data is lost.
```c
  #define I2S_BUFFERSIZE ((BUF_COUNT - 1) * BUF_LEN) 
```
  provides a small headroom. If the function 
  i2s_channel_read (a blocking function) cannot be called in time, I2S data will overwrite samples.
  Some documentation does not recommend using BUF_LEN > 1024 or 2048.
  BUF_LEN 8 * 1024 could be tested in future board versions.

  Since this hardware is battery-powered, the SD card needs to be shut down most of the time until the PSRAM buffer 
  is almost full. This is why I2S_BUFFERSIZE needs to be large enough to avoid losing samples while the SD
  card initializes (which includes power-on and calling SD_MMC.begin(), which requires ~50 ms).
  I2S_BUFFERSIZE is 16 * 512 / 44100 / 2 = ~92 ms.

  Once the SD card is initialized, data is saved in chunks of 3 KB, which provides the fastest data rate on
  the SD card used. Data is saved every ~80 ms (SD_WRITE_TIMER), which is the time DMA requires to fill the I2S buffers.

  To fill the PSRAM buffer, 1000 cycles are required (512 * 15 * 1000 = 7.32 MB).
  To save all the data to the SD card requires ~54 cycles.
  This is why at cycle no. '1000-4', the SD card is initialized. This value may change in the future.
  The idea is to start saving data before I2S overwrites the first samples in the PSRAM buffer.
  This method keeps the SD card ON only 54 / 1000 = 5% of the time.
  It could be said that most of the system's power consumption comes from the MCU and the PSRAM.
  Selecting a low-power PSRAM might improve power efficiency.
  Selecting a lower-power MCU requires an MCU with all the necessary peripherals (SDIO, I2S, OPI or SPI for the
  PSRAM) and could require a different IDE.
  The values mentioned were either calculated or measured using a specific sketch for this particular hardware.
  Improvements in SD management would not have a significant enough impact to justify further system evaluation.
#

# Future releases:
  - External RTC must be implemented. Time shift while in sleep mode could be as high as 4 mins per day.
  - Check SD capacity left before start recording.
  - Re-define macros for debugging functions such as digitalWrite and and Serial print, also add a datalog.txt.
  - Battery level checker (requires hardware modifications).
#

