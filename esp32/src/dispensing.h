struct DispensingSettings
{
    using DispensingEvent = int[DispensingEventSettings::_COUNT];

    DispensingEvent dispensingEvents[DispensingEventMax] = {};

    constexpr static const char* dispensingSettingsId = "DisSet";

    void reset();
    void load();
    void save() const;
    void send(AsyncWebSocket& ws, uint32_t clientId) const;
    void receive(const Task& task);
};

Preferences preferences;

void DispensingSettings::reset()
{
	*this = {}; 
}

void DispensingSettings::load()
{
	if(preferences.getBytes(dispensingSettingsId, (void*)this, sizeof(*this)) != sizeof(*this))
	{
		reset();
	}
}

void DispensingSettings::save() const
{
    preferences.putBytes(dispensingSettingsId, (void*)this, sizeof(*this));
}

void DispensingSettings::send(AsyncWebSocket& ws, uint32_t clientId) const
{
	FixedString<256> str;
	str += Commands::ESP_SetDispensingSetting;
	for(size_t x = 0; x < DispensingEventMax; x++)
	{
		auto& dispensingEvent = dispensingEvents[x];
		for(size_t y = 0; y < DispensingEventSettings::_COUNT; y++)
		{
			str += ',';
			str += dispensingEvent[y];
		}
	}

	ws.text(clientId, str.c_str(), str.length());
}

void DispensingSettings::receive(const Task& task)
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

	if(settingIndex < 0 || settingIndex >= DispensingEventSettings::_COUNT)
	{
		return;
	}

	dispensingEvents[eventIndex][settingIndex] = value;
}

DispensingSettings dispensingSettings;

void execSetDispensingSetting(Task& task)
{
    dispensingSettings.receive(task);
}

void execSaveDispensingSettings(Task& task)
{
    dispensingSettings.save();
}

void execDispense(Task& task)
{
    const auto amount = task.args[0];
    const auto positions = task.args[1];
    
    pushTask({Commands::ARD_Reservoir});
    pushTask({Commands::ARD_Dispense, amount, positions});
	//executeArduino();
}

bool isReservoirLow = false;
void onArdReservoir(const TaskResult& result)
{
    isReservoirLow = !result.args[1];
}

void onArdDispense(const TaskResult& result)
{
    if(result.argCount < 4)
    {
        return;
    }

    capture();

    FixedString<256> message;
    
    static const char* dispensedMessages[] =
    {
        "Fed and content! 🐾",
        "Mission: Fed Accomplished!",
        "Cat feast success! 😺",
        "Dinner served! 🍲",
        "Happy cats, full bowls! 😸",
        "Nom nom time done! 🐱",
        "Mealtime magic! ✨",
        "Purr-fection achieved! 🐈",
        "Content kitties! 😽",
        "Feeder success! 🍽️",
        "Belly bliss! 🥣",
        "Cat chef on duty! 🍗",
        "Well-fed wonders! 🐾",
        "Satisfied meows! 😻",
        "Full and purring! 🐱💤",
        "Feeding triumph! 🏆",
        "Happy dining cats! 🐾🍴",
        "Culinary delight! 🍲",
        "Cat happiness delivered! 😺",
        "Dine and delight! 🍽️",
    };

    const auto errorsCount = result.args[1] + result.args[2] + result.args[3];

    if(errorsCount == 0)
    {
        const auto messageIndex = random(sizeof(dispensedMessages)/sizeof(*dispensedMessages));
        message = dispensedMessages[messageIndex];
    }
    else
    {
        message = "DISPENSING ERROR (";

        bool first = true;

        const char* errorMsgs[3]  = {};
        
        errorMsgs[DispensingResult::PaddleStuck - DispensingResult::Ok - 1] = "Stuck";
        errorMsgs[DispensingResult::NoOutput - DispensingResult::Ok - 1] = "NoOutput";
        errorMsgs[DispensingResult::FoodJammed - DispensingResult::Ok - 1] = "Jammed";

        for(auto errorIndex = 0; errorIndex < 3; errorIndex++)
        {
            const auto errorCount = result.args[errorIndex + 1];
            if(errorCount > 0)
            {
                if(!first)
                {
                    message += " ";
                }
                first = false;

                message += errorMsgs[errorIndex];
                message += " x";
                message += errorCount;
            }
        }

        message += ")";
    }

    if(isReservoirLow)
    {
        message += "\nReservoir is low!";
    }

    Serial.print("Pushover msg: ");
    Serial.println(message.c_str());

	PushoverMessage pushoverMsg;
	pushoverMsg.message = message.c_str();
	pushoverMsg.fileName = "capture.jpeg";
	pushoverMsg.fileBuf = lastCatpure;
	pushoverMsg.fileSize = lastCatpureLen;
	sendMsg(pushoverMsg);

    if(errorsCount > 0 || isReservoirLow)
    {
        message += "\n@everyone";
    }

    const auto newLine = "\n";
    const auto newLineEscaped = "\\n";
    
    message.replace(newLine, newLineEscaped);

    Serial.print("Dsicord msg: ");
    Serial.println(message.c_str());

    DiscordMessage discordMsg;
	discordMsg.message = message.c_str();
	discordMsg.fileName = "capture.jpeg";
	discordMsg.fileBuf = lastCatpure;
	discordMsg.fileSize = lastCatpureLen;
	sendMsg(discordMsg);
}

Timer dispensingCheckTimer(1000);
int lastUpdate = -1;
void updateDispensing()
{
	if(!dispensingCheckTimer.update())
	{
		return;
	}

	struct tm timeinfo;
	getLocalTime(&timeinfo, 100);

	const auto currentTime = timeinfo.tm_hour * 60 * 60 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

    if(lastUpdate <= 0)
    {
        lastUpdate = currentTime;
    }

	for(size_t x = 0; x < DispensingEventMax; x++)
	{
		const auto& dispensingEvent = dispensingSettings.dispensingEvents[x];
		const auto time = dispensingEvent[DispensingEventSettings::Time];

		if(lastUpdate < time && currentTime >= time)
		{
			const auto amount = dispensingEvent[DispensingEventSettings::Amount];

            if(amount <= 0)
            {
                continue;
            }

			const auto positions = dispensingEvent[DispensingEventSettings::Positions];

			Serial.print(F("Dispensing "));
			Serial.print(amount);
			Serial.print(F("% at "));
			Serial.print(asctime(&timeinfo));

			pushTask({Commands::ESP_Dispense, amount, positions});
		}
	}

    lastUpdate = currentTime;
}

