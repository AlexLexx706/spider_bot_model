#include "utils.h"
#include <unistd.h>

void delay_microseconds(unsigned long mks) {
	usleep(mks);
}