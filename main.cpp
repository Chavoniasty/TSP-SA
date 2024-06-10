#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

std::vector<std::string> getFilesInDirectory(const std::string& directory) {
    std::vector<std::string> fileNames;

    std::cout << "[dir: " << std::filesystem::current_path() << "]\n";

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                fileNames.push_back(entry.path().filename().string());
            }
        }
    } catch (fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return fileNames;
}

void loadData(std::string filename, std::vector<std::vector<double>>& vertex) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<double> row;
        std::istringstream iss(line);
        double value;
        while (iss >> value) {
            row.push_back(value);
        }
        vertex.push_back(row);
    }
}

template <typename T>
T calculateCost(const std::vector<int>& permutation,
                const std::vector<std::vector<T>>& vertex) {
    T cost = 0;
    for (size_t i = 0; i < permutation.size() - 1; ++i) {
        cost += vertex[permutation[i]][permutation[i + 1]];
    }
    cost += vertex[permutation.back()][permutation[0]];
    return cost;
}

std::vector<int> initialGuess(int size) {
    std::vector<int> permutation(size);
    std::iota(permutation.begin(), permutation.end(), 0);
    return permutation;
}

template <typename T>
void annihilationStep(const std::vector<std::vector<T>>& vertex,
                      std::vector<int>& path, std::vector<int>& bestPath,
                      T temperature) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::random_device rd;
    std::mt19937 rng(rd());
    double probability = 0;
    bool acceptCondition = false;
    T prevCost = calculateCost(path, vertex);
    T currentCost = 0;
    for (int i = 0; i < 100; i++) {
        std::random_shuffle(path.begin(), path.end());
        currentCost = calculateCost(path, vertex);
        if (currentCost > prevCost) {
            probability = std::exp(
                static_cast<double>((prevCost - currentCost) / temperature));
            acceptCondition = (dist(rng) < probability);
        }
        if (currentCost <= prevCost || acceptCondition) {
            prevCost = currentCost;
        } else {
            std::random_shuffle(path.begin(), path.end());
        }
        if (calculateCost(bestPath, vertex) > prevCost) {
            std::cout << "\t>> new cost found: " << calculateCost(bestPath, vertex) << std::endl;
            bestPath = path;
        }
    }
}

template <typename T>
void beginAnnihilation(const std::vector<std::vector<T>>& vertex,
                       std::vector<int>& path, std::vector<int>& bestPath) {
    T temperature = 1000;
    T alpha = 0.996;
    for (int i = 0; i < 400; i++) {
        annihilationStep(vertex, path, bestPath, temperature);
        temperature = temperature * alpha;
    }
}

class City {
    int x;
    int y;

   public:
    City(int tempX, int tempY) : x(tempX), y(tempY) {}
    int getX() const { return x; }
    int getY() const { return y; }
};

void generateData(int size, int max, std::vector<City>& cities) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, max);

    while (cities.size() < size) {
        int x = dist(rng);
        int y = dist(rng);
        bool duplicate = false;

        for (const auto& city : cities) {
            if (x == city.getX() && y == city.getY()) {
                duplicate = true;
                break;
            }
        }

        if (!duplicate) {
            cities.emplace_back(x, y);
        }
    }
}

template <typename T>
T roundToTwoDecimalPlaces(double value) {
    return static_cast<T>(std::round(value * 100.0) / 100.0);
}

template <typename T>
void setVertex(const std::vector<City>& cities,
               std::vector<std::vector<T>>& vertex) {
    size_t size = cities.size();
    vertex.resize(size, std::vector<T>(size, 0));

    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            if (i != j) {
                int deltaX = cities[i].getX() - cities[j].getX();
                int deltaY = cities[i].getY() - cities[j].getY();
                double distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
                vertex[i][j] = roundToTwoDecimalPlaces<T>(distance);
            }
        }
    }
}

