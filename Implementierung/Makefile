# all executables
ALL = 2009_DaPlSt 2011_PlRoSt playground
# all .cpp source files
SRC = $(wildcard src/*.cpp src/*/*.cpp)
# all .cpp files with a main function
SRC_MAIN = $(wildcard src/main.cpp src/*/main.cpp)
# all .o object files, except the main ones
OBJ = $(patsubst src/%.cpp, obj/%.o, $(filter-out $(SRC_MAIN), $(SRC)))
# all .d files (they store build dependency lists)
DEP = $(patsubst src/%.cpp, dep/%.d, $(SRC))

# compiler executable
CC = g++
# directories to search for .h-files in
INCLUDE = include/ libs/Gamma/ libs/simple2d/include/ /usr/include/SDL2/
# directories to search for .a-file libraries in
LIBS = libs/Gamma/build/lib/ libs/simple2d/build/
# compiler flags
CFLAGS = -std=c++14 $(addprefix -I , $(INCLUDE)) $(addprefix -L , $(LIBS)) -D_REENTRANT -pthread
# linker flags
LDFLAGS = -lGamma -lportaudio -lsimple2d -lSDL2 -lGL -lm -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lpthread


# phony targets
.PHONY: all clean run

all: $(ALL)

clean:
	rm -rf $(ALL) dep/ obj/

run: 2011_PlRoSt
	./2011_PlRoSt

# executables
2009_DaPlSt: obj/2009_DaPlSt/main.o
2011_PlRoSt: obj/2011_PlRoSt/main.o
playground: obj/main.o

$(ALL): $(OBJ)
	@echo -e 'LD' $@
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# .o files
.SECONDEXPANSION:
obj/%.o: src/%.cpp | $$(dir obj/%.o)
	@echo 'CC' $<
	@$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

# .d files
.SECONDEXPANSION:
dep/%.d: src/%.cpp | $$(dir dep/%.d)
	@echo 'CC -M' $<
	@$(CC) $(CFLAGS) -MM $< -MT $(patsubst src/%.cpp, obj/%.o, $<) -MF $@

# folders
%/:
	mkdir -p $@
