bool setClock()
{
  const auto gmtOffset = -18000;
  const auto daylightOffset = 3600;
  configTime(gmtOffset, daylightOffset, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);

  int tries = 0;

  while (nowSecs < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);

    if(tries++ >= 10)
    {
      break;
    }
  }

  Serial.println();
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));

  return nowSecs > 8 * 3600 * 2;
}

void listDir(fs::FS& fs, const char* dirname, uint8_t levels = 255)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file)
    {
        if(file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels)
            {
                listDir(fs, file.path(), levels -1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void initLITTLEFS()
{
  if (!LittleFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    ESP.restart();
  }
  else
  {
    delay(500);
    Serial.println("LittleFS mounted successfully");
  }

  listDir(LittleFS, "/");
}

class Timer
{
  const uint64_t delay = 0;
  uint64_t lastUpdate = 0;


public:
  Timer(uint64_t d) : delay(d), lastUpdate(millis())
  {

  }

  bool update()
  {
    const auto now = millis();
    if(now - lastUpdate >= delay)
    {
      lastUpdate = now;
      return true;
    }
    
    return false;
  }

  void reset()
  {
    const auto now = millis();
    lastUpdate = now;
  }
};