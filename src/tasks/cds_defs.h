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
		ProgressState,
		CompleteState,
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
	uint16_t value;
	bool active;
};

struct ServoLinkDesc {
	uint8_t servo_id;
	int model_part_id;
	LimmitDesc min;
	LimmitDesc max;
};

//contain servos links desc
extern ServoLinkDesc servo_links[12];

//store for manage_servo_task
extern ManagaServoTaskNS::Store managa_servo_task_store;
#endif //_CDS_DEFS_H_