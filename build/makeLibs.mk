CCFLAGS = -I../ -O3 -std=c++11
CCFLAGS += -I../
CCFLAGS += -I/opt/netgen/include
CCFLAGS += -I/opt/netgen/include/include
CCFLAGS += -I/usr/local/include/opencascade/
DEF = -DLINUX -DHAS_NETGEN -DHAS_OCC
LIBDIR = $(notdir $(CURDIR))
CURLIB = $(shell echo $(LIBDIR) | tr A-Z a-z)

SRC = $(wildcard *.cpp)
DEP = $(wildcard *.h)
OBJ = $(SRC:.cpp=.o)

LIB = ../build/lib/lib$(CURLIB).a

$(LIB): $(OBJ)
	ar crs -o $(LIB) $(OBJ)

%.o: %.cpp $(DEP)
	$(CC) -c $(DEF) $(CCFLAGS) -o $@ $<

clean:
	$(RM) *.o *.d $(LIB)

