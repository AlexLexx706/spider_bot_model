#ifndef _SERIAL_SERVO_RP_H_
#define _SERIAL_SERVO_RP_H_
#include <stdint.h>

class Serial;
namespace Servo {
	void servo_move(Serial &serial_x, uint8_t id, int16_t position, uint16_t time);
	void stop_move(Serial &serial_x, uint8_t id);
	void set_id(Serial &serial_x, uint8_t oldID, uint8_t newID);
	void set_mode(Serial &serial_x, uint8_t id, uint8_t Mode, int16_t Speed);
	void load(Serial &serial_x, uint8_t id);
	void unload(Serial &serial_x, uint8_t id);
	uint16_t read_position(Serial &serial_x, uint8_t id);
	uint16_t read_vin(Serial &serial_x, uint8_t id);
	int limit_write(Serial &serial_x, uint8_t id, uint16_t min_angle, uint16_t max_angle);
	int limit_read(Serial &serial_x, uint8_t id, uint16_t & min_angle, uint16_t & max_angle);
}

#endif //_SERIAL_SERVO_RP_H_