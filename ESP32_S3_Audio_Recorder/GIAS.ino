void GIAS (void)
{
  UpdateRTC();        // Update RTC if wifi.txt and wifi connection is available.
  checkCalendar();    // Check for Calendar.cvs and start recording/sleep.
}

void UpdateRTC(void)  // Update RTC if wifi is available.
{
  const char* fileName = "/config.txt";

  initSDMMC();     // Init SD in MMC mode. Required to read config.txt and calendar.cvs.
  if (!SD_MMC.exists(fileName))
  {
    createConfig(fileName); // File not found. Create config.txt and HALT program.
    ErrorHandler();
  } else
  {
    readConfig(fileName);   // Read config.txt, get the GMT and connect to wifi.
  }
  deInitSDMMC();            // Close SD.
  return;
}

void checkCalendar(void)    // Check recording schudle.
{
  const char* fileName = "/Calendar.csv";
  initSDMMC();              // init SD in MMC mode. Required to read wifi.txt and calendar.cvs.
  if (!SD_MMC.exists(fileName))
  {
    createCalendar(fileName);
    ErrorHandler();
  } else
  {
    Serial.printf("'%s' found.\n", fileName);
    nextRecordingSchedule(fileName);
  }
  return;
}