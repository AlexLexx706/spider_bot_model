#ifndef GEOMETRY_DESC_H
#define GEOMETRY_DESC_H

#include "vmath.h"

template<class T>
struct  LegGeometry {
	Vector3<T> pos;
	T shoulder_offset;
	T shoulder_lenght;
	T forearm_lenght;
};


template<class T>
struct LegDesc {
	LegGeometry<T> geometry;
	T a_0;
	T a_1;
	T a_2;
};

#endif //GEOMETRY_DESC_H