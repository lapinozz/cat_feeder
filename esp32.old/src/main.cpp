
//#define ASYNC_TCP_SSL_ENABLED true

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "ESPAsyncWebServer.h"
#include <StringArray.h>
#include <FixedString.h>
#include <CircularBuffer.h>
#include <LITTLEFS.h>
#include <FS.h>

#include "./shared/shared.h"

#include "consts.h"
#include "camera.h"
#include "secret.h"
#include "utils.h"
#include "discord.h"
#include "ws.h"
#include "tasks.h"

AsyncWebServer server(4560);
uint8_t* lastCatpure = nullptr;
size_t lastCatpureLen = -1;

using DispensingEvent = int[DispensingSettings::_COUNT];

DispensingEvent dispensingEvents[DispensingEventMax];
size_t dispensingEventsSize = sizeof(dispensingEvents);
const char* dispensingEventsPath = "/dispensingEvents.conf";

void resetDispensingSettings()
{
	for(size_t x = 0; x < DispensingEventMax; x++)
	{
		auto& dispensingEvent = dispensingEvents[x];
		for(size_t y = 0; y < DispensingSettings::_COUNT; y++)
		{
			dispensingEvent[y] = {};
		}
	}
}

void loadDispensingSettings()
{
	File file = LittleFS.open(dispensingEventsPath, FILE_READ);
	if(file && file.size() == dispensingEventsSize)
	{
		file.read((uint8_t*)&dispensingEvents, dispensingEventsSize);
	}
	else
	{
		resetDispensingSettings();
	}
	file.close();
}

void saveDispensingSettings()
{
	File file = LittleFS.open(dispensingEventsPath, FILE_WRITE);
	if(file)
	{
		file.write((const uint8_t*)&dispensingEvents, dispensingEventsSize);
	}
	file.close();
}

void sendDispensingSettings(uint32_t clientId)
{
	FixedString<256> str;
	str += Commands::ESP_SetDispensingSetting;
	for(size_t x = 0; x < DispensingEventMax; x++)
	{
		auto& dispensingEvent = dispensingEvents[x];
		for(size_t y = 0; y < DispensingSettings::_COUNT; y++)
		{
			str += ',';
			str += dispensingEvent[y];
		}
	}

	ws.text(clientId, str.c_str(), str.length());
}

TaskResult executeArduino(const Task& task)
{
	do
	{
		Serial2.read();
	} while (Serial2.available() > 0);

	Serial2.print(+task.cmd);

	for(size_t x = 0; x < task.argCount; x++)
	{
		Serial2.print(",");
		Serial2.print(+task.args[x]);
	}

	Serial2.print('\n');

	while (Serial2.available() <= 0)
	{
	}
	String str = Serial2.readStringUntil('\n');

	TaskResult result;
	result.argCount = splitStr(str.c_str(), result.args, Task::MaxArgCount);

	ws.textAll("ard " + str);

	return result;
}

void execCapture(Task& task)
{
	if(lastCatpure)
	{
		delete[] lastCatpure;
		lastCatpureLen = -1;
	}

	auto* fb = esp_camera_fb_get();
	if (fb)
	{
		if(lastCatpure = new uint8_t[fb->len])
		{
			lastCatpureLen = fb->len;
			memcpy(lastCatpure, fb->buf, lastCatpureLen);
		}
		else
		{
			Serial.println("Camera buffer allocation failed");
		}
	}
	else
	{
		Serial.println("Camera capture failed");
	}

	esp_camera_fb_return(fb);
}

void execSetDispensingSetting(Task& task)
{
	if(task.argCount != 3)
	{
		return;
	}

	const auto eventIndex = task.args[0];
	const auto settingIndex = task.args[1];
	const auto value = task.args[2];

	if(eventIndex < 0 || eventIndex >= DispensingEventMax)
	{
		return;
	}

	if(settingIndex < 0 || settingIndex >= DispensingSettings::_COUNT)
	{
		return;
	}

	dispensingEvents[eventIndex][settingIndex] = value;
}

void execSaveDispensingSettings(Task& task)
{
	saveDispensingSettings();
}

void execArd(Task& task)
{
	executeArduino(task);
}

void onWsMessage(uint32_t id, const char* msg)
{
	int buf[Task::MaxArgCount + 1];
	size_t count = splitStr(msg, buf, Task::MaxArgCount + 1);

	Task task;

	if(count > 0)
	{
		task.cmd = buf[0];

		for(int x = 1; x < min(count, Task::MaxArgCount + 1); x++)
		{
			task.args[x - 1] = buf[x];
		}

		task.argCount = count - 1;

		pushTask(task);
	}
}

void onWsConnect(uint32_t id) {
	sendDispensingSettings(id);
}

Timer streamTimer(1000);
void streamCamera()
{
	if(!streamTimer.update())
	{
		return;
	}

	camera_fb_t* fb = NULL;

	fb = esp_camera_fb_get();
	if (!fb)
	{
		Serial.println("Camera capture failed");
		return;
	}

	ws.binaryAll(fb->buf, fb->len);

	/*
	DiscordMessage msg;
	msg.message = "test";
	msg.fileName = "test.jpeg";
	msg.fileBuf = fb->buf;
	msg.fileSize = fb->len;
	sendMsg(msg);
	*/

	esp_camera_fb_return(fb);
}

