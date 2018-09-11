#ifndef _MANAGA_SERVO_ID_TASK_H_
#define _MANAGA_SERVO_ID_TASK_H_


class ManagaServoIDTask {
public:
	enum Cmd{
		GetIDCmd,
		SetIDCmd
	};

	enum State {
		NoneState,
		CompleteState,
		ErrorState,
	};

	struct InputStore {
		int cmd;
		int param;
	};

	struct OutputStore {
		int state;
	};

	struct DataStore{
		InputStore input;
		OutputStore output;
	} store;

	bool init();
	bool proc();
};

#endif
