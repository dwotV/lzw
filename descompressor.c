/*
PROYECTO LZW 

Gabriel Ponce Peña A01369913
Dael 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DICTIONARY_SIZE 4096
#define MAX_ENTRY_SIZE 256

typedef struct { 
    int length;
    unsigned char *ent;
} DictionaryEntry;

void startDictionary(DictionaryEntry *in_dict);
void appendtoDictionary(DictionaryEntry *dict, unsigned char *entry, int length, int *dict_size);
void freeDictionary(DictionaryEntry *dict, int dict_size);
int readCode(FILE *input); 
void decompress(FILE *input, FILE *output);


/**
 * MAIN FUNCTION
 * ALL THE CODE IS RUN HERE
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
       
        printf("The execution was written incorrectly.\nArguments: <compressed_filename> <uncompressed_filename>\n");
        return 1;

    }

    FILE *input_file = fopen(argv[1], "rb");
    FILE *output_file = fopen(argv[2], "wb");

    if (!input_file) {
        perror("There was an error when opening the INPUT file. Please try Again"); 
        return 1;
        }
    if (!output_file) { 
        perror("There was an error when opening the OUTPUT file. Please try Again"); 
        return 1;
        }

    decompress(input_file, output_file);

    fclose(input_file);
    fclose(output_file);
    
    return 0;
}


/**
 * startDictionary
 * Instantiates the struct we have stablished in order to be used
 *  @param in_dict: Pointer to the dictionary struct
 */
void startDictionary(DictionaryEntry *in_dict) {

    for (int i = 0; i < 256; i++) {

        in_dict[i].ent = (unsigned char *)malloc(1);
        in_dict[i].ent[0] = i;
        in_dict[i].length = 1;

    }

}


/**
 * appendtoDictionary
 * Adds an entry to an established dictionary
 *  @param in_dict: Pointer to the dictionary struct
 *  @param entry: The entry to be appended
 *  @param length:
 *  @param dict_size: Points to the index where the entry will be appended
 */
void appendtoDictionary(DictionaryEntry *dict, unsigned char *entry, int length, int *dict_size) {

    if (*dict_size < MAX_DICTIONARY_SIZE) {
        
        dict[*dict_size].ent = (unsigned char *)malloc(length);
        memcpy(dict[*dict_size].ent, entry, length);
        dict[*dict_size].length = length;
        (*dict_size)++;
    }

} 


/**
 * freeDictionary
 * Frees the Dictionary memery up to a certain index
 *  @param dict: Pointer to the dictionary struct
 *  @param dict_size: Points up to which point the dictionary will be freed
 */
void freeDictionary(DictionaryEntry *dict, int dict_size) {

    for (int i = 256; i < dict_size; i++) {

        free(dict[i].ent);
        printf("\n------------\nFREEING INDEX %i\n", i);
    }

}


/**
 * readCode
 * Uses a buffer to read a 12 bit code
 * @param input: The file to be read
 */
int readCode(FILE *input) {

    static int buffer = 0;
    static int bitsinbuff = 0;
    int code;

    while (bitsinbuff < 12)
    {
        int next_byte = fgetc(input);
        if (next_byte == EOF) return EOF;
        buffer = (buffer << 8) | next_byte;
        bitsinbuff += 8;
    }

    bitsinbuff -= 12;
    code = (buffer >> bitsinbuff) & 0xFFF;

    return code;
    

}

/**
 * decompress
 * Decompressed the file by using all the other functions and by applying the LZW algorthm
 * @param input: The input file to be read
 * @param output: The final file where the decompression is going to be saved
 */
void decompress(FILE *input, FILE *output) {

    DictionaryEntry dictionary[MAX_DICTIONARY_SIZE];
    startDictionary(dictionary);
    int dict_size = 256;
    
    int newcode, oldcode = readCode(input);
    if (oldcode == EOF) return;
    fwrite(dictionary[oldcode].ent, 1, dictionary[oldcode].length, output);

    unsigned char currententry[MAX_ENTRY_SIZE];
    
    memcpy(currententry, dictionary[oldcode].ent, dictionary[oldcode].length);
    int currentlength = dictionary[oldcode].length;

    while ((newcode = readCode(input)) != EOF) {
        printf("dict_size: %i\n", dict_size);
        if (dict_size == MAX_DICTIONARY_SIZE) {
            
            freeDictionary(dictionary, MAX_DICTIONARY_SIZE);
            dict_size = 256;

        }
        
        int entrylength;
        unsigned char entry[MAX_ENTRY_SIZE];

        if (newcode < dict_size) {
           
            memcpy(entry, dictionary[newcode].ent, dictionary[newcode].length);  
            entrylength = dictionary[newcode].length;

        } 
        else {
           
            memcpy(entry, currententry, currentlength);
            entry[currentlength] = currententry[0];  
            entrylength = currentlength + 1;

        }

        fwrite(entry, 1, entrylength, output);

        unsigned char new_entry[MAX_ENTRY_SIZE];
        memcpy(new_entry, currententry, currentlength);
        new_entry[currentlength] = entry[0];  // Primer carácter de la nueva cadena
        appendtoDictionary(dictionary, new_entry, currentlength + 1, &dict_size);

        memcpy(currententry, entry, entrylength);
        currentlength = entrylength;
        oldcode = newcode;


        
    }
    
    freeDictionary(dictionary, dict_size);

}


