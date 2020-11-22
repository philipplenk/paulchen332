TARGET					=	paulchen332
CFLAGS					=	-Wfatal-errors -std=c++17 -Wall -pedantic -Werror -O3 -march=native -ggdb -flto
INCLUDE_PATH			=	-Iinclude
PTL_INCLUDE_PATH		=	-Idep/ptl/include
PCL_INCLUDE_PATH		=	-Idep/pcl/include
LIBS					=	-lpthread

SRCS					=	src/*.cpp src/engine/*.cpp src/utility/*.cpp src/eval/*.cpp
OBJS					=	$(patsubst src/%.cpp,src/%.o,$(wildcard $(SRCS)))
LIBOBJS					=	$(filter-out src/main.o, $(OBJS))

all: $(OBJS) libphilchess.a
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)
	
src/%.o: src/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE_PATH) $(PTL_INCLUDE_PATH) $(PCL_INCLUDE_PATH) -c $< -o $@
	
libphilchess.a: $(LIBOBJS)
	ar rcs $@ $^
	ranlib $@

tests: libphilchess.a
	$(MAKE) -C tests

clean:
	rm -f $(TARGET) $(OBJS) libphilchess.a

.PHONY: clean tests
