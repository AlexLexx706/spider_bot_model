#include "node.h"

#include <iostream>

void test_node() {
	NodeF node_1;
	std::cout << "matrix:" << std::endl << node_1.matrix() << std::endl;

	NodeF node_2;
	node_2.set_parent(&node_1);
	std::cout << "matrix:" << std::endl << node_2.matrix() << std::endl;

	node_2.set_pos(Vector3f(1, 2, 0));
	std::cout << "matrix:" << std::endl << node_2.matrix() << std::endl;

	node_1.set_g_pos(Vector3f(10, -4, 0));
	std::cout << "matrix:" << std::endl << node_2.matrix() << std::endl;

	node_1.from_world(Vector3f(0,0,0));
	node_1.set_angle_z(12.0);
	std::cout << "ang_z:" << node_1.get_angle_z() << std::endl;

}
