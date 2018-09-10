#ifndef _SERVER_H_
#define _SERVER_H_

class Server {
public:
	Server();
	~Server();
	bool init();
	bool close();
	void process();
};

#endif //_SERVER_H_