#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <getopt.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

const char *searchString; // Глобальная переменная для хранения строки для поиска
int found = 0;

// Список шуток
const char *jokes[] = {
    "Штирлиц на даче открыл дверь. Включился свет. Штирлиц закрыл дверь. Свет выключился. Холодильник, догадался он.",
    "Из окна дуло. Штирлиц подошел к окну. Дуло исчезло.",
    "Штирлиц топил печку. Через час печка утонула.",
    "Штирлиц мог спать сутками. Но утки с ним спать не хотели.",
    "Встретив гестаповцев, Штирлиц выхватил шашку и закричал: “Порублю!”. Гестаповцы скинулись по рублю и убежали.",
    "Штирлиц вошёл в комнату, из окна дуло. Штирлиц закрыл окно, дуло исчезло.",
    "Штирлиц попал в глубокую яму и чудом из нее вылез. «Чудес не бывает», — подумал Штирлиц и на всякий случай залез обратно.",
    "Штирлицу за шиворот упала гусеница. «Неподалеку взорвался танк», — догадался Штирлиц."};
const int jokesCount = sizeof(jokes) / sizeof(jokes[0]);

void printJoke()
{
    srand(time(NULL));                   // Инициализация генератора случайных чисел
    int jokeIndex = rand() % jokesCount; // Получение случайного индекса
    printf("%s\n", jokes[jokeIndex]);    // Вывод шутки
}

void debugPrint(const char *message)
{
    if (getenv("LAB11DEBUG") != NULL)
    {
        fprintf(stderr, "DEBUG: %s\n", message);
    }
}

int searchStringInFile(const char *filepath, const char *searchStr)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        fprintf(stderr, "Ошибка при открытии файла: %s", filepath);
        perror("Ошибка при открытии файла");
        return -1;
    }

    const size_t searchLength = strlen(searchStr); // Вычисляем длину строки для поиска
    if (searchLength == 0)
    {
        fclose(file); // Если строка поиска пуста, закрываем файл и выходим
        return 0;
    }

    unsigned char buffer[1024 + searchLength - 1]; // Увеличенный буфер
    size_t prevPortion = 0;                        // Размер сохраненных данных от предыдущего чтения

    while (1)
    {
        size_t bytesRead = fread(buffer + prevPortion, 1, sizeof(buffer) - prevPortion, file);
        size_t totalBytes = bytesRead + prevPortion; // Общее количество байт в буфере

        if (totalBytes == 0)
            break; // Если ничего не прочитано и ничего не сохранено, выходим

        for (size_t i = 0; i < totalBytes; ++i)
        {
            if (i + searchLength <= totalBytes && memcmp(&buffer[i], searchStr, searchLength) == 0)
            {
                printf("Найдено \"%s\" (поз. %zu): ", searchStr, i);
                for (int j = -2; j < (int)searchLength + 2; ++j)
                {
                    if ((j >= 0 || (size_t)(-j) <= i) && (i + j < totalBytes))
                    {
                        if (j == 0)
                        {
                            printf("[ "); // Начало искомой последовательности
                        }
                        printf("%02x ", buffer[i + j]);
                        if (j == (int)searchLength - 1)
                        {
                            printf("] "); // Конец искомой последовательности
                        }
                    }
                }
                printf("в файле: %s\n", filepath);
                found++;
                i += searchLength - 1; // Перемещаем индекс за конец найденной последовательности
            }
        }

        if (bytesRead == 0)
            break; // Если ничего не прочитано, выходим из цикла

        if (bytesRead < sizeof(buffer) - prevPortion)
        {
            // Если это последнее чтение и оно не заполнило весь буфер, завершаем
            break;
        }

        prevPortion = searchLength - 1; // Сохраняем последние байты для следующего чтения
        memmove(buffer, buffer + sizeof(buffer) - prevPortion, prevPortion);
    }

    fclose(file);
    return 0;
}

int processEntry(const char *fpath, const struct stat *sb, int typeflag) {
    (void)sb; // Не используемый параметр

    struct stat path_stat;
    if (lstat(fpath, &path_stat) < 0) {
        fprintf(stderr, "Не удалось получить информацию о файле: %s\n", fpath);
        perror("lstat");
        return 0; // Возвращаем 0, чтобы ftw продолжала обход
    }

    // Пропускаем символические ссылки
    if (S_ISLNK(path_stat.st_mode)) {
        fprintf(stderr, "Путь является символической ссылкой, поэтому пропускаем его: %s\n", fpath);
        return 0; // Пропускаем символическую ссылку
    }

    // Пропускаем, если нет доступа на чтение
    if (access(fpath, R_OK) < 0) {
        fprintf(stderr, "Не удалось открыть файл на чтение: %s\n", fpath);
        perror("Доступ на чтение файла отсутствует");
        return 0; // Возвращаем 0, чтобы ftw продолжала обход
    }

    if (typeflag == FTW_F) {
        searchStringInFile(fpath, searchString);
    }

    return 0; // Возвращаем 0, чтобы ftw продолжала обход
}

void printHelp()
{
    printf("Использование:\n");
    printf("lab11vslN3245 [опции] <директория> \"строка для поиска\"\n");
    printf("Опции:\n");
    printf("-v, --version    Вывод версии программы\n");
    printf("-h, --help       Вывод этой справки\n");
    printf("-j, --joke       Вывод случайной шутки\n");
}

void printVersion()
{
    printf("lab11vslN3245, версия 1.0\n");
}

int main(int argc, char *argv[])
{
    int opt;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"joke", no_argument, 0, 'j'}, // Добавление новой опции
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "hvj", long_options, NULL)) != -1)
    {
        switch (opt)
        {
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

    if (argc - optind < 2)
    {
        fprintf(stderr, "Недостаточно аргументов\n\n");
        printHelp();
        return EXIT_FAILURE;
    }

    searchString = argv[optind + 1];

    // Используем lstat для проверки на символические ссылки и наличие файла
    struct stat path_stat;
    if (lstat(argv[optind], &path_stat) < 0)
    {
        perror("Ошибка при получении информации об указанной директории");
        return EXIT_FAILURE;
    }

    // Проверяем, является ли путь директорией
    if (!S_ISDIR(path_stat.st_mode))
    {
        fprintf(stderr, "Указанный аргумент не является директорией\n");
        return EXIT_FAILURE;
    }

    // Проверка на доступность директории для чтения
    if (access(argv[optind], R_OK) < 0)
    {
        perror("Указанная директория не доступна для чтения");
        return EXIT_FAILURE;
    }

    if (ftw(argv[optind], processEntry, 20) == -1)
    {
        perror("ftw");
        return EXIT_FAILURE;
    }

    printf("\nВ каталоге %s найдено %d совпадений\n", argv[optind], found);

    return EXIT_SUCCESS;
}