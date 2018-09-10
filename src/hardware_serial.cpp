#include <assert.h>
#include <stdio.h>
#include "hardware_serial.h"
// #include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

HardwareSerial::HardwareSerial():fd(-1), read_res(-1) {

}

bool HardwareSerial::open(const char * port_path) {
	if (fd != 0) {
		fprintf(stderr, "port already open!!\n");
		return false;		
	}

	assert(port_path);
	fd = ::open(port_path, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd == -1) {
		perror("open");
		return false;
	}

	//work with settings
	struct termios settings;
	::memset(&settings, 0, sizeof(settings));

	//set speed
	::cfsetispeed(&settings, B115200);
	::cfsetospeed(&settings, B115200);

	//setting
	settings.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);//make 8n1, no flow control
	settings.c_cflag |= (CLOCAL | CREAD | CS8);//ignore modem controls
	settings.c_lflag = 0;
	settings.c_oflag = 0;
	settings.c_iflag = 0;

	settings.c_cc[VMIN] = 0; 	// have that minimal number of bytes in buffer to exit read
	settings.c_cc[VTIME] = 1; 	// wait number of 0.1 before exit read

	if (::tcsetattr(fd, TCSANOW, &settings) < 0) {
		perror("tcsetattr");
		::close(fd);
		fd = -1;
		return false;
	}
	return true;
}

bool HardwareSerial::close() {
	if (fd == -1){
		fprintf(stderr, "port not open\n");
		return false;
	}
	if (::close(fd) == -1) {
		perror("close");
		return false;
	}
	fd = -1;
	return true;
}

HardwareSerial::~HardwareSerial() {
	if (fd != -1) {
		::close(fd);
	}
}

int HardwareSerial::write(const byte * buf, int size) {
	assert(fd == -1);
	return ::write(fd, buf, size);
}

int HardwareSerial::available() {
	assert(fd == -1);
	read_res = -1;
	return ::read(fd, &read_res, 1);
}

int HardwareSerial::read() {
	assert(fd == -1);
	return read_res;
}
