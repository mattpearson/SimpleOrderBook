#HEADERS = Book.h Enums.h OrderEvents.h Order.h

default: main

main.o: main.cpp $(HEADERS)
	g++ -O3 -g0 -Wall -c -fmessage-length=0 -MMD -MP -L/usr/local/lib -std=c++11 -o main.o  main.cpp

main: main.o
	g++ main.o -o main -lboost_unit_test_framework 

clean:
	-rm -f main.o
	-rm -f main
