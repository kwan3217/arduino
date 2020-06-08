test_gprmc: main.o gprmc.o StubArduino.o
	g++ -g -Og -o $@ $^

CPPFLAGS=-I../src/ -I.

main.o: main.cpp
	g++ -g -Og -c $(CPPFLAGS) -o $@ $^

gprmc.o: ../src/gprmc.cpp
	g++ -g -Og -c $(CPPFLAGS) -o $@ $^

StubArduino.o: StubArduino.cpp
	g++ -g -Og -c $(CPPFLAGS) -o $@ $^


