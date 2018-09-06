#include <iostream>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "spider_bot.h"

#define BUFLEN 1024

static SpiderBot<float> bot;
static long period = long(1./30. * 1e9L);
static GetStateRes<float> get_state_res;
static int PORT = 8888;
static char buf[BUFLEN];


void die(const char *sock) {
	perror(sock);
	exit(1);
}



int main() {
	struct sockaddr_in si_me, si_other;
	int sock, recv_len;
	socklen_t slen = sizeof(si_other);

	//create a UDP socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die("socket");
	}

	//not blocking
	fcntl(sock, F_SETFL, O_NONBLOCK);

	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	 
	//bind socket to port
	if( bind(sock , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
		die("bind");
	}
	 
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC ,&t);

	while(1) {
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		
		// 1. step
		bot.step();
		// 2. get stte
		bot.get_state(get_state_res);

		// 3. try get data
		if ((recv_len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				die("recvfrom()");
			}
		//read data
		} else {
			//now reply the client with the same data
			if (sendto(sock, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
				die("sendto()");
			}
		}

		t.tv_nsec += period;

		while (t.tv_nsec >= 1e9L)
		{
			t.tv_nsec -= 1e9L;
			t.tv_sec++;
		}
	}

	close(sock);
	return 0;
}