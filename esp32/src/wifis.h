IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);


bool bWifiScanning = false;
void sendWifiList()
{
	int n = WiFi.scanNetworks(true);
	bWifiScanning = true;

	Serial.println("Scanning networks");
}

void updateWifiListStreaming()
{
	if(!bWifiScanning)
	{
		return;
	}

	const int n = WiFi.scanComplete();

	if(n == WIFI_SCAN_RUNNING)
	{
		return;
	}

	bWifiScanning = false;

	if(n < 0)
	{
		return;
	}

	FixedString<256> str;
	str += Commands::ESP_WifiList;
	str += ',';
	
	for (int i = 0; i < n; ++i)
	{
		if(i > 0)
		{
			str += '+';
		}

		str += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? 'O' : 'L');
		str += '$';
		str += WiFi.SSID(i);
		str += '$';
		str += WiFi.RSSI(i);
	}

	ws.textAll(str.c_str());
}

void sendWifiStatus()
{
	FixedString<256> str;
	str += Commands::ESP_WifiStatus;
	str += ',';
	str += (WiFi.status() == WL_CONNECTED ? '1' : '0');
	str += ',';
	str += WiFi.RSSI();
	

	ws.textAll(str.c_str());
}

void onWifiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
	Serial.println("Wifi Disconnected");
	//WiFi.disconnect();

	//WiFi.begin();
  
	//sendWifiStatus();
}

bool waitingWifiConnect = false;
void onWifiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
	Serial.println("Wifi Connected");

	WiFi.softAPdisconnect(true);

	Serial.print("WiFi strength: ");
	Serial.println(WiFi.RSSI());

	Serial.print("IP Address: http://");
	Serial.println(WiFi.localIP());

	waitingWifiConnect = true;
}

void updateWifiConnect()
{
	if(waitingWifiConnect)
	{
		waitingWifiConnect = false;

		initClock();
		
		DiscordMessage msg;
		msg.message = "Device connected to wifi";
		sendMsg(msg);
	
		sendWifiStatus();
	}
}

void execWifiList(Task& task)
{
	sendWifiList();
}

void execWifiConnect(const char* msg)
{
	FixedString<256> str(msg);
	
	const auto commaIndex = str.indexOf(',');
	if(commaIndex < 0)
	{
		return;
	}

	str = str.substring(commaIndex + 1);

	const auto dollarIndex = str.indexOf('$');
	if(dollarIndex < 0)
	{
		return;
	}

	const auto password = str.substring(dollarIndex + 1);
	const auto ssid = str.substring(0, dollarIndex);

	Serial.println("Connecting to Wifi");
	Serial.print("\tSSID: ");
	Serial.println(ssid.c_str());
	Serial.print("\tpassword: ");
	Serial.println(password.c_str());

	WiFi.begin(ssid.c_str(), password.c_str());
}

void initWifi()
{
	//WiFi.onEvent(onWifiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
	WiFi.onEvent(onWifiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
	WiFi.onEvent(onWifiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	WiFi.setSleep(false);

	delay(500);

	//WiFi.begin(ssid, password);
	//WiFi.begin("test", "testtest");
	WiFi.begin();

	delay(500);

	Serial.print("Connecting WiFi: ");
	Timer wifiTimeout(10000);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");

		if(wifiTimeout.update())
		{
			break;
		}
	}
	
	Serial.println("");

	if(WiFi.status() != WL_CONNECTED)
	{
		WiFi.disconnect(true);
  		WiFi.mode(WIFI_AP);

		Serial.println("WiFi Connection failed");
		Serial.println("Starting Station");

		WiFi.softAP(configSSID, __SECRET_CONFIG_PASSWORD__); 
		WiFi.softAPConfig(local_ip, gateway, subnet);

		delay(500);

		Serial.print("[+] AP Created with IP Gateway ");
		Serial.println(WiFi.softAPIP());
	}
}

void updateWifi()
{
	updateWifiListStreaming();
	updateWifiConnect();
}