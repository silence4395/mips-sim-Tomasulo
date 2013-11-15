CFS: CFS.h CFS.cpp
	g++ CFS.cpp -c
BTB: BTB.h BTB.cpp
	g++ BTB.cpp -c
Tomasulo: Tomasulo.h Tomasulo.cpp
	g++ Tomasulo.cpp -c
all: main.cpp CFS.o BTB.o Tomasulo.o
	g++ main.cpp CFS.o BTB.o Tomasulo.o
clean: *.out *.o
	rm *.out *.o
