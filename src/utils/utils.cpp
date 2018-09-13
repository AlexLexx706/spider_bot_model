#include "utils.h"
#include <unistd.h>
#include <sys/time.h>

void delay_microseconds(unsigned long mks) {
	usleep(mks);
}


double get_time_sec() {
	struct timeval te; 
	gettimeofday(&te, NULL);
	return te.tv_sec + te.tv_usec/1e6;
}