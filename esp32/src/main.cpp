
//#define ASYNC_TCP_SSL_ENABLED true

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "esp_pm.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "esp_phy_init.h"
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
#include <Preferences.h>
#include <AsyncElegantOTA.h>

#include "./shared/shared.h"

#include "consts.h"
#include "camera.h"
#include "secret.h"
#include "utils.h"
#include "ws.h"
#include "discord.h"
#include "pushover.h"
#include "tasks.h"
#include "wifis.h"
#include "dispensing.h"

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

AsyncWebServer server(4560);

TaskResult executeArduino(const Task& task)
{
	Serial1.print(+task.cmd);

	for(size_t x = 0; x < task.argCount; x++)
	{
		Serial1.print(",");
		Serial1.print(+task.args[x]);
	}

	Serial1.print('\n');

	return {};
}

void onArduinoMsg(const TaskResult& result)
{
	if(result.argCount >= 1)
	{
		if(result.args[0] == Commands::ARD_Dispense)
		{
			onArdDispense(result);
		}
		else if(result.args[0] == Commands::ARD_Reservoir)
		{
			onArdReservoir(result);
		}
	}
}

FixedString<256> arduinoBuffer;
void updateArduino()
{
	while (Serial1.available())
	{
		arduinoBuffer += (char)Serial1.read();
	}

	const auto endIndex = arduinoBuffer.indexOf('\n');
	if(endIndex >= 0)
	{
		const auto command = arduinoBuffer.substring(0, endIndex);
		arduinoBuffer = arduinoBuffer.substring(endIndex + 1);

		const auto msg = "ard " + command;
		ws.textAll(msg.c_str());

		TaskResult result;
		result.argCount = splitStr(command.c_str(), result.args, Task::MaxArgCount, ' ');

		onArduinoMsg(result);
	}
}

void execCapture(Task& task)
{
	capture();
}

void execTemperature(Task& task)
{
	const auto temp = (temprature_sens_read() - 32) / 1.8f;
	Serial.println(temp);

	FixedString<64> str(temp);
	ws.textAll(str.c_str());
}

void execArd(Task& task)
{
	const auto& result = executeArduino(task);

	if(task.cmd == Commands::ARD_Dispense)
	{
		onArdDispense(result);
	}
}

void onWsMessage(uint32_t id, const char* msg)
{
	int buf[Task::MaxArgCount + 1];
	size_t count = splitStr(msg, buf, Task::MaxArgCount + 1);

	Task task;

	if(count > 0)
	{
		task.cmd = buf[0];

		if(task.cmd == Commands::ESP_WifiConnect)
		{
			execWifiConnect(msg);
			return;
		}

		for(int x = 1; x < min(count, Task::MaxArgCount + 1); x++)
		{
			task.args[x - 1] = buf[x];
		}

		task.argCount = count - 1;

		pushTask(task);
	}
}

void onWsConnect(uint32_t id)
{
	dispensingSettings.send(ws, id);
	sendWifiStatus();
}

void streamCamera()
{
	if(ws.count() <= 0)
	{
		setupCamera(CameraSetting::Deinit);
		return;
	}

	static AsyncWebSocketMessageBuffer* buffer = nullptr;
	if(buffer && buffer->count() > 1)
	{
		return;
	}

	camera_fb_t* fb = NULL;

	setupCamera(CameraSetting::LowRes);

	fb = esp_camera_fb_get();
	if (!fb)
	{
		Serial.println("Stream: Camera capture failed");
		return;
	}

	if(!buffer)
	{
		buffer = new AsyncWebSocketMessageBuffer();
		(*buffer)++;
	}

	buffer->reserve(fb->len);
	memcpy(buffer->get(), fb->buf, fb->len);
	
	esp_camera_fb_return(fb);
	setupCamera(CameraSetting::Deinit);

	ws.binaryAll(buffer);
}

void noopTask(Task& task) {}

TaskExec taskExecs[Commands::_ESP_COUNT] = {};
void initTasks()
{
	taskExecs[Commands::ESP_Capture] = &execCapture;
	taskExecs[Commands::ESP_Dispense] = &execDispense;
	taskExecs[Commands::ESP_SetDispensingSetting] = &execSetDispensingSetting;
	taskExecs[Commands::ESP_SaveDispensingSettings] = &execSaveDispensingSettings;
	taskExecs[Commands::ESP_WifiList] = &execWifiList;
	taskExecs[Commands::ESP_Temperature] = &execTemperature;
}

void setup()
{
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	Serial.println();

	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
	
	esp_pm_config_esp32_t pmConfig;
	pmConfig.max_freq_mhz = 240; // 80, 160, 240 
	pmConfig.min_freq_mhz = 240;
	pmConfig.light_sleep_enable = false;
	esp_pm_configure(&pmConfig);

	initWifi();

	tasksSemaphore = xSemaphoreCreateMutex();
	initTasks();

	pinMode(4, OUTPUT);

	setupCamera(CameraSetting::LowRes);

    Task task;
    execCapture(task);
	initLITTLEFS();

	setupCamera(CameraSetting::Deinit);

	if(!preferences.begin("esp", false))
	{
		Serial.println("Failed to initialize preferences");
	}

	dispensingSettings.load();

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

	server.serveStatic("/", LittleFS, "/web").setDefaultFile("index.html");

	setupWs(server, onWsMessage, onWsConnect);

  	AsyncElegantOTA.begin(&server);

	server.begin();
	Serial.println("Server Started");

	Serial1.begin(9600, SERIAL_8N1, 14, 15);
	//Serial1.begin(9600, SERIAL_8N1, 2, 15);
}

void loop()
{
	Task task;
	while (popTask(task))
	{
		if(task.cmd >= Commands::_ESP_BEGIN && task.cmd <= Commands::_ESP_END)
		{
			if(taskExecs[task.cmd])
			{
				taskExecs[task.cmd](task);
			}
			else
			{
				Serial.print("Task exec not found: ");
				Serial.println(+task.cmd);
			}
		}
		else if(task.cmd >= Commands::_ARD_BEGIN && task.cmd <= Commands::_ARD_END)
		{
			execArd(task);
		}
	}

	ws.cleanupClients();

	streamCamera();
	updateWifi();
	updateArduino();
	updateDispensing();

	delay(10);
}