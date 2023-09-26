#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Максимальное количество поддиректорий
#define MAX_SUBDIRECTORIES 100

// Максимальное количество entry
#define MAX_ENTRIES 10000

// Структура для представления каталогов и файлов
typedef struct Entry {
    char name[255];
    int is_directory;
    char content[1024];
    int sub_entries[MAX_SUBDIRECTORIES];
    int num_sub_entries;
    int parent; // Index of the parent entry
} Entry;

// Глобальный массив entry
Entry entries[MAX_ENTRIES];

// Функция для создания новой записи (файла или папки)
int create_entry(Entry entries[], char name[], int is_directory, char content[], int parent) {
    static int entryIndex = 0;
    if (entryIndex < MAX_SUBDIRECTORIES) {
        strcpy(entries[entryIndex].name, name);
        entries[entryIndex].is_directory = is_directory;
        strcpy(entries[entryIndex].content, content);
        entries[entryIndex].num_sub_entries = 0;
        entries[entryIndex].parent = parent;
        return entryIndex++;
    }
    return -1; // Return -1 to indicate failure (array is full)
}

// Функция для добавления подзаписи (файла или папки) к записи
void add_sub_entry(Entry entries[], int parentIndex, int subEntryIndex) {
    if (entries[parentIndex].num_sub_entries < MAX_SUBDIRECTORIES) {
        entries[parentIndex].sub_entries[entries[parentIndex].num_sub_entries] = subEntryIndex;
        entries[parentIndex].num_sub_entries++;
    }
}

// Функция для поиска записи (файла или папки) по имени внутри папки
int find_entry_by_name(Entry entries[], int parentIndex, const char *name) {
    for (int i = 0; i < entries[parentIndex].num_sub_entries; i++) {
        int subEntryIndex = entries[parentIndex].sub_entries[i];
        if (strcmp(entries[subEntryIndex].name, name) == 0) {
            return subEntryIndex;
        }
    }
    return -1; // Return -1 to indicate not found
}

// Функция для вывода содержимого текущей директории
void list_directory(Entry entries[], int currentDirectoryIndex) {
    if (currentDirectoryIndex != 0) {
        printf(".\n");
        printf("..\n");
    }
    else {
        printf(".\n");
    }
    for (int i = 0; i < entries[currentDirectoryIndex].num_sub_entries; i++) {
        int subEntryIndex = entries[currentDirectoryIndex].sub_entries[i];
        printf("%s\n", entries[subEntryIndex].name);
    }
}

// Function to change the current directory
int change_directory(Entry entries[], int currentDirectoryIndex, const char *target) {
    if (strcmp(target, "..") == 0 && currentDirectoryIndex != 0) {
        // Move up one level
        return entries[currentDirectoryIndex].parent;
    } else if (strcmp(target, ".") == 0 || strcmp(target, "") == 0) {
        // Stay in the current directory
        return currentDirectoryIndex;
    } else if (strcmp(target, "/") == 0) {
        // Change to the root directory
        return 0;
    }

    if (target[0] == '.' && target[1] == '/') {
        // Handle paths starting with './' (current directory)
        target += 2; // Skip the "./" prefix
    } else if (target[0] == '/') {
        // Handle absolute paths
        currentDirectoryIndex = 0;
        target++; // Skip the leading '/'
    }

    char *pathCopy = strdup(target);
    char *token = strtok(pathCopy, "/");
    int newDirectoryIndex = currentDirectoryIndex;

    while (token != NULL) {
        if (strcmp(token, "..") == 0) {
            if (entries[newDirectoryIndex].parent != -1) {
                newDirectoryIndex = entries[newDirectoryIndex].parent;
            }
        } else {
            int foundEntryIndex = find_entry_by_name(entries, newDirectoryIndex, token);

            if (foundEntryIndex != -1 && entries[foundEntryIndex].is_directory) {
                newDirectoryIndex = foundEntryIndex;
            } else {
                printf("Folder not found: %s\n", token);
                free(pathCopy);
                return currentDirectoryIndex;
            }
        }
        token = strtok(NULL, "/");
    }

    free(pathCopy);
    return newDirectoryIndex;
}

// Функция для создания новой папки
int create_directory(Entry entries[], int currentDirectoryIndex, const char *name) {
    int newDirectoryIndex = create_entry(entries, name, 1, "", currentDirectoryIndex);
    if (newDirectoryIndex != -1) {
        add_sub_entry(entries, currentDirectoryIndex, newDirectoryIndex);
    } else {
        printf("Maximum subdirectories reached.\n");
    }
    return newDirectoryIndex;
}

