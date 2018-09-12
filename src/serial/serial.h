#ifndef _HARDWARE_SERIAL_H_
#define _HARDWARE_SERIAL_H_
#include <stdint.h>

class Serial {
	int fd;
	int read_res;
public:
	Serial();
	~Serial();
	bool open(const char * port_path);
	bool close();
	int write(const uint8_t * buf, int size);
	int available();
	int read();
};

#endif //_HARDWARE_SERIAL_H_