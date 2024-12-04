#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <iomanip>
#include <vector>

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

#define GRID_SIZE 32

void printGrid(const std::vector<bool>& grid, int size);
void determineState(std::vector<bool>& grid, int size);
void clearScreen(void);
void printGridToFile(const std::vector<bool>& grid, int size, const std::string& filename);
std::vector<bool> initializeGrid(int size);
inline int getIndex(int x, int y, int size);

int main(int argc, char* argv[]) {
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

                grid[getIndex(x, y, GRID_SIZE)] = true;
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

                        grid[getIndex(x, y, GRID_SIZE)] = true;
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
                grid[getIndex(x, y, GRID_SIZE)] = true;
                printGrid(grid, GRID_SIZE);
            }
        }
        std::cout << "Grid setup is done. Start the game? (y/n): ";
        std::cin >> start;
    }

    int incrementor = 0;
    if (start == "y" || start == "Y") {
        while (true) {
            printGrid(grid, GRID_SIZE);
            determineState(grid, GRID_SIZE);

            std::ostringstream filenameStream;
            filenameStream << "./outputs/output_grid_" << std::setw(4) << std::setfill('0') << incrementor << ".txt";
            std::string outputFilename = filenameStream.str();
            printGridToFile(grid, GRID_SIZE, outputFilename);
            incrementor++;

            usleep(200000);
            clearScreen();
        }
    } else {
        return 0;
    }
}

std::vector<bool> initializeGrid(int size) {
    return std::vector<bool>((size + 1) * (size + 1), false);
}

inline int getIndex(int x, int y, int size) {
    return x * (size + 1) + y;
}

void clearScreen(void) {
    //working on Ubuntu (22.04) and Arch (6.11.8-arch1-2)
    //not working on Windows 11 atm
    // pos on POSIX systems
    std::cout << "\033[2J;" << "\033[1;1H"; // clears screen and moves cursor to home
}

void printGrid(const std::vector<bool>& grid, int size) {
    for (int a = 1; a < size; a++) {
        for (int b = 1; b < size; b++) {
            std::cout << (grid[getIndex(a, b, size)] ? " O " : " . ");
        }
        std::cout << std::endl;
    }
}

void printGridToFile(const std::vector<bool>& grid, int size, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    for (int a = 1; a < size; a++) {
        for (int b = 1; b < size; b++) {
            outFile << (grid[getIndex(a, b, size)] ? " O " : " . ");
        }
        outFile << std::endl;
    }
    outFile.close();
}

void determineState(std::vector<bool>& grid, int size) {
    auto copy = grid;

    for (int a = 1; a < size; a++) {
        for (int b = 1; b < size; b++) {
            int alive = 0;
            for (int c = -1; c <= 1; c++) {
                for (int d = -1; d <= 1; d++) {
                    if (!(c == 0 && d == 0)) {
                        alive += copy[getIndex(a + c, b + d, size)];
                    }
                }
            }

            if (alive < 2 || alive > 3) {
                grid[getIndex(a, b, size)] = false;
            } else if (alive == 3) {
                grid[getIndex(a, b, size)] = true;
            }
        }
    }
}