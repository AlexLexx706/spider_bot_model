#ifndef LEG_H_
#define LEG_H_

#include "node.h"
#include <iostream>

#define SHOULDER_OFFSET 2
#define SHOULDER_LENGHT 10
#define FOREARM_LENGHT 10

template<class T>
class Leg: public Node<T> {
	Node<T> p_0;
	Node<T> p_1;
	Node<T> p_2;
	Node<T> end;
	T len_a;
	T len_b;

public:
	Leg(Node<T> * _parent,
		const Vector3<T> & pos=Vector3<T>(0.0, 0.0, 0),
		T shoulder_lenght=SHOULDER_LENGHT,
		T forearm_lenght=FOREARM_LENGHT):
			len_a(shoulder_lenght),
			len_b(forearm_lenght) {
				p_0.set_parent(this);
				// p_0.set_angle_z(0.5);
				p_1.set_parent(&p_0);
				p_1.set_pos(Vector3<T>(0.0, 0.0, SHOULDER_OFFSET));
				// std::cout << p_1.matrix() << std::endl;

				p_2.set_parent(&p_1);
				p_2.set_pos(Vector3<T>(0.0, 0.0, len_a));

				end.set_parent(&p_2);
				end.set_pos(Vector3<T>(0.0, 0.0, len_b));
	}

	~Leg() {
	}

	void move_end(const Vector3<T> & pos) {
		//1. calk p_0 angle - vertical angle
		//std::cout << "move_end 1. pos:" << pos << std::endl;

		p_0.set_angle_y(
			Leg::get_proj_angle(
				Vector3<T>(0.0, 0.0, 1.0),
				Vector3<T>(0.1, 0.0, 0.0),
				Leg::from_world(pos)));

		//std::cout << "move_end 2. angle_y:" << Leg::get_angle_y() << std::endl;

	   	//2. calk triangle
		Vector3<T> cur_pos(p_0.from_world(pos) - p_1.get_pos());
		T len_c = cur_pos.length();

		//2.1 p_1 angle
		T angle = 0.0;
		T tmp = 2 * len_a * len_c;
		//std::cout << "move_end 2.2 cur_pos:" << cur_pos << " len_c:" << len_c << " len_a:" << len_a << " len_b" << len_b << " tmp:" << tmp << std::endl;

		if (tmp != 0) {
			T tmp2 = (len_a * len_a + len_c * len_c - len_b * len_b);
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
		std::cout << p_1.matrix() << std::endl;
		//std::cout << "move_end 3. p_1.angle_x:" << p_1.get_angle_x() << std::endl;

		//2.2 p_2 angle
		angle = M_PI;
		tmp = (len_b * len_b + len_a * len_a - len_c * len_c) / (2 * len_a * len_b);

		if (fabs(tmp) <= 1.0) {
			angle = acos(tmp);
		}

		//std::cout << "move_end 4. angle:" << angle << std::endl;

		p_2.set_angle_x(M_PI - angle);
		//std::cout << "move_end 3. p_2.angle_x:" << p_2.get_angle_x() << std::endl;
		std::cout << "move_end len_a:" << len_a <<
			" len_b:" << len_b <<
			" pos:" << pos <<
			" p_0.ang_y:" << p_0.get_angle_y() <<
			" p_1.ang_x:" << p_1.get_angle_x() << 
			" p_2.ang_x:" << p_2.get_angle_x() <<
			" end_pos:" << end.to_world(Vector3<T>(0, 0, 0)) << std::endl;
	}
};
#endif // LEG_H_