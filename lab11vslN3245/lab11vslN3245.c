#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <getopt.h>


unsigned char *searchBytes = NULL; // Глобальная переменная для хранения байтовой последовательности
size_t searchLength = 0; // Длина байтовой последовательности
int found = 0;


// Функция для преобразования одного шестнадцатеричного символа в число
int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Функция для преобразования строки с шестнадцатеричными числами в массив байтов
size_t hexStringToByteArray(const char* hexString, unsigned char** byteArray) {
    size_t hexStringLength = strlen(hexString);
    size_t byteArrayLength = hexStringLength / 2;
    *byteArray = malloc(byteArrayLength);
    if (*byteArray == NULL) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0, j = 0; i < hexStringLength; i += 2, ++j) {
        (*byteArray)[j] = (hexCharToInt(hexString[i]) << 4) + hexCharToInt(hexString[i+1]);
    }

    return byteArrayLength;
}

// работа с переменной окружения
void debugPrint(const char *message) {
    if (getenv("LAB11DEBUG") != NULL) {
        fprintf(stderr, "DEBUG: %s\n", message);
    }
}

// функция для ftw, передаем конкретный файл в процессе обхода
int processEntry(const char *fpath, const struct stat *sb, int typeflag) {
    if (typeflag == FTW_F) {
        searchStringInFile(fpath, searchBytes, searchLength);
    }
    return 0;
}

// Предполагаем, что searchBytes уже преобразована из строки в байтовый массив
int searchStringInFile(const char *filepath, const unsigned char *searchBytes, size_t searchLength) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Ошибка при открытии файла");
        return -1; // Ошибка при открытии файла
    }

    unsigned char buffer[1024]; // Буфер для чтения файла
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytesRead; ++i) {
            if (i + searchLength <= bytesRead && memcmp(&buffer[i], searchBytes, searchLength) == 0) {
                // Найдена искомая последовательность
                printf("Файл: %s, найдена последовательность: ", filepath);
                // Выводим пару байтов до найденной последовательности
                for (int j = -2; j < (int)searchLength + 2; ++j) {
                    if (i+j >= 0 && i+j < bytesRead) {
                        if (j >= 0 && j < (int)searchLength) {
                            printf("[%02x]", buffer[i+j]); // Выделяем искомую последовательность
                        } else {
                            printf(" %02x ", buffer[i+j]);
                        }
                    }
                }
                printf("\n");
                found++;
                // Пропускаем уже найденную последовательность
                i += searchLength - 1;
            }
        }
    }

    fclose(file);
    return 0;
}

// вывод информации о командах
void printHelp() {
    printf("Использование: lab11vslN3245 [опции] <директория> <последовательность байтов>\n");
    printf("Опции:\n");
    printf("-v, --version    Вывод версии программы\n");
    printf("-h, --help       Вывод этой справки\n");
}

void printVersion() {
    printf("lab11vslN3245, версия 1.0\n");
}


int main(int argc, char *argv[]) {
    int opt;
    int option_index = 0;
    // список доступных опций
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    }; 

    while ((opt = getopt_long(argc, argv, "hv", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return EXIT_SUCCESS;
            case 'v':
                printVersion();
                return EXIT_SUCCESS;
            default:
                printHelp();
                return EXIT_FAILURE;
        }
    }

    if (argc - optind < 2) {
        printf("Недостаточно аргументов\n");
        printHelp();
        return EXIT_FAILURE;
    }

    if (argc - optind >= 2) {
        // Предполагаем, что argv[optind + 1] содержит шестнадцатеричную строку
        searchLength = hexStringToByteArray(argv[optind + 1], &searchBytes);
        if (searchBytes == NULL) {
            fprintf(stderr, "Ошибка при преобразовании строки в байты\n");
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "Не указана последовательность байтов для поиска\n");
        return EXIT_FAILURE;
    }

    // Обход директории с использованием ftw
    if (ftw(argv[optind], processEntry, 20) == -1) {
        printf("В каталоге %s найдено %d совпадений", found, argv[optind]);
        perror("ftw");
        free(searchBytes); // Освобождаем память
        return EXIT_FAILURE;
    }

    free(searchBytes); // Освобождаем память
    return EXIT_SUCCESS;
}
