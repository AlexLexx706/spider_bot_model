#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>

#include "cds_defs.h"
#include "manage_servo_task.h"
#include "serial.h"
#include "servo_proto.h"
#include "utils.h"
 
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

void ManagaServoTask::read_links() {
	FILE * fd = fopen(links_data_file_path, "rb");
	if (fd != NULL) {
		int res = fread(servo_links, 1, sizeof(servo_links), fd);
		if (res != sizeof(servo_links)) {
			fprintf(stderr, "wrong links file res: %d not equal:%d\n", res, sizeof(servo_links));
			memset(&servo_links, 0, sizeof(servo_links));
		} else {
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
		}
		fclose(fd);
	} else {
		fprintf(stderr, "read_links 5. links_data_file:%s not exist\n", links_data_file_path);
	}

}
void ManagaServoTask::save_links() {
	FILE * fd = fopen(links_data_file_path, "wb");

	if (fd) {
		int res = fwrite(servo_links, 1, sizeof(servo_links), fd);
		if (res != sizeof(servo_links)) {
			fprintf(stderr, "wrong links file\n");
		}
		fclose(fd);
	}
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
			save_links();
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
			servo_links[managa_servo_task_store.input.address].calibrated = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CompleteState;
			save_links();
			return;
		}
		case ManagaServoTaskNS::ResetLimmits: {
			fprintf(stderr, "ResetLimmits 1.\n");
			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}

			if (!servo_links[managa_servo_task_store.input.address].active) {
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

			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}
			fprintf(stderr, "StartCalibration 2.\n");


			if (!servo_links[managa_servo_task_store.input.address].active) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotActive;
				return;
			}

			//1. read pos
			start_calibration_data.servo_value = Servo::read_position(serial, managa_servo_task_store.input.address);
			start_calibration_data.model_value = managa_servo_task_store.input.value; 
			servo_links[managa_servo_task_store.input.address].calibrated = false;
			managa_servo_task_store.output.state = ManagaServoTaskNS::CalibrationProgressState;
			fprintf(
				stderr,
				"StartCalibration 2. address:%d model_value:%f servo_value:%d\n",
				managa_servo_task_store.input.address,
				start_calibration_data.model_value,
				start_calibration_data.servo_value);
			save_links();
			return;
		}
		case ManagaServoTaskNS::CompliteCalibration: {
			fprintf(stderr, "CompliteCalibration 1.\n");
			if (managa_servo_task_store.output.state != ManagaServoTaskNS::CalibrationProgressState) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotCalibrated;
				return;
			}

			fprintf(stderr, "CompliteCalibration 2.\n");

			if (managa_servo_task_store.input.address < 0 || managa_servo_task_store.input.address >= sizeof(servo_links) / sizeof(servo_links[0])) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorWrongAddress;
				return;
			}

			fprintf(stderr, "CompliteCalibration 3.\n");

			if (!servo_links[managa_servo_task_store.input.address].active) {
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
				managa_servo_task_store.input.value);

			//check servo is calibrated
			if (!servo_links[managa_servo_task_store.input.address].calibrated) {
				managa_servo_task_store.output.state = ManagaServoTaskNS::ErrorNotCalibrated;
				return;
			}

			const LimmitDesc & min(servo_links[managa_servo_task_store.input.address].min);
			const LimmitDesc & max(servo_links[managa_servo_task_store.input.address].max);
			FLOAT tmp = (managa_servo_task_store.input.value - min.model_value) /  (max.model_value - min.model_value);

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
	}

	//move sin
	if (managa_servo_task_store.output.state == ManagaServoTaskNS::MoveSinState) {
		const LimmitDesc & min(servo_links[managa_servo_task_store.input.address].min);
		const LimmitDesc & max(servo_links[managa_servo_task_store.input.address].max);

		FLOAT sin_value = (sin(get_time_sec() * sin_period) + 1.0) / 2.0;
		FLOAT model_value = (max.model_value - min.model_value) * sin_value +  min.model_value;
		FLOAT tmp = (model_value - min.model_value) /  (max.model_value - min.model_value);
		uint16_t servo_pos = (max.servo_value - min.servo_value) * tmp + min.servo_value;
		// fprintf(stderr, "move_sin sin_value:%f model_value:%f servo_pos:%d\n", sin_value, model_value, servo_pos);
		Servo::servo_move(serial, managa_servo_task_store.input.address, servo_pos, (1.0 / FREQ * 1000));
	}

	//read angles
	if (managa_servo_task_store.output.state == ManagaServoTaskNS::ReadAnglesState) {
		for (int i = 0; i < sizeof(servo_links) / sizeof(servo_links[1]); i++) {
			//check servo calibrated and 
			if (servo_links[i].active) {
				servo_links[i].servo_angle = Servo::read_position(serial, i);

				if (servo_links[i].calibrated) {
					servo_links[i].model_angle =
						((servo_links[i].servo_angle - servo_links[i].min.servo_value) /
						FLOAT(servo_links[i].max.servo_value - servo_links[i].min.servo_value)) * (
							servo_links[i].max.model_value - servo_links[i].min.model_value) + servo_links[i].min.model_value;
				}
				fprintf(
					stderr,
					"servo_%d calibrated:%d row_angle:%d model_angle:%f\n",
					i,
					servo_links[i].calibrated,
					servo_links[i].servo_angle,
					servo_links[i].model_angle); 
			}
		}
	}
}
