CROSS_COMPILE?=arm-linux-gnueabihf-
CXX=$(CROSS_COMPILE)g++

VPATH = :./src

OFILES += main.o
OFILES += node.o
OFILES += leg.o

CPPFLAGS = -g -I -g2 -Wall
LDFLAGS = -ldl

OFILES_PATH=$(addprefix obj/,$(OFILES))


all: obj  $(OFILES_PATH)
	@$(CXX) $(LDFLAGS) -o test $(OFILES_PATH)

obj:
	@mkdir obj

clean:
	@rm -rf obj test

obj/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@
