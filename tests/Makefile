CFLAGS					=	-std=c++17 -Wfatal-errors -Wall -pedantic -Werror -O3 -march=native -flto -ggdb
INCLUDE_PATH			=	-I../include
PTL_INCLUDE_PATH		=	-I../dep/ptl/include
PCL_INCLUDE_PATH		=	-I../dep/pcl/include
PFL_INCLUDE_PATH		=	
LIBS					=	-lpthread ../libphilchess.a 

SRCS					=	*.cpp

TARGETS					=	$(patsubst %.cpp,%,$(wildcard $(SRCS)))

all: $(TARGETS)

%: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDE_PATH) $(PFL_INCLUDE_PATH) $(PTL_INCLUDE_PATH) $(PCL_INCLUDE_PATH) $< -o $@ $(LIBS)
	
	
clean:
	rm -f $(TARGETS)

.PHONY: clean
