#ifndef _SERIAL_SERVO_RP_H_
#define _SERIAL_SERVO_RP_H_
#include <stdint.h>

class Serial;

void LobotSerialServoMove(Serial &serial_x, uint8_t id, int16_t position, uint16_t time);
void LobotSerialServoStopMove(Serial &serial_x, uint8_t id);
void LobotSerialServoSetID(Serial &serial_x, uint8_t oldID, uint8_t newID);
void LobotSerialServoSetMode(Serial &serial_x, uint8_t id, uint8_t Mode, int16_t Speed);
void LobotSerialServoLoad(Serial &serial_x, uint8_t id);
void LobotSerialServoUnload(Serial &serial_x, uint8_t id);
int16_t LobotSerialServoReadPosition(Serial &serial_x, uint8_t id);
int16_t LobotSerialServoReadVin(Serial &serial_x, uint8_t id);

int lobot_serial_limit_write(Serial &serial_x, uint8_t id, uint16_t min_angle, uint16_t max_angle);
int lobot_serial_limit_read(Serial &serial_x, uint8_t id, uint16_t & min_angle, uint16_t & max_angle);


#endif //_SERIAL_SERVO_RP_H_