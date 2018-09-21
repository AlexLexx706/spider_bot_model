#ifndef LEG_H_
#define LEG_H_

#include "node.h"
#include <stdio.h>
// #include <iostream>

#define SHOULDER_OFFSET 4.2
#define SHOULDER_LENGHT 6.2
#define FOREARM_LENGHT 7.8

template<class T>
class Leg: public Node<T> {
public:
	Node<T> p_0;
	Node<T> p_1;
	Node<T> p_2;
	Node<T> end;
	T shoulder_offset;
	T shoulder_lenght;
	T forearm_lenght;

public:
	Leg(
		T _shoulder_offset=SHOULDER_OFFSET,
		T _shoulder_lenght=SHOULDER_LENGHT,
		T _forearm_lenght=FOREARM_LENGHT):
			shoulder_offset(_shoulder_offset),
			shoulder_lenght(_shoulder_lenght),
			forearm_lenght(_forearm_lenght) {
				p_0.set_parent(this);
				// p_0.set_angle_z(0.5);
				p_1.set_parent(&p_0);
				p_1.set_pos(Vector3<T>(0.0, 0.0, shoulder_offset));
				// std::cout << p_1.matrix() << std::endl;

				p_2.set_parent(&p_1);
				p_2.set_pos(Vector3<T>(0.0, 0.0, shoulder_lenght));

				end.set_parent(&p_2);
				end.set_pos(Vector3<T>(0.0, 0.0, forearm_lenght));
	}

	~Leg() {
	}

	void set_shoulder_offset(T len) {
		if (shoulder_offset != len) {
			shoulder_offset = len;
			p_1.set_pos(Vector3<T>(0.0, 0.0, shoulder_offset));
		}
	}

	T get_shoulder_offset() const { return shoulder_offset;}

	void set_shoulder_lenght(T len) {
		if (shoulder_lenght != len) {
			shoulder_lenght = len;
			p_2.set_pos(Vector3<T>(0.0, 0.0, shoulder_lenght));

		}
	}

	T get_shoulder_lenght() const { return shoulder_lenght;}

	void set_forearm_lenght(T len) {
		if (forearm_lenght != len) {
			forearm_lenght = len;
			end.set_pos(Vector3<T>(0.0, 0.0, forearm_lenght));
		}
	}

	T get_forearm_lenght() const { return forearm_lenght;}

	void move_end(const Vector3<T> & pos) {
		//1. calk p_0 angle - vertical angle
		// std::cout << "move_end 1. pos:" << pos << std::endl;

		p_0.set_angle_y(
			Leg::get_proj_angle(
				Vector3<T>(0.0, 0.0, 1.0),
				Vector3<T>(1.0, 0.0, 0.0),
				Leg::from_world(pos)));

		// std::cout << "move_end 2. angle_y:" << p_0.get_angle_y() << std::endl;
	   	//2. calk triangle
		Vector3<T> cur_pos(p_0.from_world(pos) - p_1.get_pos());
		T len_c = cur_pos.length();

		//2.1 p_1 angle
		T angle = 0.0;
		T tmp = 2 * shoulder_lenght * len_c;
		//std::cout << "move_end 2.2 cur_pos:" << cur_pos << " len_c:" << len_c << " shoulder_lenght:" << shoulder_lenght << " forearm_lenght" << forearm_lenght << " tmp:" << tmp << std::endl;

		if (tmp != 0) {
			T tmp2 = (shoulder_lenght * shoulder_lenght + len_c * len_c - forearm_lenght * forearm_lenght);
			T tmp3 = tmp2 / tmp;
			//std::cout << "move_end 2.3 tmp2:" << tmp2 << " tmp3:" << tmp3 << std::endl;

			if (fabs(tmp3) <= 1.0) {
				angle = acos(tmp3);
			}
		}
		//std::cout << "move_end 2.4 angle:" << angle << std::endl;


		T dir_angle(acos(Vector3<T>(0.0, 0.0, 1.0).dotProduct(cur_pos/cur_pos.length())));
		//std::cout << "move_end 2.5 dir_angle:" << dir_angle << std::endl;

		//detect sign
		dir_angle = cur_pos.dotProduct(
			Vector3<T>(0., -1.0, 0.0)) > 0.0 ? dir_angle: -dir_angle;

		//std::cout << "move_end 2.6 dir_angle:" << dir_angle << std::endl;

		p_1.set_angle_x(dir_angle - angle);
		//std::cout << p_1.matrix() << std::endl;
		//std::cout << "move_end 3. p_1.angle_x:" << p_1.get_angle_x() << std::endl;

		//2.2 p_2 angle
		angle = M_PI;
		tmp = (forearm_lenght * forearm_lenght + shoulder_lenght * shoulder_lenght - len_c * len_c) / (2 * shoulder_lenght * forearm_lenght);

		if (fabs(tmp) <= 1.0) {
			angle = acos(tmp);
		}

		//std::cout << "move_end 4. angle:" << angle << std::endl;

		p_2.set_angle_x(M_PI - angle);

	}
};
#endif // LEG_H_