struct PushoverMessage
{
  const char* message = nullptr;
  const char* fileName = nullptr;
  uint8_t* fileBuf = nullptr;
  size_t fileSize = 0;
};

void sendMsg(const PushoverMessage& msg)
{
  WiFiClientSecure client;
  client.setInsecure();
  
  int tryCount = 0;

  do
  {
    if(client.connect("api.pushover.net", 443) == 1)
    {
      break;
    }
    else
    {
      Serial.println("Pushover connection failed");
      client.stop();

      if(tryCount++ > 5)
      {
        return;
      }

      delay(100);
    }
  }
  while(true);

  delay(10);

  struct BodyPart
  {
    const uint8_t* data = nullptr;
    const size_t len = 0;

    BodyPart(const uint8_t* d, size_t l) : data(d), len(l)
    {
      
    }

    BodyPart(const char* c) : data(reinterpret_cast<const uint8_t*>(c)), len(strlen(c))
    {
      
    }
  };

  const auto message = msg.message ? msg.message : "";
  const auto fileName = msg.fileName ? msg.fileName : "";

  const BodyPart bodyParts[] =
  {
    "--AAZBBYCCXDDWEEV\r\n",

     "Content-Disposition: form-data; name=\"token\"\r\n\r\n",
     __SECRET_PUSHOVER_APP__,
     "\r\n",
     "--AAZBBYCCXDDWEEV\r\n",

     "Content-Disposition: form-data; name=\"user\"\r\n\r\n",
     __SECRET_PUSHOVER_USER__,
     "\r\n",
     "--AAZBBYCCXDDWEEV\r\n",

     "Content-Disposition: form-data; name=\"message\"\r\n\r\n",
     message,
     "\r\n",
     "--AAZBBYCCXDDWEEV\r\n",

     "Content-Disposition: form-data; name=\"attachment\"; filename=\"", {fileName}, "\""
     "Content-Type: image/jpeg\r\n\r\n",
     
     {msg.fileBuf, msg.fileSize},
     {"\r\n--AAZBBYCCXDDWEEV--\r\n"},
  };
  
  size_t bodySize = 0;
  for(const auto& part : bodyParts)
  {
    bodySize += part.len;
  }
  
  client.print("POST /1/messages.json HTTP/1.1\r\n"
               "Host: api.pushover.net\r\n"
               "Content-Type: multipart/form-data; boundary=AAZBBYCCXDDWEEV\r\n"
               "Content-Length: ");
  client.print(bodySize);
  client.print("\r\n\r\n");
  
  for(const auto& part : bodyParts)
  {
    if(part.len > 0)
    { 
      const int bufSize = 4096;
      const uint8_t* end = part.data + part.len;
      for(const uint8_t* ptr = part.data; ptr < end; ptr += bufSize)
      {
        const size_t len = min(end - ptr, bufSize);
        client.write(ptr, len);
      }
    }
  }

 // /*
  Timer t(2000);

  uint8_t data = -1;
  while(client.read(&data, 1) < 0 && !t.update())
  {

  }

  t.reset();

  while (client.read(&data, 1) > 0 && !t.update())
  {
    Serial.write(data);
  }
  //*/
}