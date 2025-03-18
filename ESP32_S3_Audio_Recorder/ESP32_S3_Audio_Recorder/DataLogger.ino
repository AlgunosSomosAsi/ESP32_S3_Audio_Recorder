void I2S_read(void)  // Read I2S and save data to PSRAM BUFFER.
{
  size_t readsize, written;
  // Audio loopback (debugging).
  i2s_channel_read(rx_handle, rxbuf, sizeof(rxbuf), &readsize, 1000);
  i2s_channel_write(tx_handle, rxbuf, readsize, &written, 100);

  for (size_t i = 0; i < readsize / 2; i += 2)
  { // Save 1 channel.
    psramBuffer[I2SWritePos] = rxbuf[i];
    I2SWritePos++;
    if (I2SWritePos == PSRAM_BUFFER_SIZE / sizeof(uint16_t))
    { // PSRAM buffer index.
      I2SWritePos = 0;
    }
  }
}

void SD_write(void)  // Save PSRAM BUFFER to SD Card.
{
  unsigned long starttime = millis();
  switch (actualState) {
    case sdBegin:
      if (cycleCount == (MAX_CICLE_COUNT - 10))
      { // Start SD card.
        createSdInitTask();
      }
      break;
    case sdRecording:
      createSdWriteTask(); // Record PRAM buffer to SD.
      actualState = sdBegin;
      break;
  }
}

void createSdInitTask(void)
{
  xTaskCreatePinnedToCore(
    sdInitTask,    // Task.
    "sdInitTask",  // Name.
    10000,         // Stack size.
    NULL,          // No parameter.
    1,             // Priority.
    &taskSD,       // Handle.
    0              // Run on core #0.
    );
}

void sdInitTask(void *parameter)  // Init SD MMC with a FreeRTOS task.
{
  digitalWrite(LED_BUILTIN, HIGH);
  if (SD_MMC.begin("/sdcard", true, SDMMC_SPEED)) {
    audioFile = SD_MMC.open(audioFileName, FILE_APPEND);
    actualState = sdRecording;
    sdWritePos = 0;
    Serial.printf("            SD ON. - ");
    Serial.printf("Cycle : %d .\n", cycleCount);
    timeRecording = millis();
  }
  vTaskDelete(NULL);  // Kill task.
}

void createSdWriteTask(void)
{
  xTaskCreatePinnedToCore(
    sdWriteTask,    // Task function
    "sdWriteTask",  // Name
    10000,          // Stack size
    NULL,           // No parameter
    1,              // Priority
    &taskSD,        // Handle
    0               // Run on core #0
  );
}

void sdWriteTask(void *parameter)
{
  unsigned long starttime = millis();

  while (sdWritePos < PSRAM_BUFFER_SIZE)
  {
      int bytesDisponibles = PSRAM_BUFFER_SIZE - sdWritePos;
      int bytesAEscribir = min(bytesDisponibles, BLOCK_SD);
      audioFile.write((uint8_t *)psramBuffer + sdWritePos, bytesAEscribir);
      sdWritePos += bytesAEscribir;
  } 
  audioFile.flush();
  audioFile.close();
  SD_MMC.end();
  actualState = sdBegin;
  Serial.printf("SD OFF. - ");
  Serial.printf("Ciclo : %d . - ", cycleCount);
  Serial.printf("Recording time : %lu ms\n", millis() - timeRecording);
  digitalWrite(LED_BUILTIN, LOW);
  vTaskDelete(NULL);  // Kill task.
}

