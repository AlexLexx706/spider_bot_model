#ifndef _SERIAL_SERVO_RP_H_
#define _SERIAL_SERVO_RP_H_
#include <stdint.h>

class HardwareSerial;

void LobotSerialServoMove(HardwareSerial &serial_x, uint8_t id, int16_t position, uint16_t time);
void LobotSerialServoStopMove(HardwareSerial &serial_x, uint8_t id);
void LobotSerialServoSetID(HardwareSerial &serial_x, uint8_t oldID, uint8_t newID);
void LobotSerialServoSetMode(HardwareSerial &serial_x, uint8_t id, uint8_t Mode, int16_t Speed);
void LobotSerialServoLoad(HardwareSerial &serial_x, uint8_t id);
void LobotSerialServoUnload(HardwareSerial &serial_x, uint8_t id);
int16_t LobotSerialServoReadPosition(HardwareSerial &serial_x, uint8_t id);
int16_t LobotSerialServoReadVin(HardwareSerial &serial_x, uint8_t id);


#endif //_SERIAL_SERVO_RP_H_