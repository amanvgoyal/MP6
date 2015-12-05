# makefile

all: dataserver client 

NetworkRequestChannel.o: NetworkRequestChannel.h NetworkRequestChannel.cpp
	g++-4.7 -c -g NetworkRequestChannel.cpp

reqchannel.o: reqchannel.h reqchannel.cpp
	g++-4.7 -c -g reqchannel.cpp

semaphore.o: semaphore.h semaphore.cpp
	g++-4.7 -std=c++11 -c -g semaphore.cpp

BoundedBuffer.o: BoundedBuffer.h BoundedBuffer.cpp
	g++-4.7 -c -g BoundedBuffer.cpp

dataserver: dataserver.cpp reqchannel.o NetworkRequestChannel.o
	g++-4.7 -g -o dataserver dataserver.cpp reqchannel.o NetworkRequestChannel.o -lpthread

client: client.cpp reqchannel.o NetworkRequestChannel.o semaphore.o BoundedBuffer.o
	g++-4.7 -std=c++11 -pthread -g -o client client.cpp reqchannel.o NetworkRequestChannel.o semaphore.o BoundedBuffer.o

#client: cclient.cpp reqchannel.o semaphore.o BoundedBuffer.o
#	g++-4.7 -std=c++11 -g -o client -Wno-write-strings cclient.cpp reqchannel.o semaphore.o BoundedBuffer.o
