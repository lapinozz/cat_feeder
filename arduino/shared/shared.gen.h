struct Commands : public SmartEnum<>
{
	constexpr Commands(char x = 0) : SmartEnum(x) {}
	const static Commands
	_ESP_BEGIN,
	ESP_Capture,
	ESP_SetDispensingSetting,
	ESP_SaveDispensingSettings,
	_ESP_END,
	_ESP_COUNT,
	_ARD_BEGIN,
	ARD_Tare,
	ARD_ResetTare,
	ARD_Weight,
	ARD_Dispense,
	ARD_SetWheelPosition,
	ARD_SetSplitterPosition,
	_ARD_END,
	_ARD_COUNT;
};

constexpr const Commands
	Commands::_ESP_BEGIN{0},
	Commands::ESP_Capture{0},
	Commands::ESP_SetDispensingSetting{1},
	Commands::ESP_SaveDispensingSettings{2},
	Commands::_ESP_END{3},
	Commands::_ESP_COUNT{3},
	Commands::_ARD_BEGIN{4},
	Commands::ARD_Tare{4},
	Commands::ARD_ResetTare{5},
	Commands::ARD_Weight{6},
	Commands::ARD_Dispense{7},
	Commands::ARD_SetWheelPosition{8},
	Commands::ARD_SetSplitterPosition{9},
	Commands::_ARD_END{10},
	Commands::_ARD_COUNT{6};

struct WheelPosition : public SmartEnum<>
{
	constexpr WheelPosition(char x = 0) : SmartEnum(x) {}
	const static WheelPosition
	_BEGIN,
	OPEN,
	CLOSE_LOOSER,
	CLOSE_LOOSE,
	CLOSE_TIGHT,
	_END,
	_COUNT;
};

constexpr const WheelPosition
	WheelPosition::_BEGIN{0},
	WheelPosition::OPEN{0},
	WheelPosition::CLOSE_LOOSER{1},
	WheelPosition::CLOSE_LOOSE{2},
	WheelPosition::CLOSE_TIGHT{3},
	WheelPosition::_END{4},
	WheelPosition::_COUNT{4};

struct SplitterPosition : public SmartEnum<>
{
	constexpr SplitterPosition(char x = 0) : SmartEnum(x) {}
	const static SplitterPosition
	_BEGIN,
	LEFT,
	RIGHT,
	_END,
	_COUNT;
};

constexpr const SplitterPosition
	SplitterPosition::_BEGIN{0},
	SplitterPosition::LEFT{0},
	SplitterPosition::RIGHT{1},
	SplitterPosition::_END{2},
	SplitterPosition::_COUNT{2};

struct DispensingSettings : public SmartEnum<>
{
	constexpr DispensingSettings(char x = 0) : SmartEnum(x) {}
	const static DispensingSettings
	_BEGIN,
	Amount,
	Time,
	_END,
	_COUNT;
};

constexpr const DispensingSettings
	DispensingSettings::_BEGIN{0},
	DispensingSettings::Amount{0},
	DispensingSettings::Time{1},
	DispensingSettings::_END{2},
	DispensingSettings::_COUNT{2};

const auto DispensingEventMax = 4;
