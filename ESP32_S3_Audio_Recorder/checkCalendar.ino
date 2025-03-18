void createCalendar(const char* fileName) // Create calendar.cvs example.
{
  File file = SD_MMC.open(fileName, FILE_WRITE);
  if (file) {
    if (file) {
      file.println("hora;domingo;lunes;martes;miercoles;jueves;viernes;sabado");  // Table header. (spanish)

      for (int hour = 0; hour < 24; hour++)
      {       // Fill data.
        file.print(hour);
        for (int day = 0; day < 7; day++)
        {
          file.print(";1");  // Initial values as example.
        }
        file.println();
      }
      file.close();
      Serial.println("'Calendar.csv' created.");
    }
  } else {
    Serial.println("Error creating 'Calendar.csv'.");
  }
  deInitSDMMC();
}

void nextRecordingSchedule(const char* fileName) // Calculate next recording time.
{
  File file = SD_MMC.open(fileName, FILE_READ);
  if (!file)
  {
    Serial.println("Can't open 'config.txt'.");
    return;
  }

  int calendar[24][7];
  file.readStringUntil('\n'); // Skip first row (header).
  for (int i = 0; i < 24; i++)
  {
    String line = file.readStringUntil('\n');
    sscanf(line.c_str(), "%*d;%d;%d;%d;%d;%d;%d;%d",
           &calendar[i][0], &calendar[i][1], &calendar[i][2], &calendar[i][3], 
           &calendar[i][4], &calendar[i][5], &calendar[i][6]);
    Serial.printf("Hour %d: %d %d %d %d %d %d %d\n", i, calendar[i][0], calendar[i][1], calendar[i][2],
                                       calendar[i][3], calendar[i][4], calendar[i][5], calendar[i][6]);
  }
  deInitSDMMC();

  int startHour = rtc.getHour(true);
  int startDay = rtc.getDayofWeek();
  int currentValue = calendar[startHour][startDay];
  uint64_t nextChangeInMinutes = getNextChangeTime(currentValue, startHour, startDay, calendar);
  Serial.printf("Minutes until next change: %d\n", nextChangeInMinutes);
  if (currentValue == 1) // Recording...
  {
    recordingTime(nextChangeInMinutes);
    currentValue = calendar[startHour][startDay]; // Once the recording is done, calculate sleep time.
    nextChangeInMinutes = getNextChangeTime(currentValue, startHour, startDay, calendar);
  } 
  sleepUntil(nextChangeInMinutes); // Sleep until next recording.
  return;
}

/* getNextChangeTime checks calendar.cvs and the current state. For example, if current state is 1
   means its time for recording. It must record until a 0 is found. Every 1 means +60 mins.  */
int getNextChangeTime(int currentValue, int startHour, int startDay, const int calendar[24][7])
{
  int hour = startHour;
  int day = startDay;
  int minutesPassed = 60 - rtc.getMinute();  // Minutes remaining in the current hour

  // Check if the value changes within the remaining minutes of the current hour
  if (calendar[hour][day] != currentValue)
  {
    return minutesPassed;  // If the change happens in the current hour, return the minutes remaining
  }
  // If no change in the current hour, start checking the next hours
  while (true)
  {
    hour++;
    if (hour == 24)
    {
      hour = 0;
      day++;
      if (day == 7)
      {
      day = 0;
      }
    }
    // Check if a change happens in the current hour
    if (calendar[hour][day] != currentValue)
    {
        return minutesPassed;  // Return minutes until change happens
    }
    minutesPassed += 60;  // Add the full hour
  }
  return minutesPassed;
}

void recordingTime(uint64_t minutes) // Start recording.
{
  Serial.printf("Recording %llu minutes.\n", minutes);
  Serial.flush();
  initPSRAM();   // Init PSRAM before recording.
  initI2SSTD();  // Init I2S in standard mode. 
                 // https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32/api-reference/peripherals/i2s.html

  uint64_t startTime = millis();
  uint64_t duration = minutes * 60 * 1000; 

  audioFileName = rtc.getTime("/%Y%m%d %H-%M-%S")+ ".bin";
  Serial.print("Recording: ");
  Serial.println(audioFileName);  // Use audioFileName to print the name


  while (millis() - startTime < duration)
  {
    I2S_read();
    SD_write();

    if (cycleCount >= MAX_CICLE_COUNT)
    {
      cycleCount = 0;
      Serial.printf("Start. - ");
      Serial.printf("Cycle : %d .  -  ", cycleCount);
      Serial.printf("Full cycle time : %d . \n", millis() - cycleTime);
      cycleTime = millis();
    }
    cycleCount++;
  }
}

void sleepUntil(uint64_t minutes) // Wait until next recording.
{
  Serial.printf("Sleeping %d minutes.", minutes);
  Serial.flush();
  GIASDeInit(); // Close/end all peripherals.
  esp_sleep_enable_timer_wakeup(minutes* 60 * 1000000); // 1 minute
  esp_deep_sleep_start();
  return;
}

void GIASDeInit(void) // De-init peripherals before going to sleep.
{
  Serial.end();
  SD_MMC.end();
  deinitPSRAM();
  deInitI2SSTD();
  digitalWrite(LED_BUILTIN, LOW);
}
