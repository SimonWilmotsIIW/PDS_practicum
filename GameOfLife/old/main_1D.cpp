#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <iomanip>
#include <algorithm>

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

const int GRID_SIZE = 32;

inline int toIndex(int x, int y, int size);
inline bool getCell(bool grid[], int x, int y, int size);
inline void setCell(bool grid[], int x, int y, int size, bool value);
void clearScreen();
void printGrid(bool grid[], int size);
void printGridToFile(bool grid[], int size, const std::string &filename);
void determineState(bool grid[], int size);


int main(int argc, char *argv[]) {
    bool gridOne[(GRID_SIZE + 1) * (GRID_SIZE + 1)] = {};
    int x, y;

    std::string filename;
    std::string start;
    std::string number_of_cells;

    std::cout << COLOR_RED;

    if (argc == 2) {
        filename = argv[1];
        std::ifstream readFile(filename);
        if (readFile.is_open()) {
            std::string fileLine, xx, yy;
            while (std::getline(readFile, fileLine)) {
                std::stringstream ss(fileLine);
                std::getline(ss, xx, ' ');
                std::getline(ss, yy, ' ');
                x = std::stoi(xx);
                y = std::stoi(yy);
                setCell(gridOne, x, y, GRID_SIZE, true);
            }
            start = "y";
        } else {
            std::cerr << "Error: File not found." << std::endl;
            return 1;
        }
    } else 
    {
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
        std::cout << "O - living cell" << std::endl;
        std::cout << ". - dead cell" << std::endl;
        std::cout << std::endl;
        std::cout << "Enter the number of cells, or 'r' to read cells from file: ";
        std::cin >> number_of_cells;
        std::cout << std::endl;

        if (number_of_cells == "r") {
            while (true) {
                std::cout << "Enter name of file to read from: ";
                std::cin >> filename;

                std::ifstream readFile(filename);
                if (readFile.is_open()) {
                    std::string fileLine, xx, yy;
                    while (std::getline(readFile, fileLine)) {
                        std::stringstream ss(fileLine);
                        std::getline(ss, xx, ' ');
                        std::getline(ss, yy, ' ');
                        x = std::stoi(xx);
                        y = std::stoi(yy);
                        setCell(gridOne, x, y, GRID_SIZE, true);
                    }
                    break;
                } else {
                    std::cerr << "File not found. Try again." << std::endl;
                }
            }
        } else {
            for (int i = 0; i < std::stoi(number_of_cells); i++) {
                std::cout << "Cell " << i + 1 << "/" << number_of_cells << ": Enter x y: ";
                std::cin >> x >> y;
                setCell(gridOne, x, y, GRID_SIZE, true);
            }
        }

        std::cout << "Start the game? (y/n): ";
        std::cin >> start;
    }

    int incrementor = 0;
    if (start == "y" || start == "Y") {
        while (true) {
            clearScreen();
            printGrid(gridOne, GRID_SIZE);

            std::ostringstream filenameStream;
            filenameStream << "./outputs/output_grid_" << std::setw(4) << std::setfill('0') << incrementor << ".txt";
            std::string filename = filenameStream.str();
            printGridToFile(gridOne, GRID_SIZE, filename);

            determineState(gridOne, GRID_SIZE);
            incrementor++;
            usleep(200000);
        }
    }

    return 0;
}

inline int toIndex(int x, int y, int size) {
    return y * (size + 1) + x;
}

inline bool getCell(bool grid[], int x, int y, int size) {
    return grid[toIndex(x, y, size)];
}

inline void setCell(bool grid[], int x, int y, int size, bool value) {
    grid[toIndex(x, y, size)] = value;
}

void clearScreen() {
    //working on Ubuntu (22.04) and Arch (6.11.8-arch1-2)
    //not working on Windows 11 atm
    // pos on POSIX systems
    std::cout << "\033[2J\033[1;1H"; // clears screen and moves cursor to home
}

void printGrid(bool grid[], int size) {
    std::cout << COLOR_BLUE;
    for (int y = 1; y < size; y++) {
        for (int x = 1; x < size; x++) {
            if (getCell(grid, x, y, size)) {
                std::cout << " O ";
            } else {
                std::cout << " . ";
            }
            if (x == size - 1) {
                std::cout << std::endl;
            }
        }
    }
    std::cout << COLOR_RED;
}

void printGridToFile(bool grid[], int size, const std::string &filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    for (int y = 1; y < size; y++) {
        for (int x = 1; x < size; x++) {
            if (getCell(grid, x, y, size)) {
                outFile << " O ";
            } else {
                outFile << " . ";
            }
            if (x == size - 1) {
                outFile << std::endl;
            }
        }
    }

    outFile.close();
    std::cout << "Grid written to file: " << filename << std::endl;
}

void determineState(bool grid[], int size) {
    bool gridCopy[(size + 1) * (size + 1)] = {};
    std::copy(grid, grid + (size + 1) * (size + 1), gridCopy);

    for (int y = 1; y < size; y++) {
        for (int x = 1; x < size; x++) {
            int alive = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (!(dx == 0 && dy == 0) && getCell(gridCopy, x + dx, y + dy, size)) {
                        ++alive;
                    }
                }
            }
            if (alive < 2 || alive > 3) {
                setCell(grid, x, y, size, false);
            } else if (alive == 3) {
                setCell(grid, x, y, size, true);
            }
        }
    }
}
