#include <string.h>
#include <stdio.h>
#include "serial_servo_rp.h"
#include "hardware_serial.h"
#include "utils.h"


#define GET_LOW_BYTE(A) (uint8_t)((A))
//Macro function	get lower 8 bits of A
#define GET_HIGH_BYTE(A) (uint8_t)((A) >> 8)
//Macro function	get higher 8 bits of A
#define BYTE_TO_HW(A, B) ((((uint16_t)(A)) << 8) | (uint8_t)(B))
//Macro Function	put A as higher 8 bits	 B as lower 8 bits	 which amalgamated into 16 bits integer

#define LOBOT_SERVO_FRAME_HEADER				0x55
#define LOBOT_SERVO_MOVE_TIME_WRITE				1
#define LOBOT_SERVO_MOVE_TIME_READ				2
#define LOBOT_SERVO_MOVE_TIME_WAIT_WRITE 		7
#define LOBOT_SERVO_MOVE_TIME_WAIT_READ			8
#define LOBOT_SERVO_MOVE_START					11
#define LOBOT_SERVO_MOVE_STOP					12
#define LOBOT_SERVO_ID_WRITE					13
#define LOBOT_SERVO_ID_READ						14
#define LOBOT_SERVO_ANGLE_OFFSET_ADJUST			17
#define LOBOT_SERVO_ANGLE_OFFSET_WRITE			18
#define LOBOT_SERVO_ANGLE_OFFSET_READ			19
#define LOBOT_SERVO_ANGLE_LIMIT_WRITE			20
#define LOBOT_SERVO_ANGLE_LIMIT_READ			21
#define LOBOT_SERVO_VIN_LIMIT_WRITE				22
#define LOBOT_SERVO_VIN_LIMIT_READ				23
#define LOBOT_SERVO_TEMP_MAX_LIMIT_WRITE 		24
#define LOBOT_SERVO_TEMP_MAX_LIMIT_READ			25
#define LOBOT_SERVO_TEMP_READ					26
#define LOBOT_SERVO_VIN_READ					27
#define LOBOT_SERVO_POS_READ					28
#define LOBOT_SERVO_OR_MOTOR_MODE_WRITE			29
#define LOBOT_SERVO_OR_MOTOR_MODE_READ			30
#define LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE 		31
#define LOBOT_SERVO_LOAD_OR_UNLOAD_READ			32
#define LOBOT_SERVO_LED_CTRL_WRITE				33
#define LOBOT_SERVO_LED_CTRL_READ				34
#define LOBOT_SERVO_LED_ERROR_WRITE				35
#define LOBOT_SERVO_LED_ERROR_READ				36

//#define LOBOT_DEBUG 1	/*Debug ：print debug value*/

uint8_t LobotCheckSum(uint8_t buf[]) {
	uint8_t i;
	uint16_t temp = 0;
	for (i = 2; i < buf[3] + 2; i++) {
		temp += buf[i];
	}
	temp = ~temp;
	i = (uint8_t)temp;
	return i;
}

void LobotSerialServoMove(HardwareSerial &serial_x, uint8_t id, int16_t position, uint16_t time) {
	fprintf(stderr, "LobotSerialServoMove id:%hhu position:%hd time:%hu\n", id, position, time);

	uint8_t buf[10];
	if(position < 0)
		position = 0;
	if(position > 1000)
		position = 1000;
	buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
	buf[2] = id;
	buf[3] = 7;
	buf[4] = LOBOT_SERVO_MOVE_TIME_WRITE;
	buf[5] = GET_LOW_BYTE(position);
	buf[6] = GET_HIGH_BYTE(position);
	buf[7] = GET_LOW_BYTE(time);
	buf[8] = GET_HIGH_BYTE(time);
	buf[9] = LobotCheckSum(buf);
	serial_x.write(buf, 10);
}

