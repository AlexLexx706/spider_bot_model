#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <stdint.h>
#include "vmath.h"
#include "common_defs.h"
#include "geometry_desc.h"
#include "servo_desc.h"

enum LegNum {
	FRONT_RIGHT_LEG_NUM = 0,
	REAR_RIGHT_LEG_NUM = 1,
	FRONT_LEFT_LEG_NUM = 2,
	REAR_LEFT_LEG_NUM = 3,
};

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
	CMD_MANAGE_SERVO = 4,
	CMD_SET_LEG_GEOMETRY = 5,
	CMD_GET_SERVO_STATE = 6
};

enum Error {
	NO_ERROR = 0,
	WRONG_COMMAND = 1,
	UNKNOWN_ERROR = 2,
	WRONG_DATA = 3,
	WRONG_PARAMS = 4,
	WRONG_SERVO_ID = 5,
};


struct __attribute__((__packed__)) Header {
	int32_t cmd;
    int32_t resp_flag;
	uint32_t size; 
};

struct __attribute__((__packed__)) ResHeader {
	Header header;
	int32_t error;
};


template<class T>
struct GetStateRes {
	ResHeader header;
	Matrix4<T> body_mat;
	LegDesc<T> front_right_leg;
	LegDesc<T> front_left_leg;
	LegDesc<T> rear_right_leg;
	LegDesc<T> rear_left_leg;
};


struct __attribute__((__packed__)) SetActionCmd {
	Header headr;
	int32_t action;
};

struct __attribute__((__packed__)) AddNotifyCmd {
	Header header;
	uint16_t port;
};

struct __attribute__((__packed__)) RmNotifyCmd {
	Header header;
	uint16_t port;
};

struct __attribute__((__packed__)) ManageServoCmd {
	Header header;
	uint8_t cmd;
	uint8_t address;
	FLOAT value;
};

struct __attribute__((__packed__)) ManageServoRes: public ResHeader {
	int32_t state;
};

struct  SetLegGeometry {
	Header header;
	uint32_t leg_num;
	LegGeometry<FLOAT> geometry;
};

//used for request servo state
struct __attribute__((__packed__)) GetServoStateCmd {
	Header header;
	uint8_t servo_id;
};

//return servo state result
struct __attribute__((__packed__)) GetServoStateRes {
	ResHeader header;
	uint8_t servo_id;
	ServoLinkDesc desc;
};


#endif // _DEFINES_H_