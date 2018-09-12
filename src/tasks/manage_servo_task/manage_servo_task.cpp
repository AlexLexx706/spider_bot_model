#include "cds_defs.h"
#include <string.h>
#include "manage_servo_task.h"
#include "serial.h"
#include "servo_proto.h"

ManagaServoTaskNS::Store managa_servo_task_store;
ServoLinkDesc servo_links[12];
extern Serial serial;
	
bool ManagaServoTask::init() {
	memset(&managa_servo_task_store, 0, sizeof(managa_servo_task_store));
	memset(&servo_links, 0, sizeof(servo_links));
	return false;
}

bool ManagaServoTask::proc() {
	ManagaServoTaskNS::Cmd cmd = managa_servo_task_store.input.cmd;

	if (cmd == ManagaServoTaskNS::ResetAddressesCmd) {
		//reset all address
		Servo::set_id(serial, 0xFE, 1);

		//unload all servos
		Servo::unload(serial, 0xFE);
		managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
	} else if (cmd == ManagaServoTaskNS::SetAddressCmd) {
		Servo::set_id(serial, 0xFE, managa_servo_task_store.input.address);
	} else if (cmd == ManagaServoTaskNS::SetMinLimmitCmd) {
		Servo::set_id(serial, 0xFE, managa_servo_task_store.input.address);
	}
}