void LobotSerialServoStopMove(HardwareSerial &serial_x, uint8_t id) {
	uint8_t buf[6];
	buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
	buf[2] = id;
	buf[3] = 3;
	buf[4] = LOBOT_SERVO_MOVE_STOP;
	buf[5] = LobotCheckSum(buf);
	serial_x.write(buf, 6);
}

void LobotSerialServoSetID(HardwareSerial &serial_x, uint8_t oldID, uint8_t newID) {
	uint8_t buf[7];
	buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
	buf[2] = oldID;
	buf[3] = 4;
	buf[4] = LOBOT_SERVO_ID_WRITE;
	buf[5] = newID;
	buf[6] = LobotCheckSum(buf);
	serial_x.write(buf, 7);
}

void LobotSerialServoSetMode(HardwareSerial &serial_x, uint8_t id, uint8_t Mode, int16_t Speed) {
	uint8_t buf[10];

	buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
	buf[2] = id;
	buf[3] = 7;
	buf[4] = LOBOT_SERVO_OR_MOTOR_MODE_WRITE;
	buf[5] = Mode;
	buf[6] = 0;
	buf[7] = GET_LOW_BYTE((uint16_t)Speed);
	buf[8] = GET_HIGH_BYTE((uint16_t)Speed);
	buf[9] = LobotCheckSum(buf);
	serial_x.write(buf, 10);
}

void LobotSerialServoLoad(HardwareSerial &serial_x, uint8_t id) {
	uint8_t buf[7];
	buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
	buf[2] = id;
	buf[3] = 4;
	buf[4] = LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE;
	buf[5] = 1;
	buf[6] = LobotCheckSum(buf);
	
	serial_x.write(buf, 7);
}

void LobotSerialServoUnload(HardwareSerial &serial_x, uint8_t id) {
	uint8_t buf[7];
	buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
	buf[2] = id;
	buf[3] = 4;
	buf[4] = LOBOT_SERVO_LOAD_OR_UNLOAD_WRITE;
	buf[5] = 0;
	buf[6] = LobotCheckSum(buf);
	
	serial_x.write(buf, 7);
}


int LobotSerialServoReceiveHandle(HardwareSerial &serial_x, uint8_t *ret) {
	bool frameStarted = false;
	bool receiveFinished = false;
	uint8_t frameCount = 0;
	uint8_t dataCount = 0;
	uint8_t dataLength = 2;
	uint8_t rxBuf;
	uint8_t recvBuf[32];
	uint8_t i;

	while (serial_x.available()) {
		rxBuf = serial_x.read();
		delay_microseconds(100);

		if (!frameStarted) {
			if (rxBuf == LOBOT_SERVO_FRAME_HEADER) {
				frameCount++;
				if (frameCount == 2) {
					frameCount = 0;
					frameStarted = true;
					dataCount = 1;
				}
			}
			else {
				frameStarted = false;
				dataCount = 0;
				frameCount = 0;
			}
		}
		if (frameStarted) {
			recvBuf[dataCount] = (uint8_t)rxBuf;
			if (dataCount == 3) {
				dataLength = recvBuf[dataCount];
				if (dataLength < 3 || dataCount > 7) {
					dataLength = 2;
					frameStarted = false;
				}
			}
			dataCount++;
			if (dataCount == dataLength + 3) {

				if (LobotCheckSum(recvBuf) == recvBuf[dataCount - 1]) {
					frameStarted = false;
					memcpy(ret, recvBuf + 4, dataLength);
					return 1;
				}
				return -1;
			}
		}
	}
}


