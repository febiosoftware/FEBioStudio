CCFLAGS = -g -I../ -O3 -std=c++11
CCFLAGS += -I../
DEF = -DLINUX
LIBDIR = $(notdir $(CURDIR))
POSTLIB = $(shell echo $(LIBDIR) | tr A-Z a-z)

SRC = $(wildcard *.cpp)
DEP = $(wildcard *.h)
OBJ = $(SRC:.cpp=.o)

LIB = ../build/lib/lib$(POSTLIB).a

$(LIB): $(OBJ)
	ar crs -o $(LIB) $(OBJ)

%.o: %.cpp $(DEP)
	$(CC) -c $(DEF) $(CCFLAGS) -o $@ $<

clean:
	$(RM) *.o *.d $(LIB)
