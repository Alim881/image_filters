#include "image_filters.h"
#include <random>
#include <stdexcept>

bool Image::load(const std::string &filename) {//метод для загрузки изображения
    FILE *file = fopen(filename.c_str(), "rb");//чтение в бинарном режиме
    if (!file)//если не открылся
        return false;//оишбка

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);//структура для чтения png. nullptr-указатели на функции обработки ошибок(берем у linbpng)
    png_infop info = png ? png_create_info_struct(png) : nullptr; //структура с информацикй о пнг
    if (!png || !info || setjmp(png_jmpbuf(png))) {//если png, info не созданы
        if (png) png_destroy_read_struct(&png, &info, nullptr);//очищаем память
        fclose(file);//закрываем файл
        return false;//ошибка
    }

    png_init_io(png, file); //инициализируем png с файлм
    png_read_info(png, info);//читаем информацию

    width = png_get_image_width(png, info);//получаем ширину
    height = png_get_image_height(png, info);//высоту
    png_byte color_type = png_get_color_type(png, info);//тип цвета
    png_byte bit_depth = png_get_bit_depth(png, info);//глубину цвета

    if (bit_depth == 16) png_set_strip_16(png);//если 16-битный канал-перевод в 8б
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);//если палитра-переводим в rgb
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);//если серый с глубиной<8-увеличиваем до 8
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);//если прозраночть-переводим в альфа-канал
    if (color_type != PNG_COLOR_TYPE_RGBA) png_set_filler(png, 0xFF, PNG_FILLER_AFTER);//если не формат RGBA-добавлякем альфа-канал

    png_read_update_info(png, info);//записываем изменения

    pixels.resize(height, std::vector<std::vector<unsigned char>>(width, std::vector<unsigned char>(4)));//создаем вектор для хранения данных в формате RGBA

    std::vector<std::vector<png_byte>> row_data(height, std::vector<png_byte>(width * 4));//только временно для png
    std::vector<png_bytep> rows(height);//массив указаталей(требование libong)
    for (int y = 0; y < height; ++y) rows[y] = row_data[y].data();//Она настраивает указатели на строки для библиотеки libpng

    png_read_image(png, rows.data());//читаем пиксели из файла и записываем в буфер

    for (int y = 0; y < height; ++y) {//цикл по высоте
        for (int x = 0; x < width; ++x) {//по ширине
            //Это перевод данных из "плоского" массива байт в удобную структуру с индексацией по x и y пикселя и цветовым каналам.
            pixels[y][x][0] = row_data[y][x * 4];//R
            pixels[y][x][1] = row_data[y][x * 4 + 1];//G
            pixels[y][x][2] = row_data[y][x * 4 + 2];//B
            pixels[y][x][3] = row_data[y][x * 4 + 3];//Alpha
        }
    }

    png_destroy_read_struct(&png, &info, nullptr);//очищаем память
    fclose(file);//закрываем файл
    return true;//успех ебать
}

bool Image::save(const std::string &filename) const {//сохранение изображения
    FILE *file = fopen(filename.c_str(), "wb");//запись в бинарном режиме
    if (!file)//если файла нет
        return false;//ошибка

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);//структура для чтения png
    png_infop info = png ? png_create_info_struct(png) : nullptr;//информация о png
    if (!png || !info || setjmp(png_jmpbuf(png))) {//если нет png,info
        if (png)png_destroy_write_struct(&png, &info);//очищаем память
        fclose(file);//закрываем файл
        return false;//ошибка ебать
    }

    png_init_io(png, file);//инициализируем пнг с файлом
    //эти команды устанавливают параметры изображения и начинают процесс записи PNG-файла.
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    std::vector<std::vector<png_byte>> row_data(height, std::vector<png_byte>(width * 4));//создаем вектор для хранения данных в формате RGBA
    std::vector<png_bytep> rows(height);//только для png
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            //обратное для подготовки данных пикселей из вашего внутреннего представления в формате pixels в структуру row_data, которую библиотека libpng ожидает для записи в файл.
            row_data[y][x * 4] = pixels[y][x][0];
            row_data[y][x * 4 + 1] = pixels[y][x][1];
            row_data[y][x * 4 + 2] = pixels[y][x][2];
            row_data[y][x * 4 + 3] = pixels[y][x][3];
        }
        rows[y] = row_data[y].data();//присваивание указателя на первой элемент вектора
    }

    png_write_image(png, rows.data());//записываем данные
    png_write_end(png, nullptr);//завершается записб

    png_destroy_write_struct(&png, &info);//очищаем память
    fclose(file);//закрываем файл
    return true;//we are the champions
}
//получаем цвет изображения
std::vector<unsigned char> Image::getPixel(int x, int y) const {
    return (x >= 0 && x < width && y >= 0 && y < height) ? pixels[y][x] : std::vector<unsigned char>{0, 0, 0, 255};
}

