#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "cachelab.h"

/* A cache simulator, take trace as input and output cache hits/misses/evictions 
 * Program take 4 flags:
 * -s <num>   Number of set index bits.
 * -E <num>   Number of lines per set.
 * -b <num>   Number of block offset bits.
 * -t <file>  Trace file.
 */

uint32_t SET_INDEX_BITS;
uint32_t LINES_PER_SET;
uint32_t BLOCK_OFFSET_BITS;
char* FILE_PATH;
/*
 * cache organization:
 * cache is array of set
 * each set contains one or more line
 * each line contains bits that is
 * partition of m(64) address bits into
 * t tag bits
 * s set index bits
 * b block offset bits
 */

typedef struct {
    bool valid_bit;
    uint64_t tag;

} cache_line;
typedef cache_line* cache_set;

bool lookup(cache_set* cache, uint64_t addr) {
    // extrac set index bits from addr
    // [s+b, b)
    uint64_t set_index = (addr >> BLOCK_OFFSET_BITS) & ((1ULL << SET_INDEX_BITS) - 1);
    uint64_t tag = addr >> (BLOCK_OFFSET_BITS + SET_INDEX_BITS);
    cache_set set = cache[set_index];
    for (int i = 0; i < LINES_PER_SET; i++) {
        cache_line *line = &set[i];
        if (line->tag == tag) {
            bool valid_bit = line->valid_bit;
            // fetch data from low-level cache
            line->valid_bit = true;
            return valid_bit;
        }
    }
    return false;
}

char* access_cache(cache_set* cache, uint64_t addr, uint64_t *cache_hits, 
                  uint64_t *cache_misses, uint64_t *cache_evictions) {
    if (lookup(cache, addr)) {
        (*cache_hits)++;
        return " hit";
    } else {
        (*cache_misses)++;
        if (lookup(cache, addr)) {
            (*cache_evictions)++;
            return " miss eviction";
        } else {
            return " miss";
        }
    }
}

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
            case 's':
                SET_INDEX_BITS = atoi(optarg);
                break;
            case 'E':
                LINES_PER_SET = atoi(optarg);
                break;
            case 'b':
                BLOCK_OFFSET_BITS = atoi(optarg);
                break;
            case 't':
                FILE_PATH = optarg;
                break;
            case '?':
                printf("Usage: ./csim -s <num> -E <num> -b <num> -t <file>\n"
                       "Options:\n"
                          "-s <num>   Number of set index bits.\n"
                          "E <num>   Number of lines per set.\n"
                          "-b <num>   Number of block offset bits.\n"
                          "-t <file>  Trace file.\n"
                       "\n"
                       "Examples:\n"
                         "linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
                exit(1);
        }
    }

    // dynamic allocate memory for cache
    uint64_t num_sets = 1ULL << SET_INDEX_BITS;
    cache_set *cache = malloc(num_sets * sizeof(cache_set));
    for (uint64_t i = 0; i < num_sets; i++) {
        cache[i] = malloc(LINES_PER_SET * sizeof(cache_line));
    }

    FILE *fp = fopen(FILE_PATH, "r");
    if (fp == NULL) {
        printf("open file fail\n");
        exit(1);
    }
    /* 
     * main simulation logic:
     * when read a word from memory, request the word from cache
     * if cache have the word, count a cache hit
     * if not, count a cache miss
     */
    char buf[64]; // buf store a trace and cache state, for example, L 04f6b868,8 hit
    uint64_t cache_hits = 0;
    uint64_t cache_misses = 0;
    uint64_t cache_evictions = 0;
    // 'L' and 'S' have no difference
    // 'M' does 2 memory reference
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (buf[0] != ' ') continue;
        uint64_t addr = strtoull(buf+3, NULL, 16);
        switch (buf[1]) {
            case 'M':
                strcat(buf, access_cache(cache, addr, &cache_hits, &cache_misses, &cache_evictions));
            case 'L':
            case 'S':
                strcat(buf, access_cache(cache, addr, &cache_hits, &cache_misses, &cache_evictions));
                break;
        }
        printf("%s", strcat(buf, "\n"));
    }
    printSummary(cache_hits, cache_misses, cache_evictions);

    fclose(fp);
    return 0;
}
