EXE = a.out
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
DEP = $(patsubst %.cpp, %.d, $(SRC))

# C compiler flags
CC = g++
CFLAGS = -std=c++14 -Wall -Wextra -Werror -I libs/Gamma/ -L libs/Gamma/build/lib/
LDFLAGS =

# phony targets
.PHONY: all clean run

all: $(EXE)

clean:
	rm -rf $(EXE) $(OBJ) $(DEP)

run: all
	./$(EXE)


# main executable
$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# .o file
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

# .d file
%.d: %.cpp
	$(CC) $(CFLAGS) -MM $< -MT $(patsubst %.cpp, %.o, $<) -MF $@
