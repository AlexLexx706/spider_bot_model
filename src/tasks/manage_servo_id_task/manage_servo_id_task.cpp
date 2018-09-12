#include "manage_servo_id_task.h"
#include <string.h>

bool ManagaServoIDTask::init() {
	memset(&store, 0, sizeof(store));
	return false;
}

bool ManagaServoIDTask::proc() {
	return false;
}
