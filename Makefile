#CROSS_COMPILE?=arm-linux-gnueabihf-
CXX=$(CROSS_COMPILE)g++

VPATH = :./src
VPATH += :./src/serial
VPATH += :./src/model
VPATH += :./src/servo_proto
VPATH += :./src/server
VPATH += :./src/utils
VPATH += :./src/tasks
VPATH += :./src/tasks/manage_servo_task
VPATH += :./src/tasks/walk_task
VPATH += :./src/tasks/save_model_angles_task
VPATH += :./src/tasks/read_servos_status

OFILES += main.o
OFILES += node.o
OFILES += leg.o
OFILES += serial.o
OFILES += servo_proto.o
OFILES += utils.o
OFILES += server.o
OFILES += manage_servo_task.o
OFILES += walk_task.o
OFILES += save_model_angles_task.o
OFILES += tasks_utils.o
OFILES += read_servos_status.o


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
