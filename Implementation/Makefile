EXE = a.out
SRC = $(wildcard src/*.cpp src/*/*.cpp)
OBJ = $(patsubst src/%.cpp, obj/%.o, $(SRC))
DEP = $(patsubst src/%.cpp, dep/%.d, $(SRC))

# TODO: fix Makefile, so that only file that have changed get recompiled

# C compiler flags
CC = g++
CFLAGS = -std=c++14 -I include/ -I libs/Gamma/ -L libs/Gamma/build/lib/
# might also need those in the future: -lrt -lasound -ljack -lpthread
LDFLAGS = -lGamma -lportaudio


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
