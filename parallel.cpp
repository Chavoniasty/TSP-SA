#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

void loadData(std::vector<std::vector<int>>& vertex) {
    std::ifstream file("data/42city.txt");
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

int calculateCost(std::vector<int> permutation,
                  std::vector<std::vector<int>> vertex) {
    int cost = 0;
    for (int i = 0; i < permutation.size() - 1; i++) {
        cost += vertex[permutation[i]][permutation[i + 1]];
    }
    cost += vertex[permutation[permutation.size() - 1]][permutation[0]];
    return cost;
}

std::vector<int> initialGuess(std::vector<std::vector<int>> vertex) {
    std::vector<int> permutation;
    for (int i = 0; i < vertex.size(); i++) {
        permutation.push_back(i);
    }
    return permutation;
}

void annihilationStep(std::vector<std::vector<int>> vertex,
                      std::vector<int>& path, std::vector<int>& bestPath,
                      int temperature) {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::random_device rd;
    std::mt19937 rng(rd());
    double probability = 0;
    bool acceptCondition = false;
    int prevCost = calculateCost(path, vertex);
    int currentCost = 0;
    for (int i = 0; i < 100; i++) {
        std::random_shuffle(path.begin(), path.end());
        // for (auto elem : path) {
        //     std::cout << elem << " ";
        // }
        // std::cout << calculateCost(path, vertex);
        // std::cout << std::endl;
        currentCost = calculateCost(path, vertex);
        if (currentCost > prevCost) {
            probability = std::exp((prevCost - currentCost) / temperature);
            bool acceptCondition = (dist(rng) < probability);
        }
        if (currentCost <= prevCost or acceptCondition) {
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

void beginAnnihilation(std::vector<std::vector<int>> vertex,
                       std::vector<int>& path, std::vector<int>& bestPath) {
    double temperature = 1000;
    double alpha = 0.996;
    for (int i = 0; i < 400; i++) {
        annihilationStep(vertex, path, bestPath, temperature);
        temperature = float(temperature * alpha);
    }
}

int main() {
    std::vector<std::vector<int>> vertex;
    loadData(vertex);

    std::vector<int> path = initialGuess(vertex);
    std::vector<int> bestPath = path;
    // for (auto v : vertex) {
    //     for (auto i : v) {
    //         std::cout << i << " ";
    //     }
    //     std::cout << std::endl;
    // }

    std::cout << "Initial guess: " << calculateCost(path, vertex) << std::endl;
    int start = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
    beginAnnihilation(vertex, path, bestPath);
    int end = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::system_clock::now().time_since_epoch())
                  .count();
    std::cout << "Time: " << end - start << std::endl;
    std::cout << "Final cost: " << calculateCost(bestPath, vertex);

    return 0;
}