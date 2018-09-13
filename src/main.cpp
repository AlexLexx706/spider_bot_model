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

static long period = long(1. / FREQ * 1e9L);
static int port = 8888;
static bool exit_flag = false;
static Server server;

SpiderBot<FLOAT> bot;
Serial serial;
ManagaServoTask manage_servo_task;


//exist programm
void sig_handler(int signo) {
	if (signo == SIGINT) {
		exit_flag = true;
	}
}


int main() {
	if (!serial.open("/dev/ttyUSB0")) {
		return 1;
	}
	if (!manage_servo_task.init()) {
		return 1;
	}


	// int16_t res = Servo::read_position(serial, 1);
	// std::cerr << "servo pos:" << res << std::endl;
	// return 1;
	// uint16_t min, max;
	// res = Servo::limit_write(serial, 1, 20, 500);
	// res = Servo::limit_read(serial, 1, min, max);
	// // Servo::servo_move(serial, 1, 10, 1000);
	// fprintf(stderr, "limit_read res:%hu min:%hu max:%hu\n", res, min, max);
	// return 1;
	// res = LobotSerialServoReadVin(hs, 1);
	// fprintf(stderr, "servo vin:%hd\n", res);

	// LobotSerialServoMove(hs, 1, 1000, 1000);

	// res = LobotSerialServoReadPosition(hs, 1);
	// fprintf(stderr, "servo pos:%hd\n", res);
	// return 0;

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
		// 1. step
		bot.step();

		// 2. server update
		server.process();

		// 3. process_tasks
		manage_servo_task.proc();
		
		// 3. send task results and notifys
		server.post_process();
		abs_dt += get_time_sec() - start_time;

		if (++count == FREQ) {
			fprintf(
				stderr,
				"cur_duration:%f max_duration:%f load:%f%%\n",
				abs_dt / count,
				1.0 / FREQ,
				((abs_dt / count) / (1.0 / FREQ)) * 100);
			abs_dt = 0.0;
			count = 0;
		}

		// 3. update times
		t.tv_nsec += period;
		while (t.tv_nsec >= 1e9L)
		{
			t.tv_nsec -= 1e9L;
			t.tv_sec++;
		}
	}
	return 0;
} 