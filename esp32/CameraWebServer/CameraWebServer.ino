#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>

#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "Dunder MiffLAN";
const char* password = "***REMOVED***";

AsyncWebServer server(80);

#define FILE_PHOTO "/photo.jpg"

bool takeNewPhoto = false;

void setClock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
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

// Check if photo capture was successful
bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs( void ) {
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly

  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while ( !ok );
}

void sendMsg()
{
  WiFiClientSecure client;
  client.setInsecure();
  
  delay(1000);
  
  File file = SPIFFS.open(FILE_PHOTO, FILE_READ);
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

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 6;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, -2);     // -2 to 2
  s->set_contrast(s,2);       // -2 to 2
  s->set_saturation(s, 1);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 1);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 0);  // 0 = disable , 1 = enable
  s->set_aec2(s, 1);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 400);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  setClock();

  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else
  {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
  
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    takeNewPhoto = true;
    request->send_P(200, "text/plain", "Taking Photo");
  });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  });

  server.on("/send", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", "Sending");
    sendMsg();
  });

  server.on("/arduino", HTTP_GET, [](AsyncWebServerRequest * request) {
    const char* PARAM_INPUT_1 = "output";

    String result = "NONE";

    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1)) {
      String inputMessage1 = request->getParam(PARAM_INPUT_1)->value();

      do
      { 
        Serial2.read();
      } while (Serial2.available() > 0);
      
      Serial2.println(inputMessage1);

      result = "";

      while(result.indexOf("READY") < 0)
      {
        result += Serial2.readString();
      }
    }
    
    request->send(200, "text/plain", result);
  });

  // Start server
  server.begin();

  Serial2.begin(9600,SERIAL_8N1,14,15);
}

void loop() {  
  if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
    takeNewPhoto = false;
  }
  delay(10);
}
