#include "spider_bot.h"
#include "tasks_utils.h"
#include <stdio.h>
#include <assert.h>

extern SpiderBot<FLOAT> bot;

FLOAT get_model_angle(uint8_t address) {
	switch (address) {
		case 0:
			return bot.front_right_leg.p_0.get_angle_y();
		case 1:
			return bot.front_right_leg.p_1.get_angle_x();
		case 2:
			return bot.front_right_leg.p_2.get_angle_x();

		case 3:
			return bot.rear_right_leg.p_0.get_angle_y();
		case 4:
			return bot.rear_right_leg.p_1.get_angle_x();
		case 5:
			return bot.rear_right_leg.p_2.get_angle_x();

		case 6:
			return bot.front_left_leg.p_0.get_angle_y();
		case 7:
			return bot.front_left_leg.p_1.get_angle_x();
		case 8:
			return bot.front_left_leg.p_2.get_angle_x();

		case 9:
			return bot.rear_left_leg.p_0.get_angle_y();
		case 10:
			return bot.rear_left_leg.p_1.get_angle_x();
		case 11:
			return bot.rear_left_leg.p_2.get_angle_x();
	}
	return 0.;
}

Leg<FLOAT> * get_len_by_num(LegNum num) {
	switch (num) {
		case FRONT_RIGHT_LEG_NUM: {
			return &bot.front_right_leg;
		}
		case REAR_RIGHT_LEG_NUM: {
			return &bot.rear_right_leg;
		}
		case FRONT_LEFT_LEG_NUM: {
			return &bot.front_left_leg;
		}
		case REAR_LEFT_LEG_NUM: {
			return &bot.rear_left_leg;
		}
	}
	return NULL;	
}


bool set_leg_geometry(LegNum num, const LegGeometry<FLOAT> & geometry) {
	Leg<FLOAT> * leg = get_len_by_num(static_cast<LegNum>(num));
	if (leg == NULL) {
		return false;
	}

	leg->set_pos(geometry.pos);
	leg->set_shoulder_offset(geometry.shoulder_offset);
	leg->set_shoulder_lenght(geometry.shoulder_lenght);
	leg->set_forearm_lenght(geometry.forearm_lenght);
	return true;
}

bool get_leg_geometry(LegNum num, LegGeometry<FLOAT> & geometry) {
	Leg<FLOAT> * leg = get_len_by_num(static_cast<LegNum>(num));
	if (leg == NULL) {
		return false;
	}
	geometry.pos = leg->get_pos();
	geometry.shoulder_offset = leg->shoulder_offset;
	geometry.shoulder_lenght = leg->shoulder_lenght;
	geometry.forearm_lenght = leg->forearm_lenght;
	return true;
}

bool load_legs_geometry() {

}
