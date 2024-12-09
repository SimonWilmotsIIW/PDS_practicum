CC = g++
CFLAGS = -O3

TARGET = main_1D
 
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp
main:
	mpic++ -g -Wall -I/usr/include/lam -o main main.cpp
clean:
	$(RM) $(TARGET)
