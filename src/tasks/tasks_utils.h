#ifndef tasks_utils_h_
#define tasks_utils_h_
#include "protocol.h"
#include "leg.h"

//used for save/restore leg geometry from file
typedef LegGeometry<FLOAT> LegsGeometryStore[LEGS_COUNT]; 

FLOAT get_model_angle(uint8_t address);
Leg<FLOAT> * get_len_by_num(LegNum num);
bool set_leg_geometry(LegNum num, const LegGeometry<FLOAT> & geometry);
bool get_leg_geometry(LegNum num, LegGeometry<FLOAT> & geometry);
bool load_legs_geometry();
bool save_legs_geometry();
#endif //tasks_utils_h_