void execDispense(Task& task)
{
	executeArduino({Commands::ARD_SetWheelPosition, WheelPosition::CLOSE_LOOSE});

	executeArduino({Commands::ARD_SetSplitterPosition, SplitterPosition::LEFT}); 
	executeArduino({Commands::ARD_Tare});

	auto weight = executeArduino({Commands::ARD_Weight}).args[0];

	const auto filledWeight = 10000;
	const auto targetWeight = 10000 * task.args[0] / 100;
	
	do
	{
		executeArduino({Commands::ARD_Dispense});
		weight = executeArduino({Commands::ARD_Weight}).args[0];
		FixedString<128> str;
		str = weight;
		ws.textAll("weight");
		ws.textAll(str.c_str());

	}
	while(weight < targetWeight);

	executeArduino({Commands::ARD_SetSplitterPosition, SplitterPosition::RIGHT}); 
	executeArduino({Commands::ARD_Tare});

	do
	{
		executeArduino({Commands::ARD_Dispense});
		weight = executeArduino({Commands::ARD_Weight}).args[0];
	}
	while(weight < targetWeight);
	/*
	
  setWheelPosition(WheelPosition::CLOSE_LOOSE);
  
  updateScale(true);

  setSplitterPosition(SplitterPosition::LEFT);
  while(lastScaleValue < leftFillWeight)
  {
    dispense();
    updateScale();
    mainSerial->println(lastScaleValue, 5);
  }
  
  setSplitterPosition(SplitterPosition::RIGHT);
  while(lastScaleValue < rightFillWeight)
  {
    dispense();
    updateScale();
    mainSerial->println(lastScaleValue, 5);
  }

  setWheelPosition(WheelPosition::OPEN);
  */

}

TaskExec taskExecs[Commands::_ESP_COUNT];
void initTasks()
{
	taskExecs[Commands::ESP_Capture] = &execCapture;
	taskExecs[Commands::ESP_Dispense] = &execDispense;
	taskExecs[Commands::ESP_SetDispensingSetting] = &execSetDispensingSetting;
	taskExecs[Commands::ESP_SaveDispensingSettings] = &execSaveDispensingSettings;
}

Timer dispensingCheckTimer(1000);
int lastDispensing = -1;
void updateDispensing()
{
	if(!dispensingCheckTimer.update())
	{
		return;
	}

	struct tm timeinfo;
	getLocalTime(&timeinfo);

	const auto currentTime = timeinfo.tm_hour * 60 * 60 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

	if(lastDispensing == -1)
	{
		lastDispensing = currentTime;
		return;
	}

	for(size_t x = 0; x < DispensingEventMax; x++)
	{
		const auto& dispensingEvent = dispensingEvents[x];
		const auto time = dispensingEvent[DispensingSettings::Time];
		
		if(lastDispensing < time && currentTime >= time)
		{
			lastDispensing = currentTime;

			const auto amount = dispensingEvent[DispensingSettings::Amount];

			Serial.print(F("Dispensing "));
			Serial.print(amount);
			Serial.print(F("% at "));
			Serial.print(asctime(&timeinfo));

			pushTask({Commands::ESP_Dispense, amount});
		}
	}
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

	loadDispensingSettings();

	server.on("/capture", HTTP_GET, [](AsyncWebServerRequest* request) {
		Serial.println("capture");
		pushTask({Commands::ESP_Capture});
		request->send_P(200, "text/plain", "Taking Photo");
	});

	server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest* request) {
		if(lastCatpure)
		{
			AsyncResponseStream *response = request->beginResponseStream("image/jpg", lastCatpureLen);
			response->write(lastCatpure, lastCatpureLen);
			request->send(response);
		}
		else
		{
			request->send(200, "text/plain", "No Img");
		}
	});

	server.on("/send", HTTP_GET, [](AsyncWebServerRequest* request) {
		request->send(200, "text/plain", "Sending");
		//sendMsg();
	});

	server.on("/arduino", HTTP_GET, [](AsyncWebServerRequest* request) {
		const char* PARAM_INPUT_1 = "output";

		// GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
		if (request->hasParam(PARAM_INPUT_1)) {

			//pendingArduinoCmd.push({ request->getParam(PARAM_INPUT_1)->value() });
		}

		request->send(200, "text/plain", "Received");
	});

	server.on("/weight", HTTP_GET, [](AsyncWebServerRequest* request) {
		FixedString<128> str;
		str = executeArduino({Commands::ARD_Weight}).args[0];
		request->send(200, "text/plain", str.c_str());
	});

	server.on("/dispense", HTTP_GET, [](AsyncWebServerRequest* request) {
		FixedString<128> str;
		pushTask({Commands::ESP_Dispense, 50});
		request->send(200, "text/plain", "");
	});

	server.serveStatic("/", LittleFS, "/web").setDefaultFile("index.html");

	setupWs(server, onWsMessage, onWsConnect);

	server.begin();

	Serial2.begin(9600, SERIAL_8N1, 14, 15);
	
	DiscordMessage msg;
	msg.message = "Device Rebooted";
	sendMsg(msg);
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

	updateDispensing();
}