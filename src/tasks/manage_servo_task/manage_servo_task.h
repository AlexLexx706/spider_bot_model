#ifndef _MANAGA_SERVO_TASK_H_
#define _MANAGA_SERVO_TASK_H_
#include "common_defs.h"
#include <stdint.h>

class ManagaServoTask {
	struct StartCalibrationData {
		uint16_t servo_value;
		FLOAT model_value;
	};
	StartCalibrationData start_calibration_data;
public:
	bool init();
	void proc();
};

#endif
