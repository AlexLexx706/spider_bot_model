#ifndef _SERVER_H_
#define _SERVER_H_

#include "common_defs.h"
#include "defines.h"
#include <netinet/in.h>
#include <list>
#include <stdint.h>

class Server {
	int sock;
	struct sockaddr_in si_me;
	struct sockaddr_in si_other;
	int recv_len;
	socklen_t slen;
	char in_buffer[BUFLEN];
	char out_buffer[BUFLEN];

	//used for notify clients
	typedef std::list<sockaddr_in> notify_list_t;
	notify_list_t notify_list;
	const char * post_processing_cmd;

	int cmd_handler(
		const void * in_data, uint32_t in_size,
		void * out_data, uint32_t max_out_size,
		const sockaddr_in & addr);

public:
	Server();
	~Server();
	bool start(uint16_t port);
	bool close();
	bool process();
	bool post_process();
};

#endif //_SERVER_H_