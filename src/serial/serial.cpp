#include <assert.h>
#include <stdio.h>
#include "serial.h"
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

Serial::Serial():fd(-1), read_res(-1) {

}

bool Serial::open(const char * port_path) {
	fprintf(stderr, "open port_path:%s\n", port_path);

	if (fd != -1) {
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

bool Serial::close() {
	if (fd == -1) {
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

Serial::~Serial() {
	if (fd != -1) {
		::close(fd);
	}
}

int Serial::write(const uint8_t * buf, int size) {
	assert(fd != -1);
	int res = ::write(fd, buf, size);
	return res;
}

int Serial::available() {
	assert(fd != -1);
	if (read_res != -1) {
		return 1;
	}
	int res = ::read(fd, &read_res, 1);

	if (res == -1) {
		fprintf(stderr, "read error:%s\n", strerror(errno));
	}
	return res;
}

int Serial::read() {
	assert(fd != -1);
	int tmp = read_res;
	read_res = -1;
	return tmp;
}
