CC = g++
CFLAGS = -O3

TARGET = main_1D
 
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp
main:
	mpic++ -g -Wall -I/usr/include/lam -o mpi_v2_1d vector_mpi_2.cpp
clean:
	$(RM) $(TARGET)
