#include <iostream>
#include <time.h>
#include "spider_bot.h"

static SpiderBot<float> bot;
static long period = long(1./30. * 1e9L);
static GetStateRes<float> get_state_res;

int main() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC ,&t);

	while(1)
	{
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		bot.step();
		// bot.print();
		bot.get_state(get_state_res);

		t.tv_nsec += period;

		while (t.tv_nsec >= 1e9L)
		{
			t.tv_nsec -= 1e9L;
			t.tv_sec++;
		}
	}

	return 0;
}