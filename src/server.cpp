#include "server.h"
#include "spider_bot.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

extern SpiderBot<FLOAT> bot;


Server::Server():sock(-1) {

}

Server::~Server() {
	if (sock != -1) {
		if (::close(sock)) {
			perror("bind");
		}
	}
}

bool Server::start(uint16_t port) {
	if (sock != -1) {
		fprintf(stderr, "server already started\n");
		return false;
	}
	struct sockaddr_in si_me, si_other;
	//create a UDP socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		return false;
	}

	//not blocking
	fcntl(sock, F_SETFL, O_NONBLOCK);

	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if( bind(sock , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
		perror("bind");
		return false;
	}
	return true;
}

bool Server::close() {
	if (sock == -1) {
		fprintf(stderr, "server already closed\n");
		return false;
	}

	if (::close(sock)) {
		perror("bind");
		return false;
	}

	sock = -1;
	return true;
}

bool Server::process() {
	if (sock == -1) {
		fprintf(stderr, "server not started\n");
		return false;
	}

	// 3.1 try get data
	slen = sizeof(si_other);
	if ((recv_len = recvfrom(sock, in_buffer, sizeof(in_buffer), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("recvfrom()");
			return false;
		}
	// 3.2 read data
	} else {
		//rend responce
		recv_len = cmd_handler(in_buffer, recv_len, out_buffer, sizeof(out_buffer), si_other);

		if (sendto(sock, out_buffer, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
			perror("sendto()");
			return false;
		}
	}

	//send notify to clients
	if (notify_list.size()) {
		GetStateRes<FLOAT> bot_state;
		bot.get_state(bot_state);

		// 4. send notify
		for (notify_list_t::iterator iter=notify_list.begin(), end=notify_list.end(); iter != end; iter++) {
			if (sendto(
					sock,
					&bot_state,
					sizeof(bot_state),
					0,
					(struct sockaddr *)&(*iter),
					sizeof(*iter)) == -1) {
				perror("fcntl");
				return false;
			}
		}
	}
	return true;
}



//create reply packet
static int reply(Cmd cmd, Error error, void * out_data, int max_out_size) {
	ResHeader * res(static_cast<ResHeader *>(out_data));
	res->header.cmd= cmd;
	res->header.size = sizeof(ResHeader) - sizeof(Header);
	res->error = error;
	return sizeof(ResHeader);
}


//handle command and return result
int Server::cmd_handler(const void * in_data, uint32_t in_size, void * out_data, uint32_t max_out_size, const sockaddr_in & addr) {
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
			if (in_size < sizeof(AddNotifyCmd)) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_DATA, out_data, max_out_size);
			}
			const AddNotifyCmd * notify(static_cast<const AddNotifyCmd *>(in_data));
			//add new notify
			sockaddr_in new_addr = addr;
			new_addr.sin_port = htons(notify->port);
			notify_list_t::iterator iter = notify_list.begin();

			//search old addres
			for (; iter != notify_list.end(); iter++) {
				if (iter->sin_port == new_addr.sin_port && iter->sin_addr.s_addr == new_addr.sin_addr.s_addr)
					break;
			}

			//1. new notifier
			if (iter == notify_list.end()) {
				notify_list.push_back(new_addr);
				return reply(static_cast<Cmd>(header->cmd), NO_ERROR, out_data, max_out_size);
			}

			//2. notify exist
			return reply(static_cast<Cmd>(header->cmd), WRONG_PARAMS, out_data, max_out_size);
		}
		case CMD_RM_NOTIFY: {
			if (in_size < sizeof(RmNotifyCmd)) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_DATA, out_data, max_out_size);
			}

			const RmNotifyCmd * notify(static_cast<const RmNotifyCmd *>(in_data));
			//add new notify
			sockaddr_in new_addr = addr;
			new_addr.sin_port = notify->port;
			notify_list_t::iterator iter = notify_list.begin();

			//search old addres
			for (; iter != notify_list.end(); iter++) {
				if (iter->sin_port == new_addr.sin_port && iter->sin_addr.s_addr == new_addr.sin_addr.s_addr)
					break;
			}


			//1. notifier not exist
			if (iter == notify_list.end()) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_PARAMS, out_data, max_out_size);
			}

			notify_list.erase(iter);
			return reply(static_cast<Cmd>(header->cmd), NO_ERROR, out_data, max_out_size);
		}
	}

	// 3. wrong command repry
	return reply(static_cast<Cmd>(header->cmd), WRONG_COMMAND, out_data, max_out_size);
}
