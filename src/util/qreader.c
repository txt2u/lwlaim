#include <stdio.h>
#include <stdlib.h>

char* read_file(const char* filePath) {
    FILE* file = fopen(filePath, "rb");  // Open file in binary mode
    if (!file) {
        fprintf(stderr, "Could not open file %s\n", filePath);
        return NULL;
    }

    // Seek to the end of the file to get its size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    if (length == -1) {
        fprintf(stderr, "Failed to determine file size: %s\n", filePath);
        fclose(file);
        return NULL;
    }

    // Seek back to the beginning of the file
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char* content = (char*)malloc(length + 1);  // +1 for the null-terminator
    if (!content) {
        fprintf(stderr, "Failed to allocate memory for file content.\n");
        fclose(file);
        return NULL;
    }

    // Read the file content into memory
    size_t bytesRead = fread(content, 1, length, file);
    if (bytesRead != length) {
        fprintf(stderr, "Failed to read the full file %s (only %zu bytes read).\n", filePath, bytesRead);
        free(content);  // Free the allocated memory if read fails
        fclose(file);
        return NULL;
    }

    // Null-terminate the string
    content[length] = '\0';

    // Close the file
    fclose(file);

    return content;
}