#include <Arduino.h>

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
#include "ESPAsyncWebServer.h"
#include <StringArray.h>
#include <FixedString.h>
#include <CircularBuffer.h>
#include <LITTLEFS.h>
#include <FS.h>

#include "shared/shared.h"

#include "consts.h"
#include "camera.h"
#include "secret.h"
#include "utils.h"
#include "discord.h"
#include "ws.h"
#include "tasks.h"

AsyncWebServer server(80);

String executeArduino(Task& task)
{
    do {
      Serial2.read();
    } while (Serial2.available() > 0);

    Serial2.print(task.cmd);
    Serial2.print("'");
    Serial2.print(task.args[0]);

    String result = "";

    while (result.indexOf("READY") < 0) {
      result += Serial2.readString();
    }

    ws.textAll("ard " + result);

    return result;
}

void execCapture(Task& task)
{
  capturePhotoSaveSpiffs(FILE_PHOTO);
}

void execArd(Task& task)
{
  executeArduino(task);
}

TaskExec taskExecs[Commands::_ESP_COUNT];
void initTasks()
{
  taskExecs[Commands::ESP_Capture] = &execCapture;
}

void notifyClients() {
  ws.textAll("HELOOOO");
}

void onWsMessage(uint32_t id, const char* msg) {
  FixedString<256> str(msg);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  tasksSemaphore = xSemaphoreCreateMutex();
  initTasks();

  setupCamera();

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  setClock();
  initLITTLEFS();

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("capture");
    pushTask({Commands::ESP_Capture});
    request->send_P(200, "text/plain", "Taking Photo");
  });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, FILE_PHOTO, "image/jpg", false);
  });

  server.on("/send", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", "Sending");
    sendMsg();
  });

  server.on("/arduino", HTTP_GET, [](AsyncWebServerRequest* request) {
    const char* PARAM_INPUT_1 = "output";

    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1)) {

      //pendingArduinoCmd.push({ request->getParam(PARAM_INPUT_1)->value() });
    }

    request->send(200, "text/plain", "Received");
  });

  setupWs(server, onWsMessage);

  server.begin();

  Serial2.begin(9600, SERIAL_8N1, 14, 15);
}

auto lastStream = millis();
void streamCamera()
{
  const auto now = millis();
  if(now - lastStream < 1000 && now > lastStream)
  {
    return;
  }

  lastStream = now;

  camera_fb_t * fb = NULL;

  fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }

  //AsyncWebSocketMessageBufferInline buff(fb->buf, fb->len);
  //ws.binaryAll((AsyncWebSocketMessageBuffer*)&buff);
  ws.binaryAll(fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

void loop() {
  Task task;
  while (popTask(task))
  {
    if(task.cmd >= Commands::_ESP_BEGIN && task.cmd <= Commands::_ESP_END)
    {
      taskExecs[task.cmd](task);
    }
    else if(task.cmd >= Commands::_ARD_BEGIN && task.cmd <= Commands::_ARD_END)
    {
      execArd(task);
    }
  }

  ws.cleanupClients();

  if(ws.count() > 0)
  {
    streamCamera();
  }

  delay(100);
}