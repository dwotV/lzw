#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DICT_SIZE 4096 // 2^12 = 4096 posibles códigos (12 bits)
#define MAX_STRING_SIZE 256

// Estructura de entrada en el diccionario
typedef struct {
    char *str;
} DictionaryEntry;

// Funciones para inicializar y gestionar el diccionario
void initialize_dictionary(DictionaryEntry *dictionary) {
    for (int i = 0; i < 256; i++) {
        dictionary[i].str = (char *)malloc(2 * sizeof(char));
        dictionary[i].str[0] = i;
        dictionary[i].str[1] = '\0';
    }
}

int search_in_dictionary(DictionaryEntry *dictionary, char *str, int dict_size) {
    for (int i = 0; i < dict_size; i++) {
        if (strcmp(dictionary[i].str, str) == 0) {
            return i;
        }
    }
    return -1;
}

void add_to_dictionary(DictionaryEntry *dictionary, char *str, int *dict_size) {
    if (*dict_size < MAX_DICT_SIZE) {
        dictionary[*dict_size].str = (char *)malloc(strlen(str) + 1);
        strcpy(dictionary[*dict_size].str, str);
        (*dict_size)++;
    }
}

void free_dictionary(DictionaryEntry *dictionary, int dict_size) {
    for (int i = 0; i < dict_size; i++) {
        free(dictionary[i].str);
    }
}

void output_code(int code, FILE *output_file) {
    // Salida de código con 12 bits
    static int buffer = 0, bits_in_buffer = 0;
    buffer = (buffer << 12) | code;
    bits_in_buffer += 12;

    if (bits_in_buffer >= 16) {
        // Escribir los primeros 16 bits en el archivo
        fputc((buffer >> (bits_in_buffer - 8)) & 0xFF, output_file);
        bits_in_buffer -= 8;
        fputc((buffer >> (bits_in_buffer - 8)) & 0xFF, output_file);
        bits_in_buffer -= 8;
    }
}

void flush_buffer(FILE *output_file) {
    // Vaciar el buffer restante
    static int buffer = 0, bits_in_buffer = 0;
    if (bits_in_buffer > 0) {
        fputc((buffer >> (bits_in_buffer - 8)) & 0xFF, output_file);
        bits_in_buffer = 0;
    }
}

void compress(FILE *input_file, FILE *output_file) {
    DictionaryEntry dictionary[MAX_DICT_SIZE];
    initialize_dictionary(dictionary);
    int dict_size = 256;

    char current_string[MAX_STRING_SIZE] = "";
    int current_char;

    while ((current_char = fgetc(input_file)) != EOF) {
        char combined_string[MAX_STRING_SIZE];
        snprintf(combined_string, sizeof(combined_string), "%s%c", current_string, current_char);

        if (search_in_dictionary(dictionary, combined_string, dict_size) != -1) {
            strcpy(current_string, combined_string);
        } else {
            output_code(search_in_dictionary(dictionary, current_string, dict_size), output_file);
            add_to_dictionary(dictionary, combined_string, &dict_size);
            current_string[0] = current_char;
            current_string[1] = '\0';
        }
    }

    if (current_string[0] != '\0') {
        output_code(search_in_dictionary(dictionary, current_string, dict_size), output_file);
    }

    flush_buffer(output_file);
    free_dictionary(dictionary, dict_size);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo_entrada> <archivo_salida>\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "rb");
    if (!input_file) {
        perror("Error al abrir el archivo de entrada");
        return 1;
    }

    FILE *output_file = fopen(argv[2], "wb");
    if (!output_file) {
        perror("Error al abrir el archivo de salida");
        fclose(input_file);
        return 1;
    }

    compress(input_file, output_file);

    fclose(input_file);
    fclose(output_file);

    return 0;
}

