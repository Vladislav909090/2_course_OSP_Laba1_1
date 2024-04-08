#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <getopt.h>

const char* searchString; // Глобальная переменная для хранения строки для поиска
int found = 0;

void debugPrint(const char *message) {
    if (getenv("LAB11DEBUG") != NULL) {
        fprintf(stderr, "DEBUG: %s\n", message);
    }
}

int searchStringInFile(const char *filepath, const char *searchStr) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("Ошибка при открытии файла");
        return -1;
    }

    const size_t searchLength = strlen(searchStr); // Вычисляем длину строки для поиска
    unsigned char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytesRead; ++i) {
            if (i + searchLength <= bytesRead && memcmp(&buffer[i], searchStr, searchLength) == 0) {
                printf("%s, найдена последовательность: ", filepath);
                for (int j = -2; j < (int)searchLength + 2; ++j) {
                    if (i+j >= 0 && i+j < bytesRead) {
                        if (j >= 0 && j < (int)searchLength) {
                            printf("[%02x]", buffer[i+j]);
                        } else {
                            printf(" %02x ", buffer[i+j]);
                        }
                    }
                }
                printf("\n");
                found++;
                i += searchLength - 1;
            }
        }
    }

    fclose(file);
    return 0;
}

int processEntry(const char *fpath, const struct stat *sb, int typeflag) {
    if (typeflag == FTW_F) {
        searchStringInFile(fpath, searchString);
    }
    return 0;
}

void printHelp() {
    printf("Использование:\n");
    printf("lab11vslN3245 [опции] <директория> \"<строка для поиска>\"\n");
    printf("Опции:\n");
    printf("-v, --version    Вывод версии программы\n");
    printf("-h, --help       Вывод этой справки\n");
}

void printVersion() {
    printf("lab11vslN3245, версия 1.0\n");
}

int main(int argc, char *argv[]) {
    int opt;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
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
        fprintf(stderr, "Недостаточно аргументов\n");
        printHelp();
        return EXIT_FAILURE;
    }

    searchString = argv[optind + 1];

    if (ftw(argv[optind], processEntry, 20) == -1) {
        perror("ftw");
        return EXIT_FAILURE;
    }

    printf("\nВ каталоге %s найдено %d совпадений\n", argv[optind], found);

    return EXIT_SUCCESS;
}
