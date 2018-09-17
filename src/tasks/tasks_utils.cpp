#include "spider_bot.h"
#include "tasks_utils.h"
#include <stdio.h>
#include <assert.h>

extern SpiderBot<FLOAT> bot;
extern const char * LEGS_GEOMETRY_PATH;


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
	fprintf(stderr, "leg:%d not exist\n", num);
	return NULL;	
}


Vector3<FLOAT> * get_control_point_by_num(LegNum num) {
	switch (num) {
		case FRONT_RIGHT_LEG_NUM: {
			return &bot.front_right_pos;
		}
		case REAR_RIGHT_LEG_NUM: {
			return &bot.rear_right_pos;
		}
		case FRONT_LEFT_LEG_NUM: {
			return &bot.front_left_pos;
		}
		case REAR_LEFT_LEG_NUM: {
			return &bot.rear_left_pos;
		}
	}
	fprintf(stderr, "leg:%d not exist\n", num);
	return NULL;	
}


void set_leg_geometry_internal(Leg<FLOAT> & leg, const LegGeometry<FLOAT> & geometry) {
	leg.set_pos(geometry.pos);
	leg.set_shoulder_offset(geometry.shoulder_offset);
	leg.set_shoulder_lenght(geometry.shoulder_lenght);
	leg.set_forearm_lenght(geometry.forearm_lenght);
}

void set_control_point_internal(Vector3<FLOAT> & pos, const LegGeometry<FLOAT> & geometry) {
	
	//right
	if (geometry.pos.z > 0.) {
		pos = geometry.pos +
			Vector3<FLOAT>(0.0, -geometry.forearm_lenght * 1.5, geometry.shoulder_offset + geometry.shoulder_lenght * 0.5);
	//left
	} else {
		pos = geometry.pos +
			Vector3<FLOAT>(0.0, -geometry.forearm_lenght * 1.5, -(geometry.shoulder_offset + geometry.shoulder_lenght * 0.5));
	}
	std::cout << "control pos:" <<  pos << std::endl;
}


bool set_leg_geometry(LegNum num, const LegGeometry<FLOAT> & geometry) {
	fprintf(stderr, "set_leg_geometry 1. num:%d\n", num);

	Leg<FLOAT> * leg = get_len_by_num(static_cast<LegNum>(num));
	if (leg == NULL) {
		return false;
	}
	fprintf(
		stderr,
		"set_leg_geometry 2. geometry: pos:%f %f %f "
		"shoulder_offset:%f, shoulder_lenght:%f, forearm_lenght:%f\n",
		geometry.pos.x, geometry.pos.y, geometry.pos.z,
		geometry.shoulder_offset,
		geometry.shoulder_lenght,
		geometry.forearm_lenght
	);
	set_leg_geometry_internal(*leg, geometry);
	set_control_point_internal(*get_control_point_by_num(static_cast<LegNum>(num)), geometry);
	save_legs_geometry();
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

	FILE * fd = fopen(LEGS_GEOMETRY_PATH, "rb");

	if (fd != NULL) {
		LegsGeometryStore lgs;
		int res = fread(lgs, 1, sizeof(lgs), fd);

		if (res != sizeof(lgs)) {
			fprintf(stderr, "cannot read leg geometry from file:%s\n", LEGS_GEOMETRY_PATH);
			fclose(fd);
			return false;
		}

		Leg<FLOAT> * leg;
		for (int num = 0; num < LEGS_COUNT; num++) {
			if ((leg = get_len_by_num(static_cast<LegNum>(num))) == NULL) {
				return false;
			}
			set_leg_geometry_internal(*leg, lgs[num]);
		}
		fclose(fd);
		return true;
	}
	fprintf(stderr, "legs geometry file:%s not exist\n", LEGS_GEOMETRY_PATH);
	return false;
}


bool save_legs_geometry() {
	LegsGeometryStore lgs;
	for (int i = 0; i < LEGS_COUNT; i++) {
		get_leg_geometry(static_cast<LegNum>(i), lgs[i]);
	}

	FILE * fd = fopen(LEGS_GEOMETRY_PATH, "wb");
	if (fd) {
		int res = fwrite(lgs, 1, sizeof(lgs), fd);
		if (res != sizeof(lgs)) {
			return false;
		}
		fclose(fd);
		return true;
	}
	return false;
}
