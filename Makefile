CROSS_COMPILE?=arm-linux-gnueabihf-
CXX=$(CROSS_COMPILE)g++

VPATH = :./src

OFILES += main.o
OFILES += node.o
OFILES += leg.o
OFILES += hardware_serial.o
OFILES += serial_servo_rp.o
OFILES += utils.o
OFILES += server.o


CPPFLAGS = -O3 -I  -Wall
LDFLAGS = -ldl

OFILES_PATH=$(addprefix obj/,$(OFILES))


all:  obj $(OFILES_PATH)
	@$(CXX) $(LDFLAGS) -o test $(OFILES_PATH)

obj:
	@mkdir obj

clean:
	@rm -rf obj test

obj/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@
