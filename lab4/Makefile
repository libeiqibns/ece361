#the compiler: gcc for C program, define as g++ for C++
CC = gcc

#compiler flags:
#-g adds debugging information to the executable file
#-Wall turns on most, but not all, compiler warnings
CFLAGS = -g	-Wall	-Werror	-lm	-pthread

#the build target executable:
TARGET = chatserver	chatclient

all:$(TARGET)

chatclient:chatclient.c chatclient.h
	$(CC) $(CFLAGS) -o chatclient chatclient.c

chatserver:chatserver.c chatserver.h lookup_table.c lookup_table.h
	$(CC) $(CFLAGS) -o chatserver chatserver.c lookup_table.c

clean:
	$(RM) $(TARGET) *~
