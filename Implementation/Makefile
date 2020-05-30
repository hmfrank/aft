EXE = a.out
SRC = $(wildcard src/*.cpp src/*/*.cpp)
OBJ = $(patsubst src/%.cpp, obj/%.o, $(SRC))
DEP = $(patsubst src/%.cpp, dep/%.d, $(SRC))

# TODO: fix Makefile, so that only file that have changed get recompiled

# C compiler flags
CC = g++
INCLUDE = include/ libs/Gamma/ libs/simple2d/include/ /usr/include/SDL2/
LIBS = libs/Gamma/build/lib/ libs/simple2d/build/
CFLAGS = -std=c++14 $(addprefix -I , $(INCLUDE)) $(addprefix -L , $(LIBS)) -D_REENTRANT
# might also need those in the future: -lrt -lasound -ljack -lpthread
LDFLAGS = -lGamma -lportaudio -lsimple2d -lSDL2 -lGL -lm -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lpthread


# phony targets
.PHONY: all clean run

all: $(EXE)

clean:
	rm -rf $(EXE) dep/ obj/

run: all
	./$(EXE)


# main executable
$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# .o file
.SECONDEXPANSION:
obj/%.o: src/%.cpp $$(dir obj/%.o)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

# .d file
.SECONDEXPANSION:
dep/%.d: src/%.cpp $$(dir dep/%.d)
	$(CC) $(CFLAGS) -MM $< -MT $(patsubst %.cpp, %.o, $<) -MF $@

# folders
%/:
	mkdir -p $@
