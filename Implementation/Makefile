EXE = a.out
SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp, obj/%.o, $(SRC))
DEP = $(patsubst src/%.cpp, dep/%.d, $(SRC))

# C compiler flags
CC = g++
CFLAGS = -std=c++14 -Wall -Wextra -I include/ -I libs/Gamma/ -L libs/Gamma/build/lib/
LDFLAGS =

# phony targets
.PHONY: all clean run

all: $(EXE)

clean:
	rm -rf $(EXE) $(OBJ) $(DEP) dep/ obj/

run: all
	./$(EXE)


# main executable
$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# .o file
obj/%.o: src/%.cpp obj/
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

# .d file
dep/%.d: src/%.cpp dep/
	$(CC) $(CFLAGS) -MM $< -MT $(patsubst %.cpp, %.o, $<) -MF $@

# folders
dep/:
	mkdir -p dep/

obj/:
	mkdir -p obj/