void saveResult(std::vector<City> cities, std::vector<int> bestPath) {
    std::ofstream file("result.txt");
    if (!file.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }

    for (size_t i = 0; i < bestPath.size(); ++i) {
        file << cities[bestPath[i]].getX() << " " << cities[bestPath[i]].getY()
             << std::endl;
    }
}

void clearConsole() {
    // ANSI escape code to clear screen
    std::cout << "\033[2J\033[1;1H";
}

void menuWrapperStart() {
    clearConsole();
    std::cout << "---------- MENU ----------\n";
}

void menuWrapperEnd() {
    std::cout << "--------------------------\n\n";
    std::cout << "Choose option: ";
}

void showMenu() {
    menuWrapperStart();
    std::cout << "1. Load from file\n";
    std::cout << "2. Generate city data\n";
    std::cout << "3. Exit\n";
    menuWrapperEnd();
}

void showLoadFromFileMenu() {
    menuWrapperStart();
    std::cout << "1. Enter file name\n";
    std::cout << "2. Choose file from /data directory\n";
    std::cout << "3. Exit\n";
    menuWrapperEnd();
}

void runVisualisation() {
    system("python display.py");
}

int main() {
    showMenu();
    int option, size;
    std::cin >> option;
    std::vector<City> cities;
    std::vector<std::vector<double>> vertex;

    if (option == 3) {
        return 0;
    }

    if (option != 1 && option != 2) {
        std::cerr << "Error: invalid option" << std::endl;
        return 1;
    }

    if (option == 1) {
        showLoadFromFileMenu();
        int subOption;
        std::cin >> subOption;

        if (subOption == 3) {
            return 0;
        }
        if (subOption != 1 && subOption != 2) {
            std::cerr << "Error: invalid option" << std::endl;
            return 1;
        }

        if (subOption == 1) {
            std::cout << "Enter the file path: ";
            std::string fileName;
            std::cin >> fileName;

            loadData(fileName, vertex);
        } else if (subOption == 2) {
            menuWrapperStart();
            std::vector<std::string> fileNames = getFilesInDirectory("data");

            for (size_t i = 0; i < fileNames.size(); ++i) {
                std::cout << i + 1 << ". " << fileNames[i] << std::endl;
            }
            menuWrapperEnd();

            int fileIndex;
            std::cin >> fileIndex;

            if (fileIndex < 1 || fileIndex > fileNames.size()) {
                std::cerr << "Error: invalid file index" << std::endl;
                return 1;
            }

            loadData("data/" + fileNames[fileIndex - 1], vertex);
            clearConsole();
        }
    } else if (option == 2) {
        size = 10;
        int max = 20;
        generateData(size, max, cities);
        std::cout << "--------------------------\n";
        for (const auto& city : cities) {
            std::cout << city.getX() << " " << city.getY() << std::endl;
        }
        std::cout << "--------------------------\n";
        setVertex(cities, vertex);
    }

    std::cout << "--------------------------\n";
    std::cout << "Distance matrix:" << std::endl;
    for (const auto& row : vertex) {
        for (const auto& value : row) {
            std::cout << std::fixed << std::setprecision(2) << value << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "--------------------------\n";

    std::vector<int> path = initialGuess(size);
    std::vector<int> bestPath = path;

    std::cout << "Initial guess: " << calculateCost<double>(path, vertex) << std::endl;

    auto start = std::chrono::system_clock::now();
    beginAnnihilation(vertex, path, bestPath);
    auto end = std::chrono::system_clock::now();
    
    std::chrono::duration<double> elapsed_seconds = end - start;

    std::cout << "--------------------------\n";
    std::cout << "Time: " << elapsed_seconds.count() << "s" << std::endl;
    std::cout << "Final cost: " << calculateCost<double>(bestPath, vertex)
              << std::endl;

    saveResult(cities, bestPath);

    if (option == 2) {
        std::cout << "--------------------------\n";
        std::cout << "Visualisation is starting..." << std::endl;
        runVisualisation();
    }

    return 0;
}
