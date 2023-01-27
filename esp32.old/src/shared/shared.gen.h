struct Commands : public SmartEnum<>
{
	constexpr Commands(char x = 0) : SmartEnum(x) {}
	const static Commands
	_ESP_BEGIN,
	ESP_Capture,
	ESP_Dispense,
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
	ARD_SetWheelAngle,
	ARD_SetSplitterAngle,
	_ARD_END,
	_ARD_COUNT;
};

constexpr const Commands
	Commands::_ESP_BEGIN{0},
	Commands::ESP_Capture{0},
	Commands::ESP_Dispense{1},
	Commands::ESP_SetDispensingSetting{2},
	Commands::ESP_SaveDispensingSettings{3},
	Commands::_ESP_END{4},
	Commands::_ESP_COUNT{4},
	Commands::_ARD_BEGIN{5},
	Commands::ARD_Tare{5},
	Commands::ARD_ResetTare{6},
	Commands::ARD_Weight{7},
	Commands::ARD_Dispense{8},
	Commands::ARD_SetWheelPosition{9},
	Commands::ARD_SetSplitterPosition{10},
	Commands::ARD_SetWheelAngle{11},
	Commands::ARD_SetSplitterAngle{12},
	Commands::_ARD_END{13},
	Commands::_ARD_COUNT{8};

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
