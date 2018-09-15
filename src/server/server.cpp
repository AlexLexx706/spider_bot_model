#include "server.h"
#include "spider_bot.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "cds_defs.h"

extern SpiderBot<FLOAT> bot;

//create reply packet
static int reply(Cmd cmd, Error error, void * out_data, int max_out_size) {
	ResHeader * res(static_cast<ResHeader *>(out_data));
	res->header.cmd= cmd;
	res->header.size = sizeof(ResHeader) - sizeof(Header);
	res->error = error;
	return sizeof(ResHeader);
}


Server::Server():
	sock(-1),
	post_processing_cmd(NULL) {
}

Server::~Server() {
	if (sock != -1) {
		if (::close(sock)) {
			perror("bind");
		}
	}
}

bool Server::start(uint16_t _port) {
	if (sock != -1) {
		fprintf(stderr, "server already started\n");
		return false;
	}
	port = _port;
	struct sockaddr_in si_me;
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
			//try recreate service
			if (errno == EBADF) {
				fprintf(stderr, "restart server\n");
				close();
				start(port);
			}
			return false;
		}
	// 3.2 read data
	} else {
		//rend responce
		recv_len = cmd_handler(in_buffer, recv_len, out_buffer, sizeof(out_buffer), si_other);
		
		//not send empty result
		if (recv_len != 0) {
			if (sendto(sock, out_buffer, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
				perror("sendto()");
				return false;
			}
		}
	}
	return true;
}

bool Server::post_process() {
	if (sock == -1) {
		fprintf(stderr, "server not started\n");
		return false;
	}

	//send result after manage servos 
	if (post_processing_cmd != NULL) {
		const Header * header(reinterpret_cast<const Header *>(post_processing_cmd));

		// 1. process servo manage command result
		if (header->cmd == CMD_MANAGE_SERVO) {
			Error error = (
				(managa_servo_task_store.output.state > ManagaServoTaskNS::NoneState) &&
				(managa_servo_task_store.output.state < ManagaServoTaskNS::ErrorState)) ? NO_ERROR  : UNKNOWN_ERROR;

			int reply_len = reply(
				static_cast<Cmd>(header->cmd),
				error,
				out_buffer,
				sizeof(out_buffer));
			//send reply
			if (sendto(sock, out_buffer, reply_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
				perror("sendto");
			}
		// 2 other commands
		} else {
			int reply_len = reply(
				static_cast<Cmd>(header->cmd),
				UNKNOWN_ERROR,
				out_buffer,
				sizeof(out_buffer));
			if (sendto(sock, out_buffer, reply_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
				perror("sendto");
			}
		}
		post_processing_cmd = NULL;
	}

	//send notify to clients
	if (notify_list.size()) {
		GetStateRes<FLOAT> bot_state;
		get_geometry_state(bot_state);

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

size_t Server::get_geometry_state(GetStateRes<FLOAT> & res) {
	res.header.header.cmd = CMD_GET_STATE;
	res.header.error = NO_ERROR;
	res.header.header.size = sizeof(GetStateRes<FLOAT>) - sizeof(Header);
	
	// 0. mat
	res.body_mat = bot.matrix(); 
	
	// 1. front_right_leg		
	res.front_right_leg.pos = bot.front_right_leg.get_pos();
	res.front_right_leg.shoulder_offset = bot.front_right_leg.shoulder_offset;
	res.front_right_leg.shoulder_lenght = bot.front_right_leg.shoulder_lenght;
	res.front_right_leg.forearm_lenght = bot.front_right_leg.forearm_lenght;
	res.front_right_leg.a_0 = servo_links[0].model_angle;
	res.front_right_leg.a_1 = servo_links[1].model_angle;
	res.front_right_leg.a_2 = servo_links[2].model_angle;

	// 2. rear_right_leg		
	res.rear_right_leg.pos = bot.rear_right_leg.get_pos();
	res.rear_right_leg.shoulder_offset = bot.rear_right_leg.shoulder_offset;
	res.rear_right_leg.shoulder_lenght = bot.rear_right_leg.shoulder_lenght;
	res.rear_right_leg.forearm_lenght = bot.rear_right_leg.forearm_lenght;
	res.rear_right_leg.a_0 = servo_links[3].model_angle;
	res.rear_right_leg.a_1 = servo_links[4].model_angle;
	res.rear_right_leg.a_2 = servo_links[5].model_angle;

	// 2. front_left_leg		
	res.front_left_leg.pos = bot.front_left_leg.get_pos();
	res.front_left_leg.shoulder_offset = bot.front_left_leg.shoulder_offset;
	res.front_left_leg.shoulder_lenght = bot.front_left_leg.shoulder_lenght;
	res.front_left_leg.forearm_lenght = bot.front_left_leg.forearm_lenght;
	res.front_left_leg.a_0 = servo_links[6].model_angle;
	res.front_left_leg.a_1 = servo_links[7].model_angle;
	res.front_left_leg.a_2 = servo_links[8].model_angle;


	// 4.rear_left_leg		
	res.rear_left_leg.pos = bot.rear_left_leg.get_pos();
	res.rear_left_leg.shoulder_offset = bot.rear_left_leg.shoulder_offset;
	res.rear_left_leg.shoulder_lenght = bot.rear_left_leg.shoulder_lenght;
	res.rear_left_leg.forearm_lenght = bot.rear_left_leg.forearm_lenght;
	res.rear_left_leg.a_0 = servo_links[9].model_angle;
	res.rear_left_leg.a_1 = servo_links[10].model_angle;
	res.rear_left_leg.a_2 = servo_links[11].model_angle;
	return sizeof(res); 
}

//handle command and return result
int Server::cmd_handler(const void * in_data, uint32_t in_size, void * out_data, uint32_t max_out_size, const sockaddr_in & addr) {
	assert(in_data && in_size && max_out_size);
	const Header * header(static_cast<const Header *>(in_data));
	post_processing_cmd = NULL;

	//1. check in buffer size
	if (in_size < sizeof(Header)) {
		return reply(UNKNOWN_CMD, WRONG_DATA, out_data, max_out_size);
	}
	//2. process commands
	switch (header->cmd) {
		case CMD_GET_STATE: {
			return get_geometry_state(*static_cast<GetStateRes<FLOAT> *>(out_data));
			break;
		}
		case CMD_SET_ACTION: {
			if (in_size < sizeof(SetActionCmd)) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_DATA, out_data, max_out_size);
			}

			bot.set_action(static_cast<Action>(static_cast<const SetActionCmd *>(in_data)->action));
			break;
		}
		case CMD_MANAGE_SERVO: {
			if (in_size < sizeof(ManageServoCmd)) {
				return reply(static_cast<Cmd>(header->cmd), WRONG_DATA, out_data, max_out_size);
			}

			//set data for cds
			post_processing_cmd = static_cast<const char *>(in_data);
			const ManageServoCmd * cmd_data(reinterpret_cast<const ManageServoCmd *>(post_processing_cmd));
			managa_servo_task_store.input.cmd = static_cast<ManagaServoTaskNS::Cmd>(cmd_data->cmd);
			managa_servo_task_store.input.address = cmd_data->address;
			managa_servo_task_store.input.value = cmd_data->value;
			return 0;
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
