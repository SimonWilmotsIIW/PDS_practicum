#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <iomanip>
#include <vector>
#include <mpi.h>

#define OS_LINUX

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

//using namespace std;

//TODO: increase size by a lot
//TODO: wrap-around grid instead of borders
//TODO: write to file not write to console
//TODO: work on VNC & time, max 72 cores on 1 node (max 4 nodes)
//TODO: compare cores on 1 node with spread cores

const int GRID_SIZE = 32;
void printGrid(std::vector<std::vector<int>>& gridOne);
void determineState(std::vector<std::vector<int>>& gridOne);
void clearScreen(void);
void printGridToFile(std::vector<std::vector<int>>& gridOne, const std::string& filename);
void printLocalGrid(std::vector<std::vector<int>>& localGrid);

int main(int argc, char *argv[]) {

    int global_comm_sz;
    int comm_sz;
    int my_rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    global_comm_sz = comm_sz;
    if(my_rank==0){
        // system( "color A" );//LGT green
        std::cout << COLOR_GREEN;
        // clearScreen();

        std::vector<std::vector<int>> gridOne(GRID_SIZE + 1, std::vector<int>(GRID_SIZE + 1, false));
        int x, y, n;

        std::string number_of_cells;
        std::string start;
        std::string filename;

        if(argc == 2){
            filename = argv[1];
            std::ifstream readfile(filename);
            if (readfile.is_open()) {
            std::string fileline, xx, yy;

            while (std::getline(readfile, fileline)) {
                std::stringstream ss(fileline);

                std::getline(ss, xx, ' ');
                std::getline(ss, yy, ' ');

                x = stoi(xx);
                y = stoi(yy);

                gridOne[x][y] = true;
                }
            }
            start = 'y';
        }else{

            std::cout << "GAME OF LIFE - By Simon Wilmots & Stijn Jacobs for Parallel and Distributed Systems [PDS_4717]" << std::endl;
            std::cout << "Devised by the British mathematician John Horton Conway in 1970." << std::endl;
            std::cout << std::endl;
            std::cout << "Rules:" << std::endl;
            std::cout << "Played on a two-dimensional orthogonal grid of square cells, each of which is in one of two possible states, life or dead."<< std::endl;
            std::cout << "Every cell interacts with its eight neighbours." << std::endl;
            std::cout << "At each step in time, the following transitions occur:" << std::endl;
            std::cout << "1. Any live cell with fewer than two live neighbours dies, as if caused by under-population." << std::endl;
            std::cout << "2. Any live cell with two or three live neighbours lives on to the next generation." << std::endl;
            std::cout << "3. Any live cell with more than three live neighbours dies, as if by over-population." << std::endl;
            std::cout << "4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction." << std::endl;
            std::cout << "Text from https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life" << std::endl;
            std::cout << std::endl;
            std::cout << "";
            std::cout << COLOR_GREEN;
            std::cout << "O - living cell" << std::endl;
            std::cout << ". - dead cell" << std::endl;
            std::cout << std::endl;
            std::cout << "Enter the number of cells, or 'r' to read cells from file: ";
            std::cin >> number_of_cells;
            std::cout << std::endl;

            if (number_of_cells == "r") {
                while (true) {

                    std::cout << "Enter name of file to read from: " << std::endl;
                    std::cin >> filename;

                    std::ifstream readfile(filename);
                    if (readfile.is_open()) {
                        std::string fileline, xx, yy;

                        while (std::getline(readfile, fileline)) {
                            std::stringstream ss(fileline);

                            std::getline(ss, xx, ' ');
                            std::getline(ss, yy, ' ');

                            x = stoi(xx);
                            y = stoi(yy);

                            gridOne[x][y] = true;
                        }
                        break;
                    } else {
                        std::cout << "File not found :(" << std::endl;
                    }
                }
            } else {

                for (int i = 0; i < stoi(number_of_cells); i++) {
                    std::cout << "Cell " << i+1 << "/" << stoi(number_of_cells) << "\nEnter the x & y coordinate of cell " << i + 1
                        << " with format x <space> y :" << "\n>> " ; 
                    std::cin >> x >> y;
                    gridOne[x][y] = true;
                    printGrid(gridOne);
                }
            }
            std::cout << "Grid setup is done. Start the game ? (y/n)" << std::endl;
            printGrid(gridOne);
            std::cin >> start;
        }

        int incrementor = 0;
        if (start == "y" || start == "Y") {
            while (true) {
                printGrid(gridOne);
                int chunkSize = 32 / 8;
                for (int rank = 1; rank < comm_sz; ++rank) {
                    //start at 0 - buffer row
                    int startRow = (rank - 1) * chunkSize;
                    //go 1 beyond regular chunk size - buffer row
                    int endRow = startRow + chunkSize +1;
                    //send to worker threads - they get 2 buffer rows, but shouldn't iterate over them
                    MPI_Send(&gridOne[startRow][0], (endRow - startRow) * (GRID_SIZE + 1), MPI_INT, rank, 0, MPI_COMM_WORLD);
                    std::cout << "Sent chunk\n";
                    usleep(2000000);

                }
                std::ostringstream filenameStream;
                filenameStream << "./outputs/output_grid_" << std::setw(4) << std::setfill('0') << incrementor << ".txt";
                std::string filename = filenameStream.str();
                printGridToFile(gridOne, filename);
                incrementor++;
                determineState(gridOne);

                usleep(2000000);
                return 1;
                clearScreen();
            }
        } else {
        std::cout << COLOR_RESET;
        clearScreen();
        return 0;
        }
    }else{
        std::cout << "ELLO\n";
        std::vector<std::vector<int>> localGrid(GRID_SIZE/8 +2, std::vector<int>(GRID_SIZE + 1, false));
        //is this [0][0] or [1][0]
        MPI_Recv(&localGrid[0][0], (GRID_SIZE/8 +2) * (GRID_SIZE + 1), MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << COLOR_RED;
        printLocalGrid(localGrid);
    }
}

void clearScreen(void) {
    //working on Ubuntu (22.04) and Arch (6.11.8-arch1-2)
    //not working on Windows 11 atm
    // pos on POSIX systems
    std::cout << "\033[2J;" << "\033[1;1H"; // clears screen and moves cursor to home
}

void printGrid(std::vector<std::vector<int>>& gridOne) {
  for (int a = 1; a < GRID_SIZE; a++) {
    for (int b = 1; b < GRID_SIZE; b++) {
      if (gridOne[a][b] == true) {
        std::cout << " O ";
      } else {
        std::cout << " . ";
      }
      if (b == GRID_SIZE - 1) {
        std::cout << std::endl;
      }
    }
  }
}

void printLocalGrid(std::vector<std::vector<int>>& gridOne) {
  for (int a = 1; a < (32/8)+2; a++) {
    for (int b = 1; b < GRID_SIZE; b++) {
      if (gridOne[a][b] == true) {
        std::cout << " O ";
      } else {
        std::cout << " . ";
      }
      if (b == GRID_SIZE - 1) {
        std::cout << std::endl;
      }
    }
  }
}


void printGridToFile(std::vector<std::vector<int>>& gridOne, const std::string& filename) {
    std::ofstream outFile(filename); 
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return; 
    }

    for (int a = 1; a < GRID_SIZE; a++) {
        for (int b = 1; b < GRID_SIZE; b++) {
            if (gridOne[a][b] == true) {
                outFile << " O ";
            } else {
                outFile << " . ";
            }
            if (b == GRID_SIZE - 1) {
                outFile << std::endl;
            }
        }
    }

    outFile.close(); // Close the file after writing
    std::cout << "Grid written to file: " << filename << std::endl; // Inform the user
}

void determineState(std::vector<std::vector<int>>& gridOne) {
    std::vector<std::vector<int>> gridTwo = gridOne;

    for (int a = 1; a < GRID_SIZE; a++) {
        for (int b = 1; b < GRID_SIZE; b++) {
            int alive = 0;
            for (int c = -1; c < 2; c++) {
                for (int d = -1; d < 2; d++) {
                    if (!(c == 0 && d == 0)) {
                        if (gridTwo[a + c][b + d]) {
                            ++alive;
                        }
                    }
                }
            }
            if (alive < 2) {
                gridOne[a][b] = false;
            } else if (alive == 3) {
                gridOne[a][b] = true;
            } else if (alive > 3) {
                gridOne[a][b] = false;
            }
        }
    }
}