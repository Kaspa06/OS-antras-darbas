#include "pch.h"
#include "Dll.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <windows.h>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <execution>
#include <omp.h>

namespace fs = std::filesystem;
using namespace std::chrono;

static high_resolution_clock::time_point start_time;
static high_resolution_clock::time_point end_time;

extern "C" __declspec(dllexport) void StartTimer() {
    start_time = high_resolution_clock::now();
}

extern "C" __declspec(dllexport) void EndTimer() {
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    std::cout << "Total Execution Time: " << duration.count() << " ms" << std::endl;
}

extern "C" DLL_API void PingWithTempFile() {
    std::string address;
    std::cout << "Enter website or IP to ping: ";
    std::cin >> address;

    std::string file = "ping.txt";
    std::string command = "ping " + address + " > " + file;
    system(command.c_str());

    std::string openFileCommand = "start " + file;
    system(openFileCommand.c_str());
}


extern "C" DLL_API void CreateCustomFolderStructure() {
    std::string root = "Sumskis";
    std::string name = "Kasparas";

    fs::create_directory(root);
    for (int i = 1; i <= 3; ++i) {
        std::string level1 = root + "\\" + name + std::to_string(i);
        fs::create_directory(level1);
        std::cout << "Sukurtas folderis: " << level1 << std::endl;

        for (int j = 1; j <= 3; ++j) {
            std::string level2 = level1 + "\\" + name + std::to_string(i) + name + std::to_string(j);
            fs::create_directory(level2);
            std::cout << "Sukurtas folderis: " << level2 << std::endl;
            std::ofstream file(level2 + "\\data.txt"); // tuscias failas
            file << "";
            file.close();
            std::cout << "Sukurtas failas: data.txt" << std::endl;
        }
    }
}

extern "C" DLL_API void ComputeTschirnhausen(double F, double x_start, double x_end, double step) {
    std::cout << "Funkcija ComputeTschirnhausen pradejo darba!" << std::endl;

    std::vector<std::string> outputFolders;
    const std::string rootFolder = "Sumskis";
    int writeIndex = 0;

    // 1. Surenkam visus galimus aplankus su data.txt failais
    for (const auto& level1 : fs::directory_iterator(rootFolder)) {
        if (level1.is_directory()) {
            for (const auto& level2 : fs::directory_iterator(level1.path())) {
                if (level2.is_directory()) {
                    outputFolders.push_back(level2.path().string());
                }
            }
        }
    }

    if (outputFolders.empty()) {
        std::cerr << "Nerasta jokiu aplanku su data.txt failais!" << std::endl;
        return;
    }

    // 2. Atidarom kiekviena faila irasymui
    std::vector<std::ofstream> outputFiles;
    for (const auto& folder : outputFolders) {
        std::string filePath = folder + "\\data.txt";
        std::ofstream file(filePath, std::ios::app);
        if (!file) {
            std::cerr << "Nepavyko atidaryti failo: " << filePath << std::endl;
            continue;
        }
        outputFiles.push_back(std::move(file));
    }

    if (outputFiles.empty()) {
        std::cerr << "Nepavyko atidaryti ne vieno data.txt failo!" << std::endl;
        return;
    }

    const int fileCount = static_cast<int>(outputFiles.size());

    // 3. Skaiciavimu ciklas su OpenMP
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i <= static_cast<int>((x_end - x_start) / step); ++i) {
        double x = x_start + i * step;
        double y_squared = x * x * x + 3 * x * x - F;
        if (y_squared < 0) continue;

        double y = std::sqrt(y_squared);

        for (int sign = -1; sign <= 1; sign += 2) {
            if (y == 0.0 && sign == 1) break;

            std::stringstream stream;
            stream << std::fixed << std::setprecision(6) << x << "," << y * sign << "\n";
            std::string line = stream.str();

            // Naudojam critical, kad keli gijos nerasytu i ta pati faila tuo paciu metu
#pragma omp critical
            {
                int targetFileIndex = writeIndex % fileCount;
                outputFiles[targetFileIndex] << line;
                writeIndex++;
            }
        }
    }

    // 4. Uzdaryti failus
    for (auto& file : outputFiles) {
        file.close();
    }

    std::cout << "Funkcija ComputeTschirnhausen baige darba!" << std::endl;
}

std::vector<std::string> GetAllDataFiles() {
    std::vector<std::string> files;
    std::string root = "Sumskis";

    for (const auto& l1_entry : fs::directory_iterator(root)) {
        if (l1_entry.is_directory()) {
            for (const auto& l2_entry : fs::directory_iterator(l1_entry.path())) {
                if (l2_entry.is_directory()) {
                    std::string file_path = l2_entry.path().string() + "\\data.txt";
                    if (fs::exists(file_path)) {
                        files.push_back(file_path);
                    }
                }
            }
        }
    }

    return files;
}

extern "C" DLL_API void MergeResults(double F) {
    std::cout << "Funkcija MergeResults pradeda darba!" << std::endl;

    std::vector<std::pair<double, double>> dataPoints;
    const std::string rootDirectory = "Sumskis";

    // Traverse folders and collect all data.txt values
    for (const auto& entry : fs::recursive_directory_iterator(rootDirectory)) {
        if (entry.path().filename() == "data.txt") {
            std::ifstream inputFile(entry.path());
            double x, y;
            char comma;

            while (inputFile >> x >> comma >> y) {
                dataPoints.emplace_back(x, y);
            }

            inputFile.close();
            fs::remove(entry.path()); // Remove data.txt after reading
        }
    }

    // Sort the points by x (parallel if supported)
    std::sort(std::execution::par, dataPoints.begin(), dataPoints.end());

    std::string outputFileName = rootDirectory + "\\F" + std::to_string(static_cast<int>(F)) + "_merged.txt";
    std::ofstream outputFile(outputFileName, std::ios::out);
    std::cout << "Created file: " << outputFileName << std::endl;

    std::vector<char> writeBuffer;
    for (const auto& [x, y] : dataPoints) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << x << "," << y << "\n";
        const std::string line = oss.str();

        writeBuffer.insert(writeBuffer.end(), line.begin(), line.end());

        // 64 * 1024 = 65536 Bytes = 64 KB balansuoja tarp atminties naudojimo ir writinimo
        if (writeBuffer.size() > 64 * 1024) {
            outputFile.write(writeBuffer.data(), writeBuffer.size());
            writeBuffer.clear();
        }
    }

    if (!writeBuffer.empty()) {
        outputFile.write(writeBuffer.data(), writeBuffer.size());
    }

    outputFile.close();

    std::cout << "Funkcija MergeResults baige darba!" << std::endl;
}



extern "C" DLL_API void DeleteGeneratedFolders() {
    std::cout << "Istrinami sugeneruoti failai ir folderiai" << std::endl;
    fs::remove_all("Sumskis");
    fs::remove_all("ping.txt");
}
