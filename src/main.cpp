#include <iostream>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "spider_bot.h"
#include <map>

#define BUFLEN 1024L
#define FLOAT float

static SpiderBot<FLOAT> bot;
static long period = long(1./30. * 1e9L);
static GetStateRes<FLOAT> get_state_res;
static int PORT = 8888;
static char in_buffer[BUFLEN];
static char out_buffer[BUFLEN];

//describe notifier
struct Notifier {
	int sock;
};

//used for std::map
bool operator <(const sockaddr_in & a, const sockaddr_in & b) {
	unsigned long long r_a = a.sin_port + a.sin_addr.s_addr;
	unsigned long long r_b = b.sin_port + b.sin_addr.s_addr;
	return r_a < r_b;
}

typedef std::map<sockaddr_in, Notifier> notify_map_t; 
static notify_map_t notify_map;

//todo!!!!
void die(const char *sock) {
	perror(sock);
	exit(1);
}

bool init_notify(Notifier & notifier) {
	if ( (notifier.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return false;
	}

	if (fcntl(notifier.sock, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl");
		close(notifier.sock);
		return false;
	}
	return true;
}

bool deinit_notify(Notifier & notifier) {
	close(notifier.sock);
	return true;
}

//create reply packet
int reply(Cmd cmd, Error error, void * out_data, int max_out_size) {
	ResHeader * res(static_cast<ResHeader *>(out_data));
	res->header.cmd= cmd;
	res->header.size = sizeof(ResHeader) - sizeof(Header);
	res->error = error;
	return sizeof(ResHeader);
}


//handle command and return result
int cmd_handler(const void * in_data, unsigned long in_size, void * out_data, unsigned long max_out_size, const sockaddr_in & addr) {
	assert(in_data && in_size && max_out_size);
	const Header * header(static_cast<const Header *>(in_data));

	//1. check in buffer size
	if (in_size < sizeof(Header)) {
		return reply(UNKNOWN_CMD, WRONG_DATA, out_data, max_out_size);
	}

	//2. process commands
	switch (header->cmd) {
		case CMD_GET_STATE: {
			bot.get_state(*static_cast<GetStateRes<FLOAT> *>(out_data));
			return sizeof(GetStateRes<FLOAT>);
			break;
		}
		case CMD_SET_ACTION: {
			if (in_size < sizeof(SetActionCmd)) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_DATA, out_data, max_out_size);
			}

			bot.set_action(static_cast<Action>(static_cast<const SetActionCmd *>(in_data)->action)); 
			break;
		}
		case CMD_ADD_NOTIFY: {
			if (in_size < sizeof(AddNotify)) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_DATA, out_data, max_out_size);
			}

			const AddNotify * notify(static_cast<const AddNotify *>(in_data));
			//add new notify
			sockaddr_in new_addr = addr;
			new_addr.sin_port = notify->port;
			notify_map_t::iterator iter = notify_map.find(new_addr);

			//1. new notifier
			if (iter == notify_map.end()) {
				notify_map_t::mapped_type & notifier = notify_map[addr];

				if (init_notify(notifier)) {
					return reply(static_cast<Cmd>(header->cmd), NO_ERROR, out_data, max_out_size);
				} else {
					notify_map.erase(addr);
					return reply(static_cast<Cmd>(header->cmd), UNKNOWN_ERROR, out_data, max_out_size);					
				}
			}

			//2. notify exist
			return reply(static_cast<Cmd>(header->cmd), WRONG_PARAMS, out_data, max_out_size);
		}
		case CMD_RM_NOTIFY: {
			if (in_size < sizeof(RmNotify)) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_DATA, out_data, max_out_size);
			}

			const RmNotify * notify(static_cast<const RmNotify *>(in_data));
			//add new notify
			sockaddr_in new_addr = addr;
			new_addr.sin_port = notify->port;
			notify_map_t::iterator iter = notify_map.find(new_addr);

			//1. notifier not exist
			if (iter == notify_map.end()) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_PARAMS, out_data, max_out_size);
			}

			// 2. remove socket
			deinit_notify(iter->second);
			notify_map.erase(addr);
		}
	}

	// 3. wrong command repry
	return reply(static_cast<Cmd>(header->cmd), WRONG_COMMAND, out_data, max_out_size);
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

		// 3.1 try get data
		if ((recv_len = recvfrom(sock, in_buffer, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				perror("recvfrom()");
			}
		// 3.2 read data
		} else {
			//rend responce
			recv_len = cmd_handler(in_buffer, recv_len, out_buffer, BUFLEN, si_other);

			if (sendto(sock, out_buffer, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
				perror("sendto()");
			}
		}


		static GetStateRes<FLOAT> bot_state;
		bot.get_state(bot_state);

		// 4. send notify
		for (notify_map_t::iterator iter=notify_map.begin(), end=notify_map.end(); iter != end; iter++) {
			if (sendto(iter->second.sock, &bot_state, sizeof(bot_state), 0, (struct sockaddr *)&iter->first, sizeof(iter->first)) == -1) {
				perror("fcntl");
			}
		}
		std::const << "1" << std::endl;
		// 4. update times
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