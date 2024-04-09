#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <getopt.h>
#include <time.h>

const char* searchString; // Глобальная переменная для хранения строки для поиска
int found = 0;

// Список шуток
const char* jokes[] = {
    "Штирлиц на даче открыл дверь. Включился свет. Штирлиц закрыл дверь. Свет выключился. Холодильник, догадался он.",
    "Из окна дуло. Штирлиц подошел к окну. Дуло исчезло.",
    "Штирлиц топил печку. Через час печка утонула.",
    "Штирлиц мог спать сутками. Но утки с ним спать не хотели.",
    "Встретив гестаповцев, Штирлиц выхватил шашку и закричал: “Порублю!”. Гестаповцы скинулись по рублю и убежали.",
    "Штирлиц вошёл в комнату, из окна дуло. Штирлиц закрыл окно, дуло исчезло.",
    "Штирлиц попал в глубокую яму и чудом из нее вылез. «Чудес не бывает», — подумал Штирлиц и на всякий случай залез обратно.",
    "Штирлицу за шиворот упала гусеница. «Неподалеку взорвался танк», — догадался Штирлиц."
};
const int jokesCount = sizeof(jokes) / sizeof(jokes[0]);

void printJoke() {
    srand(time(NULL)); // Инициализация генератора случайных чисел
    int jokeIndex = rand() % jokesCount; // Получение случайного индекса
    printf("%s\n", jokes[jokeIndex]); // Вывод шутки
}

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
        for (size_t i =  0; i < bytesRead; ++i) {
            if (i + searchLength <= bytesRead && memcmp(&buffer[i], searchStr, searchLength) == 0) {
                printf("найдено \"%s\" (позиция начала - %zu): ", searchStr, i);
                for (int j = -2; j < (int)searchLength + 2; ++j) {
                    if (i+j >= 0 && i+j < bytesRead) {
                        if (j == 0) {
                            printf("[%02x ", buffer[i+j]);
                        } else if (j == (int)searchLength - 1) {
                            printf("%02x] ", buffer[i+j]);
                        } else {
                            printf("%02x ", buffer[i+j]);
                        }
                    }
                }
                printf(" файл: %s\n", filepath);
                found++;
                i += searchLength - 1; // Перемещаем индекс на конец найденной последовательности
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
    printf("lab11vslN3245 [опции] <директория> \"строка для поиска\"\n");
    printf("Опции:\n");
    printf("-v, --version    Вывод версии программы\n");
    printf("-h, --help       Вывод этой справки\n");
    printf("-j, --joke       Вывод случайной шутки\n");
}

void printVersion() {
    printf("lab11vslN3245, версия 1.0\n");
}

int main(int argc, char *argv[]) {
    int opt;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"joke", no_argument, 0, 'j'}, // Добавление новой опции
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hvj", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return EXIT_SUCCESS;
            case 'v':
                printVersion();
                return EXIT_SUCCESS;
            case 'j':
                printJoke(); // Вызов функции вывода шутки
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
