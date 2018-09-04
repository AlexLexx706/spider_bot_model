#include "leg.h"

void test_leg() {
	Leg<float> leg(NULL);

	Vector3f pos(0, -7.0, 12.0);
	leg.move_end(pos);
}
