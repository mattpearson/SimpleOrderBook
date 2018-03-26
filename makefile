#HEADERS = Book.h Enums.h OrderEvents.h Order.h

default: HCTechHomework

HCTechHomework.o: HCTechHomework.cpp $(HEADERS)
	g++ -O3 -g0 -Wall -c -fmessage-length=0 -MMD -MP -L/usr/local/lib -std=c++11 -o HCTechHomework.o  HCTechHomework.cpp

HCTechHomework: HCTechHomework.o
	g++ HCTechHomework.o -o HCTechHomework -lboost_unit_test_framework 

clean:
	-rm -f HCTechHomework.o
	-rm -f HCTechHomework
