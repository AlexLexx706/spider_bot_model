#include "cds_defs.h"
#include <string.h>
#include "manage_servo_task.h"
#include "serial.h"
#include "serial_servo_rp.h"

ManagaServoTaskNS::Store managa_servo_task_store;
extern Serial serial;

	
bool ManagaServoTask::init() {
	memset(&managa_servo_task_store, 0, sizeof(managa_servo_task_store));
	return false;
}

bool ManagaServoTask::proc() {
	ManagaServoTaskNS::Cmd cmd = managa_servo_task_store.input.cmd;

	if (cmd == ManagaServoTaskNS::ResetAddressesCmd) {
		LobotSerialServoSetID(serial, 0xFE, 1);
		managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
	} else if (cmd == ManagaServoTaskNS::SetAddressCmd) {
		LobotSerialServoSetID(serial, 0xFE, managa_servo_task_store.input.address);
	} else if (cmd == ManagaServoTaskNS::SetMinLimmitCmd) {
		LobotSerialServoSetID(serial, 0xFE, managa_servo_task_store.input.address);
	}

}
