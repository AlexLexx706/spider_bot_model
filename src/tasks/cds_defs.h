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
		SetMinLimmitCmd,
		SetMaxLimmitCmd,
		LoadServosCmd,
		UnloadServosCmd,
		EnableSteringCmd,
		DisableSteringCmd,
		EnableReadAngles,
		DisableReadAngles
	};

	enum State {
		NoneState,
		ReadAnglesState,
		CompleteState,
		ErrorWrongAddress,
		ErrorState,
	};

	struct Input {
		Cmd cmd;
		uint8_t address;
		FLOAT limmit;
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
	bool active;
};

struct ServoLinkDesc {
	bool active;
	LimmitDesc min;
	LimmitDesc max;
	uint16_t servo_angle;
	FLOAT model_angle;
};

//contain servos links desc
extern ServoLinkDesc servo_links[12];

//store for manage_servo_task
extern ManagaServoTaskNS::Store managa_servo_task_store;
#endif //_CDS_DEFS_H_