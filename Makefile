assembly: assembly.h assembly.cpp
	g++ assembly.cpp -c
CFS: CFS.h CFS.cpp
	g++ CFS.cpp -c
BTB: BTB.h BTB.cpp
	g++ BTB.cpp -c
Tomasulo: Tomasulo.h Tomasulo.cpp
	g++ Tomasulo.cpp -c
all: main.cpp CFS.o BTB.o assembly.o Tomasulo.o
	g++ main.cpp CFS.o BTB.o assembly.o Tomasulo.o
clean: *.out *.o mid.txt out.txt
	rm *.out *.o mid.txt out.txt
