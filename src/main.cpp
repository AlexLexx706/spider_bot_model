#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include "spider_bot.h"
#include "serial.h"
#include "servo_proto.h"
#include "server.h"
#include "common_defs.h"
#include "manage_servo_task.h"
#include "utils.h"
#include "save_model_angles_task.h"
#include "tasks_utils.h"


static long period = long(1. / FREQ * 1e9L);
static int port = 8888;
static bool exit_flag = false;
static Server server;
static SaveModelAnglesTask save_model_angles_task;

const char * links_data_file_path = "links.bin";
const char * LEGS_GEOMETRY_PATH = "legs_geometry.bin";

SpiderBot<FLOAT> bot;
Serial serial;
ManagaServoTask manage_servo_task;


//exist programm
void sig_handler(int signo) {
	if (signo == SIGINT) {
		exit_flag = true;
	}
}


#include "protocol.h"
int main() {
	fprintf(stderr, "sizeof(GetServoStateCmd):%lu\n", sizeof(GetServoStateCmd));
	fprintf(stderr, "sizeof(GetServoStateRes):%lu\n", sizeof(GetServoStateRes));
	fprintf(stderr, "sizeof(LimmitDesc):%lu\n", sizeof(LimmitDesc));
	fprintf(stderr, "sizeof(ServoLinkDesc):%lu\n", sizeof(ServoLinkDesc));

	//save_legs_geometry();
	load_legs_geometry();

	if (!serial.open("/dev/ttyUSB0")) {
		return 1;
	}

	if (!manage_servo_task.init()) {
		return 1;
	}

	if (!save_model_angles_task.init()) {
		return 1;
	}

	if (!server.start(port)) {
		return 1;
	}

	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC ,&t);

	double start_time;
	int count = 0;
	double abs_dt = 0.0;

	while(not exit_flag) {
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		
		start_time = get_time_sec();

		// 0. server update
		server.process();

		// 1. step
		bot.step();

		// 2. save model angle
		save_model_angles_task.proc();

		// 3. process_tasks
		manage_servo_task.proc();

		// 4. send task results and notifys
		server.post_process();

		abs_dt += get_time_sec() - start_time;

		if (false && ++count == FREQ) {
			fprintf(
				stderr,
				"cur_duration:%f max_duration:%f load:%f%%\n",
				abs_dt / count,
				1.0 / FREQ,
				((abs_dt / count) / (1.0 / FREQ)) * 100);
			abs_dt = 0.0;
			count = 0;
		}

		// 6. update times
		t.tv_nsec += period;
		while (t.tv_nsec >= 1e9L)
		{
			t.tv_nsec -= 1e9L;
			t.tv_sec++;
		}
	}
	return 0;
} 