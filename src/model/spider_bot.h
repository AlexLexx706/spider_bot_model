#ifndef SPIDER_BOT_H
#define SPIDER_BOT_H
#include "leg.h"
#include "defines.h"

#define LENGTH 13.
#define WIDTH 7.
#define HEIGHT 3.


template<class T>
class SpiderBot:public Node<T> {
public:
	Leg<T> front_right_leg;
	Leg<T> rear_right_leg;
	Leg<T> front_left_leg;
	Leg<T> rear_left_leg;
private:
	Vector3<T>front_right_pos;
	Vector3<T>front_left_pos;
	Vector3<T>rear_right_pos;
	Vector3<T>rear_left_pos;
	Action action;

	T HALF_STEP_LEN;
	T MOVE_TIME;
	T STEP_HEIGHT;
	T ROTATE_ANGLE;

	int move_state;
	bool begin_move;
	bool first;
	T half_step_len;
	T move_time;
	T step_height;

	Vector3<T> start_front_left;
	Vector3<T> start_front_right;
	Vector3<T> start_rear_left;
	Vector3<T> start_rear_right;
   	Vector3<T> direction;
	T start_time;
	T cur_half_step_len;
	int test_state;
	int rotate_state;
	T turn_angle;
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
			rear_left_leg(shoulder_offset, shoulder_lenght, forearm_lenght),
			action(NOT_MOVE),
			HALF_STEP_LEN(6.0),
			MOVE_TIME(0.3),
			STEP_HEIGHT(4.0),
			ROTATE_ANGLE(0.5),
			move_state(-1),
			begin_move(true),
			first(true),
			test_state(-1),
			rotate_state(-1),
			turn_angle(0.5) {

		front_right_leg.set_parent(this);
		front_right_leg.set_pos(Vector3<T>(length / 2.0, 0.0, width / 2.0));
		// front_right_leg.print_info = true;

		rear_right_leg.set_parent(this);
		rear_right_leg.set_pos(Vector3<T>(-length / 2.0, 0.0, width / 2.0));


		front_left_leg.set_parent(this);
		front_left_leg.set_pos(Vector3<T>(length / 2.0, 0.0, -width / 2.0));
		front_left_leg.set_angle_y(M_PI);

		rear_left_leg.set_parent(this);
		rear_left_leg.set_pos(Vector3<T>(-length / 2.0, 0.0, -width / 2.0));
		rear_left_leg.set_angle_y(M_PI);

		//init default leg pos
		front_right_pos = Vector3<T>(length / 2, -8, (width / 2 + shoulder_offset));
		front_left_pos = Vector3<T>(length / 2, -8, -(width / 2 + shoulder_offset));
		rear_right_pos = Vector3<T>(-length / 2, -8, (width / 2 + shoulder_offset));
		rear_left_pos = Vector3<T>(-length / 2, -8, -(width / 2 + shoulder_offset));
		// std::cerr << "front_right_pos:" << front_right_pos << std::endl;
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

	void print() {
		//print state
		front_right_leg.print();
		rear_right_leg.print();
		front_left_leg.print();
		rear_left_leg.print();
	}

	void set_action(Action _action) {
		action = _action;
	}

	static T get_time() {
		struct timespec tms;
		assert(clock_gettime(CLOCK_MONOTONIC, &tms) == 0);
		return (tms.tv_sec + tms.tv_nsec / 1e9);
	}

	void step() {
		// set_action(ROTATE_LEFT);

		if (move_state == -1 && test_state == -1 && rotate_state == -1) {
			switch (action) {
				case MOVE_FORWARD: {
					move_state = 0;
					begin_move = true;
					half_step_len = HALF_STEP_LEN;
					move_time = MOVE_TIME;
					step_height = STEP_HEIGHT;
					break;
				} case MOVE_BACKWARD: {
					move_state = 0;
					begin_move = true;
					half_step_len = -HALF_STEP_LEN;
					move_time = MOVE_TIME;
					step_height = STEP_HEIGHT;
					break;
				} case FRONT_RIGHT_TEST: {
					test_state = 0;
					break;
				} case ROTATE_LEFT: {
					rotate_state = 0;
					begin_move = true;
					turn_angle = ROTATE_ANGLE;
					move_time = MOVE_TIME;
					step_height = STEP_HEIGHT;
					break;
				} case ROTATE_RIGHT: {
					rotate_state = 0;
					begin_move = true;
					turn_angle = -ROTATE_ANGLE;
	  				move_time = MOVE_TIME;
					step_height = STEP_HEIGHT;
					break;
				}
			}
		}
		action = NOT_MOVE;
		process_move();
		process_test();
		process_rotate();
	}

	void process_test() {
		//1. begin
		if (test_state == 0) {
			start_time = get_time();
			start_front_right = front_right_pos;
			test_state = 1;
		}

		if (test_state == 1) {
			double dt = (get_time() - start_time) / 10.0;
			if (dt >= 1.0) {
				test_state = -1;
				dt = 1.0;
			}
			// 2.update pos
			Vector3<T> pos = start_front_right + Vector3<T>(0.0, 10.0, 0.0) * sin(M_PI * dt);
			front_right_leg.move_end(pos);
		}
	}

	void process_move() {
		if (move_state < 0) {
			return;
		}

		if (begin_move) {
			start_time = get_time();
			begin_move = false;
			first = true;
		}
		double dt = (get_time() - start_time) / move_time;

		// 1. move front_left forward
		if (move_state == 0) {
			// begin move leg
			if (first) {
				first = false;
				start_front_left = front_left_leg.end.get_g_pos();
				direction = (front_left_pos + Vector3<T>(half_step_len, 0.0, 0.0)) - start_front_left;
			// end step
			} else if (dt >= 1.0) {
				begin_move = true;
				move_state = 1;
				first = true;
				dt = 1.0;
			}
			// move leg
			front_left_leg.move_end(
				start_front_left + direction * dt +
				Vector3<T>(0.0, step_height * sin(M_PI * dt), 0.0));
		//2. 4 points
		} else if (move_state == 1) {
			if (first) {
				first = false;
				start_front_left = front_left_leg.end.get_g_pos();
				start_front_right = front_right_leg.end.get_g_pos();
				start_rear_left = rear_left_leg.end.get_g_pos();
				start_rear_right = rear_right_leg.end.get_g_pos();
			} else if (dt >= 1.0) {
				begin_move = true;
				move_state = 2;
				first = true;
				dt = 1.0;
			}
			// move legs
			direction = -Vector3<T>(half_step_len, 0, 0) * dt;

			front_left_leg.move_end(start_front_left + direction);
			front_right_leg.move_end(start_front_right + direction);
			rear_left_leg.move_end(start_rear_left + direction);
			rear_right_leg.move_end(start_rear_right + direction);
		//4. move right rear leg
		} else if (move_state == 2) {
			if (first) {
				first = false;
				start_rear_right = rear_right_leg.end.get_g_pos();
				cur_half_step_len = trunc((start_rear_right - front_right_leg.end.get_g_pos()).length() / fabs(half_step_len)) * half_step_len;
			} else if (dt >= 1.0) {
				begin_move = true;
				move_state = 3;
				first = true;
				dt = 1.0;
			}
			//move leg
			rear_right_leg.move_end(
				start_rear_right +
				Vector3<T>(
					cur_half_step_len * dt,
					step_height * sin(M_PI * dt),
					0.0));
		//3. move right front leg
		} else if (move_state == 3) {
			if (first) {
				first = false;
				start_front_right = front_right_leg.end.get_g_pos();
			//end
			} else if (dt >= 1.0) {
				begin_move = true;
				move_state = 4;
				first = true;
				dt = 1.0;
			}
			//move leg
			front_right_leg.move_end(
				start_front_right +
				Vector3<T>(
					half_step_len * 2 * dt,
					step_height * sin(M_PI * dt),
					0));
		//4. 4 points
		} else if (move_state == 4) {
			if (first) {
				first = false;
				start_front_left = front_left_leg.end.get_g_pos();
				start_front_right = front_right_leg.end.get_g_pos();
				start_rear_left = rear_left_leg.end.get_g_pos();
				start_rear_right = rear_right_leg.end.get_g_pos();
			//end
			} else if (dt >= 1.0) {
				begin_move = true;
				move_state = 5;
				first = true;
				dt = 1.0;
			}
			//move legs
			direction = Vector3<T>(-half_step_len, 0, 0) * dt;

			front_left_leg.move_end(start_front_left + direction);
			rear_left_leg.move_end(start_rear_left + direction);
			front_right_leg.move_end(start_front_right + direction);
			rear_right_leg.move_end(start_rear_right + direction);
		//5. move rear left
		} else if (move_state == 5) {
			if (first) {
				first = false;
				start_rear_left = rear_left_leg.end.get_g_pos();
			//end
			} else if (dt >= 1.0) {
				begin_move = true;
				move_state = -1;
				dt = 1.0;
			}
			// move leg
			rear_left_leg.move_end(
				start_rear_left +
				Vector3<T>(half_step_len * 2 * dt, step_height * sin(M_PI * dt), 0));
		}
	}

	void process_rotate() {
		if (rotate_state < 0) {
			return;
		}

		if (begin_move) {
			start_time = get_time();
			begin_move = false;
			first = true;
		}

		T dt = (get_time() - start_time) / move_time;
		if (dt >= 1.0) {
				dt = 1.0;
		}

		Vector3<T> up_vector = Vector3<T>(0, step_height * sin(M_PI * dt), 0);

		// 1. move front left leg
		if (rotate_state == 0) {
			if (first) {
				first = false;
				start_front_left = front_left_leg.end.get_g_pos();
				Vector3<T> tmp(front_left_pos); tmp.rotate(0., turn_angle, 0.);
				direction = tmp - start_front_left;
			} else if (dt >= 1.0) {
				dt = 1.0;
				begin_move = true;
				rotate_state = 1;
			}

			front_left_leg.move_end(
				start_front_left + direction * dt + up_vector);
		// 2. move rear left leg
		} else if (rotate_state == 1) {
			if (first) {
				first = false;
				start_rear_left = rear_left_leg.end.get_g_pos();
				Vector3<T> tmp(rear_left_pos); tmp.rotate(0., turn_angle, 0.);
				direction = tmp - start_rear_left;
			} else if (dt >= 1.0) {
				dt = 1.0;
				begin_move = true;
				rotate_state = 2;
			}
			rear_left_leg.move_end(
				start_rear_left + direction * dt + up_vector);
		// 3. rotate all points
		} else if (rotate_state == 2) {
			if (first) {
				first = false;
				start_front_left = front_left_leg.end.get_g_pos();
				start_rear_left = rear_left_leg.end.get_g_pos();
				start_front_right = front_right_leg.end.get_g_pos();
				start_rear_right = rear_right_leg.end.get_g_pos();
			} else if (dt >= 1.0) {
				dt = 1.0;
				begin_move = true;
				rotate_state = 3;
			}

			Matrix3<T> mat = Matrix3<T>::createRotationAroundAxis(0, -(-turn_angle) * dt, 0);
			front_left_leg.move_end(mat * start_front_left);
			rear_left_leg.move_end(mat * start_rear_left);
			front_right_leg.move_end(mat * start_front_right);
			rear_right_leg.move_end(mat * start_rear_right);
		//move rear right leg
		} else if (rotate_state == 3) {
			if (first) {
				first = false;
				start_rear_right = rear_right_leg.end.get_g_pos();
				Vector3<T> tmp(start_rear_right); tmp.rotate(0., turn_angle, 0.);
				direction = tmp - start_rear_right;
			} else if (dt >= 1.0) {
				dt = 1.0;
				begin_move = true;
				rotate_state = 4;
			}

			rear_right_leg.move_end(
				start_rear_right + direction * dt + up_vector);
		//move front right leg
		} else if (rotate_state == 4) {
			if (first) {
				first = false;
				start_front_right = front_right_leg.end.get_g_pos();

				Vector3<T> tmp(start_front_right); tmp.rotate(0., turn_angle, 0.);
				direction = tmp - start_front_right;
			} else if (dt >= 1.0) {
				dt = 1.0;
				begin_move = true;
				rotate_state = -1;
			}
			front_right_leg.move_end(
				start_front_right + direction * dt + up_vector);
		}
	}
};

#endif