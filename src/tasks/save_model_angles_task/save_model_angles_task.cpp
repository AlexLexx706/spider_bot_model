#include "save_model_angles_task.h"
#include "cds_defs.h"
#include "tasks_utils.h"
#include <stdio.h>

bool SaveModelAnglesTask::init() {
	return true;
}

void SaveModelAnglesTask::proc(){
	for(int servo_address = 0; servo_address < SERVOS_COUNT; servo_address++) {
		servo_links[servo_address].model_angle = get_model_angle(servo_address);
	}
}