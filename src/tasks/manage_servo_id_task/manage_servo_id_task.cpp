#include "manage_servo_id_task.h"

bool ManagaServoIDTask::init() {
	mamset(&store, 0, sizeof(store));
	return false;
}

bool ManagaServoIDTask::proc() {
	return false;
}
