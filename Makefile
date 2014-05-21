CXX=		/usr/local/bin/g++-4.9

CPPFLAGS=	-I/opt/libs/boost-trunk \
			-I/opt/libs/cplex/include \
			-I/opt/libs/concert/include \
			-I/opt/libs/linenoise \
			-I. \
			-std=c++11 -O3 -DIL_STD -DNDEBUG -m64

LDFLAGS=	-L/opt/libs/cplex/lib/x86-64_darwin/static_pic \
			-L/opt/libs/concert/lib/x86-64_darwin/static_pic \
			-L/opt/libs/linenoise
					
LDLIBS=		-lilocplex -lconcert -lcplex -lm -llinenoise

OBJS=		network/graph.o \
			parser/data.o \
			parser/parser.o \
			solver/mip_solver.o \
			solver/greedy_solver.o \
			solver/heuristic_solver.o \
			solver/insertion_heuristic_solver.o \
			solver/label.o \
			solver/labelling_solver.o \
			cli/program.o

main: $(OBJS) main.cpp
	$(CXX) $(CPPFLAGS) $(LDFLAGS) $(OBJS) -o tsppddl $(LDLIBS) main.cpp

all: main

clean:
	rm $(OBJS)