#ifndef _DEFINES_H_
#define _DEFINES_H_
#include "vmath.h"
#include <stdint.h>

enum Action { 
    NOT_MOVE = 0,
    MOVE_FORWARD = 1,
    MOVE_BACKWARD = 2,
    ROTATE_LEFT = 3,
    ROTATE_RIGHT = 4,
    FRONT_RIGHT_TEST = 5,
};

enum Cmd {
	UNKNOWN_CMD = -1,
	CMD_GET_STATE = 0,
	CMD_SET_ACTION = 1,
	CMD_ADD_NOTIFY = 2,
	CMD_RM_NOTIFY = 3,
	CMD_MANAGE_SERVO = 4

};

enum Error {
	NO_ERROR = 0,
	WRONG_COMMAND = 1,
	UNKNOWN_ERROR = 2,
	WRONG_DATA = 3,
	WRONG_PARAMS = 4,

};


struct Header {
	int32_t cmd;
	uint32_t size; 
};

struct ResHeader {
	Header header;
	int32_t error;
};

template<class T>
struct LegDesc {
	Vector3<T> pos;
	T shoulder_offset;
	T shoulder_lenght;
	T forearm_lenght;
	Vector3<T> end;
	
};

template<class T>
struct GetStateRes {
	ResHeader header;
	Matrix4<T> body_mat;
	LegDesc<T> front_right_leg;
	LegDesc<T> front_left_leg;
	LegDesc<T> rear_right_leg;
	LegDesc<T> rear_left_leg;
	int32_t action; 
};

struct SetActionCmd {
	Header headr;
	int32_t action;
};

struct AddNotifyCmd {
	Header header;
	uint16_t port;
};

struct RmNotifyCmd {
	Header header;
	uint16_t port;
};

struct ManageServoCmd {
	Header header;
	uint16_t port;
};


#endif // _DEFINES_H_