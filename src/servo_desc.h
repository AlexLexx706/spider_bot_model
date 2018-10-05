#ifndef _SERVO_DESC_H_
#define _SERVO_DESC_H_

#include "common_defs.h"

//use as  value on limmit servo limmit
struct LimmitDesc {
	uint16_t servo_value;	// servo value
	FLOAT model_value;		// model angle value
};

//describe servo state
struct ServoLinkDesc {
	uint8_t active;		//servo has address
	uint8_t calibrated;	//servo colibrated
	LimmitDesc min;		//min calibration value
	LimmitDesc max;		//max calibration value
	uint16_t servo_angle;	//curent servo angle
	FLOAT model_angle;	//current model angle
};

#endif //_SERVO_DESC_H_