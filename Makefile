#compiler
CC = gcc
#flags
CFLAGS = -O -Wall
#target
TARGET = myshell
MAIN = main

all:	$(TARGET)
$(TARGET):	$(MAIN).c
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN).c 
clean:	
	$(RM) $(TARGET)