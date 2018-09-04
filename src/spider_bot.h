#ifndef SPIDER_BOT_H
#define SPIDER_BOT_H
#include "leg.h"

template<class T>
class SpiderBot:Node<T> {
	Leg<T> front_right_leg;
	Leg<T> front_left_leg;
	Leg<T> rear_left_leg;
	Leg<T> rear_right_leg;
public:
	SpiderBot(
		T length=10,
        T width=10,
        T height=3) {
	}
};

#endif