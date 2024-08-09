#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define RAM_SIZE 4096
#define LINE_SIZE 16
#define NUM_ROWS 8

// Cache Line structure
typedef struct {
    unsigned char tag;
    unsigned char data[LINE_SIZE];
} CacheLine;

// Prototypes
int loadCacheFile(unsigned int **memoryAccess); 
int loadRamFile(unsigned char *ramSimulator); 
void initializeCache(CacheLine cache[NUM_ROWS]);  
void dumpCache(CacheLine *cache); 
void printRam(unsigned char *ramSimulator); 
void printCache(CacheLine *cache);
void parseAddress(unsigned int address, int *tag, int *word, int *line, int *block);
void handleCacheMiss(CacheLine *cache, unsigned char *ram, int tag, int line, int block);

// Globals
int globalTime = 0;
int cacheMisses = 0;

// Main
int main(int argc, char **argv) {
    unsigned char ramSimulator[RAM_SIZE];
    CacheLine cache[NUM_ROWS];
    unsigned int *memoryAccess = NULL;
    unsigned char outputText[100] = {0};
    int numAddresses = 0;

    int word, tag, line, block;

    initializeCache(cache);

    if (loadRamFile(ramSimulator) == -1 || (numAddresses = loadCacheFile(&memoryAccess)) <= 0) {
        printf("Error loading RAM or cache file.\n");
        return 1;
    }

    for (int i = 0; i < numAddresses; i++) {
        parseAddress(memoryAccess[i], &tag, &word, &line, &block);
        globalTime++;

        if (tag != cache[line].tag) {
            cacheMisses++;
            printf("T: %d, Cache MISS %d, ADDR %04X Tag %X Line %02X Word %02X Block %02X\n", globalTime, cacheMisses, memoryAccess[i], tag, line, word, block);
            printf("Loading block %02X into line %02X\n", block, line);
            handleCacheMiss(cache, ramSimulator, tag, line, block);
        } else {
            printf("T: %d, Cache HIT, ADDR %04X Tag %X Line %02X Word %02X Data %02X\n", globalTime, memoryAccess[i], tag, line, word, cache[line].data[word]);
        }

        outputText[i] = cache[line].data[word];

        printCache(cache);
        printf("\n");
        sleep(1);
    }

    dumpCache(cache);

    printf("\n\nResults --> \n");
    printf("Total Accesses --> %d\n", numAddresses);
    printf("Cache Misses --> %d\n", cacheMisses);
    printf("Total Time --> %d\n", globalTime);
    printf("Average Time --> %.2f\n", (float)globalTime / numAddresses);
    printf("Text Read --> %s\n", outputText);

    free(memoryAccess);
    return 0;
}

void initializeCache(CacheLine cache[NUM_ROWS]) {
    for (int i = 0; i < NUM_ROWS; i++) {
        cache[i].tag = 0xFF;  // Initialize tag to 0xFF
        memset(cache[i].data, 0x23, LINE_SIZE);  // Initialize data to 0x23
    }
}

void dumpCache(CacheLine *cache) {
    FILE *cacheFile = fopen("CONTENTS_CACHE.bin", "wb");
    if (cacheFile == NULL) {
        printf("Error opening cache file for writing.\n");
        return;
    }

    for (int i = 0; i < NUM_ROWS; i++) {
        fprintf(cacheFile, "Tag --> %X, Data --> ", cache[i].tag);
        for (int j = LINE_SIZE - 1; j >= 0; j--) fprintf(cacheFile, "%X", cache[i].data[j]);
        fprintf(cacheFile, "\n");
    }

    fclose(cacheFile);
}

void parseAddress(unsigned int address, int *tag, int *word, int *line, int *block) {
    *word = address & 0xF;  // Extract the word (last 4 bits)
    *block = address >> 4;  // Extract the block (remaining bits after word)
    *line = *block & 0x7;  // Extract the line (last 3 bits of block)
    *tag = (*block & 0xF8) >> 3;  // Extract the tag (remaining bits after line)
}

void handleCacheMiss(CacheLine *cache, unsigned char *ram, int tag, int line, int block) {
    int ramIndex = block * LINE_SIZE;

    cache[line].tag = tag;  // Update cache line tag

    // Copy block data from RAM to cache line
    memcpy(cache[line].data, &ram[ramIndex], LINE_SIZE);

    globalTime += 10;  // Add penalty for cache miss
}

int loadCacheFile(unsigned int **memoryAccess) {
    FILE *addressFile = fopen("MEMORY.txt", "r");
    if (addressFile == NULL) {
        printf("Error opening memory access file.\n");
        return -1;
    }

    int numAddresses = 0;
    unsigned int address;

    *memoryAccess = malloc(sizeof(unsigned int) * 100);  // Initial allocation for 100 addresses

    while (fscanf(addressFile, "%X", &address) != EOF) {
        if (address > 0xFFF || address < 0x000) {
            printf("Error: Invalid address.\n");
            fclose(addressFile);
            free(*memoryAccess);
            return -2;
        }

        (*memoryAccess)[numAddresses++] = address;

        // Reallocate memory only if needed (doubling strategy)
        if (numAddresses % 100 == 0) *memoryAccess = realloc(*memoryAccess, sizeof(unsigned int) * (numAddresses + 100));
    }

    fclose(addressFile);
    return numAddresses;
}

int loadRamFile(unsigned char *ramSimulator) {
    FILE *ramFile = fopen("CONTENTS_RAM.bin", "rb");
    if (ramFile == NULL) {
        printf("Error opening RAM file.\n");
        return -1;
    }

    fread(ramSimulator, RAM_SIZE, 1, ramFile);
    fclose(ramFile);
    return 0;
}

void printRam(unsigned char *ramSimulator) {
    for (int i = 0; i < RAM_SIZE; i++) printf("%c", ramSimulator[i]);
    printf("\n");
}

void printCache(CacheLine *cache) {
    for (int i = 0; i < NUM_ROWS; i++) {
        printf("Tag --> %X, Data --> ", cache[i].tag);
        for (int j = LINE_SIZE - 1; j >= 0; j--) printf("%X", cache[i].data[j]); 
        printf("\n");
    }
}
