struct Commands : public SmartEnum<>
{
	constexpr Commands(char x = 0) : SmartEnum(x) {}
	const static Commands
	_ESP_BEGIN,
	ESP_Capture,
	ESP_Dispense,
	ESP_SetDispensingSetting,
	ESP_SaveDispensingSettings,
	ESP_WifiList,
	ESP_WifiStatus,
	ESP_WifiConnect,
	ESP_Temperature,
	_ESP_END,
	_ESP_COUNT,
	_ARD_BEGIN,
	ARD_Dispense,
	ARD_Reservoir,
	_ARD_END,
	_ARD_COUNT;
};

constexpr const Commands
	Commands::_ESP_BEGIN{0},
	Commands::ESP_Capture{0},
	Commands::ESP_Dispense{1},
	Commands::ESP_SetDispensingSetting{2},
	Commands::ESP_SaveDispensingSettings{3},
	Commands::ESP_WifiList{4},
	Commands::ESP_WifiStatus{5},
	Commands::ESP_WifiConnect{6},
	Commands::ESP_Temperature{7},
	Commands::_ESP_END{8},
	Commands::_ESP_COUNT{8},
	Commands::_ARD_BEGIN{9},
	Commands::ARD_Dispense{9},
	Commands::ARD_Reservoir{10},
	Commands::_ARD_END{11},
	Commands::_ARD_COUNT{2};

struct PaddleActions : public SmartEnum<>
{
	constexpr PaddleActions(char x = 0) : SmartEnum(x) {}
	const static PaddleActions
	_BEGIN,
	CLOCKWISE,
	COUNTER_CLOCKWISE,
	JIGGLE,
	_END,
	_COUNT;
};

constexpr const PaddleActions
	PaddleActions::_BEGIN{0},
	PaddleActions::CLOCKWISE{0},
	PaddleActions::COUNTER_CLOCKWISE{1},
	PaddleActions::JIGGLE{2},
	PaddleActions::_END{3},
	PaddleActions::_COUNT{3};

struct Lasers : public SmartEnum<>
{
	constexpr Lasers(char x = 0) : SmartEnum(x) {}
	const static Lasers
	_BEGIN,
	CHUTE,
	RESERVOIR,
	_END,
	_COUNT;
};

constexpr const Lasers
	Lasers::_BEGIN{0},
	Lasers::CHUTE{0},
	Lasers::RESERVOIR{1},
	Lasers::_END{2},
	Lasers::_COUNT{2};

struct DispensingEventSettings : public SmartEnum<>
{
	constexpr DispensingEventSettings(char x = 0) : SmartEnum(x) {}
	const static DispensingEventSettings
	_BEGIN,
	Amount,
	Time,
	Positions,
	_END,
	_COUNT;
};

constexpr const DispensingEventSettings
	DispensingEventSettings::_BEGIN{0},
	DispensingEventSettings::Amount{0},
	DispensingEventSettings::Time{1},
	DispensingEventSettings::Positions{2},
	DispensingEventSettings::_END{3},
	DispensingEventSettings::_COUNT{3};

struct ShiftOutputs : public SmartEnum<>
{
	constexpr ShiftOutputs(char x = 0) : SmartEnum(x) {}
	const static ShiftOutputs
	_BEGIN,
	ChuteLaser,
	ReservoirLaser,
	MotorA,
	MotorB,
	_END,
	_COUNT;
};

constexpr const ShiftOutputs
	ShiftOutputs::_BEGIN{0},
	ShiftOutputs::ChuteLaser{0},
	ShiftOutputs::ReservoirLaser{1},
	ShiftOutputs::MotorA{2},
	ShiftOutputs::MotorB{3},
	ShiftOutputs::_END{4},
	ShiftOutputs::_COUNT{4};

struct DispensingResult : public SmartEnum<>
{
	constexpr DispensingResult(char x = 0) : SmartEnum(x) {}
	const static DispensingResult
	_BEGIN,
	Started,
	Ok,
	NoOutput,
	PaddleStuck,
	FoodJammed,
	_END,
	_COUNT;
};

constexpr const DispensingResult
	DispensingResult::_BEGIN{0},
	DispensingResult::Started{0},
	DispensingResult::Ok{1},
	DispensingResult::NoOutput{2},
	DispensingResult::PaddleStuck{3},
	DispensingResult::FoodJammed{4},
	DispensingResult::_END{5},
	DispensingResult::_COUNT{5};

const auto DispensingEventMax = 4;
const auto PositionCount = 3;
