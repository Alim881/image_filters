#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../external/doctest.h"
#include "../src/image_filters.h"
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

bool loadImage(Image &img, const std::string &filename = "input.png") {
    bool loaded = img.load(filename);
    if (!loaded && filename == "input.png") {
        loaded = img.load("../input.png");
    }
    return loaded;
}

TEST_CASE("Image::load - положительный и отрицательный") {
    Image img;
    CHECK_FALSE(img.load("nonexistent.png"));
    CHECK(loadImage(img));
}

TEST_CASE("Image::save - положительный и отрицательный тесты") {
    Image img;
    REQUIRE(loadImage(img));
    CHECK(img.save("output.png"));
    CHECK_FALSE(img.save("/nonexistent_directory/output.png"));
}

TEST_CASE("Image::getWidth/getHeight - положительный и отрицательный тесты") {
    Image img;
    CHECK(img.getWidth() == 0);
    CHECK(img.getHeight() == 0);
    REQUIRE(loadImage(img));
    CHECK(img.getWidth() > 0);
    CHECK(img.getHeight() > 0);
}

TEST_CASE("Image::getPixel - положительный и отрицательный тесты") {
    Image img;
    REQUIRE(loadImage(img));
    CHECK(img.getPixel(-1, 0) == std::vector<unsigned char>{0, 0, 0, 255});
    CHECK(img.getPixel(0, 0) != std::vector<unsigned char>{0, 0, 0, 0});
}

TEST_CASE("Image::setPixel - положительный и отрицательный тесты") {
    Image img;
    REQUIRE(loadImage(img));
    std::vector<unsigned char> color{255, 128, 0, 255};
    img.setPixel(0, 0, color);
    CHECK(img.getPixel(0, 0) == color);
    img.setPixel(img.getWidth(), img.getHeight(), color);
    CHECK(img.getPixel(img.getWidth(), img.getHeight()) == std::vector<unsigned char>{0, 0, 0, 255});
}

TEST_CASE("applyWaveDistortion - положительный и отрицательный тесты") {
    Image img;
    REQUIRE(loadImage(img));
    Image original = img;
    applyWaveDistortion(img, 15.0f);
    bool isChanged = false;
    for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
            if (img.getPixel(x, y) != original.getPixel(x, y)) {
                isChanged = true;
                break;
            }
        }
        if (isChanged)
            break;
    }
    CHECK(isChanged);
    img = original;
    applyWaveDistortion(img, 0.0f);
    bool isUnchanged = true;
    for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
            if (img.getPixel(x, y) != original.getPixel(x, y)) {
                isUnchanged = false;
                break;
            }
        }
        if (!isUnchanged)
            break;
    }
    CHECK(isUnchanged);
}

TEST_CASE("applySolarRays - положительный и простой отрицательный тест") {
    Image img;
    REQUIRE(loadImage(img));
    Image original = img;
    applySolarRays(img);
    bool isChanged = false;
    for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
            if (img.getPixel(x, y) != original.getPixel(x, y)) {
                isChanged = true;
                break;
            }
        }
        if (isChanged)
            break;
    }
    CHECK(isChanged);
    Image empty;
    applySolarRays(empty);
    CHECK(empty.getWidth() == 0);
    CHECK(empty.getHeight() == 0);
}

TEST_CASE("applyGrayscale - положительный и отрицательный тесты") {
    Image img;
    REQUIRE(loadImage(img));
    Image original = img;
    applyGrayscale(img);
    bool isGrayscale = true;
    for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
            auto pixel = img.getPixel(x, y);
            if (!(pixel[0] == pixel[1] && pixel[1] == pixel[2])) {
                isGrayscale = false;
                break;
            }
        }
        if (!isGrayscale)
            break;
    }
    CHECK(isGrayscale);
    Image beforeSecondApply = img;
    applyGrayscale(img);
    bool isUnchanged = true;
    for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
            if (img.getPixel(x, y) != beforeSecondApply.getPixel(x, y)) {
                isUnchanged = false;
                break;
            }
        }
        if (!isUnchanged)
            break;
    }
    CHECK(isUnchanged);
}

TEST_CASE("applyGlitch - стандартное изображение, появляются артефакты") {
    Image img;
    REQUIRE(loadImage(img));
    REQUIRE(img.getHeight() >= 10);
    Image original = img;
    applyGlitch(img);
    bool isChanged = false;
    for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
            if (img.getPixel(x, y) != original.getPixel(x, y)) {
                isChanged = true;
                break;
            }
        }
        if (isChanged)
            break;
    }
    CHECK(isChanged);
}

TEST_CASE("applyColorNoise - положительный и отрицательный тесты") {
    Image img;
    REQUIRE(loadImage(img));
    Image original = img;
    applyColorNoise(img, 0.5f);
    bool isChanged = false;
    for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
            if (img.getPixel(x, y) != original.getPixel(x, y)) {
                isChanged = true;
                break;
            }
        }
        if (isChanged)
            break;
    }
    CHECK(isChanged);
    img = original;
    applyColorNoise(img, 0.0f);
    bool isUnchanged = true;
    for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
            if (img.getPixel(x, y) != original.getPixel(x, y)) {
                isUnchanged = false;
                break;
            }
        }
        if (!isUnchanged)
            break;
    }
    CHECK(isUnchanged);
}
