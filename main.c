#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Максимальное количество поддиректорий
#define MAX_SUBDIRECTORIES 100

// Структура для представления каталогов и файлов
typedef struct Entry {
    char name[255];
    int is_directory;
    char content[1024];
    struct Entry *sub_entries[MAX_SUBDIRECTORIES];
    int num_sub_entries;
    struct Entry *parent;
} Entry;

// Функция для создания новой записи (файла или папки)
Entry* create_entry(char name[], int is_directory, char content[], Entry *parent) {
    Entry *entry = (Entry*)malloc(sizeof(Entry));
    strcpy(entry->name, name);
    entry->is_directory = is_directory;
    strcpy(entry->content, content);
    entry->num_sub_entries = 0;
    entry->parent = parent;
    return entry;
}

// Функция для добавления подзаписи (файла или папки) к записи
void add_sub_entry(Entry *parent, Entry *sub_entry) {
    if (parent->num_sub_entries < MAX_SUBDIRECTORIES) {
        parent->sub_entries[parent->num_sub_entries] = sub_entry;
        parent->num_sub_entries++;
    }
}

// Функция для поиска записи (файла или папки) по имени внутри папки
Entry* find_entry_by_name(Entry *parent, const char *name) {
    for (int i = 0; i < parent->num_sub_entries; i++) {
        if (strcmp(parent->sub_entries[i]->name, name) == 0) {
            return parent->sub_entries[i];
        }
    }
    return NULL;
}

// Функция для вывода содержимого текущей директории
void list_directory(Entry *current_directory) {
    if (current_directory->parent != NULL) {
        printf(".\n");
        printf("..\n");
    }
    else {
        printf(".\n");
    }
    for (int i = 0; i < current_directory->num_sub_entries; i++) {
        printf("%s\n", current_directory->sub_entries[i]->name);
    }
}

// Function to change the current directory
Entry* change_directory(Entry *current_directory, Entry *root, const char *target) {
    if (strcmp(target, "..") == 0 && current_directory->parent != NULL) {
        // Move up one level
        return current_directory->parent;
    } else if (strcmp(target, ".") == 0 || strcmp(target, "") == 0) {
        // Stay in the current directory
        return current_directory;
    } else if (strcmp(target, "/") == 0) {
        // Change to the root directory
        return root;
    }

    if (target[0] == '.' && target[1] == '/') {
        // Handle paths starting with './' (current directory)
        target += 2; // Skip the "./" prefix
    } else if (target[0] == '/') {
        // Handle absolute paths
        current_directory = root;
        target++; // Skip the leading '/'
    }

    char *path_copy = strdup(target);
    char *token = strtok(path_copy, "/");
    Entry *new_directory = current_directory;

    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            if (new_directory->parent != NULL) {
                new_directory = new_directory->parent;
            }
        } else {
            Entry *found_entry = find_entry_by_name(new_directory, token);

            if (found_entry != NULL && found_entry->is_directory) {
                new_directory = found_entry;
            } else {
                printf("Folder not found: %s\n", token);
                free(path_copy);
                return current_directory;
            }
        }
        token = strtok(NULL, "/");
    }

    free(path_copy);
    return new_directory;
}

// Функция для создания новой папки
void create_directory(Entry *current_directory, const char *name) {
    Entry *new_directory = create_entry(name, 1, "", current_directory);
    add_sub_entry(current_directory, new_directory);
}

// Функция для создания нового файла
void create_file(Entry *current_directory, const char *name, const char *content) {
    Entry *new_file = create_entry(name, 0, content, current_directory);
    add_sub_entry(current_directory, new_file);
}

// Функция для получения полного пути текущей директории
void get_current_path(Entry *current_directory, char *path) {
    Entry *stack[MAX_SUBDIRECTORIES]; // Стек для хранения директорий
    int depth = 0; // Глубина стека

    // Помещаем директории в стек, начиная с текущей и двигаясь вверх по иерархии
    while (current_directory != NULL) {
        stack[depth] = current_directory;
        current_directory = current_directory->parent;
        depth++;
    }

    if (depth == 1) {
        strcat(path, "/");
        return;
    }

    depth--; // skip root dir

    // Собираем путь, начиная с корневой директории и двигаясь вниз по стеку
    while (depth > 0) {
        depth--;
        strcat(path, "/");
        strcat(path, stack[depth]->name);
    }
}

// Function to clear the content of an Entry and its sub-entries
void clear_entry(Entry *entry) {
    if (entry->is_directory) {
        for (int i = 0; i < entry->num_sub_entries; i++) {
            clear_entry(entry->sub_entries[i]);
            free(entry->sub_entries[i]);
        }
        entry->num_sub_entries = 0; // Clear the number of sub-entries
    } else {
        entry->content[0] = '\0'; // Clear file content
    }
}

int main() {
    Entry *root = create_entry("", 1, "", NULL);
    Entry *current_directory = root;

    char command[255];

    while (1) {
        char current_path[1024] = "";
        get_current_path(current_directory, current_path);
        printf("%s>", current_path);
        scanf("%s", command);

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "ls") == 0) {
            list_directory(current_directory);
        } else if (strcmp(command, "cd") == 0) {
            char target[255];
            scanf("%s", target);
            current_directory = change_directory(current_directory, root, target);
        } else if (strcmp(command, "mkdir") == 0) {
            char name[255];
            scanf("%s", name);
            if (find_entry_by_name(current_directory, name) == NULL) {
                create_directory(current_directory, name);
            } else {
                printf("Папка с таким именем уже существует.\n");
            }
        } else if (strcmp(command, "touch") == 0) {
            char name[255];
            char content[1024];
            scanf("%s", name);
            scanf("%s", content);
            if (find_entry_by_name(current_directory, name) == NULL) {
                create_file(current_directory, name, content);
            } else {
                printf("Файл с таким именем уже существует.\n");
            }
        } else if (strcmp(command, "format") == 0) {
            clear_entry(root);
            root = create_entry("", 1, "", NULL);
            current_directory = root;
            printf("Ok\n");
        }
        else {
            printf("Неизвестная команда.\n");
        }
    }

    // Очистка памяти
    free(root);

    return 0;
}
