all:
	g++ -std=c++11 -O3 -g -Wall -fmessage-length=0 -o three three.cpp
clean:
	rm three
