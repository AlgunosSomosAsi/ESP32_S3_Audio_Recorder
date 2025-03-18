void initPinOut(void)  // LED and SD MMC pin config.
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Visual feedback init.
  while (!SD_MMC.setPins(MMC_CLK, MMC_CMD, MMC_D0, MMC_D1, MMC_D2, MMC_D3))
    ; // SDIO pin config init.
}

void initSerial(void)   // Init Serial port. Debugging purposes.
{
  Serial.begin(115200);
  while (!Serial) ;
  Serial.println("Serial OK");
}

void initPSRAM(void)   // Init PSRAM.
{
  if (!psramInit()) {
    while (1)
      ;
  }
  psramBuffer = (uint16_t*)ps_malloc(PSRAM_BUFFER_SIZE);
  if (!psramBuffer) {
    while (1)
      ;
  }
}

void deinitPSRAM(void)   // De-init PSRAM.
{
  if (psramBuffer) {
    free(psramBuffer);  // Free PSRAM buffer.
    psramBuffer = NULL; // Null PSRAM pointer.
  }
}

void initI2SSTD(void) // Init I2S standard mode (i2s_std.h driver).
{
  i2s_chan_config_t chan_cfg = {
    .id = I2S_CHANNEL,               // I2S channel (0 and 1 available).
    .role = I2S_ROLE_MASTER,         // Master I2S.
    .dma_desc_num = BUF_COUNT,       // DMA buffers.
    .dma_frame_num = BUF_LEN,        // DMA buffer length.
    .auto_clear_after_cb = false,    // Do not clear the I2S buffer after reading.
    .auto_clear_before_cb = false,   // Do not clear the I2S buffer after writing.
    .intr_priority = 7,              // Max interrupt priority (0 to 7).
  };
  i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);
  i2s_std_clk_config_t clk_cfg = {
    .sample_rate_hz = SAMPLERATE,           // Sample rate is 44.1 kHz for this proyect.
    .clk_src = I2S_CLK_SRC_DEFAULT,
    .mclk_multiple = I2S_MCLK_MULTIPLE_384, // Use 384. Other values might induce noise.
  };
  i2s_std_config_t std_cfg = {
    .clk_cfg = clk_cfg,
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
    .gpio_cfg = {
      .mclk = (gpio_num_t)PM_MCK,
      .bclk = (gpio_num_t)PM_BCK,
      .ws = (gpio_num_t)PM_WS,
      .dout = (gpio_num_t)PM_SDO,
      .din = (gpio_num_t)PM_SDIN,
      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv = false,
      },
    },
  };
  i2s_channel_init_std_mode(tx_handle, &std_cfg); // Init TX channnel.
  i2s_channel_init_std_mode(rx_handle, &std_cfg); // Init RX channnel.
  i2s_channel_enable(tx_handle);                  // Enable TX channnel.
  i2s_channel_enable(rx_handle);                  // Enable RX channnel.
}

void deInitI2SSTD(void) // De-Init I2S standard mode (i2s_std.h driver).
{
  i2s_channel_disable(tx_handle);  // Disable TX channel.
  i2s_channel_disable(rx_handle);  // Disable RX channel.
  i2s_del_channel(tx_handle);      // Delete TX channel.
  i2s_del_channel(rx_handle);      // Delete RX channel.
  delay(1);
}

void ErrorHandler(void) // Visual feedback. Device not ready for recording.
{
  while(1){
    delay(250); 
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void initSDMMC(void)    // Init SD in MMC mode. Required to read wifi.txt and calendar.cvs.
{
  if (!SD_MMC.begin("/sdcard", true, SDMMC_SPEED))
  {
    if (!SD_MMC.begin("/sdcard", true, SDMMC_SPEED)) // Retry.
    {
      Serial.println("Mount SD error.");
      ErrorHandler();
    }
  }
  Serial.println("SD MMC OK.");
}

void deInitSDMMC(void) // Close SD MMC.
{
  SD_MMC.end();
  // Implement SD power off controlled by hardware.
}
