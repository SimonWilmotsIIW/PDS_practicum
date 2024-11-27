#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <iomanip>
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
int global_comm_sz;
const int  global_chunk_width = 32 / (9 - 1);
const int GRID_SIZE = 32;
void printGrid(bool gridOne[GRID_SIZE + 1][GRID_SIZE + 1]);
void determineState(bool gridOne[GRID_SIZE + 1][GRID_SIZE + 1]);
void clearScreen(void);
void printGridToFile(bool gridOne[GRID_SIZE + 1][GRID_SIZE + 1], const std::string& filename);
void determineLocalState(bool gridOne[global_chunk_width + 2][GRID_SIZE + 1]);

int main(int argc, char *argv[]) {

    int comm_sz;
    int my_rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    global_comm_sz = comm_sz;
    //thread 0 reads in file
    if (my_rank == 0){
      // system( "color A" );//LGT green
      std::cout << COLOR_GREEN;
      clearScreen();

      bool gridOne[GRID_SIZE + 1][GRID_SIZE + 1] = {};
      int x, y, n;

      std::string number_of_cells;
      std::string start;
      std::string filename;

      if(argc == 2){
        std::cout << "Reading file\n";
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
    std::cout << "Starting run\n";
    if (start == "y" || start == "Y") {
      std::cout << "In run\n";
        int incrementor = 0;
        while (true) {
          //send to MPI
          int chunkSize = GRID_SIZE / (comm_sz - 1); // Divide grid among workers
          std::cout << "Sending chunks \n";
          for (int rank = 1; rank < comm_sz; ++rank) {
              int startRow = (rank - 1) * chunkSize + 1; // Calculate slice
              int endRow = startRow + chunkSize;

                // Send the assigned chunk to each worker
                MPI_Send(&gridOne[startRow][0], (endRow - startRow) * (GRID_SIZE + 1), MPI_CHAR, rank, 0, MPI_COMM_WORLD);
                std::cout << "Sent chunk\n";
            }
          //END send to MPI
          // determineState(gridOne);

          std::ostringstream filenameStream;
          filenameStream << "./outputs_mpi/output_grid_" << std::setw(4) << std::setfill('0') << incrementor << ".txt";
          std::string filename = filenameStream.str();
          //receive MPI (reduce?) and recombine
          std::cout << "Now receiving chunks\n";
          for (int rank = 1; rank < comm_sz; ++rank) {
              int startRow = (rank - 1) * chunkSize + 1;
              int endRow = startRow + chunkSize;
              //THE PROBLEM
              MPI_Recv(&gridOne[startRow][0], (endRow - startRow) * (GRID_SIZE + 1), MPI_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
              //END OF THE PROBLEM (hopefully)
              std::cout << "Now receiving chunks\n";

          }
          //END receive MPI and recombine
          std::cout << "Now received all chunks\n";
          printGridToFile(gridOne, filename);
          std::cout << "hello\n";

          incrementor++;
          if(incrementor > 10){
            return 0;
          }

        }
      } else {
        std::cout << COLOR_RESET;
        clearScreen();
        return 0;
      }
    }else{
      int chunkSize = GRID_SIZE / (comm_sz - 1);
      int startRow = (my_rank - 1) * chunkSize + 1;
      int endRow = startRow + chunkSize;

      bool localGrid[chunkSize + 2][GRID_SIZE + 1] = {}; // Include ghost rows for neighbors

      // Receive grid slice from master
      MPI_Recv(&localGrid[1][0], chunkSize * (GRID_SIZE + 1), MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      // Compute new state for the slice
      determineLocalState(localGrid);

      // Send the updated slice back to master
      MPI_Send(&localGrid[1][0], chunkSize * (GRID_SIZE + 1), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }

}

void clearScreen(void) {
    //working on Ubuntu (22.04) and Arch (6.11.8-arch1-2)
    //not working on Windows 11 atm
    // pos on POSIX systems
    std::cout << "\033[2J;" << "\033[1;1H"; // clears screen and moves cursor to home
}

void printGrid(bool gridOne[GRID_SIZE + 1][GRID_SIZE + 1]) {
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


void printGridToFile(bool gridOne[GRID_SIZE + 1][GRID_SIZE + 1], const std::string& filename) {
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

void compareGrid(bool gridOne[GRID_SIZE + 1][GRID_SIZE + 1], bool gridTwo[GRID_SIZE + 1][GRID_SIZE + 1]) {
  for (int a = 0; a < GRID_SIZE; a++) {
    for (int b = 0; b < GRID_SIZE; b++) {
      gridTwo[a][b] = gridOne[a][b];
    }
  }
}

void determineState(bool gridOne[GRID_SIZE + 1][GRID_SIZE + 1]) {
  bool gridTwo[GRID_SIZE + 1][GRID_SIZE + 1] = {};
  compareGrid(gridOne, gridTwo);

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

void determineLocalState(bool gridOne[global_chunk_width + 2][GRID_SIZE + 1]) {
  bool gridTwo[GRID_SIZE / (global_comm_sz - 1) +2][GRID_SIZE + 1] = {};
  compareGrid(gridOne, gridTwo);

  for (int a = 1; a < GRID_SIZE / (global_comm_sz - 1) +1; a++) {
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