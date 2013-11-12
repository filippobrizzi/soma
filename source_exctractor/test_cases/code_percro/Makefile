
SOURCES=Cloth.cpp pcg.cpp spmv.cpp Triangle.cpp main.cpp
CC=/opt/local/bin/g++-mp-4.8
CFLAGS=-DMACH -O4 -I/usr/local/include -mtune=native -DEIGEN_NO_DEBUG -I. -I/opt/local/include
FRAMEWORK=
LIBS=-L/opt/local/lib -lgomp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main
DEP=$(SOURCES:.cpp=.d)
-include $(DEP)

%.o: %.cpp
	$(CC) -MMD -MP -c $(CFLAGS) $< -o $@ 

main: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) $(FRAMEWORK) -o $@
