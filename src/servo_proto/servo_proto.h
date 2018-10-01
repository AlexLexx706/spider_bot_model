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
	bool limit_write(Serial &serial_x, uint8_t id, uint16_t min_angle, uint16_t max_angle);
	int limit_read(Serial &serial_x, uint8_t id, uint16_t & min_angle, uint16_t & max_angle);

	/**
	 * @brief read led error code
	 * @param serial_x com port 
	 * @param id servo id
	 * 
	 * @return -1 read error, else error codes
	 */
	int led_error_read(Serial &serial_x, uint8_t id);
}

#endif //_SERIAL_SERVO_RP_H_