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
		SetMinLimmitCmd,
		SetMaxLimmitCmd,
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

extern ManagaServoTaskNS::Store managa_servo_task_store;
#endif //_CDS_DEFS_H_