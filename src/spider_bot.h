#ifndef SPIDER_BOT_H
#define SPIDER_BOT_H
#include "leg.h"

#define LENGTH 10.
#define WIDTH 10.
#define HEIGHT 3.


template<class T>
class SpiderBot:Node<T> {
	Leg<T> front_right_leg;
	Leg<T> rear_right_leg;
	Leg<T> front_left_leg;
	Leg<T> rear_left_leg;
	Vector3<T>front_right_pos;
	Vector3<T>front_left_pos;
	Vector3<T>rear_right_pos;
	Vector3<T>rear_left_pos;
public:
	SpiderBot(
		T length=LENGTH,
		T width=WIDTH,
		T height=HEIGHT,
		T shoulder_offset=SHOULDER_OFFSET,
		T shoulder_lenght=SHOULDER_LENGHT,
		T forearm_lenght=FOREARM_LENGHT):
			front_right_leg(shoulder_offset, shoulder_lenght, forearm_lenght),
			rear_right_leg(shoulder_offset, shoulder_lenght, forearm_lenght),
			front_left_leg(shoulder_offset, shoulder_lenght, forearm_lenght),
			rear_left_leg(shoulder_offset, shoulder_lenght, forearm_lenght) {


		front_right_leg.set_parent(this);
		front_right_leg.set_pos(Vector3<T>(length / 2.0, 0.0, width / 2.0));

		rear_right_leg.set_parent(this);
		rear_right_leg.set_pos(Vector3<T>(-length / 2.0, 0.0, width / 2.0));


		front_left_leg.set_parent(this);
		front_left_leg.set_pos(Vector3<T>(length / 2.0, 0.0, -width / 2.0));
		front_left_leg.set_angle_x(M_PI);

		rear_left_leg.set_parent(this);
		rear_left_leg.set_pos(Vector3<T>(-length / 2.0, 0.0, -width / 2.0));
		rear_left_leg.set_angle_x(M_PI);

		//init default leg pos
		front_right_pos = Vector3<T>(length / 2, -4, width / 2 + 8);
		front_left_pos = Vector3<T>(length / 2, -4, -(width / 2 + 8));
		rear_right_pos = Vector3<T>(-length / 2, -4, width / 2 + 8);
		rear_left_pos = Vector3<T>(-length / 2, -4, -(width / 2 + 8));

		reset();
	}

	void reset() {
		front_left_leg.move_end(
			front_left_pos);

		front_right_leg.move_end(
			front_right_pos);

		rear_left_leg.move_end(
			rear_left_pos);

		rear_right_leg.move_end(
			rear_right_pos);
	}
};

#endif