#ifndef _CDS_DEFS_H_
#define _CDS_DEFS_H_
#include "common_defs.h"
#include <stdint.h>

//define cds for ManagaServoTask 
namespace ManagaServoTaskNS {
	enum Cmd{
		NoneCmd,
		ResetAddressesCmd,
		SetAddressCmd,
		ResetLimmits,
		StartCalibration,
		CompliteCalibration,
		LoadServosCmd,
		UnloadServosCmd,
		EnableSteringCmd,
		DisableSteringCmd,
		EnableReadAngles,
		DisableReadAngles,
		MoveServo,
		MoveServoSin,
		SetLedErrorCmd,	//use for detect servo problems via led flesh
	};

	enum State {
		NoneState,
		ReadAnglesState,
		MoveSinState,
		CalibrationProgressState,
		CompleteState,
		ReadRawState,
		SterringsProgress,

		ErrorState,
		ErrorNotActive,
		ErrorWrongAddress,
		ErrorWrondData,
		ErrorWrongServoPos,
		ErrorNotCalibrated,
	};

	struct Input {
		Cmd cmd;
		uint8_t address;
		FLOAT value;
	};

	struct Output {
		State state;
	};

	struct Store {
		Input input;
		Output output;
	};
};

struct LimmitDesc {
	uint16_t servo_value;
	FLOAT model_value;
};

struct ServoLinkDesc {
	bool active;
	bool calibrated;
	LimmitDesc min;
	LimmitDesc max;
	uint16_t servo_angle;
	FLOAT model_angle;
};

//contain servos links desc
extern ServoLinkDesc servo_links[SERVOS_COUNT];

//store for manage_servo_task
extern ManagaServoTaskNS::Store managa_servo_task_store;
#endif //_CDS_DEFS_H_