//заменяем цвет пикселя и проверяем гарниы
void Image::setPixel(int x, int y, const std::vector<unsigned char> &color) {//const-функция не меняет цвет пикселя, uns char-отбрасываем отрицательные значения
    if (x >= 0 && x < width && y >= 0 && y < height)
        pixels[y][x] = color;//устанавливаем цвет пикселя
}

void applySolarRays(Image &img) {//лучи
    int width = img.getWidth();//ширина
    int height = img.getHeight();//высота
    int centerX = width / 2;//центр ширина
    int centerY = height / 2;//центр высота
    float maxDist = std::sqrt(centerX * centerX + centerY * centerY);//макс раст от центра до угла

    for (int y = 0; y < height; y++) {//по высоте
        for (int x = 0; x < width; x++) {//по ширине
            float dx = x - centerX;//смещение по х от центра
            float dy = y - centerY;//по у
            float dist = std::sqrt(dx * dx + dy * dy);// вычисляем расстояние от центра до текущего пикселя по формуле длины вектора
            float angle = std::atan2(dy, dx);//угол наклона от пикселя к центру
            float intensity = std::sin(angle * 10) * (1.0f - dist / maxDist);//интенсивность в зависимости от удаленности к центру
            intensity = std::max(0.0f, intensity) * 100;//деалем неотрицательным и увеличиваем эфеект

            auto pixel = img.getPixel(x, y);//получаем цвет пки
            pixel[0] = std::min(255, static_cast<int>(pixel[0] + intensity));
            pixel[1] = std::min(255, static_cast<int>(pixel[1] + intensity));
            pixel[2] = std::min(255, static_cast<int>(pixel[2] + intensity));
            img.setPixel(x, y, pixel);
        }
    }
}

void applyWaveDistortion(Image &img, float amplitude) {
    int width = img.getWidth();
    int height = img.getHeight();
    Image temp = img;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float offsetX = amplitude * std::sin(2 * M_PI * y / 128.0f);
            float offsetY = amplitude * std::cos(2 * M_PI * x / 128.0f);
            int newX = x + static_cast<int>(offsetX);
            int newY = y + static_cast<int>(offsetY);
            auto pixel = temp.getPixel(newX, newY);
            img.setPixel(x, y, pixel);
        }
    }
}

void applyColorNoise(Image &img, float intensity) {
    int width = img.getWidth();
    int height = img.getHeight();

    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(-250, 250);

    float noiseFactor = intensity * 3.5f;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            auto pixel = img.getPixel(x, y);
            int noiseR = static_cast<int>(dist(gen) * noiseFactor);
            int noiseG = static_cast<int>(dist(gen) * noiseFactor);
            int noiseB = static_cast<int>(dist(gen) * noiseFactor);

            pixel[0] = std::clamp(pixel[0] + noiseR, 0, 255);
            pixel[1] = std::clamp(pixel[1] + noiseG, 0, 255);
            pixel[2] = std::clamp(pixel[2] + noiseB, 0, 255);
            img.setPixel(x, y, pixel);
        }
    }
}

void applyGlitch(Image &img) {
    int width = img.getWidth();
    int height = img.getHeight();

    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 20);

    for (int y = 0; y < height; y += 10) {
        int shift = dist(gen);
        for (int x = 0; x < width; x++) {
            int newX = (x + shift) % width;
            auto pixel = img.getPixel(x, y);
            if (y % 20 == 0)
                pixel[0] = std::min(255, pixel[0] + 50);
            else if (y % 15 == 0)
                pixel[1] = std::min(255, pixel[1] + 50);
            img.setPixel(newX, y, pixel);
        }
    }
}

void applyGrayscale(Image &img) {
    int width = img.getWidth();
    int height = img.getHeight();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            auto pixel = img.getPixel(x, y);
            unsigned char gray = static_cast<unsigned char>(0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
            pixel[0] = pixel[1] = pixel[2] = gray;
            img.setPixel(x, y, pixel);
        }
    }
}
