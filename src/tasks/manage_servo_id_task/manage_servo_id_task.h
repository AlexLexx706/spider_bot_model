#ifndef _MANAGA_SERVO_ID_TASK_H_
#define _MANAGA_SERVO_ID_TASK_H_


class ManagaServoIDTask {
public:
	struct InputStore {
		int cmd;
		int param;
	};

	struct OutputStore {
		int state;
	};

	struct DataStore{
		InputStore input;
		OutputStore input;
	} store;

	bool init();
	bool proc();
};

#endif
