# all executables
ALL = 2001_ChNaUpVe 2009_DaPlSt 2011_PlRoSt test

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
INCLUDE = include/ libs/Gamma/ libs/simple2d/include/

# compiler flags
CFLAGS = -std=c++14 $(addprefix -I , $(INCLUDE)) -L libs/ -D_REENTRANT -pthread

# linker flags
LDFLAGS = -lGamma -lportaudio -lsimple2d -lSDL2 -lGL -lm -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lpthread



# phony targets
.PHONY: all clean run

all: $(ALL)

clean:
	$(MAKE) clean -C libs/Gamma/
	$(MAKE) clean -C libs/simple2d/
	rm libs/libGamma.a libs/libsimple2d.a
	rm -rf $(ALL) dep/ obj/

run: test
	./test

# executables
2001_ChNaUpVe: obj/2001_ChNaUpVe/main.o
2009_DaPlSt: obj/2009_DaPlSt/main.o
2011_PlRoSt: obj/2011_PlRoSt/main.o
test: obj/main.o

$(ALL): $(OBJ) libs/libGamma.a libs/libsimple2d.a
	@echo -e 'LD' $@
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# libs
libs/libGamma.a:
	$(MAKE) -C libs/Gamma/
	cp libs/Gamma/build/lib/libGamma.a libs/

libs/libsimple2d.a:
	$(MAKE) -C libs/simple2d
	cp libs/simple2d/build/libsimple2d.a libs/


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
