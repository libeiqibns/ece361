#the compiler: gcc for C program, define as g++ for C++
CC = gcc

#compiler flags:
#-g adds debugging information to the executable file
#-Wall turns on most, but not all, compiler warnings
CFLAGS = -g	-Wall	-Werror	-lm

#the build target executable:
TARGET = client

all:$(TARGET)

$(TARGET):$(TARGET).c $(TARGET).h
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET) *~
