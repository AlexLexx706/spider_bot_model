#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "cds_defs.h"
#include "manage_servo_task.h"
#include "serial.h"
#include "servo_proto.h"
#include "utils.h"
#include "tasks_utils.h"


#define MAX_LIM 1000
#define BROADCAST_ADDR 0xFE

ManagaServoTaskNS::Store managa_servo_task_store;
ServoLinkDesc servo_links[12];
extern Serial serial;
extern const char * links_data_file_path;

	
bool ManagaServoTask::init() {
	memset(&managa_servo_task_store, 0, sizeof(managa_servo_task_store));
	memset(&servo_links, 0, sizeof(servo_links));

	//read links data from file
	read_links();	
	return true;
}

bool ManagaServoTask::read_links() {
	FILE * fd = fopen(links_data_file_path, "rb");
	if (fd != NULL) {
		int res = fread(servo_links, 1, sizeof(servo_links), fd);
	
		if (res != sizeof(servo_links)) {
			fprintf(stderr, "wrong links file res: %d not equal:%zu\n", res, sizeof(servo_links));
			memset(&servo_links, 0, sizeof(servo_links));
			fclose(fd);
			return false;
		}

		for (int i = 0; i < sizeof(servo_links) / sizeof(servo_links[1]); i++) {
			if (servo_links[i].calibrated) {
				uint16_t min = servo_links[i].min.servo_value > servo_links[i].max.servo_value ? servo_links[i].max.servo_value : servo_links[i].min.servo_value;
				uint16_t max = servo_links[i].min.servo_value > servo_links[i].max.servo_value ? servo_links[i].min.servo_value : servo_links[i].max.servo_value;

				if (!Servo::limit_write(
						serial,
						i,
						min,
						max)) {
				}
			}
		}
		fclose(fd);
		return true;
	}
	fprintf(stderr, "links data file:%s not exist\n", links_data_file_path);
	return false; 
}

bool ManagaServoTask::save_links() {
	FILE * fd = fopen(links_data_file_path, "wb");

	if (fd) {
		int res = fwrite(servo_links, 1, sizeof(servo_links), fd);
		if (res != sizeof(servo_links)) {
			fprintf(stderr, "wrong links file\n");
		}
		fclose(fd);
		return true;
	}
	fprintf(stderr, "cannot save links data for file:%s\n", links_data_file_path);
	return false;
}


static uint16_t model_angle_to_servo_value(uint8_t address, FLOAT model_angle) {
	assert(servo_links[address].active && servo_links[address].calibrated);
	const LimmitDesc & min(servo_links[address].min);
	const LimmitDesc & max(servo_links[address].max);

	FLOAT tmp = (model_angle - min.model_value) /  (max.model_value - min.model_value);
	uint16_t servo_value = (max.servo_value - min.servo_value) * tmp + min.servo_value;

	// fprintf(
	// 	stderr,
	// 	"model_angle_to_servo_value address:%d model_angle:%f servo_value:%d"
	// 	"min.model_value:%f min.servo_value:%d max.model_value:%f max.servo_value:%d\n",
	// 	address, model_angle, servo_value,
	// 	min.model_value, min.servo_value,
	// 	max.model_value, max.servo_value);
	return servo_value;
}


