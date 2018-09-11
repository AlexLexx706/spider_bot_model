CROSS_COMPILE?=arm-linux-gnueabihf-
CXX=$(CROSS_COMPILE)g++

VPATH = :./src
VPATH += :./src/hardware_serial
VPATH += :./src/model
VPATH += :./src/serial_servo_rp
VPATH += :./src/server
VPATH += :./src/utils
VPATH += :./src/tasks/manage_servo_id_task

OFILES += main.o
OFILES += node.o
OFILES += leg.o
OFILES += hardware_serial.o
OFILES += serial_servo_rp.o
OFILES += utils.o
OFILES += server.o
OFILES += manage_servo_id_task.o

CPPFLAGS = -O3 -I  -Wall
LDFLAGS = -ldl

OFILES_PATH=$(addprefix obj/,$(OFILES))
INCLUDE=$(subst :, -I./,$(VPATH))


all:  obj $(OFILES_PATH)
	$(CXX) $(INCLUDE) $(LDFLAGS)  -o test $(OFILES_PATH)

obj:
	mkdir obj

clean:
	rm -rf obj test

obj/%.o: %.cpp
	$(CXX) $(INCLUDE) $(CPPFLAGS) -c $< -o $@
