#include <string.h>
#include <stdio.h>
#include <math.h>
#include "cds_defs.h"
#include "manage_servo_task.h"
#include "serial.h"
#include "servo_proto.h"
#include "utils.h"


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
			fprintf(stderr, "ResetAddressesCmd 1.\n");
			// reset all address
			Servo::set_id(serial, 0xFE, 1);

			// unload all servos
			Servo::unload(serial, 0xFE);

			// reset links
			memset(&servo_links, 0, sizeof(servo_links));
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::SetAddressCmd: {
			fprintf(stderr, "SetAddressCmd 1.\n");

			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(stderr, "SetAddressCmd 2. address:%d\n", managa_servo_task_store.input.address);


			Servo::set_id(serial, 0xFE, managa_servo_task_store.input.address);
			Servo::unload(serial, managa_servo_task_store.input.address);
			servo_links[managa_servo_task_store.input.address].active = true;
			servo_links[managa_servo_task_store.input.address].min.active = false;
			servo_links[managa_servo_task_store.input.address].max.active = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::ResetLimmits: {
			fprintf(stderr, "ResetLimmits 1.\n");
			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(stderr, "ResetLimmits 2. address:%d\n", managa_servo_task_store.input.address);

			//unload servo
			Servo::unload(serial, managa_servo_task_store.input.address);
			servo_links[managa_servo_task_store.input.address].min.active = false;
			servo_links[managa_servo_task_store.input.address].max.active = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::SetMinLimmitCmd: {
			fprintf(stderr, "SetMinLimmitCmd 1.\n");

			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(
				stderr,
				"SetMinLimmitCmd 2. address:%d limmit:%f\n",
				managa_servo_task_store.input.address,
				managa_servo_task_store.input.limmit);

			//1. read pos
			servo_links[managa_servo_task_store.input.address].min.servo_value =
				Servo::read_position(serial, managa_servo_task_store.input.address);

			uint16_t tmp;
			Servo::limit_read(
				serial,
				managa_servo_task_store.input.address,
				tmp,
				servo_links[managa_servo_task_store.input.address].max.servo_value);
			if (!Servo::limit_write(
					serial,
					managa_servo_task_store.input.address,
					servo_links[managa_servo_task_store.input.address].min.servo_value,
					servo_links[managa_servo_task_store.input.address].max.servo_value)) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrondData;
				return;
			}

			servo_links[managa_servo_task_store.input.address].min.active = true;
			servo_links[managa_servo_task_store.input.address].min.model_value = managa_servo_task_store.input.limmit;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;

			fprintf(
				stderr,
				"SetMaxLimmitCmd 3. min.active:%d min.servo_value:%d min.model_value:%f\n",
				servo_links[managa_servo_task_store.input.address].min.active,
				servo_links[managa_servo_task_store.input.address].min.servo_value,
				servo_links[managa_servo_task_store.input.address].min.model_value);


			return;
		}
		case ManagaServoTaskNS::SetMaxLimmitCmd: {
			fprintf(stderr, "SetMaxLimmitCmd 1.\n");

			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(
				stderr,
				"SetMaxLimmitCmd 2. address:%d limmit:%f\n",
				managa_servo_task_store.input.address,
				managa_servo_task_store.input.limmit);

			//1. read pos
			servo_links[managa_servo_task_store.input.address].max.servo_value =
				Servo::read_position(serial, managa_servo_task_store.input.address);

			uint16_t tmp;
			Servo::limit_read(
				serial,
				managa_servo_task_store.input.address,
				servo_links[managa_servo_task_store.input.address].min.servo_value,
				tmp);
			if (!Servo::limit_write(
					serial,
					managa_servo_task_store.input.address,
					servo_links[managa_servo_task_store.input.address].min.servo_value,
					servo_links[managa_servo_task_store.input.address].max.servo_value)) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrondData;
				return;
			}

			servo_links[managa_servo_task_store.input.address].max.active = true;
			servo_links[managa_servo_task_store.input.address].max.model_value = managa_servo_task_store.input.limmit;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			fprintf(
				stderr,
				"SetMaxLimmitCmd 3. max.active:%d max.servo_value:%d max.model_value:%f\n",
				servo_links[managa_servo_task_store.input.address].max.active,
				servo_links[managa_servo_task_store.input.address].max.servo_value,
				servo_links[managa_servo_task_store.input.address].max.model_value);

			return;
		}
		case ManagaServoTaskNS::LoadServosCmd: {
			fprintf(stderr, "LoadServosCmd 1.\n");
			Servo::load(serial, 0xFE);
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::UnloadServosCmd: {
			fprintf(stderr, "UnloadServosCmd 1.\n");
			Servo::unload(serial, 0xFE);
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::EnableReadAngles: {
			fprintf(stderr, "EnableReadAngles 1.\n");
			managa_servo_task_store.output.state = ManagaServoTaskNS::ReadAnglesState;
			return;
		}
		case ManagaServoTaskNS::DisableReadAngles: {
			fprintf(stderr, "DisableReadAngles 1.\n");
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::MoveServo: {
			fprintf(stderr, "MoveServo 1.\n");

			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(
				stderr,
				"MoveServo 2. address:%d angle:%f\n",
				managa_servo_task_store.input.address,
				managa_servo_task_store.input.limmit);

			//check servo is calibrated
			if (!(servo_links[managa_servo_task_store.input.address].max.active &&
				servo_links[managa_servo_task_store.input.address].min.active)) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotCalibrated;
				return;
			}

			const LimmitDesc & min(servo_links[managa_servo_task_store.input.address].min);
			const LimmitDesc & max(servo_links[managa_servo_task_store.input.address].max);
			FLOAT tmp = (managa_servo_task_store.input.limmit - min.model_value) /  (max.model_value - min.model_value);

			uint16_t servo_pos = (max.servo_value - min.servo_value) * tmp + min.servo_value;
			fprintf(stderr, "MoveServo 3. servo_pos:%d\n", servo_pos);
			Servo::servo_move(serial, managa_servo_task_store.input.address, servo_pos, 0);
			return;
		}
		case ManagaServoTaskNS::MoveServoSin: {
			fprintf(stderr, "MoveServoSin 1.\n");

			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(
				stderr,
				"MoveServoSin 2. address:%d angle:%f\n",
				managa_servo_task_store.input.address,
				managa_servo_task_store.input.limmit);

			//check servo is calibrated
			if (!(servo_links[managa_servo_task_store.input.address].max.active &&
				servo_links[managa_servo_task_store.input.address].min.active)) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotCalibrated;
				return;
			}

			managa_servo_task_store.output.state = ManagaServoTaskNS::MoveSinState;
			return;
		}
	}

	//move sin
	if (managa_servo_task_store.output.state == ManagaServoTaskNS::MoveSinState) {
		const LimmitDesc & min(servo_links[managa_servo_task_store.input.address].min);
		const LimmitDesc & max(servo_links[managa_servo_task_store.input.address].max);

		FLOAT sin_value = (sin(get_time_sec() * 1) + 1.0) / 2.0;
		FLOAT model_value = (max.model_value - min.model_value) * sin_value +  min.model_value;
		FLOAT tmp = (model_value - min.model_value) /  (max.model_value - min.model_value);
		uint16_t servo_pos = (max.servo_value - min.servo_value) * tmp + min.servo_value;
		// fprintf(stderr, "move_sin sin_value:%f model_value:%f servo_pos:%d\n", sin_value, model_value, servo_pos);
		Servo::servo_move(serial, managa_servo_task_store.input.address, servo_pos, 0);
	}

	//read angles
	if (managa_servo_task_store.output.state == ManagaServoTaskNS::ReadAnglesState ||
			managa_servo_task_store.output.state == ManagaServoTaskNS::MoveSinState) {
		for (int i = 0; i < sizeof(servo_links) / sizeof(servo_links[1]); i++) {
			//check servo calibrated and 
			if (servo_links[i].active && servo_links[i].min.active && servo_links[i].max.active) {
				servo_links[i].servo_angle = Servo::read_position(serial, i);
				servo_links[i].model_angle =
					((servo_links[i].servo_angle - servo_links[i].min.servo_value) /
					FLOAT(servo_links[i].max.servo_value - servo_links[i].min.servo_value)) * (
						servo_links[i].max.model_value - servo_links[i].min.model_value) + servo_links[i].min.model_value;
				// fprintf(stderr, "servo_%d row_angle:%d model_angle:%f\n", i, servo_links[i].servo_angle, servo_links[i].model_angle); 
			} else {
				servo_links[i].servo_angle = servo_links[i].min.servo_value;
				servo_links[i].model_angle = servo_links[i].min.model_value;
			}
		}
	}
}
