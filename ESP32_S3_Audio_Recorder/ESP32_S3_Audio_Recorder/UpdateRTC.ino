void createConfig(const char* fileName) // Creates config.txt example.
{
  File file = SD_MMC.open(fileName, FILE_WRITE);
  if (file) {
    file.println("ssid");
    file.println("YOUR_SSID");
    file.println("password");
    file.println("YOUR_SSID_PASSWORD");
    file.println("GMT");
    file.println("-3");
    file.close();
    Serial.println("'config.txt' created.");
  } else {
    Serial.println("Error creating 'config.txt'.");
  }
}

void readConfig(const char* fileName) // Read config.txt, connect to wifi, return GMT.
{
  if (SD_MMC.exists(fileName))
  {
    Serial.println("'config.txt' found.");
    wifiConnect(fileName);
  }
  return ;
}

void wifiConnect (const char* fileName) // Connect to SSID.
{
  String ssid = "";
  String password = "";
  int GMT = 0;
  File file = SD_MMC.open(fileName, FILE_READ);
  while(file.available())
  {
    String key = file.readStringUntil('\n'); 
    key.trim();

    if (key == "ssid")
    {                        // Reading ssid.
      ssid = file.readStringUntil('\n');
      ssid.trim();
      Serial.printf("SSID: %s\n", ssid.c_str());
    } else if (key == "password")
    {             // Reading ssid password.
      password = file.readStringUntil('\n');
      password.trim();
      Serial.printf("Password: %s\n", password.c_str());
    } else if (key == "GMT")
    {                  // Reading GMT offset in hours.
      String value = file.readStringUntil('\n');
      GMT = value.toInt()*3600;                 // Convert to seconds.
      Serial.printf("GMT: %d\n", GMT);
    }
  }
  file.close();

  WiFi.begin(ssid, password);
  Serial.print("Attempting to connect to WiFi... \n");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    Serial.print(".");
    delay(500);
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connection OK."); // If connection successful update RTC.
    syncTime(GMT);  
    WiFi.disconnect(true);
  } else                              // Continue without RTC update.
  {
    Serial.println("Unable to connect to WiFi. RTC not updated.");
  }

  WiFi.mode(WIFI_OFF);                // Turn off wifi.
  return;
}

void syncTime(int GMT) // Sync RTC if wifi is available.
{
  configTime(0, 0, "pool.ntp.org");
  Serial.printf("GMT = %d \n", GMT );
  struct tm timeinfo = rtc.getTimeStruct();
  while(true)
  {
    if(getLocalTime(&timeinfo))
    {
      rtc.setTimeStruct(timeinfo);
      rtc.offset = GMT; // Set GMT offset.
      Serial.println("RTC syncronized: ");
      Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));   // (String) returns time with specified format 
      return;
    }
  }
 
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  return;
}