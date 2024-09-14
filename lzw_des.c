#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DICT_SIZE 4096  // 2^12 = 4096 posibles códigos (12 bits)
#define MAX_STRING_SIZE 256

// Estructura para representar las entradas del diccionario
typedef struct {
    char *str;
} DictionaryEntry;

// Función para inicializar el diccionario con los 256 códigos ASCII
void initialize_dictionary(DictionaryEntry *dictionary) {
    for (int i = 0; i < 256; i++) {
        dictionary[i].str = (char *)malloc(2 * sizeof(char));
        dictionary[i].str[0] = i;
        dictionary[i].str[1] = '\0';
    }
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

// Leer un código de 12 bits del archivo
int read_code(FILE *input_file) {
    static int buffer = 0, bits_in_buffer = 0;
    int code = 0;

    while (bits_in_buffer < 12) {
        int byte = fgetc(input_file);
        if (byte == EOF) return -1;

        buffer = (buffer << 8) | byte;
        bits_in_buffer += 8;
    }

    code = (buffer >> (bits_in_buffer - 12)) & 0xFFF;
    bits_in_buffer -= 12;
    return code;
}

// Descompresión usando LZW
void decompress(FILE *input_file, FILE *output_file) {
    DictionaryEntry dictionary[MAX_DICT_SIZE];
    initialize_dictionary(dictionary);
    int dict_size = 256;

    int old_code = read_code(input_file);
    if (old_code == -1) return;

    fprintf(output_file, "%s", dictionary[old_code].str);

    int new_code;
    char current_string[MAX_STRING_SIZE];
    char new_entry[MAX_STRING_SIZE];

    while ((new_code = read_code(input_file)) != -1) {
        if (new_code < dict_size) {
            strcpy(current_string, dictionary[new_code].str);
        } else if (new_code == dict_size) {
            // Caso especial: si el nuevo código es igual al tamaño actual del diccionario
            // se genera una nueva entrada con OLD_CODE + primer carácter de OLD_CODE
            snprintf(current_string, sizeof(current_string), "%s%c", dictionary[old_code].str, dictionary[old_code].str[0]);
        } else {
            fprintf(stderr, "Código inválido durante la descompresión.\n");
            break;
        }

        // Escribir la cadena correspondiente al nuevo código
        fprintf(output_file, "%s", current_string);

        // Crear una nueva entrada en el diccionario con OLD_CODE + primer carácter de STRING
        snprintf(new_entry, sizeof(new_entry), "%s%c", dictionary[old_code].str, current_string[0]);
        add_to_dictionary(dictionary, new_entry, &dict_size);

        old_code = new_code;
    }

    free_dictionary(dictionary, dict_size);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo_comprimido> <archivo_descomprimido>\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "rb");
    if (!input_file) {
        perror("Error al abrir el archivo comprimido");
        return 1;
    }

    FILE *output_file = fopen(argv[2], "wb");
    if (!output_file) {
        perror("Error al abrir el archivo descomprimido");
        fclose(input_file);
        return 1;
    }

    decompress(input_file, output_file);

    fclose(input_file);
    fclose(output_file);

    return 0;
}

