#ifndef tasks_utils_h_
#define tasks_utils_h_
#include "defines.h"
#include "leg.h"

FLOAT get_model_angle(uint8_t address);
Leg<FLOAT> * get_len_by_num(LegNum num);
bool set_leg_geometry(LegNum num, const LegGeometry<FLOAT> & geometry);
bool get_leg_geometry(LegNum num, LegGeometry<FLOAT> & geometry);
bool load_legs_geometry();

#endif //tasks_utils_h_