// Функция для создания нового файла
void create_file(Entry entries[], int currentDirectoryIndex, const char *name, const char *content) {
    int newFileIndex = create_entry(entries, name, 0, content, currentDirectoryIndex);
    if (newFileIndex != -1) {
        add_sub_entry(entries, currentDirectoryIndex, newFileIndex);
    } else {
        printf("Maximum subdirectories reached.\n");
    }
}

// Function to get the full path of the current directory
void get_current_path(Entry entries[], int currentDirectoryIndex, char *path) {
    int stack[MAX_SUBDIRECTORIES]; // Stack to store directory indices
    int depth = 0; // Depth of the stack

    // Traverse up the directory hierarchy, starting from the current directory
    while (currentDirectoryIndex != -1) {
        stack[depth] = currentDirectoryIndex;
        currentDirectoryIndex = entries[currentDirectoryIndex].parent;
        depth++;
    }

    if (depth == 1) {
        // Special case for the root directory
        strcat(path, "/");
        return;
    }

    // Build the path by descending the stack, skipping the root directory
    for (int i = depth - 2; i >= 0; i--) {
        strcat(path, "/");
        strcat(path, entries[stack[i]].name);
    }
}

// Function to clear the content of an Entry and its sub-entries
void clear_entry(Entry entries[], int entryIndex) {
    if (entries[entryIndex].is_directory) {
        for (int i = 0; i < entries[entryIndex].num_sub_entries; i++) {
            clear_entry(entries, entries[entryIndex].sub_entries[i]);
        }
        entries[entryIndex].num_sub_entries = 0; // Clear the number of sub-entries
    } else {
        entries[entryIndex].content[0] = '\0'; // Clear file content
    }
}

// Function to save the file system to a file
void save_to_file(Entry entries[], const char *filename) {
    FILE *file = fopen(filename, "wb"); // Open the file for writing in binary mode

    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    // Write the entire entries array to the file
    fwrite(entries, sizeof(Entry), MAX_ENTRIES, file);

    fclose(file);
}

// Function to load the file system from a file
void load_from_file(Entry entries[], const char *filename) {
    FILE *file = fopen(filename, "rb"); // Open the file for reading in binary mode

    if (file == NULL) {
        printf("Error opening file for reading.\n");
        return;
    }

    // Read the entire entries array from the file
    fread(entries, sizeof(Entry), MAX_ENTRIES, file);

    fclose(file);
}

int main(int argc, char *argv[]) {
    int rootIndex, currentDirectoryIndex;
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    memset(entries, 0, sizeof(entries)); // Initialize entries to zero

    // Check if the file exists and load the file system if it does
    FILE *checkFile = fopen(filename, "rb");
    if (checkFile != NULL) {
        fclose(checkFile);
        load_from_file(entries, filename);
        printf("File system loaded from %s.\n", filename);
    } else {
        // Create a new root directory if the file does not exist
        int rootIndex = create_entry(entries, "", 1, "", -1);
        int currentDirectoryIndex = rootIndex;
        printf("New file system created.\n");
    }
    char command[255];

    while (1) {
        char currentPath[1024] = "";
        get_current_path(entries, currentDirectoryIndex, currentPath);
        printf("%s>", currentPath);
        scanf("%s", command);

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "ls") == 0) {
            list_directory(entries, currentDirectoryIndex);
        } else if (strcmp(command, "cd") == 0) {
            char target[255];
            scanf("%s", target);
            currentDirectoryIndex = change_directory(entries, currentDirectoryIndex, target);
        } else if (strcmp(command, "mkdir") == 0) {
            char name[255];
            scanf("%s", name);
            int newDirectoryIndex = create_directory(entries, currentDirectoryIndex, name);
            if (newDirectoryIndex == -1) {
                printf("Folder not created.\n");
            }
        } else if (strcmp(command, "touch") == 0) {
            char name[255];
            char content[1024];
            scanf("%s", name);
            scanf("%s", content);
            create_file(entries, currentDirectoryIndex, name, content);
        } else if (strcmp(command, "format") == 0) {
            clear_entry(entries, rootIndex);
            rootIndex = create_entry(entries, "", 1, "", -1);
            currentDirectoryIndex = rootIndex;
            printf("Ok\n");
        } else {
            printf("Unknown command.\n");
        }
    }
    save_to_file(entries, filename);
    return 0;
}