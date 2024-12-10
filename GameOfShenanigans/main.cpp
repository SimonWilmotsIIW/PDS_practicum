#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <iomanip>
#include <vector>
#include <mpi.h>
#include "mpi_timer.cpp"

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"
#define GRID_SIZE 32
#define ITERATION_COUNT 32
// #define MAX_ITERATIONS 20
//view influence of more cores/threads on VSC
//Why do we even use MPI?

void printGrid(const std::vector<char>& grid, int xsize, int ysize);
void determineState(std::vector<char>& grid, int xsize, int ysize);
void clearScreen(void);
void printGridToFile(const std::vector<char>& grid, int xsize, int ysize, const std::string& filename);
std::vector<char> initializeGrid(int size);
inline int getIndex(int x, int y, int xsize, int ysize);

int main(int argc, char* argv[]) {
    int global_comm_sz;
    int comm_sz;
    int my_rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    for(int EXTRA = 0; EXTRA < 8; EXTRA++){
        // int EXTRA = 1;
        int MAX_ITERATIONS = 20;
        MAX_ITERATIONS = MAX_ITERATIONS/(EXTRA+1);
        
        if( GRID_SIZE %(comm_sz-1) != 0){
            return 1;
        }

        int rowcount_per_mpi = GRID_SIZE /(comm_sz-1);
        // nr of elements per row
        int y_size = GRID_SIZE;
        // relevant rows
        int x_size = rowcount_per_mpi;
        // main that sends to side with buffer
        int elements_received_count = (rowcount_per_mpi+2) * y_size; //+2 for 1 buffer on each side

        if(my_rank==0){
            // clearScreen();
            AutoAverageTimer mainTimer("main_program_timer");

            //std::cout << COLOR_BLUE;

            auto grid = initializeGrid(GRID_SIZE);
            int x, y;

            std::string number_of_cells;
            std::string start;
            std::string filename;
            for (int iteration=0; iteration < ITERATION_COUNT; iteration++){
                if (argc == 2) {
                    filename = argv[1];
                    std::ifstream readfile(filename);
                    if (readfile.is_open()) {
                        std::string fileline, xx, yy;
                        while (std::getline(readfile, fileline)) {
                            std::stringstream ss(fileline);
                            std::getline(ss, xx, ' ');
                            std::getline(ss, yy, ' ');

                            x = std::stoi(xx);
                            y = std::stoi(yy);

                            grid[getIndex(x, y, GRID_SIZE, GRID_SIZE)] = '1';
                        }
                    }
                    start = "y";
                } else {
                    std::cout << "Enter the number of cells, or 'r' to read cells from file: ";
                    std::cin >> number_of_cells;
                    std::cout << std::endl;

                    if (number_of_cells == "r") {
                        while (true) {
                            std::cout << "Enter name of file to read from: ";
                            std::cin >> filename;

                            std::ifstream readfile(filename);
                            if (readfile.is_open()) {
                                std::string fileline, xx, yy;
                                while (std::getline(readfile, fileline)) {
                                    std::stringstream ss(fileline);
                                    std::getline(ss, xx, ' ');
                                    std::getline(ss, yy, ' ');

                                    x = std::stoi(xx);
                                    y = std::stoi(yy);

                                    grid[getIndex(x, y, GRID_SIZE, GRID_SIZE)] = '1';
                                }
                                break;
                            } else {
                                std::cout << "File not found :(" << std::endl;
                            }
                        }
                    } else {
                        for (int i = 0; i < std::stoi(number_of_cells); i++) {
                            std::cout << "Cell " << i + 1 << "/" << number_of_cells << "\nEnter the x & y coordinates: ";
                            std::cin >> x >> y;
                            grid[getIndex(x, y, GRID_SIZE, GRID_SIZE)] = '1';
                            //printGrid(grid, GRID_SIZE, GRID_SIZE);
                        }
                    }
                    std::cout << "Grid setup is done. Start the game? (y/n): ";
                    std::cin >> start;
                }

                int incrementor = 0;
                if (start == "y" || start == "Y") {
                    printGridToFile(grid, GRID_SIZE, GRID_SIZE, "./outputs/output_grid_000000");
                    int iteration = 0;
                    mainTimer.start();
                    while (iteration < MAX_ITERATIONS) {
                        // std::cout <<"---------------------------------------------\n"<<"---------------------------------------------\n"<<"---------------------------------------------\n"<<"ITERATION: "<< iteration << std::endl;
                        iteration +=1;
                        // printGrid(grid, GRID_SIZE, GRID_SIZE);
                        for (int rank = 1; rank < comm_sz; ++rank) {
                            //1 extra buffer row at start
                            int startRow = (rank - 1) * rowcount_per_mpi -EXTRA;
                            //1 extra buffer row at end
                            int endRow = rank * rowcount_per_mpi +EXTRA;

                            int prevRow = (startRow - 1 + GRID_SIZE) % GRID_SIZE;
                            int nextRow = (endRow + GRID_SIZE) % GRID_SIZE;

                            std::vector<char> localGrid(elements_received_count + (2*EXTRA * y_size));

                            for (int i = 0; i < x_size + 2 + (2*EXTRA); ++i) {
                                // int sourceRow = (prevRow + i) % GRID_SIZE;
                                int startIdx = getIndex(prevRow+i, 0, GRID_SIZE, GRID_SIZE);
                                int targetIdx = i * y_size;

                                std::copy(
                                    grid.begin() + startIdx, 
                                    grid.begin() + startIdx + y_size, 
                                    localGrid.begin() + targetIdx
                                );
                            }

                            // I think this should work. I hope this works. If this doesn't work I give up
                            //MPI_Send(localGrid.data(), localGrid.size(), MPI_CHAR, rank, 0, MPI_COMM_WORLD);
                            MPI_Send(localGrid.data(), elements_received_count+ (2*EXTRA * y_size), MPI_CHAR, rank, 0, MPI_COMM_WORLD);

                        }
                        MPI_Barrier(MPI_COMM_WORLD);

                        for (int rank = 1; rank < comm_sz; ++rank) {
                            int start = getIndex((rank - 1) * (rowcount_per_mpi), 0, GRID_SIZE, GRID_SIZE);
                            int end = getIndex(((rank) * rowcount_per_mpi) -1, GRID_SIZE-1, GRID_SIZE, GRID_SIZE);

                            //OLD : //MPI_Recv(grid.data() + start, end - start, MPI_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            MPI_Recv(grid.data() + start, (x_size * y_size), MPI_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                        }
                        MPI_Barrier(MPI_COMM_WORLD);

                        // std::ostringstream filenameStream;
                        // filenameStream << "./outputs/output_grid_" << std::setw(4) << std::setfill('0') << incrementor << ".txt";
                        // std::string outputFilename = filenameStream.str();
                        // printGridToFile(grid, GRID_SIZE, GRID_SIZE, outputFilename);
                        // incrementor++;

                        // usleep(200000);
                        // clearScreen();
                    }
                    mainTimer.stop();
                    std::ostringstream filenameStream;
                    filenameStream << "./outputs/output_grid_final" << ".txt";
                    std::string outputFilename = filenameStream.str();
                    printGridToFile(grid, GRID_SIZE, GRID_SIZE, outputFilename);
                } else {
                    return 0;
                }
            }
        }else{
            for (int it=0; it < ITERATION_COUNT; it++){

                int iteration = 0;
                while (iteration < MAX_ITERATIONS){
                    iteration +=1;
                    // usleep(200000 * my_rank);

                    std::vector<char> localGrid(elements_received_count+ (2*EXTRA * y_size));

                    MPI_Recv(localGrid.data(), elements_received_count+ (2*EXTRA * y_size), MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    int local_extra = EXTRA;
                    while (local_extra >0){
                        determineState(localGrid, x_size + (2*local_extra), y_size);
                        localGrid.erase(localGrid.begin(), localGrid.begin() + y_size);
                        local_extra -=1;
                    }
                    determineState(localGrid, x_size, y_size);

                    MPI_Barrier(MPI_COMM_WORLD);

                    // To check:
                    MPI_Send(localGrid.data() + y_size, (x_size * y_size), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                    MPI_Barrier(MPI_COMM_WORLD);

                }
            }
        }
    }
    MPI_Finalize();
    return 0;
}

std::vector<char> initializeGrid(int size) {
    return std::vector<char>((size) * (size), '0');
}

inline int getIndex(int x, int y, int xsize, int ysize) {
    //std::cout << "getIndex(" << x << ", " << y << ") " << xsize << " & " << ysize << "= " << (((x + xsize) % xsize) * ysize) + ((y + ysize) % ysize) << std::endl;
    return (((x + xsize) % xsize) * ysize) + ((y + ysize) % ysize);
}

void clearScreen(void) {
    //working on Ubuntu (22.04) and Arch (6.11.8-arch1-2)
    //not working on Windows 11 atm
    // pos on POSIX systems
    std::cout << "\033[2J;" << "\033[1;1H"; // clears screen and moves cursor to home
}

void printGrid(const std::vector<char>& grid, int xsize, int ysize) {
    for (int a = 0; a < xsize; a++) {
        for (int b = 0; b < ysize; b++) {
            std::cout << (grid[getIndex(a, b, xsize, ysize)]);
        }
        std::cout << std::endl;
    }
}

void printGridToFile(const std::vector<char>& grid, int xsize, int ysize, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    for (int a = 0; a < xsize; a++) {
        for (int b = 0; b < ysize; b++) {
            outFile << grid[getIndex(a, b, xsize, ysize)];
        }
        outFile << std::endl;
    }
    outFile.close();
}

void determineState(std::vector<char>& returnGrid, int xsize, int ysize) {
    //copy constructor of std::vector == deep copy
    std::vector<char> dataGrid{returnGrid};

    for (int a = 1; a < xsize+1; a++) {
        for (int b = 0; b < ysize; b++) {
            int alive = 0;
            for (int c = -1; c <= 1; c++) {
                for (int d = -1; d <= 1; d++) {
                    if (!(c == 0 && d == 0)) {
                        alive += (dataGrid[getIndex(a + c, b + d, xsize+2, ysize)] == '1');
                    }
                }
            }

            if (alive < 2 || alive > 3) {
                // cell dies
                returnGrid[getIndex(a, b, xsize+2, ysize)] = '0';
            } else if (alive == 3) {
                // cell born
                returnGrid[getIndex(a, b, xsize+2, ysize)] = '1';
            } else {
                // cell survives -> do nothing
                // can this just drop?
                returnGrid[getIndex(a, b, xsize, ysize)] = dataGrid[getIndex(a, b, xsize, ysize)];
            }
        }
    }
}
