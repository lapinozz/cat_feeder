void setClock()
{
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
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
}