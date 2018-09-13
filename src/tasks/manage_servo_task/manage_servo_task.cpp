#include "cds_defs.h"
#include <string.h>
#include "manage_servo_task.h"
#include "serial.h"
#include "servo_proto.h"
#include <stdio.h>

ManagaServoTaskNS::Store managa_servo_task_store;
ServoLinkDesc servo_links[12];
extern Serial serial;
	
bool ManagaServoTask::init() {
	memset(&managa_servo_task_store, 0, sizeof(managa_servo_task_store));
	memset(&servo_links, 0, sizeof(servo_links));
	return true;
}

void ManagaServoTask::proc() {
	ManagaServoTaskNS::Cmd cmd = managa_servo_task_store.input.cmd;
	managa_servo_task_store.input.cmd = ManagaServoTaskNS::NoneCmd;
	// fprintf(stderr, "proc 1. cmd:%d\n", managa_servo_task_store.input.cmd);

	switch (cmd) { 
		case ManagaServoTaskNS::ResetAddressesCmd: {
			// reset all address
			Servo::set_id(serial, 0xFE, 1);

			// unload all servos
			Servo::unload(serial, 0xFE);

			// reset links
			memset(&servo_links, 0, sizeof(servo_links));
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
		}
		case ManagaServoTaskNS::SetAddressCmd: {
			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}

			Servo::set_id(serial, 0xFE, managa_servo_task_store.input.address);
			Servo::unload(serial, managa_servo_task_store.input.address);
			servo_links[managa_servo_task_store.input.address].active = true;
			servo_links[managa_servo_task_store.input.address].min.active = false;
			servo_links[managa_servo_task_store.input.address].max.active = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::ResetLimmits: {
			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			//unload servo
			Servo::unload(serial, managa_servo_task_store.input.address);
			servo_links[managa_servo_task_store.input.address].min.active = false;
			servo_links[managa_servo_task_store.input.address].max.active = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::SetMinLimmitCmd: {
			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}

			//1. read pos
			servo_links[managa_servo_task_store.input.address].min.servo_value =
				Servo::read_position(serial, managa_servo_task_store.input.address);

			uint16_t tmp;
			Servo::limit_read(
				serial,
				managa_servo_task_store.input.address,
				tmp,
				servo_links[managa_servo_task_store.input.address].max.servo_value);
			Servo::limit_write(
				serial,
				managa_servo_task_store.input.address,
				servo_links[managa_servo_task_store.input.address].min.servo_value,
				servo_links[managa_servo_task_store.input.address].max.servo_value);

			servo_links[managa_servo_task_store.input.address].min.active = true;
			servo_links[managa_servo_task_store.input.address].min.model_value = managa_servo_task_store.input.limmit;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::SetMaxLimmitCmd: {
			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}

			//1. read pos
			servo_links[managa_servo_task_store.input.address].max.servo_value =
				Servo::read_position(serial, managa_servo_task_store.input.address);

			uint16_t tmp;
			Servo::limit_read(
				serial,
				managa_servo_task_store.input.address,
				servo_links[managa_servo_task_store.input.address].min.servo_value,
				tmp);
			Servo::limit_write(
				serial,
				managa_servo_task_store.input.address,
				servo_links[managa_servo_task_store.input.address].min.servo_value,
				servo_links[managa_servo_task_store.input.address].max.servo_value);

			servo_links[managa_servo_task_store.input.address].max.active = true;
			servo_links[managa_servo_task_store.input.address].max.model_value = managa_servo_task_store.input.limmit;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::LoadServosCmd: {
			Servo::load(serial, 0xFE);
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::UnloadServosCmd: {
			Servo::unload(serial, 0xFE);
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::EnableReadAngles: {
			managa_servo_task_store.output.state = ManagaServoTaskNS::ReadAnglesState;
			return;
		}
		case ManagaServoTaskNS::DisableReadAngles: {
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
	}

	//read angles
	if (managa_servo_task_store.output.state == ManagaServoTaskNS::ReadAnglesState) {
		for (int i = 0; i < sizeof(servo_links) / sizeof(servo_links[1]); i++) {
			//check servo calibrated and 
			if (servo_links[i].active && servo_links[i].min.active && servo_links[i].max.active) {
				servo_links[i].servo_angle = Servo::read_position(serial, i);
				servo_links[i].model_angle =
					((servo_links[i].servo_angle - servo_links[i].min.servo_value) /
					(servo_links[i].max.servo_value - servo_links[i].min.servo_value)) * (
						servo_links[i].max.model_value - servo_links[i].min.model_value) + servo_links[i].min.model_value; 
			} else {
				servo_links[i].servo_angle = servo_links[i].min.servo_value;
				servo_links[i].model_angle = servo_links[i].min.model_value;
			}
		}
	}
}
