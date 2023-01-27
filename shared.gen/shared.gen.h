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
	ARD_Dispense,
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
	Commands::ARD_Dispense{5},
	Commands::_ARD_END{6},
	Commands::_ARD_COUNT{1};

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
	Ok,
	PaddleStuck,
	NoOutput,
	_END,
	_COUNT;
};

constexpr const DispensingResult
	DispensingResult::_BEGIN{0},
	DispensingResult::Ok{0},
	DispensingResult::PaddleStuck{1},
	DispensingResult::NoOutput{2},
	DispensingResult::_END{3},
	DispensingResult::_COUNT{3};

const auto DispensingEventMax = 4;
