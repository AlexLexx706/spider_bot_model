#include <iostream>
#include <time.h>
#include "spider_bot.h"

static SpiderBot<double> bot;
static long period = long(1./30. * 1e9L);

int main() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC ,&t);

	while(1)
	{
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		bot.step();
		bot.print();

		t.tv_nsec += period;

		while (t.tv_nsec >= 1e9L)
		{
			t.tv_nsec -= 1e9L;
			t.tv_sec++;
		}
	}

	return 0;
}