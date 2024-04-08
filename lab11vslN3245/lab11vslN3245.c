#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

// Определение длинных опций
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"add", no_argument, 0, 'a'},
    {"file", required_argument, 0, 'f'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
    int opt = 0;
    int option_index = 0;

    // Цикл обработки каждого аргумента
    while ((opt = getopt_long(argc, argv, "haf:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                printf("Показать помощь\n");
                break;
            case 'a':
                printf("Добавить запись\n");
                break;
            case 'f':
                printf("Использовать файл: %s\n", optarg);
                break;
            case '?':
                // В случае неизвестного аргумента
                break;
            default:
                abort();
        }
    }

    // Обработка позиционных аргументов
    for (int index = optind; index < argc; index++) {
        printf("Неопционный аргумент: %s\n", argv[index]);
    }

    return 0;
}
