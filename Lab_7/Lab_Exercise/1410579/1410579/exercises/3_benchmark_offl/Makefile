CXX = icpc
CXXFLAGS = -g

OBJECTS = offload.o

.SUFFIXES: .o .cpp

.cpp.o:
        $(CXX) -c $(CXXFLAGS) -o "$@" "$<"

all: runme

runme: $(OBJECTS)
       $(CXX) -fopenmp $(CXXFLAGS) -o runme $(OBJECTS)

clean:
       rm -f *.o runme

