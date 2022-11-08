
void sendMsg()
{
  WiFiClientSecure client;
  client.setInsecure();
  
  delay(1000);
  
  File file = LittleFS.open(FILE_PHOTO, FILE_READ);
  // Insert the data in the photo file
  if (!file)
  {
    Serial.println("Failed to open file in read mode");
  }
  
  do
  {
    if(client.connect("discord.com", 443) == 1)
    {
      break;
    }
    else
    {
      Serial.println("Connection Failed");
      client.stop();
      delay(200);
    }
  }
  while(true);
  
  String bounds = "--AABBCCDDEE";

  String jsonPayload = "{\"content\":\"The message to send\"}";

  String bodyStart = bounds + "\r\n" +
         "Content-Disposition: form-data; name=\"payload_json\"" + "\r\n" + "\r\n" +
         jsonPayload + "\r\n";

         bodyStart = bodyStart + bounds + "\r\n" +
         "Content-Disposition: form-data; name=\"img\"; filename=\"img.jpeg\"" + "\r\n" + "\r\n";

  String bodyEnd = "\r\n" + bounds + "--" + "\r\n";
         
  //data = "{\"content\":\"The message to send\"}";
  
  int dataSize = bodyStart.length() + bodyEnd.length() + file.size();
  client.print(String("POST ") + "/api/webhooks/1029593285041324042/g0o-vMpcQqxbSGdEwcQ96tcaBHK0_o0XD7Bs3ZCF0fahvxQuf0jN2q7P0y7gjqXMdg7r" +" HTTP/1.1\r\n" +
               "Host: " + "discord.com" + "\r\n" +
               //"Content-Type: application/json\r\n" +
               "Content-Type: multipart/form-data; boundary=" + "AABBCCDDEE" + "\r\n"
               "Content-Length: " + dataSize + "\r\n\r\n" +
               bodyStart);

  //client.write(file);

  const int bufSize = 2048;
  uint8_t clientBuf[bufSize];
  int clientCount = 0;

  while (file.available()) {
    int clientCount = file.read(clientBuf, bufSize);
    if (clientCount > 0)
    {
      client.write((const uint8_t *)clientBuf, clientCount);
    }
  }

  client.print(bodyEnd);

               
  //while (true)
  {
    char c = client.read();
    Serial.write(c);
  }
}