#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RAM_SIZE 4096
#define NUM_ADDRESSES 12

// Function prototypes
void generateRamFile();
void generateAccessFile();

int main() {
    srand(time(NULL));

    generateRamFile();
    generateAccessFile();

    printf("Files 'CONTENTS_RAM.bin' and 'MEMORY.txt' generated successfully.\n");
    return 0;
}

void generateRamFile() {
    FILE *f = fopen("CONTENTS_RAM.bin", "wb");
    if (f == NULL) {
        printf("Error: Could not create 'CONTENTS_RAM.bin'.\n");
        exit(1);
    }

    for (int i = 0; i < RAM_SIZE; i++) {
        unsigned char value = i % 256; // Value between 0x00 and 0xFF
        fwrite(&value, sizeof(unsigned char), 1, f);
    }

    fclose(f);
}

void generateAccessFile() {
    FILE *f = fopen("MEMORY.txt", "w");
    if (f == NULL) {
        printf("Error: Could not create 'accesos_memoria.txt'.\n");
        exit(1);
    }

    for (int i = 0; i < NUM_ADDRESSES; i++) {
        unsigned int f = rand() % 0x1000;  // Random address between 0x000 and 0xFFF
        fprintf(f, "0x%03X\n", address);
    }

    fclose(f);
}
