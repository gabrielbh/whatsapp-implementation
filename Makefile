CC= g++
CFLAGS=   -Wvla -c -std=c++17 -Wextra -g -Wall


#All Target

all: whatsappClient whatsappServer



#Executeabla File


whatsappClient: whatsappClient.o whatsappio.o
	$(CC)  whatsappClient.o whatsappio.o  -o whatsappClient

whatsappServer: whatsappServer.o whatsappio.o
	$(CC)  whatsappServer.o whatsappio.o  -o whatsappServer


#Object File
whatsappServer.o: whatsappServer.cpp whatsappServer.h
	$(CC) $(CFLAGS) whatsappServer.cpp -o whatsappServer.o

whatsappClient.o: whatsappClient.cpp whatsappClient.h
	$(CC) $(CFLAGS) whatsappClient.cpp -o whatsappClient.o

whatsappio.o: whatsappio.cpp whatsappio.h
	$(CC) $(CFLAGS) whatsappio.cpp -o whatsappio.o



#Other Target

tar:
	tar -cvf ex4.tar whatsappServer.cpp whatsappio.cpp whatsappio.h Makefile whatsappClient.cpp
	whatsappClient.h whatsappServer.h README



clean:
	$(RM) -f *.o
	$(RM) whatsappServer
	$(RM) whatsappClient


.PHONY: clean