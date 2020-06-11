CCFLAGS = -g -O0 -std=c++14
CCFLAGS += -I../
CCFLAGS += -I/opt/netgen/include
CCFLAGS += -I/opt/netgen/include/include
CCFLAGS += -I/usr/local/include/opencascade/
DEF = -DLINUX -DHAS_QUAZIP -DHAS_SSH -DHAS_NETGEN -DHAS_OCC -DTETLIBRARY
LIBDIR = $(notdir $(CURDIR))
CURLIB = $(shell echo $(LIBDIR) | tr A-Z a-z)

SRC = $(wildcard *.cpp)
DEP = $(wildcard *.h)
OBJ = $(SRC:.cpp=.o)

LIB = ../build/lib/lib$(CURLIB).a

$(LIB): $(OBJ)
	ar crs -o $(LIB) $(OBJ)

%.o: %.cpp $(DEP)
	g++ -c $(DEF) $(CCFLAGS) -o $@ $<

clean:
	$(RM) *.o *.d $(LIB)

