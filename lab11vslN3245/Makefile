# Определяем компилятор и флаги компиляции
CC=gcc
CFLAGS=-Wall -Wextra -Werror

# Название исполняемого файла
TARGET=lab11vslN3245

# Цель по умолчанию
all: $(TARGET)

# Как собрать исполняемый файл
$(TARGET): lab11vslN3245.c
	$(CC) $(CFLAGS) lab11vslN3245.c -o $(TARGET)

# Цель для очистки проекта от объектных файлов и исполняемого файла
clean:
	rm -f $(TARGET)
