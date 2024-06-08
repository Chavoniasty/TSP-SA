#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

void loadData(std::vector<std::vector<int>>& vertex) {
    std::ifstream file("data/15city.txt");
    if (!file.is_open()) {
        std::cerr << "Error: file not found" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<int> row;
        std::istringstream iss(line);
        int value;
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
            std::cout << "new cost!" << std::endl;
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

int main() {
    // MENU OPTION 1 -> Load from file
    // Load data from file
    // std::vector<std::vector<int>> intVertex;
    // loadData(intVertex);
    // END OPTION 1

    // MENU OPTION 2 -> Generate city data
    std::vector<City> cities;
    int size = 10;
    int max = 20;
    // ADD CIN TO THIS TWO INTS
    generateData(size, max, cities);
    for (const auto& city : cities) {
        std::cout << city.getX() << " " << city.getY() << std::endl;
    }
    std::vector<std::vector<double>> vertex;
    setVertex(cities, vertex);
    // END OPTION 2

    std::cout << "Distance matrix:" << std::endl;
    for (const auto& row : vertex) {
        for (const auto& value : row) {
            std::cout << std::fixed << std::setprecision(2) << value << " ";
        }
        std::cout << std::endl;
    }

    // Generate initial path and find best path using simulated annealing
    std::vector<int> path = initialGuess(size);
    std::vector<int> bestPath = path;

    std::cout << "Initial guess: " << calculateCost<double>(path, vertex)
              << std::endl;
    auto start = std::chrono::system_clock::now();
    beginAnnihilation(vertex, path, bestPath);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Time: " << elapsed_seconds.count() << "s" << std::endl;
    std::cout << "Final cost: " << calculateCost<double>(bestPath, vertex)
              << std::endl;

    saveResult(cities, bestPath);
    return 0;
}
