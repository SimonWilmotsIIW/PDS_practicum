#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <iomanip>
#include <vector>
#include <mpi.h>

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

#define GRID_SIZE 32
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

    if( GRID_SIZE %(comm_sz-1) != 0){
        return 1;
    }

    int rowcount_per_mpi = GRID_SIZE /(comm_sz-1);
    int y_size = GRID_SIZE;
    int x_size = rowcount_per_mpi;
    int elements_received_count = (rowcount_per_mpi+2) * y_size; //+2 for 1 buffer on each side

    if(my_rank==0){
        clearScreen();

        std::cout << COLOR_BLUE;

        auto grid = initializeGrid(GRID_SIZE);
        int x, y;

        std::string number_of_cells;
        std::string start;
        std::string filename;

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
                    printGrid(grid, GRID_SIZE, GRID_SIZE);
                }
            }
            std::cout << "Grid setup is done. Start the game? (y/n): ";
            std::cin >> start;
        }

        int incrementor = 0;
        if (start == "y" || start == "Y") {
            printGridToFile(grid, GRID_SIZE, GRID_SIZE, "./outputs/output_grid_000000");
            int iteration = 0;
            while (true) {
                std::cout <<"---------------------------------------------\n"<<"---------------------------------------------\n"<<"---------------------------------------------\n"<<"ITERATION: "<< iteration << std::endl;
                iteration +=1;
                printGrid(grid, GRID_SIZE, GRID_SIZE);
                for (int rank = 1; rank < comm_sz; ++rank) {
                    int startRow = (rank - 1) * rowcount_per_mpi;
                    int endRow = rank * rowcount_per_mpi;

                    int prevRow = (startRow - 1 + GRID_SIZE) % GRID_SIZE;
                    int nextRow = (endRow + GRID_SIZE) % GRID_SIZE;

                    std::vector<char> localGrid(elements_received_count);

                    for (int i = 0; i < elements_received_count + 2; ++i) {
                        // int sourceRow = (prevRow + i) % GRID_SIZE;
                        int startIdx = getIndex(prevRow+i, 0, GRID_SIZE, GRID_SIZE);
                        int targetIdx = i * y_size;

                        std::copy(
                            grid.begin() + startIdx, 
                            grid.begin() + startIdx + y_size, 
                            localGrid.begin() + targetIdx
                        );
                    }
                    std::cout << "Sending from Process Rank: " << my_rank << "\n"
                    << " Name: elements expected count Value: " << localGrid.size() << "\n"
                    << " Name: end row: " << nextRow << "\n"
                    << " Name: start row: " << startRow << "\n"
                    << " Name: prevRow: " << prevRow << "\n"
                    << " Name: nextRow: " << nextRow << "\n"

                    << "---------------------------------------------" << std::endl;
                    // I think this should work. I hope this works. If this doesn't work I give up
                    MPI_Send(localGrid.data(), localGrid.size(), MPI_CHAR, rank, 0, MPI_COMM_WORLD);
                    std::cout << "Sent" << std::endl;

                }
                MPI_Barrier(MPI_COMM_WORLD);

                for (int rank = 1; rank < comm_sz; ++rank) {
                    int start = getIndex((rank - 1) * (rowcount_per_mpi), 0, GRID_SIZE, GRID_SIZE);
                    int end = getIndex(((rank) * rowcount_per_mpi) -1, GRID_SIZE-1, GRID_SIZE, GRID_SIZE);
                    // std::cout << "Process Rank: " << my_rank << "\n"
                    // << " Name: elements expected count Value: " << end - start << "\n"
                    // << " Name: end Value: " << end << "\n"
                    // << " Name: start Value: " << start << "\n"
                    // << " Name: elements_rec_count: " << elements_received_count << "\n"
                    // << " Name: rowcount_per_mpi: " << rowcount_per_mpi << "\n"

                    // << "---------------------------------------------" << std::endl;
                    MPI_Recv(grid.data() + start, end - start, MPI_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                }
                MPI_Barrier(MPI_COMM_WORLD);

                std::ostringstream filenameStream;
                filenameStream << "./outputs/output_grid_" << std::setw(4) << std::setfill('0') << incrementor << ".txt";
                std::string outputFilename = filenameStream.str();
                printGridToFile(grid, GRID_SIZE, GRID_SIZE, outputFilename);
                incrementor++;

                usleep(200000);
                clearScreen();
            }
        } else {
            return 0;
        }
    }else{
        while (true){
            usleep(200000 * my_rank);
            // std::cout << "Process Rank: " << my_rank << "\n"
            // << " Name: elements received count Value: " << elements_received_count << "\n"
            // << "---------------------------------------------" << std::endl;
            std::vector<char> localGrid(elements_received_count);
            std::cout << "Process Rank: " << my_rank << "\n" << "Received count:" << elements_received_count << std::endl;

            MPI_Recv(localGrid.data(), elements_received_count, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // std::cout << "received data"<< std::endl;
            determineState(localGrid, x_size, y_size);
            // std::cout << "Process Rank: " << my_rank << "\n"
            // << " Name: elements sent count Value: " << x_size * y_size << "\n"
            MPI_Barrier(MPI_COMM_WORLD);
            localGrid[100] = '1';
            // << "---------------------------------------------" << std::endl;
            MPI_Send(localGrid.data() + y_size, (x_size * y_size) -1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);

        }
    }
}

std::vector<char> initializeGrid(int size) {
    return std::vector<char>((size) * (size), '0');
}

inline int getIndex(int x, int y, int xsize, int ysize) {
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

void determineState(std::vector<char>& grid, int xsize, int ysize) {
    auto copy = grid;

    for (int a = 1; a < xsize; a++) {
        for (int b = 0; b < ysize; b++) {
            int alive = 0;
            for (int c = -1; c <= 1; c++) {
                for (int d = -1; d <= 1; d++) {
                    if (!(c == '0' && d == '0')) {
                        alive += (copy[getIndex(a + c, b + d, xsize, ysize)]);
                    }
                }
            }

            if (alive < 2 || alive > 3) {
                grid[getIndex(a, b, xsize, ysize)] = '1';
            } else if (alive == 3) {
                grid[getIndex(a, b, xsize, ysize)] = '1';
            }
        }
    }
}
