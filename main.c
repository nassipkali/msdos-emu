#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DISK_SIZE 20 * 1024 * 1024 // Размер диска (20 МБ)
#define FILE_SYSTEM_SIGNATURE 0x12345678 // Сигнатура для проверки формата FAT32

typedef struct {
    char name[255];
    int is_directory;
} Entry;

typedef struct {
    int signature;
    Entry* entries;
    int entry_count;
    char current_path[255];
} FileSystem;

// Инициализация файловой системы
void initialize(FileSystem* fs) {
    fs->signature = FILE_SYSTEM_SIGNATURE;
    fs->entries = NULL;
    fs->entry_count = 0;
    strcpy(fs->current_path, "/");
}

// Проверка формата диска
int checkDiskFormat(FileSystem* fs) {
    return fs->signature == FILE_SYSTEM_SIGNATURE;
}

// Функция форматирования диска
void formatDisk(FileSystem* fs) {
    if (fs->entries != NULL) {
        free(fs->entries);
    }
    initialize(fs);
}

// Функция вывода содержимого текущей директории
void listDirectory(FileSystem* fs) {
    if (!checkDiskFormat(fs)) {
        printf("Unknown disk format\n");
        return;
    }

    printf(".\n");
    printf("..\n");
    for (int i = 0; i < fs->entry_count; i++) {
        if (strncmp(fs->entries[i].name, fs->current_path, strlen(fs->current_path)) == 0) {
            char* relative_name = fs->entries[i].name + strlen(fs->current_path);
            if (relative_name[0] == '/') {
                relative_name++;
            }
            printf("%s\n", relative_name);
        }
    }
}

// Функция для создания каталога
void createDirectory(FileSystem* fs, const char* name) {
    if (!checkDiskFormat(fs)) {
        printf("Unknown disk format\n");
        return;
    }

    // Проверка, что имя не пустое
    if (strlen(name) == 0) {
        printf("Invalid directory name\n");
        return;
    }

    // Проверка, что каталог с таким именем не существует
    for (int i = 0; i < fs->entry_count; i++) {
        if (fs->entries[i].is_directory && strcmp(fs->entries[i].name, name) == 0) {
            printf("Directory '%s' already exists\n", name);
            return;
        }
    }

    // Создание каталога
    Entry* new_entry = malloc(sizeof(Entry));
    strcpy(new_entry->name, name);
    new_entry->is_directory = 1;
    fs->entries = realloc(fs->entries, (fs->entry_count + 1) * sizeof(Entry));
    fs->entries[fs->entry_count] = *new_entry;
    fs->entry_count++;
}

// Функция для создания файла
void createFile(FileSystem* fs, const char* name) {
    if (!checkDiskFormat(fs)) {
        printf("Unknown disk format\n");
        return;
    }

    // Проверка, что имя не пустое
    if (strlen(name) == 0) {
        printf("Invalid file name\n");
        return;
    }

    // Проверка, что файл с таким именем не существует
    for (int i = 0; i < fs->entry_count; i++) {
        if (!fs->entries[i].is_directory && strcmp(fs->entries[i].name, name) == 0) {
            printf("File '%s' already exists\n", name);
            return;
        }
    }

    // Создание файла
    Entry* new_entry = malloc(sizeof(Entry));
    strcpy(new_entry->name, name);
    new_entry->is_directory = 0;
    fs->entries = realloc(fs->entries, (fs->entry_count + 1) * sizeof(Entry));
    fs->entries[fs->entry_count] = *new_entry;
    fs->entry_count++;
}

// Функция для перехода в другую директорию
void changeDirectory(FileSystem* fs, const char* path) {
    if (!checkDiskFormat(fs)) {
        printf("Unknown disk format\n");
        return;
    }

    // Проверка, что путь существует
    for (int i = 0; i < fs->entry_count; i++) {
        if (fs->entries[i].is_directory && strcmp(fs->entries[i].name, path) == 0) {
            strcpy(fs->current_path, path);
            return;
        }
    }

    printf("Path '%s' does not exist\n", path);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <disk_file_path>\n", argv[0]);
        return 1;
    }

    // Создание файла диска или открытие существующего
    FILE* disk_file = fopen(argv[1], "r+b");
    if (disk_file == NULL) {
        disk_file = fopen(argv[1], "w+b");
        if (disk_file == NULL) {
            perror("Error opening disk file");
            return 1;
        }
        // Устанавливаем размер файла диска
        fseek(disk_file, DISK_SIZE - 1, SEEK_SET);
        fputc('\0', disk_file);
    }

    FileSystem fs;
    initialize(&fs);

    // Загрузка файловой системы из файла диска (если существует)

    // Цикл команд
    char command[255];
    while (1) {
        printf("%s>", fs.current_path);
        scanf("%s", command);

        if (strcmp(command, "format") == 0) {
            formatDisk(&fs);
            printf("Disk formatted\n");
        } else if (strcmp(command, "ls") == 0) {
            listDirectory(&fs);
        } else if (strcmp(command, "mkdir") == 0) {
            char name[255];
            scanf("%s", name);
            createDirectory(&fs, name);
            printf("Directory '%s' created\n", name);
        } else if (strcmp(command, "touch") == 0) {
            char name[255];
            scanf("%s", name);
            createFile(&fs, name);
            printf("File '%s' created\n", name);
        } else if (strcmp(command, "cd") == 0) {
            char path[255];
            scanf("%s", path);
            changeDirectory(&fs, path);
        } else {
            printf("Unknown command: %s\n", command);
        }
    }

    // Сохранение файловой системы в файл диска перед завершением программы

    return 0;
}