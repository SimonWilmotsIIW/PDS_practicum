CC = g++
CFLAGS = -O3

TARGET = main_vector
 
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp
refactor:
	mpic++ -g -Wall -I/usr/include/lam -o main_refactored main_refactored.cpp
mpi:
	mpic++ -g -Wall -I/usr/include/lam -o mpi_main main_for_mpi.cpp
helper:
	mpic++ -g -Wall -I/usr/include/lam -o mpi_test helper.cpp
clean:
	$(RM) $(TARGET)
