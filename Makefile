CC = g++
CCFLAGS = -O2

all: main.cpp mywebsrv.o
	$(CC) $(CCFLAGS) -lpthread main.cpp mywebsrv.o -o mywebserver

mywebsrv.o: mywebsrv.cpp
	$(CC) $(CCFLAGS) -c mywebsrv.cpp

clean:
	rm -f *.o mywebserver
