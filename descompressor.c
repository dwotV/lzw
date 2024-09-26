#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DICT_SIZE 4096
#define MAX_STRING_SIZE 256

// Estructura del diccionario para manejar cadenas de bytes
typedef struct {
    unsigned char *str;
    int length;
} DictionaryEntry;

// Función para inicializar el diccionario con los primeros 256 códigos (ASCII/bytes)
void initialize_dictionary(DictionaryEntry *dictionary) {
    for (int i = 0; i < 256; i++) {
        dictionary[i].str = (unsigned char *)malloc(1);
        dictionary[i].str[0] = i;
        dictionary[i].length = 1;
    }
}

// Función para agregar una nueva entrada al diccionario
void add_to_dictionary(DictionaryEntry *dictionary, unsigned char *str, int length, int *dict_size) {
    if (*dict_size < MAX_DICT_SIZE) {
        dictionary[*dict_size].str = (unsigned char *)malloc(length);
        memcpy(dictionary[*dict_size].str, str, length);
        dictionary[*dict_size].length = length;
        (*dict_size)++;
    }
}

// Función para liberar la memoria del diccionario
void free_dictionary(DictionaryEntry *dictionary, int dict_size) {
    for (int i = 0; i < dict_size; i++) {
        free(dictionary[i].str);
    }
}

// Función para leer un código de 12 bits desde el archivo de entrada
int read_code(FILE *input_file) {
    static int buffer = 0, bits_in_buffer = 0;
    int code;
    
    while (bits_in_buffer < 12) {
        int next_byte = fgetc(input_file);
        if (next_byte == EOF) return EOF;
        buffer = (buffer << 8) | next_byte;
        bits_in_buffer += 8;
    }

    bits_in_buffer -= 12;
    code = (buffer >> bits_in_buffer) & 0xFFF;  // Extraer los 12 bits del buffer

    return code;
}

// Función para descomprimir los datos LZW
void decompress(FILE *input_file, FILE *output_file) {
    DictionaryEntry dictionary[MAX_DICT_SIZE];
    initialize_dictionary(dictionary);
    int dict_size = 256;

    int old_code = read_code(input_file);
    if (old_code == EOF) return;

    // Salida inicial del primer código
    fwrite(dictionary[old_code].str, 1, dictionary[old_code].length, output_file);

    unsigned char current_string[MAX_STRING_SIZE];
    memcpy(current_string, dictionary[old_code].str, dictionary[old_code].length);
    int current_length = dictionary[old_code].length;

    int new_code;
    while ((new_code = read_code(input_file)) != EOF) {
        unsigned char entry[MAX_STRING_SIZE];
        int entry_length;

        if (new_code < dict_size) {
            // Si el código está en el diccionario
            memcpy(entry, dictionary[new_code].str, dictionary[new_code].length);
            entry_length = dictionary[new_code].length;
        } else {
            // Caso especial: si el código no está en el diccionario
            memcpy(entry, current_string, current_length);
            entry[current_length] = current_string[0];  // Agregar el primer carácter del string actual
            entry_length = current_length + 1;
        }

        // Escribir la cadena correspondiente al código en el archivo de salida
        fwrite(entry, 1, entry_length, output_file);

        // Agregar nueva entrada al diccionario: old_code + primer carácter de entry
        unsigned char new_entry[MAX_STRING_SIZE];
        memcpy(new_entry, current_string, current_length);
        new_entry[current_length] = entry[0];  // Primer carácter de la nueva cadena
        add_to_dictionary(dictionary, new_entry, current_length + 1, &dict_size);

        // Actualizar current_string y old_code
        memcpy(current_string, entry, entry_length);
        current_length = entry_length;
        old_code = new_code;
    }

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

    decompress(input_file, output_file);

    fclose(input_file);
    fclose(output_file);

    return 0;
}