void ManagaServoTask::proc() {
	ManagaServoTaskNS::Cmd cmd = managa_servo_task_store.input.cmd;
	managa_servo_task_store.input.cmd = ManagaServoTaskNS::NoneCmd;

	// if (managa_servo_task_store.input.cmd != 0) {
	if (cmd) {
		fprintf(stderr, "proc 1. cmd:%d\n", cmd);
	}

	switch (cmd) { 
		case ManagaServoTaskNS::ResetAddressesCmd: {
			fprintf(stderr, "ResetAddressesCmd 1.\n");
			// reset all address
			Servo::set_id(serial, BROADCAST_ADDR, 1);

			// unload all servos
			Servo::unload(serial, BROADCAST_ADDR);

			// reset links
			memset(&servo_links, 0, sizeof(servo_links));
			save_links();
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::SetAddressCmd: {
			fprintf(stderr, "SetAddressCmd 1.\n");

			if (managa_servo_task_store.input.address >= SERVOS_COUNT) {
				fprintf(stderr, "SetAddressCmd 2. error: wrong addres:%d\n", managa_servo_task_store.input.address);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}

			fprintf(stderr, "SetAddressCmd 3. address:%d\n", managa_servo_task_store.input.address);
			Servo::set_id(serial, BROADCAST_ADDR, managa_servo_task_store.input.address);
			Servo::unload(serial, managa_servo_task_store.input.address);
			servo_links[managa_servo_task_store.input.address].active = true;
			servo_links[managa_servo_task_store.input.address].calibrated = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			save_links();
			return;
		}
		case ManagaServoTaskNS::ResetLimmits: {
			fprintf(stderr, "ResetLimmits 1.\n");
			if (managa_servo_task_store.input.address >= SERVOS_COUNT) {
				fprintf(stderr, "ResetLimmits 2. error: wrong addres:%d\n", managa_servo_task_store.input.address);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}

			if (!servo_links[managa_servo_task_store.input.address].active) {
				fprintf(stderr, "ResetLimmits 3. error: address:%d not set\n", managa_servo_task_store.input.address);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotActive;
				return;
			}

			fprintf(stderr, "ResetLimmits 2. address:%d\n", managa_servo_task_store.input.address);
			//unload servo
			Servo::unload(serial, managa_servo_task_store.input.address);
			servo_links[managa_servo_task_store.input.address].calibrated = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			save_links();
			return;
		}
		case ManagaServoTaskNS::StartCalibration: {
			fprintf(stderr, "StartCalibration 1.\n");

			if (managa_servo_task_store.input.address >= SERVOS_COUNT) {
				fprintf(stderr, "StartCalibration 2. error: wrong address:%d\n", managa_servo_task_store.input.address);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(stderr, "StartCalibration 2.\n");


			if (!servo_links[managa_servo_task_store.input.address].active) {
				fprintf(stderr, "StartCalibration 3. error: address:%d not set.\n", managa_servo_task_store.input.address);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotActive;
				return;
			}

			//1. read pos
			start_calibration_data.servo_value = Servo::read_position(serial, managa_servo_task_store.input.address);

			if (start_calibration_data.servo_value > MAX_LIM) {
				fprintf(
					stderr,
					"StartCalibration 4. error: address:%d, servo value:%d not in calibration range: 0 - %d\n",
					managa_servo_task_store.input.address,
					start_calibration_data.servo_value, MAX_LIM);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongServoPos;
				return;
			} 

			start_calibration_data.model_value = managa_servo_task_store.input.value; 
			servo_links[managa_servo_task_store.input.address].calibrated = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CalibrationProgressState;
			fprintf(
				stderr,
				"StartCalibration 5. address:%d model_value:%f servo_value:%d\n",
				managa_servo_task_store.input.address,
				start_calibration_data.model_value,
				start_calibration_data.servo_value);
			save_links();
			return;
		}
		case ManagaServoTaskNS::CompliteCalibration: {
			fprintf(stderr, "CompliteCalibration 1.\n");
			if (managa_servo_task_store.output.state != ManagaServoTaskNS::CalibrationProgressState) {
				fprintf(stderr, "CompliteCalibration 2. error: processor not in callibration state\n");
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotCalibrated;
				return;
			}

			fprintf(stderr, "CompliteCalibration 3.\n");
			if (managa_servo_task_store.input.address >= SERVOS_COUNT) {
				fprintf(stderr, "CompliteCalibration 4. error: address:%d not set.\n", managa_servo_task_store.input.address);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}

			fprintf(stderr, "CompliteCalibration 4.\n");
			if (!servo_links[managa_servo_task_store.input.address].active) {
				fprintf(stderr, "CompliteCalibration 5. error: address:%d not set.\n", managa_servo_task_store.input.address);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotActive;
				return;
			}

			fprintf(
				stderr,
				"CompliteCalibration 4. address:%d value:%f\n",
				managa_servo_task_store.input.address,
				managa_servo_task_store.input.value);

			//1. read pos
			uint16_t servo_value = Servo::read_position(serial, managa_servo_task_store.input.address);

			if (servo_value > MAX_LIM) {
				fprintf(
					stderr,
					"CompliteCalibration 4. error: address:%d, servo value:%d not in calibration range: 0 - %d\n",
					managa_servo_task_store.input.address,
					start_calibration_data.servo_value, MAX_LIM);
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongServoPos;
				return;
			}

			uint16_t min = servo_value > start_calibration_data.servo_value ? start_calibration_data.servo_value : servo_value;
			uint16_t max = servo_value > start_calibration_data.servo_value ? servo_value : start_calibration_data.servo_value;

			if (!Servo::limit_write(
					serial,
					managa_servo_task_store.input.address,
					min,
					max)) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrondData;
				return;
			}

			servo_links[managa_servo_task_store.input.address].calibrated = true;
			servo_links[managa_servo_task_store.input.address].min.servo_value = start_calibration_data.servo_value;
			servo_links[managa_servo_task_store.input.address].min.model_value = start_calibration_data.model_value;

			servo_links[managa_servo_task_store.input.address].max.servo_value = servo_value;
			servo_links[managa_servo_task_store.input.address].max.model_value = managa_servo_task_store.input.value;
			servo_links[managa_servo_task_store.input.address].max.model_value = managa_servo_task_store.input.value;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;

			fprintf(
				stderr,
				"CompliteCalibration 4. min.servo_value:%d min.model_value:%f max.servo_value:%d max.model_value:%f\n",
				servo_links[managa_servo_task_store.input.address].min.servo_value,
				servo_links[managa_servo_task_store.input.address].min.model_value,
				servo_links[managa_servo_task_store.input.address].max.servo_value,
				servo_links[managa_servo_task_store.input.address].max.model_value);
			save_links();
			return;
		}
		case ManagaServoTaskNS::LoadServosCmd: {
			fprintf(stderr, "LoadServosCmd 1.\n");
			Servo::load(serial, BROADCAST_ADDR);
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::UnloadServosCmd: {
			fprintf(stderr, "UnloadServosCmd 1.\n");
			Servo::unload(serial, BROADCAST_ADDR);
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::EnableReadAngles: {
			fprintf(stderr, "EnableReadAngles 1. addr:%d\n", managa_servo_task_store.input.address);

			//check address
			if (managa_servo_task_store.input.address <  SERVOS_COUNT ||
					managa_servo_task_store.input.address == BROADCAST_ADDR) {
				read_angle_addres = managa_servo_task_store.input.address; 
				managa_servo_task_store.output.state = ManagaServoTaskNS::ReadAnglesState;
				return;
			}
			managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
			return;
		}
		case ManagaServoTaskNS::DisableReadAngles: {
			fprintf(stderr, "DisableReadAngles 1.\n");
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::MoveServo: {
			fprintf(stderr, "MoveServo 1.\n");

			if (managa_servo_task_store.input.address >= BROADCAST_ADDR) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(
				stderr,
				"MoveServo 2. address:%d angle:%f\n",
				managa_servo_task_store.input.address,
				managa_servo_task_store.input.value);

			//check servo is calibrated
			if (!servo_links[managa_servo_task_store.input.address].calibrated) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotCalibrated;
				return;
			}

			//move servo
			Servo::servo_move(
				serial,
				managa_servo_task_store.input.address,
				model_angle_to_servo_value(
					managa_servo_task_store.input.address,
					managa_servo_task_store.input.value),
				0);
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			return;
		}
		case ManagaServoTaskNS::MoveServoSin: {
			fprintf(stderr, "MoveServoSin 1.\n");

			if (managa_servo_task_store.input.address >= SERVOS_COUNT) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(
				stderr,
				"MoveServoSin 2. address:%d period:%f\n",
				managa_servo_task_store.input.address,
				managa_servo_task_store.input.value);

			//check servo is calibrated
			if (!servo_links[managa_servo_task_store.input.address].calibrated) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotCalibrated;
				return;
			}

			sin_period = managa_servo_task_store.input.value; 
			managa_servo_task_store.output.state = ManagaServoTaskNS::MoveSinState;
			return;
		}
		case ManagaServoTaskNS::EnableSteringCmd: {
			fprintf(stderr, "EnableSterring 1.\n");
			managa_servo_task_store.output.state = ManagaServoTaskNS::SterringsProgress;
			return;
		}
	}

	//1. sterrings in progress
	if (managa_servo_task_store.output.state == ManagaServoTaskNS::SterringsProgress) {
		for (int servo_address = 0; servo_address < sizeof(servo_links) / sizeof(servo_links[1]); servo_address++) {
			if (servo_links[servo_address].active && servo_links[servo_address].calibrated) {
				Servo::servo_move(
					serial,
					servo_address,
					model_angle_to_servo_value(servo_address, get_model_angle(servo_address)),
					(1.0 / FREQ * 1000));
			}
		}
	//2. sin test in progress
	} else if (managa_servo_task_store.output.state == ManagaServoTaskNS::MoveSinState) {
		const LimmitDesc & min(servo_links[managa_servo_task_store.input.address].min);
		const LimmitDesc & max(servo_links[managa_servo_task_store.input.address].max);

		FLOAT sin_value = (sin(get_time_sec() * sin_period) + 1.0) / 2.0;
		FLOAT model_value = (max.model_value - min.model_value) * sin_value +  min.model_value;
		FLOAT tmp = (model_value - min.model_value) /  (max.model_value - min.model_value);
		uint16_t servo_pos = (max.servo_value - min.servo_value) * tmp + min.servo_value;
		// fprintf(stderr, "move_sin sin_value:%f model_value:%f servo_pos:%d\n", sin_value, model_value, servo_pos);
		Servo::servo_move(serial, managa_servo_task_store.input.address, servo_pos, (1.0 / FREQ * 1000));
	//3. read angles in progress
	} else if (managa_servo_task_store.output.state == ManagaServoTaskNS::ReadAnglesState) {
		for (int address = 0; address < SERVOS_COUNT; address++) {
			//check servo calibrated and
			// fprintf(stderr, "addr:%d active:%d\n", address, servo_links[address].active);
			if (servo_links[address].active &&
					(address == read_angle_addres || read_angle_addres == BROADCAST_ADDR)) {
				servo_links[address].servo_angle = Servo::read_position(serial, address);

				if (servo_links[address].calibrated) {
					servo_links[address].model_angle =
						((servo_links[address].servo_angle - servo_links[address].min.servo_value) /
						FLOAT(servo_links[address].max.servo_value - servo_links[address].min.servo_value)) * (
							servo_links[address].max.model_value - servo_links[address].min.model_value) + servo_links[address].min.model_value;
				}
				// fprintf(
				// 	stderr,
				// 	"servo_%d calibrated:%d row_angle:%d model_angle:%f\n",
				// 	address,
				// 	servo_links[address].calibrated,
				// 	servo_links[address].servo_angle,
				// 	servo_links[address].model_angle); 
			}
		}
	}
}
