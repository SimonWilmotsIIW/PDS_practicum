CC = g++
CFLAGS = -O3

TARGET = main_1D
 
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp
refactor:
	mpic++ -g -Wall -I/usr/include/lam -o main_refactored main_refactored.cpp
mpi:
	mpic++ -g -Wall -I/usr/include/lam -o mpi_main main_for_mpi.cpp
mpi_1d:
	mpic++ -g -Wall -I/usr/include/lam -o mpi_1d main_vector_mpi.cpp
main:
	mpic++ -g -Wall -I/usr/include/lam -o mpi_v2_1d vector_mpi_2.cpp
clean:
	$(RM) $(TARGET)
