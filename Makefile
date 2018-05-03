CXX = g++
CXXFLAGS = -Wall

Driver: Sally.o Driver.cpp
	$(CXX) $(CXXFLAGS) Sally.o Driver.cpp -o Driver

Sally.o: Sally.cpp Sally.h
	$(CXX) $(CXXFLAGS) -c Sally.cpp
clean:
	rm *.o

run:
	./Driver