int16_t LobotSerialServoReadPosition(HardwareSerial &serial_x, uint8_t id) {
	fprintf(stderr, "LobotSerialServoReadPosition id:%hhu\n", id);

	int count = 10000;
	int16_t ret;
	uint8_t buf[6];

	buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
	buf[2] = id;
	buf[3] = 3;
	buf[4] = LOBOT_SERVO_POS_READ;
	buf[5] = LobotCheckSum(buf);

	// while (serial_x.available()) {
	// 	serial_x.read();
	// }

	serial_x.write(buf, 6);

	while (!serial_x.available()) {
		count -= 1;
		if (count < 0) {
			return -1;
		}
	}

	if (LobotSerialServoReceiveHandle(serial_x, buf) > 0) {
		return  BYTE_TO_HW(buf[2], buf[1]);
	}
	return -1;
}

int16_t LobotSerialServoReadVin(HardwareSerial &serial_x, uint8_t id) {
	fprintf(stderr, "LobotSerialServoReadVin id:%hhu\n", id);

	int count = 10000;
	int16_t ret;
	uint8_t buf[6];

	buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
	buf[2] = id;
	buf[3] = 3;
	buf[4] = LOBOT_SERVO_VIN_READ;
	buf[5] = LobotCheckSum(buf);

	// while (serial_x.available())
	// 	serial_x.read();

	serial_x.write(buf, 6);

	while (!serial_x.available()) {
		count -= 1;
		if (count < 0)
			return -2048;
	}

	if (LobotSerialServoReceiveHandle(serial_x, buf) > 0) {
		return BYTE_TO_HW(buf[2], buf[1]);
	} else {
		return -2048;
	}
}

// void setup() {
// 	// put your setup code here, to run once:
// 	Serial.begin(115200);

// 	pinMode(2, INPUT_PULLUP);
// 	pinMode(3, INPUT_PULLUP);

// 	delay(1000);
// }

// #define ID1	 1
// #define ID2	 2

// #define KEY1	2
// #define KEY2	3
// void loop() {
// 	// put your main code here, to run repeatedly:
// 	static bool run = false;
// 	static char step = 0;
// 	static char mode = 0;
// 	static int pos[4] = {100,200,300,400};
// 	static int pos1[4] = {100,200,300,400};
// 	uint16_t temp;
// 	while (1) {
// 		if (mode == 0)
// 		{
// 			if (run)
// 			{
// 				LobotSerialServoMove(Serial, ID1, pos[step], 500);
// 				LobotSerialServoMove(Serial, ID2, pos1[step++], 500);
// 				if (step == 4)
// 				{
// 					step = 0;
// 					run = false;
// 				}
// 				delay(1000);
// 			}
// 			if (!digitalRead(KEY2))
// 			{
// 				delay(10);
// 				if (!digitalRead(KEY2))
// 				{
// 					run = true;
// 					step = 0;
// 					delay(500);
// 				}
// 			}
// 			if (!digitalRead(KEY1))
// 			{
// 				delay(10);
// 				if (!digitalRead(KEY1))
// 				{
// 					LobotSerialServoUnload(Serial, ID1);
// 					LobotSerialServoUnload(Serial, ID2);
// 					mode = 1;
// 					step = 0;
// 					delay(500);
// 				}
// 			}
// 		}
// 		if (mode == 1)
// 		{
// 			if (!digitalRead(KEY2))
// 			{
// 				delay(10);
// 				if (!digitalRead(KEY2))
// 				{
// 					pos[step] = LobotSerialServoReadPosition(Serial, ID1);
// 					pos1[step++] = LobotSerialServoReadPosition(Serial, ID2);
// 					if (step == 4)
// 						step = 0;
// 					delay(500);
// 				}
// 			}
// 			if (!digitalRead(KEY1))
// 			{
// 				delay(10);
// 				if (!digitalRead(KEY1))
// 				{
// 					temp = LobotSerialServoReadPosition(Serial, ID1);
// 					LobotSerialServoMove(Serial, ID1, temp, 200);
// 					temp = LobotSerialServoReadPosition(Serial, ID2);
// 					LobotSerialServoMove(Serial, ID2, temp, 200);
// 					mode = 0;
// 					delay(500);
// 				}
// 			}
// 		}
// 	}
// }
