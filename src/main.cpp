#include <errno.h>
#include <assert.h>
#include <signal.h>
#include "spider_bot.h"
#include "hardware_serial.h"
#include "serial_servo_rp.h"
#include "server.h"
#include "common_defs.h"

static long period = long(1./30. * 1e9L);
static int port = 8888;
static bool exit_flag = false;
SpiderBot<FLOAT> bot;
static Server server;

//exist programm
void sig_handler(int signo) {
	if (signo == SIGINT) {
		exit_flag = true;
	}
}


int main() {
	// HardwareSerial hs;
	// if (!hs.open("/dev/ttyUSB0")) {
	// 	return 1;
	// }

	// int16_t res = LobotSerialServoReadPosition(hs, 1);
	// fprintf(stderr, "servo pos:%hd\n", res);

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

	while(not exit_flag) {
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		
		// 1. step
		bot.step();

		// 2. server update
		server.process();

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