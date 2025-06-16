#include "src/image_filters.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main() {
    std::string inputFileName, outputFile;
    std::cout << "Введите название файла для обработки: ";
    std::getline(std::cin, inputFileName);

    if (inputFileName.empty()) {
        inputFileName = "input.png"; 
    }

    Image image;
    if (!image.load(inputFileName)) {
        std::string rootPath = "../" + inputFileName;
        if (!image.load(rootPath)) {
            std::cerr << "Ошибка при загрузке изображения: файл " << inputFileName
                      << " не найден ни в build, ни в корневой директории\n";
            return 1;
        }
    }

    std::cout << "Выберите фильтр:\n1. Солнечные лучи\n2. Волны\n3. Цветовой шум\n4. Глитч\n5. Ч/Б\nВыбор: ";
    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        applySolarRays(image);
        break;
    case 2:
        applyWaveDistortion(image);
        break;
    case 3:
        applyColorNoise(image);
        break;
    case 4:
        applyGlitch(image);
        break;
    case 5:
        applyGrayscale(image);
        break;
    default:
        std::cerr << "Ошибка: неверный выбор фильтра\n";
        return 1;
    }

    std::cout << "Введите имя выходного файла: ";
    std::cin.ignore(); 
    std::getline(std::cin, outputFile);

    if (outputFile.empty()) {
        std::cerr << "Ошибка: не указано имя файла\n";
        return 1;
    }

    const std::string outputDir = "../output/";
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }

    outputFile = outputDir + outputFile;

    if (!image.save(outputFile)) {
        std::cerr << "Ошибка при сохранении изображения\n";
        return 1;
    }

    std::cout << "Обработанное изображение сохранено в " << outputFile << "\n";

    return 0;
}
