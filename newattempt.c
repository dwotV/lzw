#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DICT_SIZE 4096  // 2^12 = 4096 posibles códigos (12 bits)
#define MAX_STRING_SIZE 256

// Estructura para representar las entradas del diccionario
typedef struct {
    unsigned char *str;
    int length;
} DictionaryEntry;

// Función para inicializar el diccionario con los 256 códigos ASCII
void initialize_dictionary(DictionaryEntry *dictionary) {
    for (int i = 0; i < 256; i++) {
        dictionary[i].str = (unsigned char *)malloc(1);
        dictionary[i].str[0] = i;
        dictionary[i].length = 1;
    }

    for (int i = 256; i < MAX_DICT_SIZE; i++) {
        dictionary[i].str = NULL;
        dictionary[i].length = 0;  // Aquí debes usar length = 0, no 1.
    }
}

void add_to_dictionary(DictionaryEntry *dictionary, unsigned char *data, int length, int *dict_size) {
    if (*dict_size < MAX_DICT_SIZE) {
        dictionary[*dict_size].str = (unsigned char *)malloc(length);
        memcpy(dictionary[*dict_size].str, data, length);  // Copiar la secuencia binaria
        dictionary[*dict_size].length = length;
        (*dict_size)++;
    }
}

void free_dictionary(DictionaryEntry *dictionary, int dict_size) {
    for (int i = 0; i < dict_size; i++) {
        free(dictionary[i].str);
    }
}

// Leer un código de 12 bits del archivo sin usar bitmasking
int read_code(FILE *input_file) {
    static int buffer = 0, bits_in_buffer = 0;
    int code = 0;

    // Mientras haya menos de 12 bits en el buffer, sigue leyendo bytes
    while (bits_in_buffer < 12) {
        int byte = fgetc(input_file);
        if (byte == EOF) return -1;  // Fin del archivo

        // Mueve el byte leído al buffer multiplicando por una potencia de 2 (equivalente a <<)
        buffer = buffer * 256 + byte;  // buffer = buffer << 8 | byte
        bits_in_buffer += 8;
    }

    // Extraer los 12 bits más significativos sin usar bitmasking
    code = buffer / (1 << (bits_in_buffer - 12));  // Equivalente a buffer >> (bits_in_buffer - 12)

    // Actualizar el buffer para eliminar los bits ya leídos
    buffer = buffer % (1 << (bits_in_buffer - 12));  // Eliminar los bits leídos (equivalente a buffer & ((1 << (bits_in_buffer - 12)) - 1))
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

    fwrite(dictionary[old_code].str, 1, dictionary[old_code].length, output_file);

    int new_code;
    unsigned char current_string[MAX_STRING_SIZE];
    unsigned char new_entry[MAX_STRING_SIZE];

    while ((new_code = read_code(input_file)) != -1) {
        if (new_code < dict_size) {
            memcpy(current_string, dictionary[new_code].str, dictionary[new_code].length);
        } else if (new_code == dict_size) {
            // Caso especial: si el nuevo código es igual al tamaño actual del diccionario
            // se genera una nueva entrada con OLD_CODE + primer byte de OLD_CODE
            memcpy(current_string, dictionary[old_code].str, dictionary[old_code].length);
            current_string[dictionary[old_code].length] = dictionary[old_code].str[0];
        } else {
            fprintf(stderr, "Código inválido durante la descompresión.\n");
            break;
        }

        // Escribir los bytes correspondientes al nuevo código
        fwrite(current_string, 1, dictionary[new_code].length, output_file);

        // Crear una nueva entrada en el diccionario con OLD_CODE + primer byte de STRING
        memcpy(new_entry, dictionary[old_code].str, dictionary[old_code].length);
        new_entry[dictionary[old_code].length] = current_string[0];
        add_to_dictionary(dictionary, new_entry, dictionary[old_code].length + 1, &dict_size);

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
